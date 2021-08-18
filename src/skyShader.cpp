
/*
Shader implementing layers of atmosphere,
along with capacity to place the viewer at a certain point
in a planet (or gas toroid)

*/





#include "skyShader.h"


using namespace ed;
using namespace glm;

// Node id
MTypeId SkyHWShader::id(0x00081102);
// Node attributes
MObject  SkyHWShader::aColor;
MObject  SkyHWShader::aDiffuse;
MObject  SkyHWShader::aTransparency;
MObject  SkyHWShader::aSpecularColor;
MObject  SkyHWShader::aSpecularRolloff;
MObject  SkyHWShader::aEccentricity;
MObject  SkyHWShader::aNonTexturedColor;
MObject  SkyHWShader::aNonTexturedTransparency;


MTypeId SkyHWShader::kNODE_ID(0x00081102);
MString SkyHWShader::kNODE_NAME("skyShader");

static std::vector<MObject> driverMObjects;
static std::vector<MObject> drivenMObjects;



// positions for square face
float quadVertices[] = {
	// positions     // colors
	-0.05f,  0.05f,  1.0f, 0.0f, 0.0f,
	 0.05f, -0.05f,  0.0f, 1.0f, 0.0f,
	-0.05f, -0.05f,  0.0f, 0.0f, 1.0f,

	-0.05f,  0.05f,  1.0f, 0.0f, 0.0f,
	 0.05f, -0.05f,  0.0f, 1.0f, 0.0f,
	 0.05f,  0.05f,  0.0f, 1.0f, 1.0f
};

const char* starVertPath = "F:/all_projects_desktop/common/edCode/edPlugin/src/shaders/starShaderVertex.glsl";
const char* starFragPath = "F:/all_projects_desktop/common/edCode/edPlugin/src/shaders/starShaderFragment.glsl";

// set up gl stuff
unsigned int setupGL() {

	// build vector array of coord and magnitude for buffer
	const int nEntries = 4;

	typedef vec3 entryType;

	entryType* starEntries;
	starEntries = new entryType[nEntries];

	unsigned int coordVBO;
	glGenBuffers(1, &coordVBO);
	glBindBuffer(GL_ARRAY_BUFFER, coordVBO);
	glBufferData(GL_ARRAY_BUFFER, nEntries * sizeof(entryType), &starEntries[0], GL_STATIC_DRAW);
	// GL_STATIC_DRAW is just a hint for memory optimisations, doesn't do much

	for (unsigned int i = 0; i < nEntries; i++)
	{
		// load data from star data array
		starEntries[i] = vec3(starData[3 * i], starData[3 * i + 1], starData[3 * i + 2]);
	}
	glBindBuffer(GL_ARRAY_BUFFER, 0); // unbind local reference to coord buffer


	// build buffers for quad vertex positions
	unsigned int quadVAO, quadVBO; // VAO is thin interface class describing data held in actual buffer VBO
	glGenVertexArrays(1, &quadVAO); // initialise new vertex array helper object
	glGenBuffers(1, &quadVBO); // initialise new vertex buffer storage
	glBindVertexArray(quadVAO); // bind array helper to be worked on
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO); // bind buffer storage to be worked on
	// populate
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

	// specify how to read (involved
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	/* -attribute index 0 - this corresponds to setting layout = 0 in GLSL. 2 attrs can be read from different offsets on the same buffer
	* -2 components (pos x and pos y)
	* -components are GL_FLOAT
	* -components should not be normalised (whatever that does)
	* -size of ENTIRE ENTRY in bytes is 5 * sizeof floats long (including any other indexed attributes)
	* -THIS ATTRIBUTE starts at index 0 (hoping the ridiculous void pointer isn't needed)
	*/

	glEnableVertexAttribArray(1); // 2nd attribute, colour is vec3 contained in same buffer entry
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2 * sizeof(float)));

	// also load in the coord instance data as separate buffer
	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, coordVBO); // open coord buffer to work on
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(entryType), (void*)0);
	glBindBuffer(GL_ARRAY_BUFFER, 0); // close coord buffer

	glVertexAttribDivisor(2, 1);
	/* marks coord buffer at attr index 2 as instanced
	* every 1 instances (every full render of the main vertex buffer), coord buffer advances by 1 entry
	*/

	// shader initialisation ////////////////

	// set up define variables for GLSL files
	std::map<std::string, std::string> defineMap;

	defineMap["N_ENTRIES"] = nEntries;

	auto defineBlock = formatDefines(defineMap); // not needed here but useful to test

	// load shader files
	std::string vertShaderS = fileToChar(starVertPath);
	vertShaderS = defineBlock + vertShaderS;

	std::string fragShaderS = fileToChar(starFragPath);
	fragShaderS = defineBlock + fragShaderS;

	// create shader objects
	uint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertShaderS.c_str(), NULL);

	return coordVBO;
}

unsigned int teardownGL(unsigned int buffer){
	// delete the held buffer
	glDeleteBuffers(1, &buffer);
	return buffer;
}


class Line {

	int shaderProgram;
	unsigned int VBO, VAO;
	std::vector<float> vertices;
	vec3 startPoint;
	vec3 endPoint;
	mat4 MVP = mat4(1.0);
	vec3 lineColor;
public:
	Line(vec3 start, vec3 end) {

		startPoint = start;
		endPoint = end;
		lineColor = vec3(1, 1, 1);

		const char* vertexShaderSource = "#version 330 core\n"
			"layout (location = 0) in vec3 aPos;\n"
			"uniform mat4 MVP;\n"
			"void main()\n"
			"{\n"
			"   gl_Position = MVP * vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
			"}\0";
		const char* fragmentShaderSource = "#version 330 core\n"
			"out vec4 FragColor;\n"
			"uniform vec3 color;\n"
			"void main()\n"
			"{\n"
			"   FragColor = vec4(color, 1.0f);\n"
			"}\n\0";

		// vertex shader
		int vertexShader = glCreateShader(GL_VERTEX_SHADER);
		//int vertexShader = mGL->glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
		glCompileShader(vertexShader);
		// check for shader compile errors

		// fragment shader
		int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
		glCompileShader(fragmentShader);
		// check for shader compile errors

		// link shaders
		shaderProgram = glCreateProgram();
		glAttachShader(shaderProgram, vertexShader);
		glAttachShader(shaderProgram, fragmentShader);
		glLinkProgram(shaderProgram);
		// check for linking errors

		//glDeleteShader(vertexShader);
		//glDeleteShader(fragmentShader);

		vertices = {
			 start.x, start.y, start.z,
			 end.x, end.y, end.z,

		};

		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glBindVertexArray(VAO);

		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices) * vertices.size(), vertices.data(), GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

	}

	int setMVP(mat4 mvp) {
		MVP = mvp;
		return 1;
	}

	int setColor(vec3 color) {
		lineColor = color;
		return 1;
	}

	int draw() {

		glUseProgram(shaderProgram);
		glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "MVP"), 1, GL_FALSE, &MVP[0][0]);
		glUniform3fv(glGetUniformLocation(shaderProgram, "color"), 1, &lineColor[0]);

		glBindVertexArray(VAO);
		glDrawArrays(GL_LINES, 0, 2);
		return 0;
	}

	~Line() {

		glDeleteVertexArrays(1, &VAO);
		glDeleteBuffers(1, &VBO);
		glDeleteProgram(shaderProgram);
	}
};


///////////////////////////////////////////////////////////////////////////////////////////
// Node methods
///////////////////////////////////////////////////////////////////////////////////////////
MStatus SkyHWShader::initialize()
{
	// Shader attributes for the node
	// They have been created to match internal parameters of the
	// hardware shader instance
	//
	MFnNumericAttribute nAttr;

	// Create textured mode input attributes
	aColor = nAttr.createColor("color", "c");
	nAttr.setStorable(true);
	nAttr.setKeyable(true);
	nAttr.setDefault(0.6f, 0.6f, 0.6f);
	nAttr.setAffectsAppearance(true);

	aDiffuse = nAttr.create("diffuse", "dc", MFnNumericData::kFloat);
	nAttr.setStorable(true);
	nAttr.setKeyable(true);
	nAttr.setDefault(0.8);
	nAttr.setMax(1.0f);
	nAttr.setMin(0.0f);
	nAttr.setAffectsAppearance(true);

	aTransparency = nAttr.create("transparency", "tr", MFnNumericData::kFloat);
	nAttr.setStorable(true);
	nAttr.setKeyable(true);
	nAttr.setDefault(0.0f);
	nAttr.setMax(1.0f);
	nAttr.setMin(0.0f);
	nAttr.setAffectsAppearance(true);

	aSpecularColor = nAttr.createColor("specularColor", "sc");
	nAttr.setStorable(true);
	nAttr.setKeyable(true);
	nAttr.setDefault(1.0f, 1.0f, 1.0f);
	nAttr.setAffectsAppearance(true);

	aSpecularRolloff = nAttr.create("specularRollOff", "sro", MFnNumericData::kFloat);
	nAttr.setStorable(true);
	nAttr.setKeyable(true);
	nAttr.setDefault(0.7);
	nAttr.setMax(1.0f);
	nAttr.setMin(0.0f);
	nAttr.setAffectsAppearance(true);

	aEccentricity = nAttr.create("eccentricity", "ec", MFnNumericData::kFloat);
	nAttr.setStorable(true);
	nAttr.setKeyable(true);
	nAttr.setDefault(0.3);
	nAttr.setMax(1.0f);
	nAttr.setMin(0.0f);
	nAttr.setAffectsAppearance(true);

	// Create non-textured mode input attributes
	aNonTexturedColor = nAttr.createColor("nonTexturedColor", "nc");
	nAttr.setStorable(true);
	nAttr.setKeyable(true);
	nAttr.setDefault(1.0f, 0.0f, 0.0f);
	nAttr.setAffectsAppearance(true);

	aNonTexturedTransparency = nAttr.create("nonTexturedTransparency", "nt", MFnNumericData::kFloat);
	nAttr.setStorable(true);
	nAttr.setKeyable(true);
	nAttr.setDefault(0.0f);
	nAttr.setMax(1.0f);
	nAttr.setMin(0.0f);
	nAttr.setAffectsAppearance(true);

	// create output attributes here
	// outColor is the only output attribute and it is inherited
	// so we do not need to create or add it.
	//

	// Add the attributes to the node
	/*std::vector<MObject> drivers {*/
	driverMObjects = {
		aColor, aDiffuse, aTransparency, aSpecularColor, aSpecularRolloff,
		aEccentricity, aNonTexturedColor, aNonTexturedTransparency
	};
	/*std::vector<MObject> driven { outColor };*/
	drivenMObjects = { outColor };
	addAttributes<SkyHWShader>(driverMObjects);
	setAttributesAffect<SkyHWShader>(driverMObjects, drivenMObjects);
	
	//addAttributes<SkyHWShader>(drivenMObjects);
	return MS::kSuccess;
}

//
// Very simplistic software compute for the Maya software renderer
#// returns a constant color.
//
MStatus SkyHWShader::compute(
	const MPlug& plug,
	MDataBlock& block)
{

	if ((plug != outColor) && (plug.parent() != outColor))
		return MS::kUnknownParameter;

	MFloatVector& color = block.inputValue(aColor).asFloatVector();

	// set output color attribute
	MDataHandle outColorHandle = block.outputValue(outColor);
	MFloatVector& outColor = outColorHandle.asFloatVector();
	outColor = color;

	outColorHandle.setClean();
	return MS::kSuccess;
}

////////////////////////////////////////////////////////////////////////////////////
// Swatch rendering: 
// Does not matter the mode for the viewport VP1 or VP2
// Uses material viewer utility which uses the VP2 render to draw the swatch.
////////////////////////////////////////////////////////////////////////////////////
MStatus SkyHWShader::renderSwatchImage(MImage& outImage)
{
	if (!MHWRender::MRenderer::theRenderer())
	{
		return MS::kSuccess;
	}
	// Use some sample objects for display
	MString meshSphere("meshTeapot");
	MString meshShaderball("meshShaderball");

	unsigned int targetW, targetH;
	outImage.getSize(targetW, targetH);

	return MHWRender::MRenderUtilities::renderMaterialViewerGeometry(
		//targetW > 128 ? meshShaderball : meshSphere,
		meshShaderball,
		thisMObject(),
		outImage,
		MHWRender::MRenderUtilities::kPerspectiveCamera,
		MHWRender::MRenderUtilities::kSwatchLight);
}

SkyShaderOverride::SkyShaderOverride(const MObject& obj) : MHWRender::MPxShaderOverride(obj) {
}

MHWRender::MPxShaderOverride* SkyShaderOverride::creator(const MObject& obj)
{
	return new SkyShaderOverride(obj);
}

MString SkyShaderOverride::initialize(const MInitContext& initContext,
	MSharedPtr<MUserData>& userdata)
{
	DEBUGS("vp2BlinnShaderOverride::initialize");
	//MStatus s = initialiseGL();
	//if (s != MS::kSuccess) {
	//	DEBUGS("could not initialise MGL function table");
	//}

	return MString("Autodesk Maya vp2 TestOpenGL Shader Override");
}

MStatus SkyShaderOverride::initialiseGL() {
	
	//DEBUGS(glGetString(GL_VERSION));
	MStatus s = MS::kSuccess;
	/*MHardwareRenderer* renderer = MHardwareRenderer::theRenderer();
	if (!renderer) {
		s = MS::kFailure;
		MCHECK(s, "failed to retrieve hardware renderer");
	}*/
	//mGL = (renderer->glFunctionTable());
	//line = Line(vec3(0, 0, 0), vec3(3, 3, 3));
	return s;
	
}

bool SkyShaderOverride::handlesDraw(MHWRender::MDrawContext& context) {
	return true;
}

bool SkyShaderOverride::draw(MHWRender::MDrawContext& context,
	const MHWRender::MRenderItemList& renderItemList) const
{

	//const_cast<Line*>(line).draw();
	//const_cast<Line&>(line).draw();

	//MMatrix worldViewMat = context.getMatrix(MFrameContext::kWorldViewMtx);
	MMatrix worldViewMat = context.getMatrix(MFrameContext::kViewMtx).inverse();

	Line newLine(vec3(0, 0, 0), vec3(2, 2, 2));
	mat4 newMat(2.0);
	newLine.setMVP(newMat);
	
	//newLine.setMVP(mmatrixToMat4(worldViewMat));
	newLine.draw();

	//DEBUGS("skyShader draw");

	return true; // no effect on gl, just notifies that shader is invalid
}



SkyShaderOverride::~SkyShaderOverride() {

}











//
//
//MStringArray SkyShaderOverride::sFragmentArray;
//bool SkyShaderOverride::sDebugFragment = false;
//
//static MString path = "F:/all_projects_desktop/common/edCode/external/Autodesk_Maya_2022_1_Update_DEVKIT_Windows/devkitBase/devkit";
//
//SkyShaderOverride::SkyShaderOverride(const MObject& obj)
//    : MHWRender::MPxShaderOverride(obj)
//    , fShaderInstance(NULL)
//    //, fNonTexturedShaderInstance(NULL)
//{
//    createShaderInstances();
//}
//SkyShaderOverride::~SkyShaderOverride()
//{
//    //release resources held by shader
//    MHWRender::MRenderer* renderer = MHWRender::MRenderer::theRenderer();
//    if (renderer)
//    {
//        const MHWRender::MShaderManager* shaderMgr = renderer->getShaderManager();
//        if (shaderMgr)
//        {
//            if (fShaderInstance)
//            {
//                shaderMgr->releaseShader(fShaderInstance);
//                fShaderInstance = NULL;
//            }
//        }
//    }
//}
//void SkyShaderOverride::createShaderInstances()
//{
//    MStatus s = MStatus::kSuccess;
//    MHWRender::MRenderer* renderer = MHWRender::MRenderer::theRenderer();
//    if (renderer)
//    {
//        const MHWRender::MShaderManager* shaderMgr = renderer->getShaderManager();
//        if (!shaderMgr) { // shader manager not initialised properly
//            return;
//        }
//        /*if (shaderMgr)
//        {*/
//
//        // XML-based fragment graph supports multiple connections between
//        // shade fragments, while MShaderInstance::addInputFragment can only
//        // support single connection between the shader instance and input
//        // fragment. To see how internal fragments are implemented, use the
//        // fragmentDumper plugin.
//        //
//        fShaderInstance = shaderMgr->getFragmentShader(
//            "customFileTextureBlinnShader", "outSurfaceFinal", 
//            true // should passed fragment be "decorated" with lighting data
//        );
//        // Connect the custom geometry shader.
//        //
//        s = fShaderInstance->addInputFragment(
//            "customPoint2ViewAlignedTexturedQuad", "GPUStage", "GPUStage");
//        //MCHECK(s, "unable to add input fragment for quad mesh gen");
//        
//        if (s != MS::kSuccess) {
//            DEBUGSL("unable to add input fragment for quad mesh gen");
//        }
//
//        // Acquire and bind the snow texture.
//  
//        //MString path;
//        //if (!MGlobal::executeCommand(MString("getModulePath -moduleName \"devkit\""), path))
//        //{
//        //    path = MString(getenv("MAYA_LOCATION")) + MString("/devkit");
//        //}
//        path += MString("/plug-ins/customSpriteShader/");
//        MHWRender::MTexture* texture =
//            renderer->getTextureManager()->acquireTexture(path + MString("snow.png"), "", 1);
//        if (texture)
//        {
//            MHWRender::MTextureAssignment texResource;
//            texResource.texture = texture;
//            fShaderInstance->setParameter("map", texResource);
//        }
//        else
//        {
//            MString errorMsg = MString("customSpriteShader failed to acquire texture from ") + path + MString("snow.png");
//            MGlobal::displayError(errorMsg);
//        }
//         
//         
//        // Acquire and bind the default texture sampler.
//        //
//        //MHWRender::MSamplerStateDesc samplerDesc;
//        //const MHWRender::MSamplerState* sampler =
//        //    MHWRender::MStateManager::acquireSamplerState(samplerDesc);
//        //if (sampler)
//        //{
//        //    fShaderInstance->setParameter("textureSampler", *sampler);
//        //}
//        
//        // Particle sprites cannot directly be drawn with the default non-
//        // textured shader instance which is designed for polygons, so we
//        // create a non-texture shader instance as well. Note the geometry
//        // shader is slightly different from the textured version.
//        //
//        //fNonTexturedShaderInstance = shaderMgr->getFragmentShader(
//        //    "customSolidColorBlinnShader", "outSurfaceFinal", true);
//        //fNonTexturedShaderInstance->addInputFragment(
//        //    "customPoint2ViewAlignedSolidQuad", "GPUStage", "GPUStage");
//        
//        //// Set color and transparency.
//        //float customColor[3] = { 1.0, 0.0, 0.0 };
//        //fNonTexturedShaderInstance->setParameter("customColor", customColor);
//        //float customTransparency[3] = { 0.0, 0.0, 0.0 };
//        //fNonTexturedShaderInstance->setParameter("customTransparency", customTransparency);
//        //fNonTexturedShaderInstance->setIsTransparent(false);
//        // Dump final effect source and perform validation, for debugging the
//        // custom shader fragments on all device APIs. For validation, plugin
//        // can bind a temporary draw context with a shader instance generated
//        // from the effect source dump. Make sure it's unbound afterwards.
//        if (sDebugFragment)
//        {
//            MString filePath = path + MString("customSpriteShader.fx");
//            fShaderInstance->writeEffectSourceToFile(filePath);
//            MHWRender::MDrawContext* dc =
//                MHWRender::MRenderUtilities::acquireSwatchDrawContext();
//            MHWRender::MShaderInstance* shaderInstance =
//                shaderMgr->getEffectsFileShader(filePath, "");
//            if (dc && shaderInstance)
//            {
//                if (!shaderInstance->bind(*dc))
//                {
//                    MString errorMsg = filePath +
//                        MString(":\n") +
//                        MHWRender::MShaderManager::getLastError() +
//                        MHWRender::MShaderManager::getLastErrorSource(true, true, 2);
//                    MGlobal::displayError(errorMsg);
//                }
//                shaderInstance->unbind(*dc);
//            }
//            shaderMgr->releaseShader(shaderInstance);
//            MHWRender::MRenderUtilities::releaseDrawContext(dc);
//        }
//        //}
//    }
//}
//MString SkyShaderOverride::initialize(const MInitContext&, MSharedPtr<MUserData>&)
//{
//    MString empty;
//    MHWRender::MVertexBufferDescriptor positionDesc(
//        empty,
//        MHWRender::MGeometry::kPosition,
//        MHWRender::MGeometry::kFloat,
//        3);
//    MHWRender::MVertexBufferDescriptor normalDesc(
//        empty,
//        MHWRender::MGeometry::kNormal,
//        MHWRender::MGeometry::kFloat,
//        3);
//    MString spritePP("spritePP");
//    MHWRender::MVertexBufferDescriptor spriteDesc(
//        spritePP,
//        MHWRender::MGeometry::kTexture,
//        spritePP,
//        MHWRender::MGeometry::kFloat,
//        4);
//    MHWRender::MVertexBufferDescriptor uvDesc(
//        empty,
//        MHWRender::MGeometry::kTexture,
//        MHWRender::MGeometry::kFloat,
//        2);
//    addGeometryRequirement(positionDesc);
//    addGeometryRequirement(normalDesc);
//    addGeometryRequirement(spriteDesc);
//    addGeometryRequirement(uvDesc);
//    if (fShaderInstance)
//    {
//        addShaderSignature(*fShaderInstance);
//    }
//    return MString("SkyShaderOverride");
//}
//bool SkyShaderOverride::handlesDraw(MHWRender::MDrawContext& context)
//{
//    const MHWRender::MPassContext& passCtx = context.getPassContext();
//    const MStringArray& passSem = passCtx.passSemantics();
//    bool handlePass = false;
//    for (unsigned int i = 0; i < passSem.length(); i++)
//    {
//        const MString& pass = passSem[i];
//        // For color passes, only handle if there isn't already a global override.
//        // This is the same as the default logic for this method in MPxShaderOverride
//        if (pass == MHWRender::MPassContext::kColorPassSemantic)
//        {
//            if (!passCtx.hasShaderOverride())
//            {
//                handlePass = true;
//            }
//        }
//        // Advanced transparency algorithms are supported.
//        else if (pass == MHWRender::MPassContext::kTransparentPeelSemantic ||
//            pass == MHWRender::MPassContext::kTransparentPeelAndAvgSemantic ||
//            pass == MHWRender::MPassContext::kTransparentWeightedAvgSemantic)
//        {
//            handlePass = true;
//        }
//        // If these semantics are specified then they override the color pass
//        // semantic handling.
//        else if (pass == MHWRender::MPassContext::kDepthPassSemantic ||
//            pass == MHWRender::MPassContext::kNormalDepthPassSemantic)
//        {
//            handlePass = false;
//        }
//    }
//    return handlePass;
//}
//void SkyShaderOverride::activateKey(MHWRender::MDrawContext& context, const MString& key)
//{
//    if (fShaderInstance)
//    {
//        fShaderInstance->bind(context);
//    }
//}
//bool SkyShaderOverride::draw(MHWRender::MDrawContext& context, const MHWRender::MRenderItemList& renderItemList) const
//{
//    if (fShaderInstance)
//    {
//        unsigned int passCount = fShaderInstance->getPassCount(context);
//        if (passCount)
//        {
//            for (unsigned int i = 0; i < passCount; i++)
//            {
//                fShaderInstance->activatePass(context, i);
//                MHWRender::MPxShaderOverride::drawGeometry(context);
//            }
//        }
//    }
//    return true;
//}
//void SkyShaderOverride::terminateKey(MHWRender::MDrawContext& context, const MString& key)
//{
//    if (fShaderInstance)
//    {
//        fShaderInstance->unbind(context);
//    }
//}
//bool SkyShaderOverride::addFragmentXML(MHWRender::MFragmentManager& fragMgr,
//    const MString& fileName,
//    bool asGraph)
//{
//    MString fragName = asGraph ? fragMgr.addFragmentGraphFromFile(fileName) :
//        fragMgr.addShadeFragmentFromFile(fileName, false);
//    if (fragName.length() == 0)
//    {
//        MString errorMsg = MString("customSpriteShader failed to add fragment from ") + fileName;
//        MGlobal::displayError(errorMsg);
//        return false;
//    }
//    sFragmentArray.append(fragName);
//    return true;
//}
//MStatus SkyShaderOverride::registerShadeFragments()
//{
//    MHWRender::MRenderer* renderer = MHWRender::MRenderer::theRenderer();
//    if (!renderer)
//    {
//        DEBUGSL("No available renderer");
//        return MS::kFailure;
//    }
//    MHWRender::MFragmentManager* fragMgr = renderer->getFragmentManager();
//    if (!fragMgr)
//    {
//        DEBUGSL("No available fragmentManager");
//        return MS::kFailure;
//    }
//    // Add search path (once only)
//    //
//
//    //if (!MGlobal::executeCommand(MString("getModulePath -moduleName /"devkit/""), path, false))
//    //{
//    //    path = MString(getenv("MAYA_LOCATION")) + MString("/devkit");
//    //}
//    //path += "/plug-ins/customSpriteShader";
//    //fragMgr->addFragmentPath(path);
//    //MString info = MString("customSpriteShader added a fragment search path: ") + path;
//    //MGlobal::displayInfo(info);
//    //// Fragment graphs are registered after shader fragments because of dependency.
//    //if (addFragmentXML(*fragMgr, "customFileTextureOutputColor.xml", false) &&
//    //    addFragmentXML(*fragMgr, "customFileTextureOutputTransparency.xml", false) &&
//    //    addFragmentXML(*fragMgr, "customPoint2ViewAlignedSolidQuad.xml", false) &&
//    //    addFragmentXML(*fragMgr, "customPoint2ViewAlignedTexturedQuad.xml", false) &&
//    //    addFragmentXML(*fragMgr, "customFileTextureBlinnShader.xml", true) &&
//    //    addFragmentXML(*fragMgr, "customSolidColorBlinnShader.xml", true))
//    //{
//    //    return MS::kSuccess;
//    //}
//    //return MS::kFailure;
//    return MS::kSuccess;
//}
//MStatus SkyShaderOverride::deregisterShadeFragments()
//{
//    DEBUGSL("sky deregisterShaderFragments")
//    bool success = false;
//    MHWRender::MRenderer* renderer = MHWRender::MRenderer::theRenderer();
//    if (!renderer)
//    {
//        DEBUGSL("no renderer found");
//        return MS::kFailure;
//    }
//    MHWRender::MFragmentManager* fragMgr = renderer->getFragmentManager();
//    if (!fragMgr)
//    {
//        DEBUGSL("no fragment manager found");
//        return MS::kFailure;
//    }
//    success = true;
//    for (unsigned int i = 0; i < sFragmentArray.length(); i++)
//    {
//        MString fragName = sFragmentArray[i];
//        if (fragMgr->hasFragment(fragName) && !fragMgr->removeFragment(fragName))
//        {
//            success = false;
//            MString errorMsg = MString("customSpriteShader failed to remove fragment ") + sFragmentArray[i];
//            MGlobal::displayError(errorMsg);
//        }
//    }
//    sFragmentArray.clear();
//    return success ? MS::kSuccess : MS::kFailure;
//}
//
//

