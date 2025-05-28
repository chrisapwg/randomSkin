#include <maya/MFnPlugin.h>
#include <maya/MPxCommand.h>
#include <maya/MGlobal.h>
#include <maya/MArgDatabase.h>
#include <maya/MString.h>
#include <maya/MStringArray.h>
#include <maya/MSyntax.h>
#include <maya/MArgList.h>
#include <maya/MSelectionList.h>
#include <maya/MObject.h>
#include <maya/MItGeometry.h>
#include <maya/MFnSkinCluster.h>
#include <maya/MDoubleArray.h>
#include <maya/MIntArray.h>
#include <maya/MDagPathArray.h>
#include <maya/MDagPath.h>
#include <maya/MFnSingleIndexedComponent.h>

class ChangeSkinCommand : public MPxCommand
{
public:
    MStatus doIt(const MArgList& args) override;
    static void* creator();
    static MSyntax newSyntax();
};

MStatus ChangeSkinCommand::doIt(const MArgList& args)
{
    MStatus status;
    MSyntax syntax = newSyntax();
    MArgDatabase argData(syntax, args, &status);

    if (status != MS::kSuccess) {
        MGlobal::displayError("Error parsing arguments");
        return status;
    }

    // Get skin cluster
    MString skinClusterName;
    argData.getFlagArgument("sc", 0, skinClusterName);

    // Get vertices as a comma-separated string and split them
    MString vertexStr, jointStr;
    argData.getFlagArgument("vertices", 0, vertexStr);
    argData.getFlagArgument("joints", 0, jointStr);

    MStringArray vertexList, jointList;
    vertexStr.split(',', vertexList);
    jointStr.split(',', jointList);

    // Get weight values one by one
    MDoubleArray weightValues;
    for (unsigned int i = 0; i < jointList.length(); i++) {
        double weight;
        argData.getFlagArgument("weights", i, weight);
        weightValues.append(weight);
    }

    if (jointList.length() != weightValues.length()) {
        MGlobal::displayError("Number of joints and weights must match.");
        return MS::kFailure;
    }

    // Convert vertex names to selection list
    MSelectionList selection;
    for (unsigned int i = 0; i < vertexList.length(); i++) {
        selection.add(vertexList[i]);
    }

    // Get the first DAG path
    MDagPath dagPath;
    MObject component;
    selection.getDagPath(0, dagPath, component);

    // Convert to vertex component
    MFnSingleIndexedComponent compFn;
    MObject vertexComponent = compFn.create(MFn::kMeshVertComponent);

    MItGeometry iter(dagPath, component);
    MIntArray vertexIndices;
    for (; !iter.isDone(); iter.next()) {
        vertexIndices.append(iter.index());
    }
    compFn.addElements(vertexIndices);

    // Get skin cluster node
    MSelectionList skinClusterSel;
    MObject skinClusterObj;
    skinClusterSel.add(skinClusterName);
    skinClusterSel.getDependNode(0, skinClusterObj);

    MFnSkinCluster skinCluster(skinClusterObj, &status);
    if (status != MS::kSuccess) {
        MGlobal::displayError("Invalid skin cluster.");
        return MS::kFailure;
    }

    // Get joint indices
    MDagPathArray influencePaths;
    skinCluster.influenceObjects(influencePaths);
    MIntArray jointIndices;

    for (unsigned int i = 0; i < jointList.length(); i++) {
        bool found = false;
        for (unsigned int j = 0; j < influencePaths.length(); j++) {
            if (influencePaths[j].partialPathName() == jointList[i]) {
                jointIndices.append(j);
                found = true;
                break;
            }
        }
        if (!found) {
            MGlobal::displayError("Joint " + jointList[i] + " not found in skinCluster.");
            return MS::kFailure;
        }
    }

    // Set weights
    status = skinCluster.setWeights(dagPath, vertexComponent, jointIndices, weightValues, true);
    if (status != MS::kSuccess) {
        MGlobal::displayError("Failed to set weights.");
        return status;
    }

    MGlobal::displayInfo("Skin weights updated.");
    return MS::kSuccess;
}

void* ChangeSkinCommand::creator() { return new ChangeSkinCommand; }

MSyntax ChangeSkinCommand::newSyntax()
{
    MSyntax syntax;
    syntax.addFlag("sc", "skinCluster", MSyntax::kString);
    syntax.addFlag("vtx", "vertices", MSyntax::kString);
    syntax.addFlag("jt", "joints", MSyntax::kString);
    syntax.addFlag("wt", "weights", MSyntax::kDouble, MSyntax::kUnsigned);
    return syntax;
}

MStatus initializePlugin(MObject obj)
{
    MFnPlugin plugin(obj, "YourName", "1.0", "Any");
    MStatus status = plugin.registerCommand("changeSkin", ChangeSkinCommand::creator, ChangeSkinCommand::newSyntax);

    if (status != MS::kSuccess) {
        MGlobal::displayError("Failed to register changeSkin command.");
    }
    else {
        MGlobal::displayInfo("changeSkin command registered successfully.");
    }

    return status;
}

MStatus uninitializePlugin(MObject obj)
{
    MFnPlugin plugin(obj);
    return plugin.deregisterCommand("changeSkin");
}
