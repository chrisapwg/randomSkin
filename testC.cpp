#include <maya/MPxCommand.h>
#include <maya/MArgList.h>
#include <maya/MStatus.h>
#include <maya/MString.h>
#include <maya/MStringArray.h>
#include <maya/MGlobal.h>
#include <maya/MArgDatabase.h>
#include <maya/MSyntax.h>
#include <maya/MFnPlugin.h>
#include <maya/MStringArray.h>

class testPrintCommand : public MPxCommand
{
public:
    testPrintCommand() {};
    ~testPrintCommand() override {};

    MStatus doIt(const MArgList& args) override;
    static void* creator();
    static MSyntax newSyntax();
};

void* testPrintCommand::creator()
{
    return new testPrintCommand();
}

MSyntax testPrintCommand::newSyntax()
{
    MSyntax syntax;
    syntax.addArg(MSyntax::kString);  // objects
    syntax.addFlag("b", "booleanFlag", MSyntax::kBoolean);  // Correctly define flag for booleans
    syntax.addFlag("s", "stringFlag", MSyntax::kString);
    syntax.addFlag("i", "intFlag", MSyntax::kLong);
    return syntax;
}

MStatus testPrintCommand::doIt(const MArgList& args)
{
    MStatus status;
    MStringArray objects;
    MIntArray booleans;
    MStringArray strings;
    MIntArray ints;

    MArgDatabase argData(newSyntax(), args, &status);
    if (status != MS::kSuccess) {
        MGlobal::displayError("Failed to parse arguments.");
        return status;
    }

    // Get objects from the arguments
    for (unsigned int i = 0; i < args.length(); ++i) {
        MString object;
        status = args.get(i, object);
        if (status != MS::kSuccess) {
            MGlobal::displayError("Failed to retrieve object.");
            return status;
        }
        objects.append(object);
    }

    // Get the boolean flags
    if (argData.isFlagSet("-b")) {
        bool flagValue;
        status = argData.getFlagArgument("-b", 0, flagValue);
        if (status != MS::kSuccess) {
            MGlobal::displayError("Failed to retrieve boolean flag.");
            return status;
        }
        booleans.append(flagValue ? 1 : 0);
    }

    // Get the string flags
    if (argData.isFlagSet("-s")) {
        MString flagValue;
        status = argData.getFlagArgument("-s", 0, flagValue);
        if (status != MS::kSuccess) {
            MGlobal::displayError("Failed to retrieve string flag.");
            return status;
        }
        strings.append(flagValue);
    }

    // Get the integer flags
    if (argData.isFlagSet("-i")) {
        int flagValue;
        status = argData.getFlagArgument("-i", 0, flagValue);
        if (status != MS::kSuccess) {
            MGlobal::displayError("Failed to retrieve integer flag.");
            return status;
        }
        ints.append(flagValue);
    }

    // Print objects
    MString objectOutput = "object was : ";
    for (unsigned int i = 0; i < objects.length(); ++i) {
        objectOutput += "'";
        objectOutput += objects[i];
        objectOutput += "'";
        if (i < objects.length() - 1) {
            objectOutput += ", ";
        }
    }
    MGlobal::displayInfo(objectOutput);

    // Print booleans
    MString booleanOutput = "bolean was : ";
    for (unsigned int i = 0; i < booleans.length(); ++i) {
        booleanOutput += (booleans[i] == 1 ? "true" : "false");
        if (i < booleans.length() - 1) {
            booleanOutput += ", ";
        }
    }
    MGlobal::displayInfo(booleanOutput);

    // Print strings
    MString stringOutput = "string was : ";
    for (unsigned int i = 0; i < strings.length(); ++i) {
        stringOutput += "'";
        stringOutput += strings[i];
        stringOutput += "'";
        if (i < strings.length() - 1) {
            stringOutput += ", ";
        }
    }
    MGlobal::displayInfo(stringOutput);

    // Print integers
    MString intOutput = "int was : ";
    for (unsigned int i = 0; i < ints.length(); ++i) {
        intOutput += ints[i];
        if (i < ints.length() - 1) {
            intOutput += ", ";
        }
    }
    MGlobal::displayInfo(intOutput);

    return MS::kSuccess;
}

MStatus initializePlugin(MObject obj)
{
    MStatus status;
    MFnPlugin plugin(obj, "MyPlugin", "1.0", "Any");
    status = plugin.registerCommand("testPrint", testPrintCommand::creator, testPrintCommand::newSyntax);
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
    status = plugin.deregisterCommand("testPrint");
    if (!status) {
        status.perror("deregisterCommand");
        return status;
    }
    return MS::kSuccess;
}
