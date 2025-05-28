#include "skin_combine.h"
#include "common_skin.h"

// ============================================== //
// Flag Statement
// ============================================== //

static const MString _commandName = "combineSkin";
static const char* _flagCutList[2] = { "-cl", "-cutList" };
static const char* _flagPasteList[2] = { "-pl", "-pasteList" };
static const char* _flagPrune[2] = { "-p", "-prune" };

MSyntax combineSkin::commandSyntax() {
	MSyntax syntax;
	syntax.setObjectType(MSyntax::kSelectionList, 0);
	syntax.useSelectionAsDefault(false);
	syntax.addFlag(_flagCutList[0], _flagCutList[1], MSyntax::kString);
	syntax.addFlag(_flagPasteList[0], _flagPasteList[1], MSyntax::kString);
	syntax.addFlag(_flagPrune[0], _flagPrune[1], MSyntax::kDouble);
	return (syntax);
}

// ============================================== //
// Common Statement
// ============================================== //

combineSkin::combineSkin() : // Set default value, similar like python def(args, args, args):
	MPxCommand(),
	_mIsUndoable(true),
	_mPrune(0.0001)
{
}

combineSkin::~combineSkin()
{
}

bool combineSkin::isUndoable() const {
	return (_mIsUndoable);
}

void* combineSkin::creator() {
	return (new combineSkin());
}

MString combineSkin::commandName() {
	return (_commandName);
}

// ============================================== //
// The Method
// ============================================== //

MStatus combineSkin::doIt(const MArgList& args) {
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

	MString _mString_cutListStr;
	MStringArray _mStringArray_cutNames;
	MStringArray _mString_cutSplit;

	_mString_cutListStr = _mArgData_syntax.flagArgumentString(_flagCutList[0], 0);
	_mString_cutListStr.split(',', _mString_cutSplit);
	_mStringArray_cutNames = _mString_cutSplit;

	MString _mString_pasteListStr;
	MStringArray _mStringArray_pasteNames;
	MStringArray _mString_pasteSplit;

	_mString_pasteListStr = _mArgData_syntax.flagArgumentString(_flagPasteList[0], 0);
	_mString_pasteListStr.split(',', _mString_pasteSplit);
	_mStringArray_pasteNames = _mString_pasteSplit;

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

	// ----- Convert Cut and Paste list into Index -----

	// Cut List
	std::vector<int> _vector_cutIndex;

	for (unsigned int i = 0; i < _mStringArray_cutNames.length(); ++i) {
		MString _mString_jointNameCurrent = _mStringArray_cutNames[i];
		int _foundIndex = -1;

		for (unsigned int j = 0; j < _mDagPathArray_joint.length(); ++j) {
			MFnDependencyNode _fnJoint(_mDagPathArray_joint[j].node());
			MString _influenceName = _fnJoint.name();

			if (_mString_jointNameCurrent == _influenceName) {
				_foundIndex = static_cast<int>(j);
				break;
			}
		}

		if (_foundIndex != -1) {
			_vector_cutIndex.push_back(_foundIndex);
		}
	}

	// Paste List
	std::vector<int> _vector_pasteIndex;

	for (unsigned int i = 0; i < _mStringArray_pasteNames.length(); ++i) {
		MString _mString_jointNameCurrent = _mStringArray_pasteNames[i];
		int _foundIndex = -1;

		for (unsigned int j = 0; j < _mDagPathArray_joint.length(); ++j) {
			MFnDependencyNode _fnJoint(_mDagPathArray_joint[j].node());
			MString _influenceName = _fnJoint.name();

			if (_mString_jointNameCurrent == _influenceName) {
				_foundIndex = static_cast<int>(j);
				break;
			}
		}

		if (_foundIndex != -1) {
			_vector_pasteIndex.push_back(_foundIndex);
		}
	}

	// ----- Looping per Selected Vertex List -----

	for (unsigned int a = 0; a < _mInt_vertexIndices_all.length(); ++a) {

		// ----- Reset Data -----

		MDoubleArray _mDouble_weightIndex;


		// ============================================== //
		// ============================================== //
		// ============================================== //
		// Step 03 - Get Array Data for Listing
		// ============================================== //
		// ============================================== //
		// ============================================== //

		// ----- Search and Find index weight -----

		for (unsigned int z = 0; z < _int_jointLength; ++z) {
			_mDouble_weightIndex.append(_mDouble_weightData_allSelected[a * _int_jointLength + z]);
		}

		for (unsigned int z = 0; z < _vector_cutIndex.size(); ++z) {
			double _cut = _mDouble_weightIndex[_vector_cutIndex[z]];
			double _paste = _mDouble_weightIndex[_vector_pasteIndex[z]];

			_mDouble_weightIndex[_vector_pasteIndex[z]] = _cut + _paste;
			_mDouble_weightIndex[_vector_cutIndex[z]] = 0.0;
		}

		// ============================================== //
		// ============================================== //
		// ============================================== //
		// Step 04 - Clean-up
		// ============================================== //
		// ============================================== //
		// ============================================== //

		fuck.normalizeWeights(_mDouble_weightIndex, _bool_lockedJoints, 1e-6, _mDecimal, _mPrune, 0);

		// ============================================== //
		// ============================================== //
		// ============================================== //
		// Step 05 - Return
		// ============================================== //
		// ============================================== //
		// ============================================== //

		for (unsigned int i = 0; i < _mDouble_weightIndex.length(); ++i) {
			double _double_final = _mDouble_weightIndex[i];
			_mDouble_weightFinal_redo.append(_double_final);
		}
	}
	return (redoIt());
}

MStatus combineSkin::redoIt() {
	_mFnSkinCluster.setWeights(_mDagPath_objectShape,
		_mObject_currentVertexComponent_all,
		_mInt_jointIndex,
		_mDouble_weightFinal_redo,
		false);
	return (MS::kSuccess);
}

MStatus combineSkin::undoIt() {
	_mFnSkinCluster.setWeights(_mDagPath_objectShape,
		_mObject_currentVertexComponent_all,
		_mInt_jointIndex,
		_mDouble_weightData_allSelected,
		false);
	return (MS::kSuccess);
}
