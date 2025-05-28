#pragma once
//#ifndef MY_HEADER_H
//#define MY_HEADER_H
// Header is containing all varialbe and function defintion
// .cpp is function workflow

#include <maya/MPxCommand.h>

class helloWorldCMD : public MPxCommand // public mean wew use all MPxCommand variable or def in this class
{
public:
	helloWorldCMD(); // Created in every class, to initialize the class
	virtual ~helloWorldCMD() override; // Same as above, but for delete/destructor
									   // Override is for override the same fuction name
	virtual MStatus doIt(const MArgList& args) override; // '&' is must in the def/function call, so It not make a new variable inside the command

	// Static Things
	static void* creator(); //void is empty return, * is for return special variable
	static MString commandName();
};