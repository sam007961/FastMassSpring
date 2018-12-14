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

// BIG TODO (for later): refactor code to avoid using global state, and more OOP
// G L O B A L S ///////////////////////////////////////////////////////////////////
// window size
static int g_window_width = 480, g_window_height = 480;

// constants
static const float PI = glm::pi<float>();

// TODO: refactor to remove some shader globals
// shader files
static const char* const g_basic_vshader = "./shaders/basic.vshader";
static const char* const g_phong_fshader = "./shaders/phong.fshader";
static const char* const g_pick_fshader = "./shaders/pick.fshader";

// shader handles
static GLuint g_vshaderBasic, g_fshaderPhong, g_fshaderPick; // unlinked shaders
static PhongShader g_phongShader; // linked phong shader
static PickShader g_pickShader; // link pick shader

// mesh
static Mesh g_clothMesh; // halfedge data structure
static mesh_data g_meshData; // pointers to data buffers

// Render Target
static render_target g_renderTarget; // vertex, normal, texutre, index

// mesh parameters
namespace MeshParam {
	static const int n = 20; // n * n = m, where m = n_vertices
}

// scene matrices
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
//static void mouse(int, int, int, int);
//static void motion(int, int);
//static void keyboard(unsigned char, int, int);

// draw cloth function
static void drawCloth(bool picking);

// scene update
static void updateProjection();

// cleaning
static void deleteShaders();
//static void deleteBuffers();

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

		glutMainLoop();

		deleteShaders();
		return 0;
	}
	catch (const std::runtime_error& e) {
		std::cout << "Exception caught: " << e.what() << std::endl;
		return -1;
	}
}



static void initGlutState(int argc, char** argv) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(g_window_width, g_window_height);
	glutCreateWindow("Cloth App");

	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	/*glutMotionFunc(motion);
	glutMouseFunc(mouse);
	glutKeyboardFunc(keyboard);*/
}

static void initGLState() {
	glClearColor(0.25f, 0.25f, 0.25f, 0);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
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
	const int n = MeshParam::n;
	MeshBuilder::buildGridNxN(g_clothMesh, n);

	// build index buffer
	g_meshData.ibuffLen = 6 * (n - 1) * (n - 1);
	g_meshData.ibuff = new unsigned int[g_meshData.ibuffLen];
	
	GridFillerIBuffNxN filler(g_meshData.ibuff, n);
	filler.fill();

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
}

static void initScene() {
	g_ModelViewMatrix = glm::lookAt(
		glm::vec3(1.0f, -1.0f, 2.0f),
		glm::vec3(0.0f, 0.0f, 1.0f),
		glm::vec3(0.0f, 0.0f, 1.0f)
	) * glm::translate(glm::mat4(1), glm::vec3(0.0f, 0.0f, 1.0f));
	updateProjection();
}

static void display() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	drawCloth(true);
	glutSwapBuffers();
}

static void reshape(int w, int h) {
	g_window_width = w;
	g_window_height = h;
	glViewport(0, 0, w, h);
	updateProjection();
	glutPostRedisplay();
}

static void drawCloth(bool picking) {
	
	if (picking) {
		PickShadingRenderer picker(&g_pickShader);
		picker.setModelview(g_ModelViewMatrix);
		picker.setProjection(g_ProjectionMatrix);
		picker.setTessFact(MeshParam::n);
		picker.setRenderTarget(g_renderTarget);
		picker.draw(g_meshData.ibuffLen);
	}
	else {
		PhongShadingRenderer phonger(&g_phongShader);
		phonger.setModelview(g_ModelViewMatrix);
		phonger.setProjection(g_ProjectionMatrix);
		phonger.setRenderTarget(g_renderTarget);
		phonger.draw(g_meshData.ibuffLen);
	}

}

static void updateProjection() {
	g_ProjectionMatrix = glm::perspective(PI / 4.0f,
		g_window_width * 1.0f / g_window_height, 0.01f, 1000.0f);
}

static void deleteShaders() {
	glDeleteShader(g_vshaderBasic);
	glDeleteShader(g_fshaderPhong);
	glDeleteShader(g_fshaderPick);

	glDeleteProgram(g_phongShader);
	glDeleteProgram(g_pickShader);

	checkGlErrors();
}

void checkGlErrors() {
	const GLenum errCode = glGetError();

	if (errCode != GL_NO_ERROR) {
		std::string error("GL Error: ");
		error += reinterpret_cast<const char*>(gluErrorString(errCode));
		std::cerr << error << std::endl;
		throw std::runtime_error(error);
	}
}