#include <maya/MDrawRegistry.h>
#include <maya/MFnPlugin.h>
#include <maya/MGlobal.h>
#include "helloWorldCMD.h"
#include "helloWorldDrawOverride.h"
#include "helloWorldNODE.h"
#include "simpleCommandA.h"
#include "skin_relax.h"
#include "skin_sharper.h"
#include "skin_add.h"

MStatus initializePlugin(MObject pluginObj)
{
	const char* author = "Chris Gultom";
	const char* version = "1.0";
	const char* requiredApiVersion = "Any";

	MStatus status;

	MFnPlugin pluginFn(pluginObj, author, version, requiredApiVersion, &status);
	if (!status)
	{
		MGlobal::displayInfo("Failed to initialize plugin: " + status.errorString());
		return (status);
	}

	// MStatus registerCommand (const MString &commandName,
	//							MCreatorFunction creatorFunction,
	//							MCreateSyntaxFunction createSyntaxFunction=nullptr)
	status = pluginFn.registerCommand(relaxSkin::commandName(), relaxSkin::creator, relaxSkin::commandSyntax);
	status = pluginFn.registerCommand(sharperSkin::commandName(), sharperSkin::creator, sharperSkin::commandSyntax);
	status = pluginFn.registerCommand(addSkin::commandName(), addSkin::creator, addSkin::commandSyntax);

	/*
	status = pluginFn.registerCommand(helloWorldCMD::commandName(), helloWorldCMD::creator); // using () mean is operating and return something, if def with () mean just give the function adress
	if (!status)
	{
		MGlobal::displayInfo("Failed to register command: helloWorldCMD");
		return (status);
	}

	status = pluginFn.registerCommand(simpleCommandA::commandName(), simpleCommandA::creator, simpleCommandA::commandSyntax);
	if (!status)
	{
		MGlobal::displayInfo("Failed to register command: " + simpleCommandA::commandName());
		return (status);
	}

	MString objectDrawClass_ = helloWorldNODE::objectDrawClass(); // Atau lokasi
	// registerNode (const MString &typeName,
	//				 const MTypeId &typeId,
	//				 MCreatorFunction creatorFunction,
	//				 MInitializeFunction initFunction,
	//				 MPxNode::Type type=MPxNode::kDependNode,
	//				 const MString *classification=nullptr)
	status = pluginFn.registerNode(helloWorldNODE::nodeTypeName(), // cara panggil node
								   helloWorldNODE::nodeTypeID(), // Type node saat ini adalah...
								   helloWorldNODE::creator, // Run this script
								   helloWorldNODE::initialize, // is an attribute list, Attribute Creation: You use Maya API functions (like MFnNumericAttribute::create, MFnTypedAttribute::create, etc.) to create attribute objects.
								   helloWorldNODE::kLocatorNode, // object visualnya adalah locator
								   &objectDrawClass_); // Still not know what and why
	if (!status)
	{
		MGlobal::displayInfo("Failed to register Node: helloWorldNODE");
		return (status);
	}

	status = MHWRender::MDrawRegistry::registerDrawOverrideCreator(helloWorldNODE::objectDrawClass(),
																   helloWorldNODE::objectDrawID(),
																   helloWorldDrawOverride::creator);
	if (!status)
	{
		MGlobal::displayInfo("Failed to register Draw Override: helloWorldDrawOverride");
		return (status);
	}
	*/
	return (MS::kSuccess);
}
MStatus uninitializePlugin(MObject pluginObj)
{
	MStatus status;

	MFnPlugin pluginFn(pluginObj);

	status = pluginFn.deregisterCommand(relaxSkin::commandName());
	if (!status)
	{
		MGlobal::displayInfo("Failed to deregister command: " + relaxSkin::commandName());
		return (status);
	}
	status = pluginFn.deregisterCommand(sharperSkin::commandName());
	if (!status)
	{
		MGlobal::displayInfo("Failed to deregister command: " + sharperSkin::commandName());
		return (status);
	}
	status = pluginFn.deregisterCommand(addSkin::commandName());
	if (!status)
	{
		MGlobal::displayInfo("Failed to deregister command: " + addSkin::commandName());
		return (status);
	}
	/*
	status = pluginFn.deregisterCommand(helloWorldCMD::commandName());
	if (!status)
	{
		MGlobal::displayInfo("Failed to deregister command: helloWorldCMD");
		return (status);
	}

	status = pluginFn.deregisterCommand(simpleCommandA::commandName());
	if (!status)
	{
		MGlobal::displayInfo("Failed to deregister command: " + simpleCommandA::commandName());
		return (status);
	}

	MString objectDrawClass_ = helloWorldNODE::objectDrawClass();

	status = pluginFn.deregisterNode(helloWorldNODE::nodeTypeID());
	if (!status)
	{
		MGlobal::displayInfo("Failed to deregister Node: helloWorldNODE");
		return (status);
	}

	status = MHWRender::MDrawRegistry::deregisterDrawOverrideCreator(helloWorldNODE::objectDrawClass(), helloWorldNODE::objectDrawID());
	if (!status)
	{
		MGlobal::displayInfo("Failed to deregister Draw Override: helloWorldDrawOverride");
		return (status);
	}
	*/
	return (MS::kSuccess);
}