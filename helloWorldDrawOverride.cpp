#include "helloWorldDrawOverride.h"

// ======================//
// Public Method
// ======================//

helloWorldDrawOverride::~helloWorldDrawOverride()
{
}

MHWRender::DrawAPI helloWorldDrawOverride::supportedDrawAPIs() const
{
	return (MHWRender::kAllDevices);
}

bool helloWorldDrawOverride::hasUIDrawables() const
{
	return true;
}

void helloWorldDrawOverride::addUIDrawables(const MDagPath& objPath, MHWRender::MUIDrawManager& DrawManager, const MHWRender::MFrameContext& frameContent, const MUserData* userdata)
{
	DrawManager.beginDrawable();
	DrawManager.text2d(MPoint(100, 100), "Hello World!");
	DrawManager.endDrawable();
}

MUserData* helloWorldDrawOverride::prepareForDraw(const MDagPath& objPath, const MDagPath& cameraPath, const MHWRender::MFrameContext& frameContext, MUserData* oldData)
{
	return nullptr;
}

// ======================//
// Static Method
// ======================//

MHWRender::MPxDrawOverride* helloWorldDrawOverride::creator(const MObject& obj)
{
	return (new helloWorldDrawOverride(obj));
}

// ======================//
// Private Method
// ======================//

helloWorldDrawOverride::helloWorldDrawOverride(const MObject& obj):
	MHWRender::MPxDrawOverride(obj, nullptr)
{
}