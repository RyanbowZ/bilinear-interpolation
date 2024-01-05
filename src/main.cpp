#include <iostream>
#include <vector>

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "GLSL.h"
#include "Program.h"
#include "MatrixStack.h"
#include "ShapeCage.h"
#include "Texture.h"
#include "Grid.h"

using namespace std;

GLFWwindow *window; // Main application window
string RESOURCE_DIR = ""; // Where the resources are loaded from
shared_ptr<Program> progSimple;
shared_ptr<Program> progTex;
shared_ptr<Grid> grid;
shared_ptr<ShapeCage> shape;
shared_ptr<Texture> texture;

bool keyToggles[256] = {false}; // only for English keyboards!

glm::vec2 window2world(double xmouse, double ymouse)
{
	// Convert from window coords to world coords
	// (Assumes orthographic projection)
	int width, height;
	glfwGetWindowSize(window, &width, &height);
	glm::vec4 p;
	// Inverse of viewing transform
	p.x = xmouse / (float)width;
	p.y = (height - ymouse) / (float)height;
	p.x = 2.0f * (p.x - 0.5f);
	p.y = 2.0f * (p.y - 0.5f);
	p.z = 0.0f;
	p.w = 1.0f;
	// Inverse of model-view-projection transform
	auto P = make_shared<MatrixStack>();
	auto MV = make_shared<MatrixStack>();
	double aspect = (double)width/height;
	double s = 1.1;
	P->multMatrix(glm::ortho(-s*aspect, s*aspect, -s, s));
	p = glm::inverse(P->topMatrix() * MV->topMatrix()) * p;
	return glm::vec2(p);
}

static void error_callback(int error, const char *description)
{
	cerr << description << endl;
}

static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
	if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, GL_TRUE);
	}
}

static void char_callback(GLFWwindow *window, unsigned int key)
{
	keyToggles[key] = !keyToggles[key];

	switch(key) {
		case 'r':
			grid->reset();
			break;
		case 's':
			grid->save("cps.txt");
			break;
		case 'l':
			grid->load("cps.txt");
			break;
	}
}

static void cursor_position_callback(GLFWwindow* window, double xmouse, double ymouse)
{
	int state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
	if(state == GLFW_PRESS) {
		grid->moveCP(window2world(xmouse, ymouse));
	} else {
		grid->findClosest(window2world(xmouse, ymouse));
	}
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	// Not used for this lab
}

static void init()
{
	GLSL::checkVersion();
	
	// Set background color
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	// Enable z-buffer test
	glEnable(GL_DEPTH_TEST);
	
	// Initialize the GLSL program.
	progSimple = make_shared<Program>();
	progSimple->setVerbose(true); // Set this to true when debugging.
	progSimple->setShaderNames(RESOURCE_DIR + "simple_vert.glsl", RESOURCE_DIR + "simple_frag.glsl");
	progSimple->init();
	progSimple->addUniform("P");
	progSimple->addUniform("MV");
	progSimple->setVerbose(false);

	progTex = make_shared<Program>();
	progTex->setVerbose(true); // Set this to true when debugging.
	progTex->setShaderNames(RESOURCE_DIR + "tex_vert.glsl", RESOURCE_DIR + "tex_frag.glsl");
	progTex->init();
	progTex->addAttribute("aPos");
	progTex->addAttribute("aTex");
    progTex->addAttribute("tInd");
	progTex->addUniform("P");
	progTex->addUniform("MV");
	progTex->addUniform("colorTexture");
	progTex->setVerbose(false);

	texture = make_shared<Texture>();
	texture->setFilename(RESOURCE_DIR + "wood_tex.jpg");
	texture->init();
	texture->setUnit(0);

	// Grid of control points
	grid = make_shared<Grid>();
	grid->setSize(5,5);

	// Load scene
	shape = make_shared<ShapeCage>();
	shape->load(RESOURCE_DIR + "man.obj");
	shape->setGrid(grid);
	shape->toLocal();
	shape->init();

	// If there were any OpenGL errors, this will print something.
	// You can intersperse this line in your code to find the exact location
	// of your OpenGL error.
	GLSL::checkError(GET_FILE_LINE);
}

void render()
{
	// Get current frame buffer size.
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	glViewport(0, 0, width, height);
	
	// Clear buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	if(keyToggles[(unsigned)'c']) {
		glEnable(GL_CULL_FACE);
	} else {
		glDisable(GL_CULL_FACE);
	}
	if(keyToggles[(unsigned)'z']) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	} else {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
	
	auto P = make_shared<MatrixStack>();
	auto MV = make_shared<MatrixStack>();
	P->pushMatrix();
	MV->pushMatrix();
	
	double aspect = (double)width/height;
	double s = 1.1;
	P->multMatrix(glm::ortho(-s*aspect, s*aspect, -s, s));
	
	// Draw cage
	progSimple->bind();
	glUniformMatrix4fv(progSimple->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
	glUniformMatrix4fv(progSimple->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
	grid->draw();
	progSimple->unbind();

	// Draw textured shape
	progTex->bind();
	texture->bind(progTex->getUniform("colorTexture"));
	glUniformMatrix4fv(progTex->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
	glUniformMatrix4fv(progTex->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
    glUniform2fv(progTex->getUniform("cps"), 25, glm::value_ptr(grid->getAllCPs()[0]));
	//shape->toWorld();
	shape->draw(progTex->getAttribute("aPos"), progTex->getAttribute("aTex"),progTex->getAttribute("tInd"));
	texture->unbind();
	progTex->unbind();

	// Pop matrix stacks.
	MV->popMatrix();
	P->popMatrix();

	GLSL::checkError(GET_FILE_LINE);
}

int main(int argc, char **argv)
{
	if(argc < 2) {
		cout << "Please specify the resource directory." << endl;
		return 0;
	}
	RESOURCE_DIR = argv[1] + string("/");
	
	// Set error callback.
	glfwSetErrorCallback(error_callback);
	// Initialize the library.
	if(!glfwInit()) {
		return -1;
	}
	// Create a windowed mode window and its OpenGL context.
	window = glfwCreateWindow(640, 480, "YOUR NAME", NULL, NULL);
	if(!window) {
		glfwTerminate();
		return -1;
	}
	// Make the window's context current.
	glfwMakeContextCurrent(window);
	// Initialize GLEW.
	glewExperimental = true;
	if(glewInit() != GLEW_OK) {
		cerr << "Failed to initialize GLEW" << endl;
		return -1;
	}
	glGetError(); // A bug in glewInit() causes an error that we can safely ignore.
	cout << "OpenGL version: " << glGetString(GL_VERSION) << endl;
	cout << "GLSL version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
	// Set vsync.
	glfwSwapInterval(1);
	// Set keyboard callback.
	glfwSetKeyCallback(window, key_callback);
	// Set char callback.
	glfwSetCharCallback(window, char_callback);
	// Set cursor position callback.
	glfwSetCursorPosCallback(window, cursor_position_callback);
	// Set mouse button callback.
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	// Initialize scene.
	init();
	// Loop until the user closes the window.
	while(!glfwWindowShouldClose(window)) {
		if(!glfwGetWindowAttrib(window, GLFW_ICONIFIED)) {
			// Render scene.
			render();
			// Swap front and back buffers.
			glfwSwapBuffers(window);
		}
		// Poll for and process events.
		glfwPollEvents();
	}
	// Quit program.
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
