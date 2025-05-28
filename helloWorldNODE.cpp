#include "helloWorldNODE.h"

// ======================//
// Static Constant
// ======================//

// Static is a lifetime use variable, because ce transfer this to other file
static const MString _nodeTypeName = "helloWorld"; // Cara panggil di Node
static const MTypeId _nodeTypeID = MTypeId(0x0007F7F7); // Jenis Node

// Draww varialbe is need for the flag
static const MString _objectDrawClass = "drawdb/geometry/helloWorld";
static const MString _objectDrawID = "helloWorldNODE";

// ======================//
// Public Method
// ======================//

helloWorldNODE::helloWorldNODE()
{
}

helloWorldNODE::~helloWorldNODE()
{
}

// ======================//
// Static Method
// ======================//

void* helloWorldNODE::creator()
{
	return(new helloWorldNODE());
}

MStatus helloWorldNODE::initialize()
{
	return (MS::kSuccess);
}

MString helloWorldNODE::nodeTypeName()
{
	return (_nodeTypeName);
}

MTypeId helloWorldNODE::nodeTypeID()
{
	return (_nodeTypeID);
}

MString helloWorldNODE::objectDrawClass()
{
	return (_objectDrawClass);
}

MString helloWorldNODE::objectDrawID()
{
	return (_objectDrawID);
}