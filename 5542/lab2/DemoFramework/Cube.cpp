#include "OpenGL.h"

#include <string>
#include <vector>

// Includes to create the cube
#include "ISceneNode.h"
#include "ModelManager.h"
#include "DrawableProxy.h"
#include "DrawableCubeSolution.h"
#include "DrawableSphereSolution.h"
#include "DrawableFloorSolution.h"

// Material includes
#include "SolidColorMaterialSolution.h"
#include "ShadedMaterial.h"
#include "MaterialProxy.h"
#include "MaterialManager.h"
#include "ShaderConstantMaterial.h"
#include "Color.h"

#include "BlankTexture2D.h"
#include "TextureBinding.h"
#include "TextureBindManager.h"
#include "TextureBindManagerOpenGL.h"
#include "TextureDataImage.h"
#include "SamplerApplicator.h"
#include "SimpleShaderMaterial.h"
#include "TexParam2DNoMipMap.h"
#include "TexParam2DMipMap.h"

// Includes for the camera
#include "ExaminerCameraNode.h"
#include "PerspectiveTransformSolution.h"
#include "LookAtTransformSolution.h"
#include "ShaderConstantModelView.h"

// Lights
#include "PointLight.h"
#include "DirectionalLight.h"
#include "LightManager.h"
#include "ShaderConstantLights.h"

#include "RenderTargetProxy.h"
#include "RenderTarget.h"
#include "RenderManager.h"
#include "ClearFrameCommand.h"
#include "SwapCommand.h"

// Includes for walking the scene graph
#include "DebugRenderVisitor.h"
#include "PrintSceneVisitor.h"

// Interaction
std::vector<IMouseHandler*> mouseHandlers;
std::vector<IKeyboardHandler*> keyboardHandlers;


using namespace Crawfis::Graphics;
using namespace Crawfis::Math;
using namespace std;


ISceneNode* rootSceneNode;
IVisitor* renderVisitor;
ExaminerCameraNode* examiner;

int windowGUID;
int windowWidth;
int windowHeight; 

void CreateGLUTWindow(std::string windowTitle)
{
	windowWidth = 800;
	windowHeight = 600;
	glutInitDisplayMode(GLUT_RGB);
	glutInitWindowSize(windowWidth, windowHeight);
	windowGUID = glutCreateWindow(windowTitle.c_str());
}

void InitializeOpenGLExtensions()
{
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		/* Problem: glewInit failed, something is seriously wrong. */
		fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
		throw "Error initializing GLEW";
	}

	fprintf(stdout, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));
}

void InitializeDevices()
{
	CreateGLUTWindow("OpenGL Demo Framework");
	InitializeOpenGLExtensions();
	glDisable(GL_CULL_FACE);
}

void CreateLights()
{
	PointLight* pointLight = new PointLight("light0-pt");
	pointLight->setPosition(Vector3(-2, 5, -1));

	LightManager::Instance()->SetLight(0, pointLight);
	DirectionalLight* dirLight = new DirectionalLight("light1-dir");
	//dirLight->setColor(Colors::IndianRed);
	dirLight->setDirection(Vector3(10, 1, 1));
	LightManager::Instance()->SetLight(1, dirLight);
}
void CreateGoldMaterial()
{
	VertexRoutine* vertexRoutine = new VertexRoutine("..\\Media\\Shaders\\VertexLight.glsl");
	FragmentRoutine* fragmentRoutine = new FragmentRoutine("..\\Media\\Shaders\\SolidColorSolution.frag");
	IShaderProgram* shaderProgram = new ShaderProgramWithMatrices(vertexRoutine, fragmentRoutine);

	Color gold(0.8314f, 0.6863f, 0.2169f, 1.0f);
	ShadedMaterial* shinyGold = new ShadedMaterial(shaderProgram);
	shinyGold->setAmbientReflection(0.01f*gold);
	shinyGold->setDiffuseReflection(0.7f*gold);
	shinyGold->setSpecularReflection(0.25f*gold);
	shinyGold->setShininess(100.0f);

	ShaderConstantMaterial* materialConstant = new ShaderConstantMaterial("frontMaterial");
	materialConstant->setValue(shinyGold);
	ShaderConstantLights* lightConstant = new ShaderConstantLights();
	ShaderConstantCollection* shaderConstantList = new ShaderConstantCollection();
	shaderConstantList->AddConstant(materialConstant);
	shaderConstantList->AddConstant(lightConstant);
	shaderProgram->setShaderConstant(shaderConstantList);

	MaterialManager::Instance()->RegisterMaterial("ShinyGold", shinyGold);
}

void CreateTexturedMaterial()
{
	ITextureDataObject* texture = new BlankTexture2D(1024, 1024);
	ITextureDataObject* redTexture = new BlankTexture2D(1024, 1024, Color(1, 0, 0, 1), GL_RGB);
	redTexture->setTextureParams(&TexParam2DNoMipMap::Instance);
	ITextureDataObject* imageTexture = new TextureDataImage("../Media/Textures/UVGrid.jpg", GL_RGB);
	imageTexture->setTextureParams(&TexParam2DMipMap::Instance);
	SamplerApplicator* uniformBinding = new SamplerApplicator("texture");
	TextureBinding* binding = TextureBindManager::Instance()->CreateBinding(imageTexture, uniformBinding);
	binding->Enable();
	binding->Disable();
	VertexRoutine* vertexRoutine = new VertexRoutine("..\\Media\\Shaders\\ShadedTextured-vert.glsl");
	FragmentRoutine* fragmentRoutine = new FragmentRoutine("..\\Media\\Shaders\\Textured.frag");
	IShaderProgram* shaderProgram = new ShaderProgramWithMatrices(vertexRoutine, fragmentRoutine);
	SimpleShaderMaterial* texturedMaterial = new SimpleShaderMaterial(shaderProgram);
	texturedMaterial->setShaderConstant(uniformBinding);
	texturedMaterial->AddTexture(binding);

	ShadedMaterial* white = new ShadedMaterial(shaderProgram);
	white->setAmbientReflection(0.0f*Colors::White);
	white->setDiffuseReflection(0.75f*Colors::White);
	white->setSpecularReflection(0.25f*Colors::White);
	white->setShininess(10.0f);

	ShaderConstantMaterial* materialConstant = new ShaderConstantMaterial("frontMaterial");
	materialConstant->setValue(white);
	ShaderConstantLights* lightConstant = new ShaderConstantLights();
	ShaderConstantCollection* shaderConstantList = new ShaderConstantCollection();
	shaderConstantList->AddConstant(materialConstant);
	shaderConstantList->AddConstant(lightConstant);
	shaderProgram->setShaderConstant(shaderConstantList);

	MaterialManager::Instance()->RegisterMaterial("Textured", texturedMaterial);
}
ISceneNode* CreateSceneGraph()
{
	// Create a simple scene
	// Perspective
	// LookAt camera
	// Drawable Cube
	//
	// First, create the models and register them.
	DrawableSphereSolution* sphere = new DrawableSphereSolution(10000);
	sphere->setLevel(6);
	ModelManager::Instance()->RegisterModel("Sphere", sphere);
	DrawableSphereSolution* smoothSphere = new DrawableSphereSolution(100000);
	ModelManager::Instance()->RegisterModel("SmoothSphere", smoothSphere);
	IDrawable* cube = new DrawableCubeSolution();
	ModelManager::Instance()->RegisterModel("Cube", cube);
	DrawableFloor* floor = new DrawableFloor(10,10);
	ModelManager::Instance()->RegisterModel("Floor", floor);

	DrawableProxy* sphereNode = new DrawableProxy("Sphere", "SmoothSphere");
	DrawableProxy* sphereNode2 = new DrawableProxy("Sphere", "Sphere");
	DrawableProxy* cubeNode = new DrawableProxy("Cube", "Cube"); // It is okay if they have the same name.
	DrawableProxy* floorNode = new DrawableProxy("Floor", "Floor"); // It is okay if they have the same name.
																 // Add a material
	SolidColorMaterialSolution* scarlet = new SolidColorMaterialSolution(Colors::White);
	MaterialManager::Instance()->RegisterMaterial("Scarlet", scarlet);

	CreateLights();
	CreateGoldMaterial();
	CreateTexturedMaterial();

	MaterialProxy* materialNode = new MaterialProxy("Floor Material", "Textured", floorNode);
	TransformMatrixNodeSolution* floorTransform = new TransformMatrixNodeSolution("CubeSpace", materialNode);
	floorTransform->Scale(60, 1, 60);
	//floorTransform->Rotate(1.57f, Vector3(1, 0, 0));
	//floorTransform->Translate(0, 1.0, 0);
	MaterialProxy* cubeMaterialNode = new MaterialProxy("Cube Material", "Scarlet", cubeNode);
	TransformMatrixNodeSolution* cubeTransform = new TransformMatrixNodeSolution("CubeSpace", cubeMaterialNode);
	cubeTransform->Scale(1, 2, 1);
	cubeTransform->Translate(0, 1.0, 0);
	MaterialProxy* sphereMaterialNode = new MaterialProxy("Sphere Material", "ShinyGold", sphereNode);
	TransformMatrixNodeSolution* sphereTransform = new TransformMatrixNodeSolution("SphereSpace", sphereMaterialNode);
	sphereTransform->Scale(5, 1, 5);
	sphereTransform->Translate(0, 3.9, 0);
	GroupNode* group = new GroupNode("Pedestal");
	group->AddChild(sphereTransform);
	group->AddChild(cubeTransform);
	group->AddChild(floorTransform);
	//Vector3 eyePosition = Vector3(-5.0,43.0,-4.998);
	//Vector3 centerOfInterest = Vector3(0,13,0);
	//Vector3 viewUp = Vector3(0,1,0);
	examiner = new ExaminerCameraNode("Examiner", group);
	examiner->setWidth(windowWidth);
	examiner->setHeight(windowHeight);
	//ISceneNode* lookAt = new LookAtTransformSolution("LookAt", materialNode, eyePosition, centerOfInterest, viewUp);
	//ISceneNode* rootNode = new PerspectiveTransformSolution("Perspective", lookAt, 30, windowWidth, windowHeight, 0.1, 100);
	return examiner;
}

void DisplayFrame()
{
	rootSceneNode->Accept(renderVisitor);
}

void ReshapeWindow(int newWidth, int newHeight)
{
	windowWidth = newWidth;
	windowHeight = newHeight;
	examiner->setWidth(windowWidth);
	examiner->setHeight(windowHeight);
	glViewport(0, 0, windowWidth, windowHeight);
	glutPostRedisplay();
}

ISceneNode* CreateFrameBuffer(Crawfis::Graphics::ISceneNode * scene)
{
	IRenderTarget* screen = new RenderTarget();
	RenderManager::Instance()->RegisterRenderTarget("Screen", screen);
	screen->setEnableCommand(new ClearFrameCommand(Colors::IndianRed));
	screen->setDisableCommand(new SwapCommand(true));
	RenderTargetProxy* frameBuffer = new RenderTargetProxy("Screen Display", "Screen", scene);
	return frameBuffer;
}
void KeyboardController(unsigned char key, int x, int y)
{
	printf("Key Pressed: %c\n", key);
	std::vector<IKeyboardHandler*>::iterator handlerIterator;
	for (handlerIterator = keyboardHandlers.begin(); handlerIterator != keyboardHandlers.end(); handlerIterator++)
	{
		(*handlerIterator)->KeyPress(key, x, y);
	}
	glutPostRedisplay();
}

void NumPadController(int key, int x, int y)
{
	std::vector<IKeyboardHandler*>::iterator handlerIterator;
	for (handlerIterator = keyboardHandlers.begin(); handlerIterator != keyboardHandlers.end(); handlerIterator++)
	{
		(*handlerIterator)->NumPadPress(key, x, y);
	}
	glutPostRedisplay();
}

void MousePressController(int button, int state, int ix, int iy)
{
	std::vector<IMouseHandler*>::iterator handlerIterator;
	for (handlerIterator = mouseHandlers.begin(); handlerIterator != mouseHandlers.end(); handlerIterator++)
	{
		(*handlerIterator)->MouseEvent(button, state, ix, iy);
	}
	glutPostRedisplay();
}

void MouseMotionController(int ix, int iy)
{
	std::vector<IMouseHandler*>::iterator handlerIterator;
	for (handlerIterator = mouseHandlers.begin(); handlerIterator != mouseHandlers.end(); handlerIterator++)
	{
		(*handlerIterator)->MouseMoved(ix, iy);
	}
	glutPostRedisplay();
}

void IdleCallback()
{
}
void InitializeDevIL()
{
	::ilInit();
	::iluInit();
	::ilutInit();
	::ilOriginFunc(IL_ORIGIN_LOWER_LEFT);
	::ilEnable(IL_ORIGIN_SET);
}

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	InitializeDevices();
	InitializeDevIL();
	TextureBindManagerOpenGL::Init();

	MatrixStack::Init();
	ISceneNode* scene = CreateSceneGraph();
	rootSceneNode = CreateFrameBuffer(scene);

	renderVisitor = new DebugRenderVisitor();
	PrintSceneVisitor* printScene = new PrintSceneVisitor();
	rootSceneNode->Accept(printScene);

	keyboardHandlers.push_back(examiner);
	mouseHandlers.push_back(examiner);

	glutDisplayFunc(DisplayFrame);
	glutReshapeFunc(ReshapeWindow);
	glutKeyboardFunc(KeyboardController);
	glutSpecialFunc(NumPadController);
	glutMouseFunc(MousePressController);
	glutMotionFunc(MouseMotionController);

	glutMainLoop();

	return 0;
}