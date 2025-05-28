#pragma once

#include <maya/MPxLocatorNode.h>

class helloWorldNODE : public MPxLocatorNode
{
public:
	helloWorldNODE();
	virtual ~helloWorldNODE() override;
	// Static Things
	static void* creator();
	static MStatus initialize();

	static MString nodeTypeName();
	static MTypeId nodeTypeID();

	static MString objectDrawClass();
	static MString objectDrawID();
};