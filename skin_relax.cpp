#include "skin_relax.h"
#include "common_skin.h"

// ============================================== //
// Flag Statement
// ============================================== //

static const MString _commandName = "relaxSkin";
static const char* _flagValue[2] = { "-v", "-value" };
static const char* _flagLoop[2] = { "-l", "-loop" };
static const char* _flagInfluence[2] = { "-im", "-influenceMax" };
static const char* _flagPrune[2] = { "-p", "-prune" };

MSyntax relaxSkin::commandSyntax() {
	MSyntax syntax;
	syntax.setObjectType(MSyntax::kSelectionList, 0);
	syntax.useSelectionAsDefault(false);
	syntax.addFlag(_flagValue[0], _flagValue[1], MSyntax::kDouble);
	syntax.addFlag(_flagLoop[0], _flagLoop[1], MSyntax::kLong);
	syntax.addFlag(_flagInfluence[0], _flagInfluence[1], MSyntax::kLong);
	syntax.addFlag(_flagPrune[0], _flagPrune[1], MSyntax::kDouble);
	return (syntax);
}

// ============================================== //
// Common Statement
// ============================================== //

relaxSkin::relaxSkin() : // Set default value, similar like python def(args, args, args):
	MPxCommand(),
	_mIsUndoable(true),
	_mValue(0.2),
	_mLoop(5),
	_mInfluence(8),
	_mPrune(0.0001)
{
}

relaxSkin::~relaxSkin()
{
}

bool relaxSkin::isUndoable() const {
	return (_mIsUndoable);
}

void* relaxSkin::creator() {
	return (new relaxSkin());
}

MString relaxSkin::commandName() {
	return (_commandName);
}

// ============================================== //
// The Method
// ============================================== //

MStatus relaxSkin::doIt(const MArgList& args) {
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

	_mValue = _mArgData_syntax.flagArgumentDouble(_flagValue[0], 0);
	_mLoop = _mArgData_syntax.flagArgumentInt(_flagLoop[0], 0);
	_mInfluence = _mArgData_syntax.flagArgumentInt(_flagInfluence[0], 0);
	_mPrune = _mArgData_syntax.flagArgumentDouble(_flagPrune[0], 0);
	int _mDecimal = fuck.getDecimal(_mPrune);

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
	// Step 03 - Get Array Data for Calculation
	// ============================================== //
	// ============================================== //
	// ============================================== //

	MDoubleArray _mDouble_weightZero;

	for (unsigned int p = 0; p < _int_jointLength; ++p) {
		_mInt_jointIndex.append(p);										// Add the index of influence to the list
		_mDouble_weightZero.append(0.0);								// Set default all joint weight with 0.0
	}

	// ----- Looping per Selected Vertex List -----

	for (unsigned int a = 0; a < _mInt_vertexIndices_all.length(); ++a) {

		// ----- Reset Data -----

		bool _bool_firstLoop = true;

		MDoubleArray _mDouble_weightIndex;
		MDoubleArray _mDouble_weightNeighbor;
		MDoubleArray _mDouble_weightAverage;
		MDoubleArray _mDouble_weightFinal_data;

		_mDouble_weightAverage = _mDouble_weightZero;

		// ----- Find default value from each selected vertex -----

		MFnSingleIndexedComponent _mFnComponent_vertexCurrent;
		MObject _mObject_vertexCurrent = _mFnComponent_vertexCurrent.create(MFn::kMeshVertComponent);
		MIntArray _mInt_vertexIndex;
		_mInt_vertexIndex.append(_mInt_vertexIndices_all[a]);
		_mFnComponent_vertexCurrent.addElements(_mInt_vertexIndex);

		// ============================================== //
		// ============================================== //
		// ============================================== //
		// Step 04 - Get Basic Index and Neighbor Data
		// ============================================== //
		// ============================================== //
		// ============================================== //

		// ----- Find index weight -----

		for (unsigned int z = 0; z < _int_jointLength; ++z) {
			_mDouble_weightIndex.append(_mDouble_weightData_allSelected[a * _int_jointLength + z]);
		}

		// ----- Calculating for Locked Joint -----

		double _lockedSum = 0.0;

		for (unsigned int i = 0; i < _int_jointLength; ++i) {
			if (_bool_lockedJoints[i]) {
				_lockedSum += _mDouble_weightIndex[i];
			}
		}

		double _lockedPercent = 1.0 - _lockedSum;

		if (_mValue != 0.0 && _lockedPercent != 0.0) {

			// ----- Find neighbor weight -----

			MItMeshVertex _mItVertex_vertexCurrent(_mDagPath_objectShape, _mObject_vertexCurrent, &_status);
			MIntArray _mInt_vertexNeighbor;
			MFnSingleIndexedComponent _mFnComponen_vertexNeighbor;
			_mItVertex_vertexCurrent.getConnectedVertices(_mInt_vertexNeighbor);
			MObject _mObject_vertexNeighbor = _mFnComponen_vertexNeighbor.create(MFn::kMeshVertComponent);
			_mFnComponen_vertexNeighbor.addElements(_mInt_vertexNeighbor);

			_mFnSkinCluster.getWeights(_mDagPath_objectShape, _mObject_vertexNeighbor, _mDouble_weightNeighbor, _int_jointLength);

			for (unsigned int d = 0; d < _mInt_vertexNeighbor.length(); ++d) {
				for (unsigned int e = 0; e < _int_jointLength; ++e) {
					if (!_bool_lockedJoints[e]) {
						_mDouble_weightAverage[e] += _mDouble_weightNeighbor[d * _int_jointLength + e];
					}
				}
			}

			// ----- Summary each neighbor -----

			for (unsigned int f = 0; f < _int_jointLength; ++f) {
				if (!_bool_lockedJoints[f] && _mInt_vertexNeighbor.length() > 0) {
					_mDouble_weightAverage[f] /= _mInt_vertexNeighbor.length();
				}
				else {
					_mDouble_weightAverage[f] = _mDouble_weightIndex[f];
				}
			}

			// ============================================== //
			// ============================================== //
			// ============================================== //
			// Step 05 - Calculation by Loop
			// ============================================== //
			// ============================================== //
			// ============================================== //

			for (unsigned int c = 0; c < _mLoop; ++c) {
				if (_bool_firstLoop == true) {
					_bool_firstLoop = false;
				}
				else {
					_mDouble_weightIndex = _mDouble_weightFinal_data;
				}

				_mDouble_weightFinal_data.clear();

				// ----- Calculating each loop -----

				for (unsigned int g = 0; g < _int_jointLength; ++g) {
					if (_bool_lockedJoints[g]) {
						_mDouble_weightFinal_data.append(_mDouble_weightIndex[g]);
					}
					else {
						_mDouble_weightFinal_data.append(_mDouble_weightIndex[g] + (_lockedPercent * ((_mDouble_weightAverage[g] - _mDouble_weightIndex[g]) * _mValue)));
					}
				}
			}
		}
		else {
			_mDouble_weightFinal_data = _mDouble_weightIndex;
		}

		// ============================================== //
		// ============================================== //
		// ============================================== //
		// Step 06 - Clean-up
		// ============================================== //
		// ============================================== //
		// ============================================== //

		fuck.normalizeWeights(_mDouble_weightFinal_data, _bool_lockedJoints, 1e-6, _mDecimal, _mPrune, _mInfluence);

		// ============================================== //
		// ============================================== //
		// ============================================== //
		// Step 07 - Return
		// ============================================== //
		// ============================================== //
		// ============================================== //

		for (unsigned int i = 0; i < _mDouble_weightFinal_data.length(); ++i) {
			double _double_final = _mDouble_weightFinal_data[i];
			_mDouble_weightFinal_redo.append(_double_final);
		}
	}
	return (redoIt());
}

MStatus relaxSkin::redoIt() {
	_mFnSkinCluster.setWeights(_mDagPath_objectShape,
		_mObject_currentVertexComponent_all,
		_mInt_jointIndex,
		_mDouble_weightFinal_redo,
		false);
	return (MS::kSuccess);
}

MStatus relaxSkin::undoIt() {
	_mFnSkinCluster.setWeights(_mDagPath_objectShape,
		_mObject_currentVertexComponent_all,
		_mInt_jointIndex,
		_mDouble_weightData_allSelected,
		false);
	return (MS::kSuccess);
}