#include <GL/glew.h>
#include <GL/glut.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stdexcept>
#include <iostream>
#include <string>
#include <vector>

#include "Shader.h"
#include "Mesh.h"
#include "Renderer.h"
#include "MassSpringSolver.h"
#include "UserInteraction.h"

// G L O B A L S ///////////////////////////////////////////////////////////////////
typedef glm::vec3 Point;

// Window
static int g_windowWidth = 640, g_windowHeight = 640;
static bool g_mouseClickDown = false;
static bool g_mouseLClickButton, g_mouseRClickButton, g_mouseMClickButton;
static int g_mouseClickX;
static int g_mouseClickY;

static UserInteraction* UI;

// Constants
static const float PI = glm::pi<float>();

// Shader Handles
static PhongShader* g_phongShader; // linked phong shader
static PickShader* g_pickShader; // linked pick shader

// Mesh
static Mesh* g_clothMesh; // halfedge data structure

// Render Target
static ProgramInput* g_render_target; // vertex, normal, texutre, index

// Animation
static const int g_fps = 60; // frames per second  | 60
static const int g_hps = 4; // time steps per frame | 4
static const int g_iter = 7; // iterations per time step | 7
static const int g_frame_time = 15; // approximate time for frame calculations | 15
static const int g_animation_timer = (int) ((1.0f / g_fps) * 1000 - g_frame_time);

// Mass Spring System
static mass_spring_system* g_system;
static MassSpringSolver* g_solver;

// Constraint Graph
static CgRootNode* g_cgRootNode;

// System parameters
namespace SystemParam {
	static const int n = 41; // must be odd, n * n = n_vertices | 41
	static const float w = 10.0f; // width | 10.0f
	static const float h = 0.008f; // time step, smaller for better results | 0.008f
	static const float r = w / (n - 1); // spring rest legnth
	static const float k = 0.9f; // spring stiffness | 0.9f;
	static const float m = 0.25f / (n * n); // point mass | 0.25f
	static const float a = 0.995f; // damping, close to 1.0 | 0.995f
	static const float g = 9.8f * m; // gravitational force | 9.8f
}

// Scene parameters
static const float g_camera_distance = 15.0f;

// Scene matrices
static glm::mat4 g_ModelViewMatrix;
static glm::mat4 g_ProjectionMatrix;

// F U N C T I O N S //////////////////////////////////////////////////////////////
// state initialization
static void initGlutState(int, char**);
static void initGLState();

static void initShaders(); // Read, compile and link shaders
static void initCloth(); // Generate cloth mesh
static void initScene(); // Generate scene matrices

// glut callbacks
static void display();
static void reshape(int, int);
static void mouse(int, int, int, int);
static void motion(int, int);

// draw cloth function
static void drawCloth(bool picking);
static void animateCloth(int value);

// scene update
static void updateProjection();
static void updateRenderTarget();

// cleaning
static void cleanUp();

// error checks
void checkGlErrors();

// M A I N //////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv) {
	try {
		initGlutState(argc, argv);
		glewInit();
		initGLState();

		initShaders();
		initCloth();
		initScene();

		glutTimerFunc(g_animation_timer, animateCloth, 0);
		glutMainLoop();

		cleanUp();
		return 0;
	}
	catch (const std::runtime_error& e) {
		std::cout << "Exception caught: " << e.what() << std::endl;
		return -1;
	}
}


// S T A T E  I N I T I A L I Z A T O N /////////////////////////////////////////////
static void initGlutState(int argc, char** argv) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(g_windowWidth, g_windowHeight);
	glutCreateWindow("Cloth App");

	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutMouseFunc(mouse);
	glutMotionFunc(motion);
}

static void initGLState() {
	glClearColor(0.25f, 0.25f, 0.25f, 0);
	glClearDepth(1.);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glReadBuffer(GL_BACK);
	glEnable(GL_FRAMEBUFFER_SRGB);

	checkGlErrors();
}

static void initShaders() {
	GLShader basic_vert(GL_VERTEX_SHADER);
	GLShader phong_frag(GL_FRAGMENT_SHADER);
	GLShader pick_frag(GL_FRAGMENT_SHADER);

	basic_vert.compile(std::ifstream("./shaders/basic.vshader"));
	phong_frag.compile(std::ifstream("./shaders/phong.fshader"));
	pick_frag.compile(std::ifstream("./shaders/pick.fshader"));

	g_phongShader = new PhongShader;
	g_pickShader = new PickShader;
	g_phongShader->link(basic_vert, phong_frag);
	g_pickShader->link(basic_vert, pick_frag);
	checkGlErrors();
}

static void initCloth() {
	// short hand
	const int n = SystemParam::n;
	const float w = SystemParam::w;

	// generate mesh
	g_clothMesh = MeshBuilder::buildUniformGrid(w, n);

	// build index buffer
	g_clothMesh->useIBuff(MeshBuilder::buildUniformGridTrianglesIBuff(n));

	// fill program input
	g_render_target = new ProgramInput;
	g_render_target->setPositionData(g_clothMesh->vbuff(), g_clothMesh->vbuffLen());
	g_render_target->setNormalData(g_clothMesh->nbuff(), g_clothMesh->nbuffLen());
	g_render_target->setTextureData(g_clothMesh->tbuff(), g_clothMesh->tbuffLen());
	g_render_target->setIndexData(g_clothMesh->ibuff(), g_clothMesh->ibuffLen());

	checkGlErrors();

	// initialize mass spring system
	g_system = MassSpringBuilder::buildUniformGrid(
		SystemParam::n,
		SystemParam::h,
		SystemParam::r,
		SystemParam::k,
		SystemParam::m,
		SystemParam::a,
		SystemParam::g
	);

	// initialize mass spring solver
	g_solver = new MassSpringSolver(g_system, g_clothMesh->vbuff());

	// build constraint graph
	g_cgRootNode = new CgRootNode(g_system, g_clothMesh->vbuff());
	
	// fix top corners
	CgPointFixNode* cornerFixer = new CgPointFixNode(g_system, g_clothMesh->vbuff());
	cornerFixer->fixPoint(0);
	cornerFixer->fixPoint(SystemParam::n - 1);

	// initialize user interaction
	CgPointFixNode* mouseFixer = new CgPointFixNode(g_system, g_clothMesh->vbuff());
	UI = new UserInteraction(mouseFixer, g_clothMesh->vbuff(), SystemParam::n);

	g_cgRootNode->addChild(cornerFixer);
	g_cgRootNode->addChild(mouseFixer);
}

static void initScene() {
	g_ModelViewMatrix = glm::lookAt(
		glm::vec3(0.618, -0.786, 0.0f) * g_camera_distance,
		glm::vec3(0.0f, 0.0f, -1.0f),
		glm::vec3(0.0f, 0.0f, 1.0f)
	) * glm::translate(glm::mat4(1), glm::vec3(0.0f, 0.0f, SystemParam::w / 4));
	updateProjection();
}

// G L U T  C A L L B A C K S //////////////////////////////////////////////////////
static void display() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	drawCloth(false);
	glutSwapBuffers();

	checkGlErrors();
}

static void reshape(int w, int h) {
	g_windowWidth = w;
	g_windowHeight = h;
	glViewport(0, 0, w, h);
	updateProjection();
	glutPostRedisplay();
}

static void mouse(const int button, const int state, const int x, const int y) {
	g_mouseClickX = x;
	g_mouseClickY = g_windowHeight - y - 1;

	g_mouseLClickButton |= (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN);
	g_mouseRClickButton |= (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN);
	g_mouseMClickButton |= (button == GLUT_MIDDLE_BUTTON && state == GLUT_DOWN);

	g_mouseLClickButton &= !(button == GLUT_LEFT_BUTTON && state == GLUT_UP);
	g_mouseRClickButton &= !(button == GLUT_RIGHT_BUTTON && state == GLUT_UP);
	g_mouseMClickButton &= !(button == GLUT_MIDDLE_BUTTON && state == GLUT_UP);

	g_mouseClickDown = g_mouseLClickButton || g_mouseRClickButton || g_mouseMClickButton;

	// TODO: move to UserInteraction class: add renderer member variable
	// pick point
	if (g_mouseLClickButton) {
		glClearColor(0, 0, 0, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glDisable(GL_FRAMEBUFFER_SRGB);
		drawCloth(true);
		glFlush();
		UI->grabPoint(g_mouseClickX, g_mouseClickY);
		checkGlErrors();

		glClearColor(0.25f, 0.25f, 0.25f, 0);
		glEnable(GL_FRAMEBUFFER_SRGB);
	}
	else UI->releasePoint();
}

static void motion(const int x, const int y) {
	const float dx = float(x - g_mouseClickX);
	const float dy = float (-(g_windowHeight - y - 1 - g_mouseClickY));

	if (g_mouseLClickButton) {
		//glm::vec3 ux(g_ModelViewMatrix * glm::vec4(1, 0, 0, 0));
		//glm::vec3 uy(g_ModelViewMatrix * glm::vec4(0, 1, 0, 0));
		glm::vec3 ux(0, 1, 0);
		glm::vec3 uy(0, 0, -1);
		UI->movePoint(0.03f * (dx * ux + dy * uy));
	}

	g_mouseClickX = x;
	g_mouseClickY = g_windowHeight - y - 1;
}

// C L O T H ///////////////////////////////////////////////////////////////////////
static void drawCloth(bool picking) {
	if (picking) {
		Renderer renderer;
		renderer.setProgram(g_pickShader);
		renderer.setModelview(g_ModelViewMatrix);
		renderer.setProjection(g_ProjectionMatrix);
		g_pickShader->setTessFact(SystemParam::n);
		renderer.setProgramInput(*g_render_target);
		renderer.draw(g_clothMesh->ibuffLen());
	}
	else {
		Renderer renderer;
		renderer.setProgram(g_phongShader);
		renderer.setModelview(g_ModelViewMatrix);
		
		renderer.setProjection(g_ProjectionMatrix);
		renderer.setProgramInput(*g_render_target);
		renderer.draw(g_clothMesh->ibuffLen());
		checkGlErrors();
	}
}

static void animateCloth(int value) {

	for (int i = 0; i < g_hps; i++) {
		// solve system
		g_solver->solve(g_iter);
		// fix points
		CgSatisfyVisitor visitor;
		visitor.satisfy(*g_cgRootNode);
	}

	// update normals
	g_clothMesh->request_face_normals();
	g_clothMesh->update_normals();
	g_clothMesh->release_face_normals();

	// update target
	updateRenderTarget();

	// redisplay
	glutPostRedisplay();

	// reset timer
	glutTimerFunc(g_animation_timer, animateCloth, 0);
}

// S C E N E  U P D A T E ///////////////////////////////////////////////////////////
static void updateProjection() {
	g_ProjectionMatrix = glm::perspective(PI / 4.0f,
		g_windowWidth * 1.0f / g_windowHeight, 0.01f, 1000.0f);
}

static void updateRenderTarget() {
	// update vertex positions
	g_render_target->setPositionData(g_clothMesh->vbuff(), g_clothMesh->vbuffLen());

	// update vertex normals
	g_render_target->setNormalData(g_clothMesh->nbuff(), g_clothMesh->vbuffLen());

}

// C L E A N  U P //////////////////////////////////////////////////////////////////
static void cleanUp() {
	delete g_clothMesh;
	// delete mass-spring system
	delete g_system;
	delete g_solver;
}

// E R R O R S /////////////////////////////////////////////////////////////////////
void checkGlErrors() {
	const GLenum errCode = glGetError();

	if (errCode != GL_NO_ERROR) {
		std::string error("GL Error: ");
		error += reinterpret_cast<const char*>(gluErrorString(errCode));
		std::cerr << error << std::endl;
		throw std::runtime_error(error);
	}
}