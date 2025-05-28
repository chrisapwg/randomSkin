#pragma once

#include <maya/MPxDrawOverride.h>

class helloWorldDrawOverride : public MHWRender::MPxDrawOverride
{
public:
	virtual ~helloWorldDrawOverride() override;
	virtual MHWRender::DrawAPI supportedDrawAPIs() const override;
	virtual bool hasUIDrawables() const override; // const after def meaning the object in it is const, if you put const in front, the result must const
	virtual void addUIDrawables(const MDagPath& objPath,
								MHWRender::MUIDrawManager& DrawManager,
								const MHWRender::MFrameContext& frameContent,
								const MUserData* userdata) override;
	virtual MUserData* prepareForDraw(const MDagPath& objPath,
									  const MDagPath& cameraPath,
									  const MHWRender::MFrameContext& frameContext,
									  MUserData* oldData) override;

	// Static Method
	static MHWRender::MPxDrawOverride* creator(const MObject& obj);

private:
	helloWorldDrawOverride(const MObject& obj);
};