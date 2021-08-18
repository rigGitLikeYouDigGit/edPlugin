

#ifndef _ED_SKYSHADER_H
#define _ED_SKYSHADER_H

#include <time.h>
#include <chrono>
#include <array>
#include <vector>

#include "lib/api.h"
#include "lib/topo.h"
#include "lib/mayaTopo.h"
#include "lib/json.h"


#include <maya/MIOStream.h>
#include <math.h>
#include <cstdlib>
#include "lib/shader.h"



//#include <maya/MGlobal.h>
#include <maya/MString.h>
#include <maya/MPlug.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFloatVector.h>
//#include <maya/MFnPlugin.h>
#include <maya/MFnDependencyNode.h>
//#include <maya/MUserData.h>

//#include <maya/MHardwareRenderer.h>

// Includes for swatch rendering
#include <maya/MHWShaderSwatchGenerator.h>
#include <maya/MImage.h>
#include <maya/MRenderUtilities.h>

#include <maya/MMatrix.h>

// Viewport 2.0 includes
#include <maya/MDrawRegistry.h>
#include <maya/MPxShaderOverride.h>
#include <maya/MDrawContext.h>
#include <maya/MStateManager.h>
#include <maya/MViewport2Renderer.h>
#include <maya/MShaderManager.h>

#include <maya/MPxHardwareShader.h>
#include <maya/MRenderProfile.h>
#include <maya/MHardwareRenderer.h>
//#include <maya/MGeometry.h>
//#include <maya/MGeometryList.h>


//
//#include <glad/glad.h>
//#include <GLFW/glfw3.h>
//#include <gl/glew.c>

//#include <glew.c>


//
//#include <maya/MGL.h>
//#include <maya/MGLdefinitions.h>
//#include <maya/MGLFunctionTable.h>
//


//GLEW_ARB_vertex_program

static MString starDir = "F:/all_projects_desktop/atmosphere/stars/";
static MString starCoordPath = starDir + "coords.json";


class Line;

class SkyHWShader : public MPxHardwareShader
{
public:
	SkyHWShader() {};
	~SkyHWShader() override {};

	MStatus compute(const MPlug&, MDataBlock&) override;

	// VP1 profile. Just leave as unsupported hardware profile
	// as sample code.
	const MRenderProfile& profile() override
	{
		static MRenderProfile sProfile;
		if (sProfile.numberOfRenderers() == 0)
			sProfile.addRenderer(MRenderProfile::kMayaOpenGL);
		return sProfile;
	}

	// Swatch rendering. Called irregardless of VP1 or VP2.
	//
	MStatus renderSwatchImage(MImage& image) override;

	//MStatus render(MGeometryList& iterator);

	static  void* creator() {
		return new SkyHWShader();
	}
	static  MStatus initialize();

	static  MTypeId id;
	static MTypeId kNODE_ID;
    static MString kNODE_NAME;

protected:

private:
	// Attributes
	static MObject  aColor;
	static MObject	aTransparency;
	static MObject  aDiffuse;
	static MObject  aDiffuseColor;
	static MObject  aSpecularColor;
	static MObject  aSpecularRolloff;
	static MObject  aEccentricity;
	static MObject  aNonTexturedColor;
	static MObject  aNonTexturedTransparency;
};


////////////////////////////////////////////////////////////////////////////////////
// Viewport 2.0 shader override implementation
////////////////////////////////////////////////////////////////////////////////////
class SkyShaderOverride : public MHWRender::MPxShaderOverride
{
public:

	//Line line = Line(vec3(0, 0, 0), vec3(2, 2, 2));

// Constructor. Simply initialize shader instances for usage.
// 
	SkyShaderOverride(const MObject& obj);

	// Release the textured and non-textured mode shaders.
	~SkyShaderOverride();

	// Static method to create a new override
	static MHWRender::MPxShaderOverride* creator(const MObject& obj);


	// initialise and retrieve gl function table from renderer
	MStatus initialiseGL();

	// 1. Initialize phase
	MString initialize(const MInitContext& initContext,
		MSharedPtr<MUserData>& userdata) override;

	bool handlesDraw(MHWRender::MDrawContext& context) override;

	// Transparency hint
	bool isTransparent() override
	{
		return false;
	}

	bool overridesDrawState() { return true; }
	bool overridesNonMaterialItems() { return true; }


	// 3. Draw Phase
	bool draw(MHWRender::MDrawContext& context,
		const MHWRender::MRenderItemList& renderItemList) const override;

};


#endif /* SkyShader */


