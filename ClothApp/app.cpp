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

// G L O B A L S	
// window size
static int g_window_width = 480, g_window_height = 480;

// shader files
static const char* const g_basic_vshader = "./shaders/basic.vshader";
static const char* const g_phong_fshader = "./shaders/phong.fshader";
static const char* const g_pick_fshader = "./shaders/pick.fshader";

// shader handles
static GLuint g_vshaderBasic, g_fshaderPhong, g_fshaderPick;
static PhongShader g_phongShader;
static PickShader g_pickShader;

// mesh
static Mesh g_clothMesh;
static mesh_data g_meshData;

// buffers
static GLuint g_vbo, g_ibo, g_nbo, g_tbo;

// mesh parameters
namespace MeshParam {
	static const int n = 20; // n * n = m, where m = n_vertices
}

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

//draw cloth function
static void draw_cloth(bool picking);

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
	glGenBuffers(1, &g_vbo);
	glGenBuffers(1, &g_nbo);
	glGenBuffers(1, &g_tbo);
	glGenBuffers(1, &g_ibo);

	// request mesh properties
	g_clothMesh.request_vertex_normals();
	g_clothMesh.request_vertex_texcoords2D();

	// generate mesh
	const int n = MeshParam::n;
	const float d = 1.0f / (n - 1); // step distance
	const OpenMesh::Vec3f o = OpenMesh::Vec3f(-1.0f, 1.0f, 0.0f); // origin
	const OpenMesh::Vec3f ux = OpenMesh::Vec3f(1.0f, 0.0f, 0.0f); // unit x direction
	const OpenMesh::Vec3f uy = OpenMesh::Vec3f(0.0f, -1.0f, 0.0f); // unit y direction
	std::vector<OpenMesh::VertexHandle> handle_table(n*n); // table storing vertex handles for easy grid connectivity establishment

	// index buffer
	g_meshData.ibuffLen = 6 * (n - 1) * (n - 1);
	g_meshData.ibuff = new unsigned int[g_meshData.ibuffLen];
	unsigned int idx = 0;

	for (int j = 0; j < n; j++) {
		for (int i = 0; i < n; i++) {
			handle_table[i + j * n] = g_clothMesh.add_vertex(o + d*i*ux + d*j*uy); // add vertex
			g_clothMesh.set_texcoord2D(handle_table[i + j * n], OpenMesh::Vec2f(d*i, d*j)); // add texture coordinates

			//add connectivity
			if (j > 0 && i < n - 1) {
				g_clothMesh.add_face(
					handle_table[i + j * n], 
					handle_table[i + 1 + (j - 1) * n], 
					handle_table[i + (j - 1) * n]
				);

				g_meshData.ibuff[idx++] = i + j * n;
				g_meshData.ibuff[idx++] = i + 1 + (j - 1) * n;
				g_meshData.ibuff[idx++] = i + (j - 1) * n;
			}

			if (j > 0 && i > 0) {
				g_clothMesh.add_face(
					handle_table[i + j * n],
					handle_table[i + (j - 1) * n],
					handle_table[i - 1 + j * n]
				);

				g_meshData.ibuff[idx++] = i + j * n;
				g_meshData.ibuff[idx++] = i + (j - 1) * n;
				g_meshData.ibuff[idx++] = i - 1 + j * n;
			}
		}
	}

	// calculate and update normals
	g_clothMesh.request_face_normals();
	g_clothMesh.update_normals();
	g_clothMesh.release_face_normals();

	// extract buffers
	g_meshData.vbuffLen = g_meshData.nbuffLen = n * n * 3;
	g_meshData.tbuffLen = n * n * 2;

	g_meshData.vbuff = VERTEX_DATA(g_clothMesh);
	g_meshData.nbuff = NORMAL_DATA(g_clothMesh);
	g_meshData.tbuff = TEXTURE_DATA(g_clothMesh);

	glBindBuffer(GL_ARRAY_BUFFER, g_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * g_meshData.vbuffLen,
		g_meshData.vbuff, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, g_nbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * g_meshData.nbuffLen,
		g_meshData.nbuff, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, g_tbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * g_meshData.tbuffLen,
		g_meshData.tbuff, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, g_ibo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(unsigned int) * g_meshData.ibuffLen, 
		g_meshData.ibuff, GL_STATIC_DRAW);

	checkGlErrors();
}

static void display() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	draw_cloth(false);
	glutSwapBuffers();
}

static void reshape(int w, int h) {
	g_window_width = w;
	g_window_height = h;
	glViewport(0, 0, w, h);
	glutPostRedisplay();
}

// TODO: desperate need for a refactor
static void draw_cloth(bool picking) {
	// TODO: make matrices global constants
	static const float pi = glm::pi<float>();
	glm::mat4 ModelViewMatrix = glm::lookAt(
		glm::vec3(1.0f, -1.0f, 2.0f),
		glm::vec3(0.0f, 0.0f, 1.0f),
		glm::vec3(0.0f, 0.0f, 1.0f)
	) * glm::translate(glm::mat4(1), glm::vec3(0.0f, 0.0f, 1.0f));
	glm::mat4 ProjectionMatrix = glm::perspective(pi / 4.0f,
		g_window_width * 1.0f / g_window_height, 0.01f, 1000.0f);

	if (picking) glUseProgram(g_pickShader);
	else glUseProgram(g_phongShader);

	

	if (picking) {
		glUniformMatrix4fv(g_pickShader.h_uModelViewMatrix,
			1, GL_FALSE, glm::value_ptr(ModelViewMatrix[0]));
		glUniformMatrix4fv(g_pickShader.h_uProjectionMatrix,
			1, GL_FALSE, glm::value_ptr(ProjectionMatrix[0]));

		glUniform1i(g_pickShader.h_uTessFact, MeshParam::n);
		glEnableVertexAttribArray(g_pickShader.h_aPosition);
		glEnableVertexAttribArray(g_pickShader.h_aTexCoord);

		glBindBuffer(GL_ARRAY_BUFFER, g_vbo);
		glVertexAttribPointer(g_pickShader.h_aPosition,
			3, GL_FLOAT, GL_FALSE, 0, 0);

		glBindBuffer(GL_ARRAY_BUFFER, g_tbo);
		glVertexAttribPointer(g_pickShader.h_aTexCoord,
		2, GL_FLOAT, GL_FALSE, 0, 0);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_ibo);
		glDrawElements(GL_TRIANGLES, g_meshData.ibuffLen, GL_UNSIGNED_INT, 0);

		glEnableVertexAttribArray(g_pickShader.h_aPosition);
		glEnableVertexAttribArray(g_pickShader.h_aTexCoord);
		return;
	}

	glUniformMatrix4fv(g_phongShader.h_uModelViewMatrix,
		1, GL_FALSE, glm::value_ptr(ModelViewMatrix[0]));
	glUniformMatrix4fv(g_phongShader.h_uProjectionMatrix,
		1, GL_FALSE, glm::value_ptr(ProjectionMatrix[0]));

	glEnableVertexAttribArray(g_phongShader.h_aPosition);
	glEnableVertexAttribArray(g_phongShader.h_aNormal);

	glBindBuffer(GL_ARRAY_BUFFER, g_vbo);
	glVertexAttribPointer(g_phongShader.h_aPosition,
		3, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, g_nbo);
	glVertexAttribPointer(g_phongShader.h_aNormal,
		3, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_ibo);
	glDrawElements(GL_TRIANGLES, g_meshData.ibuffLen, GL_UNSIGNED_INT, 0);

	glDisableVertexAttribArray(g_phongShader.h_aPosition);
	glDisableVertexAttribArray(g_phongShader.h_aNormal);
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