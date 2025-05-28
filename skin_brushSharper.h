#pragma once
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

class brushSharper : public MPxCommand {
public: // Public so can used in main.cpp
	brushSharper();
	virtual ~brushSharper();
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
	//double _mValue;
	unsigned int _mLoop;
	unsigned int _mInfluence;
	double _mPrune;

	// ======================//
	// Used Variable
	// ======================//

	MDagPath _mDagPath_objectShape;
	MObject _mObject_skinCluster;
	MFnSkinCluster _mFnSkinCluster;
	MDagPathArray _mDagPathArray_joint;
	MIntArray _mInt_jointIndex;

	MFnSingleIndexedComponent _MFnSingleIndexedComponent_all;
	MObject _mObject_currentVertexComponent_all = _MFnSingleIndexedComponent_all.create(MFn::kMeshVertComponent);

	MDoubleArray _mDouble_weightData_allSelected;
	MDoubleArray _mDouble_weightFinal_redo;
};
