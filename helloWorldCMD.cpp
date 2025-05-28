#include "helloWorldCMD.h"
#include <maya/MGlobal.h>
#include <maya/MIOStream.h>

// ======================//
// Static Constant
// ======================//

static const MString MEL_COMMAND = "helloWorld";

// ======================//
// Public Method
// ======================//

helloWorldCMD::helloWorldCMD()
{
}

helloWorldCMD::~helloWorldCMD()
{
}

MStatus helloWorldCMD::doIt(const MArgList& args)
{
	MGlobal::displayInfo("Hello World!");
	cout << "Hello World!" << endl;

	return (MS::kSuccess);
}

// ======================//
// Static Method
// ======================//

void* helloWorldCMD::creator()
{
	return (new helloWorldCMD());
}

MString helloWorldCMD::commandName()
{
	return (MEL_COMMAND);
}