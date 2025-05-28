#include "simpleCommandA.h"
#include <maya/MSyntax.h>

#include <maya/MArgDatabase.h>
#include <maya/MDoubleArray.h>
#include <maya/MFnTransform.h>
#include <maya/MGlobal.h>
#include <maya/MSelectionList.h>


// ======================//
// Static Constant
// ======================//

static const MString _commandName = "simpleCommandA"; // Cara panggil di Node
static const char* _flagVersion[2] = { "-v", "-version" }; // array jumlah 2
static const char* _flagTranslate[2] = { "-t", "-translate" };

// ======================//
// Public Method
// ======================//

simpleCommandA::simpleCommandA(): // Ok belum tau kenapa pake : di akhir
	MPxCommand(),
	mIsUndoable(false),
	mIsEdit(false),
	mIsQuery(false),
	mVersion(false),
	mTranslate(false)
{
}

simpleCommandA::~simpleCommandA()
{
}

MStatus simpleCommandA::doIt(const MArgList& args)
{
	MStatus status;

	MArgDatabase _syntaxData(syntax(), args, &status);
	if (!status)
	{
		MGlobal::displayInfo("Failed to create argument database: " + status.errorString());
		return (status);
	}

	MSelectionList selectionList; // Bikin variable kosong
	status = _syntaxData.getObjects(selectionList); // Ambil object dari syntax kSelectionList
	if (!status)
	{
		MGlobal::displayInfo("Failed to get objects: " + status.errorString());
		return (status);
	}

	selectionList.getDependNode(0, mObject); // Ambil object pertama dari selectionList dan simpen di mObject
	if (mObject.apiType() != MFn::kTransform)
	{
		MGlobal::displayInfo("Selected object is not a transform node");
		return (MS::kFailure);
	}

	mIsEdit = _syntaxData.isEdit();
	mIsQuery = _syntaxData.isQuery();
	mVersion = _syntaxData.isFlagSet(_flagVersion[0]);
	mTranslate = _syntaxData.isFlagSet(_flagTranslate[0]);

	if (mTranslate)
	{
		MFnTransform _transformFN(mObject); // Cari nilai translate dari mObject, bisa translate rotate scale
		mGetTranslate = _transformFN.getTranslation(MSpace::kTransform);

		if (mIsEdit)
		{
			mSetTranslate = MVector(_syntaxData.flagArgumentDouble(_flagTranslate[0], 0),
									_syntaxData.flagArgumentDouble(_flagTranslate[0], 1),
									_syntaxData.flagArgumentDouble(_flagTranslate[0], 2));
			mIsUndoable = true;
		}
	}

	return (redoIt());
}

MStatus simpleCommandA::redoIt()
{
	MFnTransform _transformFN(mObject);

	if (mIsEdit)
	{
		if (mTranslate)
		{
			_transformFN.setTranslation(mSetTranslate, MSpace::kTransform);
		}
		else
		{
			MGlobal::displayInfo("Flag 'Edit' do not need any support flag.");
			return (MS::kFailure);
		}
	}
	else if (mIsQuery)
	{
		if (mTranslate)
		{
			double result[3]; // Array dengan 3 index
			//result[0] = mGetTranslate[0];
			mGetTranslate.get(result); // Ambil isi dari mGetTranslate ke result
			setResult(MDoubleArray(result, 3)); // Set result ke dalam MDoubleArray
		}
		else
		{
			MGlobal::displayInfo("Flag 'Query' do not need any support flag.");
			return (MS::kFailure);
		}
	}
	else if (mVersion)
	{
		MGlobal::displayInfo("Version: 1.0.0");
	}
	else
	{
		setResult(_transformFN.name()); // setResult itu print di script editor maya
	}

	return (MS::kSuccess);
}

MStatus simpleCommandA::undoIt()
{
	MFnTransform _transformFN(mObject);
	_transformFN.setTranslation(mGetTranslate, MSpace::kTransform);

	return (MS::kSuccess);
}

bool simpleCommandA::isUndoable() const
{
	return (mIsUndoable);
}

// ======================//
// Static Method
// ======================//

void* simpleCommandA::creator()
{
	return 	(new simpleCommandA()); // new artinya jalanin jalanin fungsi ini
}

MString simpleCommandA::commandName()
{
	return (_commandName);
}

MSyntax simpleCommandA::commandSyntax()
{
	MSyntax syntax;

	syntax.enableEdit(true);
	syntax.enableQuery(true);
	syntax.setObjectType(MSyntax::kSelectionList, 1, 1); // Minimum & maksimum 1 object terseleksi
	syntax.useSelectionAsDefault(true); // Use selection as default *obj, if no input in the command
	syntax.addFlag(_flagVersion[0], _flagVersion[1]);
	syntax.addFlag(_flagTranslate[0], _flagTranslate[1], MSyntax::kDouble, MSyntax::kDouble, MSyntax::kDouble);

	return (syntax);
}
