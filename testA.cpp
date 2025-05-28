#include <maya/MFnPlugin.h>
#include <maya/MPxCommand.h>
#include <maya/MGlobal.h>
#include <maya/MArgDatabase.h>
#include <maya/MString.h>
#include <maya/MSyntax.h>
#include <maya/MArgList.h> // ✅ Include this to fix "incomplete type" error

class HelloCommand : public MPxCommand
{
public:
    MStatus doIt(const MArgList& args) override;
    static void* creator();
    static MSyntax newSyntax();
};

MStatus HelloCommand::doIt(const MArgList& args)
{
    MStatus status;

    MString objectName;
    if (args.length() > 0) { // ✅ Ensure object name is captured before parsing flags
        args.get(0, objectName);
    }

    MSyntax syntax = newSyntax();
    MArgDatabase argData(syntax, args, &status);

    if (status != MS::kSuccess) {
        MGlobal::displayError("Error parsing arguments");
        return status;
    }

    bool optionA = false;
    int valueInt = 0;
    double valueFloat = 0.0;

    if (argData.isFlagSet("optionA")) {
        argData.getFlagArgument("optionA", false, optionA);
    }

    if (argData.isFlagSet("valueINT")) {
        argData.getFlagArgument("valueINT", 0, valueInt);
    }

    if (argData.isFlagSet("valueFLOAT")) {
        argData.getFlagArgument("valueFLOAT", 0, valueFloat);
    }

    MGlobal::displayInfo(MString("The object was: '") + objectName + "'");
    MGlobal::displayInfo(MString("Boolean option A was: ") + (optionA ? "True" : "False"));
    MGlobal::displayInfo(MString("Value int was: ") + valueInt);
    MGlobal::displayInfo(MString("Value float was: ") + valueFloat);

    return MS::kSuccess;
}

void* HelloCommand::creator()
{
    return new HelloCommand;
}

MSyntax HelloCommand::newSyntax()
{
    MSyntax syntax;
    syntax.addArg(MSyntax::kString);
    syntax.addFlag("oa", "optionA", MSyntax::kBoolean);
    syntax.addFlag("vi", "valueINT", MSyntax::kLong);
    syntax.addFlag("vf", "valueFLOAT", MSyntax::kDouble);
    return syntax;
}

MStatus initializePlugin(MObject obj)
{
    MFnPlugin plugin(obj, "MyPluginVendor", "1.0", "Any");
    MStatus status = plugin.registerCommand("helloCommand", HelloCommand::creator, HelloCommand::newSyntax);

    if (status != MS::kSuccess) {
        MGlobal::displayError("Failed to register helloCommand");
    }
    else {
        MGlobal::displayInfo("Successfully registered helloCommand");
    }

    return status;
}

MStatus uninitializePlugin(MObject obj)
{
    MFnPlugin plugin(obj);
    return plugin.deregisterCommand("helloCommand");
}
