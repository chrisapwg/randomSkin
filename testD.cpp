#include <maya/MPxCommand.h>
#include <maya/MArgList.h>
#include <maya/MStatus.h>
#include <maya/MGlobal.h>
#include <maya/MSelectionList.h>
#include <maya/MObject.h>
#include <maya/MFnMesh.h>
#include <maya/MFnSkinCluster.h>
#include <maya/MItGeometry.h>
#include <maya/MArgDatabase.h>
#include <maya/MString.h>
#include <maya/MDagPath.h>
#include <maya/MPlug.h>
#include <maya/MDoubleArray.h>
#include <maya/MFnPlugin.h>

class ChangeSkinCommand : public MPxCommand
{
public:
    MStatus doIt(const MArgList& args) override;
    static void* creator();

private:
    MStatus parseArgs(const MArgList& args);

    MString vertexData;
    MString skinClusterName;
    MString jointName;
    double valueChange;
};

void* ChangeSkinCommand::creator()
{
    return new ChangeSkinCommand;
}

MStatus ChangeSkinCommand::parseArgs(const MArgList& args)
{
    MStatus status;
    MArgDatabase argData(syntax(), args, &status);
    if (status != MS::kSuccess)
        return status;

    if (argData.isFlagSet("vertexData"))
        argData.getFlagArgument("vertexData", 0, vertexData);
    else
        return MS::kInvalidParameter;

    if (argData.isFlagSet("skinCluster"))
        argData.getFlagArgument("skinCluster", 0, skinClusterName);
    else
        return MS::kInvalidParameter;

    if (argData.isFlagSet("jointName"))
        argData.getFlagArgument("jointName", 0, jointName);
    else
        return MS::kInvalidParameter;

    if (argData.isFlagSet("valueChange"))
        argData.getFlagArgument("valueChange", 0, valueChange);
    else
        return MS::kInvalidParameter;

    return MS::kSuccess;
}

MStatus ChangeSkinCommand::doIt(const MArgList& args)
{
    MStatus status;

    if (parseArgs(args) != MS::kSuccess)
        return MS::kInvalidParameter;

    MSelectionList selList;
    selList.add(skinClusterName);

    MDagPath skinClusterPath;
    selList.getDagPath(0, skinClusterPath);

    MFnSkinCluster skinCluster(skinClusterPath);
    if (!skinCluster.isValid()) {
        MGlobal::displayError("Failed to find skin cluster: " + skinClusterName);
        return MS::kFailure;
    }

    MStringArray vertexDataArray;
    vertexData.split('.', vertexDataArray);

    if (vertexDataArray.length() != 2) {
        MGlobal::displayError("Invalid vertex data format. Expected 'mesh.vtx[index]'.");
        return MS::kInvalidParameter;
    }

    MString meshName = vertexDataArray[0];
    MString vertexIndexStr = vertexDataArray[1];

    if (!vertexIndexStr.startswith("vtx[")) {
        MGlobal::displayError("Invalid vertex data format. Expected 'mesh.vtx[index]'.");
        return MS::kInvalidParameter;
    }

    vertexIndexStr = vertexIndexStr.substring(4, vertexIndexStr.length() - 2);

    int vertexIndex = vertexIndexStr.asInt(&status);
    if (status != MS::kSuccess) {
        MGlobal::displayError("Invalid vertex index.");
        return status;
    }

    unsigned int jointIndex = skinCluster.indexForInfluenceObject(jointName);
    if (jointIndex == UINT_MAX) {
        MGlobal::displayError("Joint not found: " + jointName);
        return MS::kInvalidParameter;
    }

    MSelectionList meshSelList;
    meshSelList.add(meshName);
    MDagPath meshDagPath;
    meshSelList.getDagPath(0, meshDagPath);

    MSelectionList componentSelList;
    componentSelList.add(meshName + ".vtx[" + vertexIndexStr + "]");

    MObject component;
    componentSelList.getDagPath(0, meshDagPath, component);

    MDoubleArray weights;
    skinCluster.getWeights(meshDagPath, component, weights);

    if (jointIndex < weights.length()) {
        weights[jointIndex] = valueChange;
        skinCluster.setWeights(meshDagPath, component, jointIndex, weights);
    }
    else {
        MGlobal::displayError("Joint index out of range.");
        return MS::kFailure;
    }

    return MS::kSuccess;
}

MStatus initializePlugin(MObject obj)
{
    MStatus status;
    MFnPlugin plugin(obj, "MyPlugin", "1.0", "Any");

    status = plugin.registerCommand("changeSkin", ChangeSkinCommand::creator);
    if (!status) {
        status.perror("registerCommand");
        return status;
    }

    return MS::kSuccess;
}

MStatus uninitializePlugin(MObject obj)
{
    MStatus status;
    MFnPlugin plugin(obj);

    status = plugin.deregisterCommand("changeSkin");
    if (!status) {
        status.perror("deregisterCommand");
        return status;
    }

    return MS::kSuccess;
}