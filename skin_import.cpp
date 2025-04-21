#include "skin_import.h"
#include "common_skin.h"

// ============================================== //
// Flag Statement
// ============================================== //

static const MString _commandName = "importSkin";
static const char* _flagPrune[2] = { "-p", "-prune" };

MSyntax importSkin::commandSyntax() {
	MSyntax syntax;
	syntax.setObjectType(MSyntax::kSelectionList, 0);
	syntax.useSelectionAsDefault(false);
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

	MArgDatabase _mArgData_syntax(syntax(), args, &_status);						// Collect semua syntax yang kita set di maya cmds
	MSelectionList _mSList_syntaxObject;											// Create array list of selected object
	_mArgData_syntax.getObjects(_mSList_syntaxObject);								// Args ini object yang ada di awal "cmds.command([this is], args, args)", save ke '_mSList_syntaxObject'
	MItSelectionList _MItSList_listVertex(_mSList_syntaxObject, MFn::kComponent);	// Specify the object to be component exp: vertex, edge, face

	// ----- Get Flag Data -----

	_mPrune = _mArgData_syntax.flagArgumentDouble(_flagPrune[0], 0);
	int _mDecimal = fuck.getDecimal(_mPrune);

	MString _mString_jointListStr;
	MStringArray _mStringArray_jointNames;
	MStringArray _mString_split;

	_mString_jointListStr.split(',', _mString_split);
	_mStringArray_jointNames = _mString_split;

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
	_mDagPath_objectShape.extendToShape();														// Make sure to select the 'Shape'

	MObject _mObject_objectShape;
	_mObject_objectShape = _mDagPath_objectShape.node();								// Convert the dagPath or now is shapeNode 'pSphereShape1', into MObject to do things
	MItDependencyGraph _mItDG_objectInputSkinCluster(_mObject_objectShape,				// Find graph of this mObject
		MFn::kSkinClusterFilter,			// Find specific object 'skinCluster'
		MItDependencyGraph::kUpstream,		// Mean fiding the input, not output
		MItDependencyGraph::kDepthFirst,	// Search the input of selected node loop until very deep
		MItDependencyGraph::kPlugLevel,		// Make sure node 'plug' also get pemeriksaan, exp type 'inMesh.'
		&_status);							// return MStatus to check

	_mObject_skinCluster = _mItDG_objectInputSkinCluster.currentItem();					// Convert the found node into MObject
	_mFnSkinCluster.setObject(_mObject_skinCluster);									// Convert the found MObject into skinCluster
	_mFnSkinCluster.influenceObjects(_mDagPathArray_joint, &_status);					// MDagPathArray _mDagPathArray_joint; Create dagPath but array, to list the joint
	unsigned int _int_jointLength = _mDagPathArray_joint.length();						// Get the length of joint influence

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
		_mInt_jointIndex.append(p);										// Add the index of influence to the list
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





	// Your mesh name, example "pCylinder1"
	std::string meshName = "Fatalis_Closemouth_UDIMs";

	// Read the file
	std::ifstream file("D:/3D World/Offline_Gacha_Game/scenes/meshSkinDict_ST.txt");
	if (!file) {
		MGlobal::displayError("Failed to open skin weight file.");
		return MS::kFailure;
	}

	// Convert to string, and parse JSON
	std::stringstream buffer;
	buffer << file.rdbuf();
	std::string fileContent = buffer.str();

	// Replace single quotes with double quotes to make it valid JSON
	std::replace(fileContent.begin(), fileContent.end(), '\'', '\"');

	// Parse JSON
	json weightData = json::parse(fileContent);

	// Access weights for the current object
	std::vector<double> weights = weightData[meshName];

	// Convert to MDoubleArray
	MDoubleArray mWeights;
	for (double val : weights) {
		mWeights.append(val);
	}





	_mFnSkinCluster.setWeights(_mDagPath_objectShape,
		_mObject_currentVertexComponent_all,
		_mInt_jointIndex,
		mWeights,
		false);










	return (redoIt());
}

MStatus importSkin::redoIt() {
	return (MS::kSuccess);
}

MStatus importSkin::undoIt() {
	return (MS::kSuccess);
}