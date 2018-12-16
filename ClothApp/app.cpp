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

// TODO: refactor to remove some shader globals
// Shader Files
static const char* const g_basic_vshader = "./shaders/basic.vshader";
static const char* const g_phong_fshader = "./shaders/phong.fshader";
static const char* const g_pick_fshader = "./shaders/pick.fshader";

// Shader Handles
static GLuint g_vshaderBasic, g_fshaderPhong, g_fshaderPick; // unlinked shaders
static PhongShader g_phongShader; // linked phong shader
static PickShader g_pickShader; // link pick shader

// Mesh
static Mesh g_clothMesh; // halfedge data structure
static mesh_data g_meshData; // pointers to data buffers

// Render Target
static render_target g_renderTarget; // vertex, normal, texutre, index

// Animation
static const int g_fps = 60; // frames per second  | 60
static const int g_hps = 4; // time steps per frame | 4
static const int g_iter = 7; // iterations per time step | 7
static const int g_frame_time = 15; // approximate time for frame calculations | 15
static const int g_animation_timer = (int) ((1.0f / g_fps) * 1000 - g_frame_time);

// Mass Spring System
static mass_spring_system* g_system;
static MassSpringSolver* g_solver;
static PointFixer * g_fixer;

// System parameters
namespace SystemParam {
	static const int n = 41; // must be odd, n * n = n_vertices | 41
	static const float w = 10.0f; // width | 10.0f
	static const float h = 0.007f; // time step, smaller for better results | 0.007f
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
//static void keyboard(unsigned char, int, int);

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
	/*glutMotionFunc(motion);
	glutMouseFunc(mouse);
	glutKeyboardFunc(keyboard);*/
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
	std::ifstream ifBasic(g_basic_vshader);
	std::ifstream ifPhong(g_phong_fshader);
	std::ifstream ifPick(g_pick_fshader);

	g_vshaderBasic = glCreateShader(GL_VERTEX_SHADER);
	g_fshaderPhong = glCreateShader(GL_FRAGMENT_SHADER);
	g_fshaderPick = glCreateShader(GL_FRAGMENT_SHADER);

	if (!g_vshaderBasic || !g_fshaderPhong || !g_fshaderPick) {
		throw std::runtime_error("glCreateShader fails.");
	}

	compile_shader(g_vshaderBasic, ifBasic);
	compile_shader(g_fshaderPhong, ifPhong);
	compile_shader(g_fshaderPick, ifPick);

	g_phongShader = glCreateProgram();
	g_pickShader = glCreateProgram();

	if (!g_phongShader || !g_pickShader) {
		throw std::runtime_error("glCreateProgram fails.");
	}

	link_shader(g_phongShader, g_vshaderBasic, g_fshaderPhong);
	link_shader(g_pickShader, g_vshaderBasic, g_fshaderPick);

	g_phongShader.h_aPosition = glGetAttribLocation(g_phongShader, "aPosition");
	g_phongShader.h_aNormal = glGetAttribLocation(g_phongShader, "aNormal");
	g_phongShader.h_uModelViewMatrix = glGetUniformLocation(g_phongShader, "uModelViewMatrix");
	g_phongShader.h_uProjectionMatrix = glGetUniformLocation(g_phongShader, "uProjectionMatrix");

	g_pickShader.h_aPosition = glGetAttribLocation(g_pickShader, "aPosition");
	g_pickShader.h_aTexCoord = glGetAttribLocation(g_pickShader, "aTexCoord");
	g_pickShader.h_uTessFact = glGetUniformLocation(g_pickShader, "uTessFact");
	g_pickShader.h_uModelViewMatrix = glGetUniformLocation(g_pickShader, "uModelViewMatrix");
	g_pickShader.h_uProjectionMatrix = glGetUniformLocation(g_pickShader, "uProjectionMatrix");
	
	checkGlErrors();
}

static void initCloth() {
	// generate buffers
	glGenBuffers(1, &g_renderTarget.vbo);
	glGenBuffers(1, &g_renderTarget.nbo);
	glGenBuffers(1, &g_renderTarget.tbo);
	glGenBuffers(1, &g_renderTarget.ibo);


	// generate mesh
	const int n = SystemParam::n;
	const float w = SystemParam::w;
	MeshBuilder::buildGridNxN(g_clothMesh, w, n);

	// build index buffer
	g_meshData.ibuffLen = 6 * (n - 1) * (n - 1);
	g_meshData.ibuff = new unsigned int[g_meshData.ibuffLen];
	
	MeshBuilder::buildGridIBuffNxN(g_meshData.ibuff, n);

	// extract data buffers
	g_meshData.vbuffLen = g_meshData.nbuffLen = n * n * 3;
	g_meshData.tbuffLen = n * n * 2;

	g_meshData.vbuff = VERTEX_DATA(g_clothMesh);
	g_meshData.nbuff = NORMAL_DATA(g_clothMesh);
	g_meshData.tbuff = TEXTURE_DATA(g_clothMesh);

	// fill render target
	glBindBuffer(GL_ARRAY_BUFFER, g_renderTarget.vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * g_meshData.vbuffLen,
		g_meshData.vbuff, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, g_renderTarget.nbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * g_meshData.nbuffLen,
		g_meshData.nbuff, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, g_renderTarget.tbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * g_meshData.tbuffLen,
		g_meshData.tbuff, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, g_renderTarget.ibo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(unsigned int) * g_meshData.ibuffLen, 
		g_meshData.ibuff, GL_STATIC_DRAW);

	checkGlErrors();

	// initialize mass spring system
	g_system = MassSpringBuilder::UniformGrid(
		SystemParam::n,
		SystemParam::h,
		SystemParam::r,
		SystemParam::k,
		SystemParam::m,
		SystemParam::a,
		SystemParam::g
	);

	// initialize mass spring solver
	g_solver = new MassSpringSolver(g_system, g_meshData.vbuff);

	// fix top corners
	g_fixer = new PointFixer(g_meshData.vbuff, g_meshData.vbuffLen);
	g_fixer->addPoint(0);
	g_fixer->addPoint((SystemParam::n - 1) * 3);

	// initialize user interaction
	UI = new UserInteraction(g_meshData.vbuff, SystemParam::n);
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
		PickShadingRenderer picker(&g_pickShader);
		picker.setModelview(g_ModelViewMatrix);
		picker.setProjection(g_ProjectionMatrix);
		picker.setTessFact(SystemParam::n);
		picker.setRenderTarget(g_renderTarget);
		picker.draw(g_meshData.ibuffLen);
	}
	else {
		PhongShadingRenderer phonger(&g_phongShader);
		phonger.setModelview(g_ModelViewMatrix);
		phonger.setProjection(g_ProjectionMatrix);
		phonger.setRenderTarget(g_renderTarget);
		phonger.draw(g_meshData.ibuffLen);
		checkGlErrors();
	}
}

static void animateCloth(int value) {

	for (int i = 0; i < g_hps; i++) {
		// solve system
		g_solver->solve(g_iter);
		// fix points
		UI->fixPoints();
		g_fixer->fixPoints();
	}

	// update normals
	g_clothMesh.request_face_normals();
	g_clothMesh.update_normals();
	g_clothMesh.release_face_normals();

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
	glBindBuffer(GL_ARRAY_BUFFER, g_renderTarget.vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * g_meshData.vbuffLen,
		g_meshData.vbuff, GL_STATIC_DRAW);

	// update vertex normals
	glBindBuffer(GL_ARRAY_BUFFER, g_renderTarget.nbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * g_meshData.nbuffLen,
		g_meshData.nbuff, GL_STATIC_DRAW);
}

// C L E A N  U P //////////////////////////////////////////////////////////////////
static void cleanUp() {
	// delete strings
	delete[] g_basic_vshader;
	delete[] g_phong_fshader;
	delete[] g_pick_fshader;

	// delete unlinked shaders
	glDeleteShader(g_vshaderBasic);
	glDeleteShader(g_fshaderPhong);
	glDeleteShader(g_fshaderPick);

	// delete shader programs
	glDeleteProgram(g_phongShader);
	glDeleteProgram(g_pickShader);

	// delete buffers
	glDeleteBuffers(1, &g_renderTarget.vbo);
	glDeleteBuffers(1, &g_renderTarget.nbo);
	glDeleteBuffers(1, &g_renderTarget.tbo);
	glDeleteBuffers(1, &g_renderTarget.ibo);
	
	// delete mass-spring system
	delete g_system;
	delete g_solver;
	delete g_fixer;

	checkGlErrors();
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