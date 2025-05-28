#pragma once
#include <maya/MPxCommand.h>
#include <maya/MObject.h>
#include <maya/MVector.h>

class simpleCommandA : public MPxCommand
{
public:
	simpleCommandA();
	virtual ~simpleCommandA();
	virtual MStatus doIt(const MArgList& args) override;
	virtual MStatus redoIt() override;
	virtual MStatus undoIt() override;
	virtual bool isUndoable() const override;

	// Static methods
	static void* creator();
	static MString commandName();
	static MSyntax commandSyntax();

private:
	MObject mObject;

	bool mIsUndoable;
	bool mIsEdit;
	bool mIsQuery;

	bool mVersion;
	bool mTranslate;

	MVector mGetTranslate;
	MVector mSetTranslate;
};