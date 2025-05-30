#pragma once
#include <maya/MPxCommand.h>
#include <maya/MGlobal.h>
#include <maya/MSyntax.h>

#include <maya/MObject.h>
#include <maya/MIntArray.h>
#include <maya/MDoubleArray.h>
#include <map>
#include <vector>
#include <algorithm>
#include <numeric>
#include <set>
#include <unordered_map>

#include <maya/MSelectionList.h>
#include <maya/MItSelectionList.h>
#include <maya/MDagPath.h>
#include <maya/MDagPathArray.h>
#include <maya/MArgDatabase.h>
#include <maya/MFnTransform.h>
#include <maya/MFnSingleIndexedComponent.h>
#include <maya/MPlug.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MItDependencyGraph.h>
#include <maya/MItMeshVertex.h>
#include <maya/MFnSkinCluster.h>
#include <maya/MFnMesh.h>
#include <maya/MItMeshPolygon.h>
#include <maya/MItMeshEdge.h>

#include <maya/MArrayDataHandle.h>
#include <maya/MObjectArray.h>
#include <maya/MFnDoubleArrayData.h>
#include <maya/MFnIntArrayData.h>

#include <iomanip>
#include <fstream>
#include <sstream>
#include "json.hpp"
using json = nlohmann::json;

class importSkin : public MPxCommand {
public: // Public so can used in main.cpp
	importSkin();
	virtual ~importSkin();
	virtual MStatus doIt(const MArgList& args) override;
	virtual MStatus redoIt() override;
	virtual MStatus undoIt() override;
	virtual bool isUndoable() const override;

	// Static Variable Listing
	static void* creator();
	static MString commandName();
	static MSyntax commandSyntax();

private: // Only The doIt .cpp can read this

	// ======================//
	// Command Variable
	// ======================//

	bool _mIsUndoable;
	double _mPrune;

	// ======================//
	// Used Variable
	// ======================//

	MString _mString_objectSource;
	MString _mString_objectTarget;
	MString _mString_locationJoint;
	MString _mString_locationWeight;
	MString _mString_nameFind;
	MString _mString_nameReplace;

	MDagPath _mDagPath_objectShape;
	MObject _mObject_skinCluster;
	MFnSkinCluster _mFnSkinCluster;
	MDagPathArray _mDagPathArray_joint;
	MIntArray _mInt_jointIndex;
	MIntArray _mInt_jointIndex_new;

	MFnSingleIndexedComponent _MFnSingleIndexedComponent_all;
	MObject _mObject_currentVertexComponent_all = _MFnSingleIndexedComponent_all.create(MFn::kMeshVertComponent);

	MDoubleArray _mDouble_weightData_allSelected;
	MDoubleArray _mDouble_weightFinal_redo;
};
