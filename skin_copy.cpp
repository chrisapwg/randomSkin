#include "skin_copy.h"
#include "common_skin.h"

// ============================================== //
// Flag Statement
// ============================================== //

static const MString _commandName = "copySkin";
static const char* _flagValue[2] = { "-v", "-value" };
static const char* _flagSourceVertex[2] = { "-sv", "-sourceVertex" };
static const char* _flagInfluence[2] = { "-im", "-influenceMax" };
static const char* _flagPrune[2] = { "-p", "-prune" };

MSyntax copySkin::commandSyntax() {
    MSyntax syntax;
    syntax.setObjectType(MSyntax::kSelectionList, 0);
    syntax.useSelectionAsDefault(false);
    syntax.addFlag(_flagValue[0], _flagValue[1], MSyntax::kDouble);
    syntax.addFlag(_flagSourceVertex[0], _flagSourceVertex[1], MSyntax::kString);
    syntax.addFlag(_flagInfluence[0], _flagInfluence[1], MSyntax::kLong);
    syntax.addFlag(_flagPrune[0], _flagPrune[1], MSyntax::kDouble);
    return (syntax);
}

// ============================================== //
// Common Statement
// ============================================== //

copySkin::copySkin() : // Set default value, similar like python def(args, args, args):
    MPxCommand(),
    _mIsUndoable(true),
    _mValue(0.2),
    _mInfluence(8),
    _mPrune(0.0001)
{
}

copySkin::~copySkin()
{
}

bool copySkin::isUndoable() const {
    return (_mIsUndoable);
}

void* copySkin::creator() {
    return (new copySkin());
}

MString copySkin::commandName() {
    return (_commandName);
}

// ============================================== //
// The Method
// ============================================== //

MStatus copySkin::doIt(const MArgList& args) {
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

    _mValue = _mArgData_syntax.flagArgumentDouble(_flagValue[0], 0);
    _mInfluence = _mArgData_syntax.flagArgumentInt(_flagInfluence[0], 0);
    _mPrune = _mArgData_syntax.flagArgumentDouble(_flagPrune[0], 0);
    int _mDecimal = fuck.getDecimal(_mPrune);

    MString _mString_SourceVertex;
    //MStringArray _mStringArray_jointNames;
    //MStringArray _mString_split;

    _mString_SourceVertex = _mArgData_syntax.flagArgumentString(_flagSourceVertex[0], 0);
    //_mString_SourceVertex.split(',', _mString_split);
    //_mStringArray_jointNames = _mString_split;

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
    // Step 03 - Get Array Data for Calculation
    // ============================================== //
    // ============================================== //
    // ============================================== //

    // ----- Find Index from Listed Joint -----
    /*
    // Map joint names to their index in the skinCluster influence list
    std::vector<int> _vector_jointIndex;

    for (unsigned int i = 0; i < _mStringArray_jointNames.length(); ++i) {
        MString _mString_jointNameCurrent = _mStringArray_jointNames[i];
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
            _vector_jointIndex.push_back(_foundIndex);
        }
    }

    // ----- Find _mvalue update -----

    int _length = _vector_jointIndex.size();
    _mValue /= _length;
    */
    // ----- Looping per Selected Vertex List -----
    /*
    for (unsigned int a = 0; a < _mInt_vertexIndices_all.length(); ++a) {

        // ----- Reset Data -----

        bool _bool_firstLoop = true;

        MDoubleArray _mDouble_weightIndex;
        MDoubleArray _mDouble_weightNeighbor;
        MDoubleArray _mDouble_weightAverage;
        MDoubleArray _mDouble_weightFinal_data;

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

            // Step 1: Add _mValue to the target joints
            double totalAdded = 0.0;
            for (unsigned int i = 0; i < _int_jointLength; ++i) {
                if (std::find(_vector_jointIndex.begin(), _vector_jointIndex.end(), i) != _vector_jointIndex.end()) {
                    _mDouble_weightIndex[i] += _mValue;
                    _bool_lockedJoints[i] = true;
                    if (_mDouble_weightIndex[i] > 1.0) _mDouble_weightIndex[i] = 1.0;
                    if (_mDouble_weightIndex[i] < 0.0) _mDouble_weightIndex[i] = 0.0;
                }
            }

            // Step 2: Normalize unlocked joints proportionally
            double remainingSum = 0.0;
            MIntArray _mInt_jointWithWeight;
            for (unsigned int i = 0; i < _int_jointLength; ++i) {
                if (std::find(_vector_jointIndex.begin(), _vector_jointIndex.end(), i) == _vector_jointIndex.end()) {
                    if (_mDouble_weightIndex[i] > _mPrune) {
                        _mInt_jointWithWeight.append(i);
                        remainingSum += _mDouble_weightIndex[i];
                    }
                }
            }

            // Get locked sum too
            double lockedSum = 0.0;
            for (int idx : _vector_jointIndex) {
                lockedSum += _mDouble_weightIndex[idx];
            }

            // Total current weight
            double totalWeight = lockedSum + remainingSum;

            // Step 3: Scale remaining joints proportionally to fill up to 1.0
            double remainingTarget = (totalWeight > 1.0) ? (1.0 - lockedSum) : (1.0 - lockedSum);
            for (unsigned int i = 0; i < _mInt_jointWithWeight.length(); ++i) {
                int idx = _mInt_jointWithWeight[i];
                double original = _mDouble_weightIndex[idx];
                double scaled = (remainingSum > 0.0) ? (original * remainingTarget / remainingSum) : 0.0;
                _mDouble_weightIndex[idx] = (scaled > _mPrune) ? scaled : 0.0;
            }

            // Recompute total after adjust
            double _total = 0.0;
            for (unsigned int i = 0; i < _int_jointLength; ++i) {
                _total += _mDouble_weightIndex[i];
            }

            // Step 4: If still > 1.0, proportionally reduce the locked joints
            if (_total > 1.0) {
                double excess = _total - 1.0;

                // Get lockedSum again (in case something changed)
                lockedSum = 0.0;
                for (int idx : _vector_jointIndex) {
                    lockedSum += _mDouble_weightIndex[idx];
                }

                if (lockedSum > 0.0) {
                    for (int idx : _vector_jointIndex) {
                        double w = _mDouble_weightIndex[idx];
                        double ratio = w / lockedSum;
                        double reduction = ratio * excess;
                        _mDouble_weightIndex[idx] = std::max(0.0, w - reduction);
                    }
                }
            }
        }

        // ============================================== //
        // ============================================== //
        // ============================================== //
        // Step 06 - Clean-up
        // ============================================== //
        // ============================================== //
        // ============================================== //

        fuck.normalizeWeights(_mDouble_weightIndex, _bool_lockedJoints, 1e-6, _mDecimal, _mPrune, _mInfluence);

        // ============================================== //
        // ============================================== //
        // ============================================== //
        // Step 07 - Return
        // ============================================== //
        // ============================================== //
        // ============================================== //

        for (unsigned int i = 0; i < _mDouble_weightIndex.length(); ++i) {
            double _double_final = _mDouble_weightIndex[i];
            _mDouble_weightFinal_redo.append(_double_final);
        }
    }*/
    return (redoIt());
}

MStatus copySkin::redoIt() {
    _mFnSkinCluster.setWeights(_mDagPath_objectShape,
        _mObject_currentVertexComponent_all,
        _mInt_jointIndex,
        _mDouble_weightData_allSelected,
        false);
    return (MS::kSuccess);
}

MStatus copySkin::undoIt() {
    _mFnSkinCluster.setWeights(_mDagPath_objectShape,
        _mObject_currentVertexComponent_all,
        _mInt_jointIndex,
        _mDouble_weightData_allSelected,
        false);
    return (MS::kSuccess);
}
