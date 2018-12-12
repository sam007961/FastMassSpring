#include <GL/glew.h>
#include <GL/glut.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stdexcept>
#include <iostream>
#include <string>

#include "Shader.h"

// globals
// window size
static int g_window_width = 480, g_window_height = 480;

// shader files
static const char * const g_basic_vshader = "./shaders/basic.vshader";
static const char * const g_phong_fshader = "./shaders/phong.fshader";
static const char * const g_pick_fshader = "./shaders/pick.fshader";

// shader handles
static GLuint g_vshaderBasic, g_fshaderPhong, g_fshaderPick;
static PhongShader g_phongShader;
static PickShader g_pickShader;

// temporary: geometry
static int vbolen = 3;
static int ibolen = 6;
static float vbuff[] = {
	-1.0f,  1.0f, 0.0f,
	-1.0f, -1.0f, 0.0f,
	 1.0f,  1.0f, 0.0f,
	 1.0f, -1.0f, 0.0f,
};

static unsigned int ibuff[] = {
	0, 1, 2,
	1, 3, 2,
};

static GLuint vbo, ibo;

// state initialization
static void initGlutState(int, char**);
static void initGLState();
static void initShaders();
static void initCloth();

// glut callbacks
static void display();
static void reshape(int, int);
//static void mouse(int, int, int, int);
//static void motion(int, int);
//static void keyboard(unsigned char, int, int);

// cleanup
static void deleteShaders();
//static void deleteBuffers();

// error checks
void checkGlErrors();

// main
int main(int argc, char** argv) {
	try {
		initGlutState(argc, argv);
		glewInit();

		initGLState();
		initShaders();
		initCloth();
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
	g_phongShader.h_uModelViewMatrix = glGetUniformLocation(g_phongShader, "uModelViewMatrix");
	g_phongShader.h_uProjectionMatrix = glGetUniformLocation(g_phongShader, "uProjectionMatrix");
	
	checkGlErrors();
}

static void initCloth() {
	// temporary: initialize vbo and ibo
	glGenBuffers(1, &vbo);
	glGenBuffers(1, &ibo);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vbuff), vbuff, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, ibo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(ibuff), ibuff, GL_STATIC_DRAW);

	checkGlErrors();
}

static void display() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// TODO: make matrices global constants
	static const float pi = glm::pi<float>();
	glm::mat4 ModelViewMatrix = glm::lookAt(
		glm::vec3(2.0f, -2.0f, 2.0f),
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 0.0f, 1.0f)
	) * glm::translate(glm::mat4(1), glm::vec3(0.0f, 0.0f, 1.0f));
	glm::mat4 ProjectionMatrix = glm::perspective(pi / 3.0f, 
		g_window_width * 1.0f / g_window_height, 0.01f, 1000.0f);

	// TODO: move to separate draw function
	glUseProgram(g_phongShader);

	glUniformMatrix4fv(g_phongShader.h_uModelViewMatrix,
		1, GL_FALSE, glm::value_ptr(ModelViewMatrix[0]));
	glUniformMatrix4fv(g_phongShader.h_uProjectionMatrix,
		1, GL_FALSE, glm::value_ptr(ProjectionMatrix[0]));

	glEnableVertexAttribArray(g_phongShader.h_aPosition);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glVertexAttribPointer(g_phongShader.h_aPosition,
		3, 
		GL_FLOAT,
		GL_FALSE,
		0, 
		0
	);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glDrawElements(GL_TRIANGLES, ibolen, GL_UNSIGNED_INT, 0);
	glDisableVertexAttribArray(g_phongShader.h_aPosition);
	glutSwapBuffers();
}

static void reshape(int w, int h) {
	g_window_width = w;
	g_window_height = h;
	glViewport(0, 0, w, h);
	glutPostRedisplay();
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