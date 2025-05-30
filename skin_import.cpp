#include "skin_import.h"
#include "common_skin.h"

// ============================================== //
// Flag Statement
// ============================================== //

static const MString _commandName = "importSkin";
static const char* _flagObjectSource[2] = { "-os", "-objectSource" };
static const char* _flagObjectTarget[2] = { "-ot", "-objectTarget" };
static const char* _flagLocationJoint[2] = { "-lj", "-locationJoint" };
static const char* _flagLocationWeight[2] = { "-lw", "-locationWeight" };
static const char* _flagNameFind[2] = { "-f", "-findRename" };
static const char* _flagNameReplace[2] = { "-r", "-replaceRename" };
static const char* _flagPrune[2] = { "-p", "-prune" };

MSyntax importSkin::commandSyntax() {
	MSyntax syntax;
	syntax.setObjectType(MSyntax::kSelectionList, 0);
	syntax.useSelectionAsDefault(false);
	syntax.addFlag(_flagObjectSource[0], _flagObjectSource[1], MSyntax::kString);
	syntax.addFlag(_flagObjectTarget[0], _flagObjectTarget[1], MSyntax::kString);
	syntax.addFlag(_flagLocationJoint[0], _flagLocationJoint[1], MSyntax::kString);
	syntax.addFlag(_flagLocationWeight[0], _flagLocationWeight[1], MSyntax::kString);
	syntax.addFlag(_flagNameFind[0], _flagNameFind[1], MSyntax::kString);
	syntax.addFlag(_flagNameReplace[0], _flagNameReplace[1], MSyntax::kString);
	syntax.addFlag(_flagPrune[0], _flagPrune[1], MSyntax::kDouble);
	return (syntax);
}

// ============================================== //
// Common Statement
// ============================================== //

importSkin::importSkin() : // Set default value, similar like python def(args, args, args):
	MPxCommand(),
	_mIsUndoable(true),
	_mPrune(0.0001)
{
}

importSkin::~importSkin()
{
}

bool importSkin::isUndoable() const {
	return (_mIsUndoable);
}

void* importSkin::creator() {
	return (new importSkin());
}

MString importSkin::commandName() {
	return (_commandName);
}

// ============================================== //
// The Method
// ============================================== //

MStatus importSkin::doIt(const MArgList& args) {
	// ============================================== //
	// ============================================== //
	// ============================================== //
	// Step 01 - Command Data
	// ============================================== //
	// ============================================== //
	// ============================================== //

	common_Function fuck;
	MStatus _status;

	// ----- Get Command Data -----

	MArgDatabase _mArgData_syntax(syntax(), args, &_status);                    	// Collect semua syntax yang kita set di maya cmds
	MSelectionList _mSList_syntaxObject;                                        	// Create array list of selected object
	_mArgData_syntax.getObjects(_mSList_syntaxObject);                            	// Args ini object yang ada di awal "cmds.command([this is], args, args)", save ke '_mSList_syntaxObject'
	MItSelectionList _MItSList_listVertex(_mSList_syntaxObject, MFn::kComponent);	// Specify the object to be component exp: vertex, edge, face

	// ----- Get Flag Data -----

	_mPrune = _mArgData_syntax.flagArgumentDouble(_flagPrune[0], 0);
	int _mDecimal = fuck.getDecimal(_mPrune);

	_mString_objectSource = _mArgData_syntax.flagArgumentString(_flagObjectSource[0], 0);
	_mString_objectTarget = _mArgData_syntax.flagArgumentString(_flagObjectTarget[0], 0);
	_mString_locationJoint = _mArgData_syntax.flagArgumentString(_flagLocationJoint[0], 0);
	_mString_locationWeight = _mArgData_syntax.flagArgumentString(_flagLocationWeight[0], 0);
	_mString_nameFind = _mArgData_syntax.flagArgumentString(_flagNameFind[0], 0);
	_mString_nameReplace = _mArgData_syntax.flagArgumentString(_flagNameReplace[0], 0);

	// ============================================== //
	// ============================================== //
	// ============================================== //
	// Step 02 - Collect Object Basic Data
	// ============================================== //
	// ============================================== //
	// ============================================== //

	// ----- Basic Object Data Component and SkinCluster -----

	// Collect data -----
	// "_mDagPath_objectShape"
	// "_mObject_currentVertexComponent"
	// "_mFnSkinCluster"
	// "_int_jointLength"

	MObject _mObject_currentVertexComponent;
	_MItSList_listVertex.getDagPath(_mDagPath_objectShape, _mObject_currentVertexComponent);	// Get the dagPath (true|path|nameShape) and component Index
	_mDagPath_objectShape.extendToShape();                                                    	// Make sure to select the 'Shape'

	MObject _mObject_objectShape;
	_mObject_objectShape = _mDagPath_objectShape.node();                            	// Convert the dagPath or now is shapeNode 'pSphereShape1', into MObject to do things
	MItDependencyGraph _mItDG_objectInputSkinCluster(_mObject_objectShape,            	// Find graph of this mObject
		MFn::kSkinClusterFilter,        	// Find specific object 'skinCluster'
		MItDependencyGraph::kUpstream,    	// Mean fiding the input, not output
		MItDependencyGraph::kDepthFirst,	// Search the input of selected node loop until very deep
		MItDependencyGraph::kPlugLevel,    	// Make sure node 'plug' also get pemeriksaan, exp type 'inMesh.'
		&_status);                        	// return MStatus to check

	_mObject_skinCluster = _mItDG_objectInputSkinCluster.currentItem();                	// Convert the found node into MObject
	_mFnSkinCluster.setObject(_mObject_skinCluster);                                	// Convert the found MObject into skinCluster
	_mFnSkinCluster.influenceObjects(_mDagPathArray_joint, &_status);                	// MDagPathArray _mDagPathArray_joint; Create dagPath but array, to list the joint
	unsigned int _int_jointLength = _mDagPathArray_joint.length();                    	// Get the length of joint influence

	// ----- Find Locked Joint -----

	// Store lock status for each joint influence
	MPlug _mPlug_lockWeightsPlug = _mFnSkinCluster.findPlug("lockWeights", true);
	std::vector<bool> _bool_lockedJoints;
	_bool_lockedJoints.resize(_int_jointLength, false);

	for (unsigned int i = 0; i < _int_jointLength; ++i) {
		MPlug _plug = _mPlug_lockWeightsPlug.elementByLogicalIndex(i);
		_bool_lockedJoints[i] = _plug.asBool();
	}

	// ----- Default Indicate -----

	for (unsigned int p = 0; p < _int_jointLength; ++p) {
		_mInt_jointIndex.append(p);                                    	// Add the index of influence to the list
	}

	// ----- Collect Weight Data -----

	// Collect data -----
	// "_mObject_currentVertexComponent_all"
	// "_mInt_vertexIndices_all"

	for (; !_MItSList_listVertex.isDone(); _MItSList_listVertex.next()) {
		MObject _component;
		MDagPath _dagPath;
		_MItSList_listVertex.getDagPath(_dagPath, _component);

		MFnSingleIndexedComponent _fnComp(_component);

		MIntArray _indices;
		_fnComp.getElements(_indices);
		_MFnSingleIndexedComponent_all.addElements(_indices);
	}

	MIntArray _mInt_vertexIndices_all;
	_MFnSingleIndexedComponent_all.getElements(_mInt_vertexIndices_all);
	_mFnSkinCluster.getWeights(_mDagPath_objectShape, _mObject_currentVertexComponent_all, _mDouble_weightData_allSelected, _int_jointLength);	// Step 3: Get all neighbor weights in one call

	// ============================================== //
	// ============================================== //
	// ============================================== //
	// Step 02 - Return
	// ============================================== //
	// ============================================== //
	// ============================================== //

	MString _mString_objectName = _mDagPath_objectShape.partialPathName();
	std::string _objectSource = _mString_objectSource.asChar();
	std::string _objectTarget = _mString_objectTarget.asChar();
	std::string _locationJoint = _mString_locationJoint.asChar();
	std::string _locationWeight = _mString_locationWeight.asChar();

	//
	// --- Read Joint File ---
	//

	std::ifstream fileJoint(_locationJoint);
	std::stringstream bufferJoint;
	bufferJoint << fileJoint.rdbuf();
	std::string fileContentJoint = bufferJoint.str();
	std::replace(fileContentJoint.begin(), fileContentJoint.end(), '\'', '\"');    	// Replace single quotes with double quotes to make it valid JSON
	json jointsData = json::parse(fileContentJoint);                            	// Parse JSON
	std::vector<std::string> jointsList = jointsData[_objectSource];                	// Access joints for the current object

	//
	// --- Read Weight File ---
	//

	std::ifstream fileWeight(_locationWeight);
	std::stringstream bufferWeight;
	bufferWeight << fileWeight.rdbuf();
	std::string fileContentWeight = bufferWeight.str();
	std::replace(fileContentWeight.begin(), fileContentWeight.end(), '\'', '\"');	// Replace single quotes with double quotes to make it valid JSON
	json weightData = json::parse(fileContentWeight);                            	// Parse JSON
	std::vector<double> weightsList = weightData[_objectSource];                    	// Access weights for the current object

	//
	// --- Convert to MVariable ---
	//

	for (int j = 0; j < _mInt_jointIndex.length(); ++j) {
		MString _jntSource = _mDagPathArray_joint[j].partialPathName();
		int _value = -1;
		for (unsigned int f = 0; f < jointsList.size(); ++f) {
			MString _jntFile = jointsList[f].c_str();
			_jntFile.substitute(_mString_nameFind, _mString_nameReplace);
			if (_jntSource == _jntFile) {
				_value = f;
				break;
			}
		}
		_mInt_jointIndex_new.append(_value);
	}

	//
	// --- Convert to MDoubleArray ---
	//

	//for (double val : weightsList) {
	//	_mDouble_weightFinal_redo.append(val);
	//}

	// ----- Looping per Selected Vertex List -----

	for (unsigned int a = 0; a < _mInt_vertexIndices_all.length(); ++a) {
		for (int j = 0; j < _mInt_jointIndex_new.length(); ++j) {
			double _weight;
			if (_mInt_jointIndex_new[j] == -1) {
				_weight = 0.0;
			}
			else {
				_weight = weightsList[_mInt_vertexIndices_all[a] * jointsList.size() + _mInt_jointIndex_new[j]];
			}
			_mDouble_weightFinal_redo.append(_weight);
		}
	}

	return (redoIt());
}

MStatus importSkin::redoIt() {
	_mFnSkinCluster.setWeights(_mDagPath_objectShape,
		_mObject_currentVertexComponent_all,
		_mInt_jointIndex,
		_mDouble_weightFinal_redo,
		false);
	return (MS::kSuccess);
}

MStatus importSkin::undoIt() {
	_mFnSkinCluster.setWeights(_mDagPath_objectShape,
		_mObject_currentVertexComponent_all,
		_mInt_jointIndex,
		_mDouble_weightData_allSelected,
		false);
	return (MS::kSuccess);
}
