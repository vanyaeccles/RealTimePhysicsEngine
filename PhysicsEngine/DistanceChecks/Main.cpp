// Adapted from the online tuturials by Joey de Vries found at 'http://learnopengl.com/'
// Also from Anton's OpenGL 4 Tutorials https://capnramses.itch.io/antons-opengl-4-tutorials


/*
     *
         *
	              *
Distance and Contact

	*                     *
	        *
*/


#pragma region "includes"

#include <iostream> // Std. Includes
#include <string>
#include <algorithm>
#include <vector>
using namespace std;
#define GLEW_STATIC // GLEW
#include <GL/glew.h> 
#include <GLFW/glfw3.h> // GLFW
#include <glm/glm.hpp> // GLM Mathematics
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/orthonormalize.hpp>
#include <SOIL/SOIL.h> // SOIL


#include "Shader.h" 
#include "Camera.h"
#include "Model.h"
#include "ParticleSystem.h"
#include "Solver.h"
#include "RigidBody.h"
#include "DistanceChecks.h"

#define PI 3.141592653589793238462643383279502

#pragma endregion




#pragma region "Function Prototypes and Variables"

//Function prototypes
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
//new mouse stuff
void cursorPositionCallback(GLFWwindow *window, double xPos, double yPos);
void cursorEnterCallback(GLFWwindow *window, int entered );
void mouseButtonCallback(GLFWwindow * window, int button, int action, int mods);


void Do_Movement();

//Texture stuff
GLuint loadCubemap(vector<const GLchar*> faces);
GLuint loadTexture(GLchar* path);

//Drawing








// Camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
bool keys[1024];
GLfloat lastX = 400, lastY = 300;
bool firstMouse = true;

bool testBasics = false;
bool reverseGrav = false;
bool leftFan = false;
bool rightFan = false;
bool movePos = false;
bool playForces = false;
bool rotateRT = true;


GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;
GLuint screenWidth = 1600, screenHeight = 1080; // Properties


float timestep = 0.01f;
float epsilon = 0.000001f;
float ninety = PI / 2;

double trianglePosX = 0.0f, trianglePosX1 = 0.0f, trianglePosZ1 = 0.0f, trianglePosZ2 = 0.0f, trianglePosZ3 = 0.0f;

double xPos = 0, yPos = 0, zPos = 0;

int i, j;
int clock;










#pragma endregion




// Start our application and run our Game loop
int main()
{

#pragma region "Initialisation"

	// Init GLFW
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	

	GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "Real-Time Physics", nullptr, nullptr); // Windowed
	glfwMakeContextCurrent(window);

	// Set the required callback functions
	glfwSetKeyCallback(window, key_callback);
	//glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
	// new mouse callback stuff
	glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1); //Returns 1 if clicked, or 0
	glfwSetCursorPosCallback(window, cursorPositionCallback);
	
	// Input Options
	//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
	glfwSetCursorEnterCallback(window, cursorEnterCallback );

	glfwSetMouseButtonCallback(window, mouseButtonCallback);
	glfwSetInputMode(window, GLFW_STICKY_MOUSE_BUTTONS, 1);



	unsigned char pixels[5 * 5 * 4];
	memset(pixels, 0xff, sizeof(pixels));
	GLFWimage image;
	image.width = 5;
	image.height = 5;
	image.pixels = pixels;
	GLFWcursor *cursor = glfwCreateCursor(&image, 0, 0);
	glfwSetCursor(window, cursor);


	
	
	GLfloat foundPos1 = 0.0f;
	GLfloat foundPos2 = 0.0f;
	GLfloat foundPos3 = 0.0f;

	
	glm::vec3 pFound = glm::vec3(0.0f, 0.0f, 0.0f), pFound2 = glm::vec3(0.0f, 0.0f, 0.0f);


	// Initialize GLEW to setup the OpenGL Function pointers
	glewExperimental = GL_TRUE;
	glewInit();

	// Define the viewport dimensions, as above
	glViewport(0, 0, screenWidth, screenHeight);

#pragma endregion

	// Setup some OpenGL options
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_LINE_SMOOTH);
	GLfloat lineWidth = 2.0f;
	glLineWidth(lineWidth);

#pragma region "Shaders"



	// Setup and compile our shaders
	Shader skyboxShader("U:/Physics/VanyaOpenGL/Stuff/Shaders/skybox/skybox.vert", "U:/Physics/VanyaOpenGL/Stuff/Shaders/skybox/skybox.frag");
	
	Shader reflectShader("U:/Physics/VanyaOpenGL/Stuff/Shaders/reflect/reflect.vert", "U:/Physics/VanyaOpenGL/Stuff/Shaders/reflect/reflect.frag");

	Shader whiteShader("U:/Physics/Phys2/Stuff/Shaders/plaincolour/white.vert", "U:/Physics/VanyaOpenGL/Phys2/Shaders/plaincolour/white.frag");

	Shader drawShader("U:/Physics/VanyaOpenGL/Stuff/Shaders/plaincolour/blue.vert", "U:/Physics/VanyaOpenGL/Stuff/Shaders/plaincolour/blue.frag");

	Shader highlightShader("U:/Physics/Phys3/Stuff/Shaders/plaincolour/blue.vert", "U:/Physics/Phys3/Stuff/Shaders/plaincolour/green.frag");
	
#pragma endregion



#pragma region "Models and Textures"

	// Load models
	Model cube("U:/Physics/Phys2/Stuff/Models/cube.obj");
	Model modelCopyMeshes("U:/Physics/Phys2/Stuff/Models/cube.obj");

	Model sphere("U:/Physics/Phys2/Stuff/Models/sphere/sphere.obj");

	Mesh cube1 = cube.returnVertices();

	// Load textures
	GLuint painting = loadTexture("U:/Rendering/RTRAssignment_2/Textures/desk.jpg");

#pragma endregion	



#pragma region "object_initialization, skybox"
	// Set the object data (buffers, vertex attributes)
	GLfloat cubeVertices[] = {
		// Positions          // Normals
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
		0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
		0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
		0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,

		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

		0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
		0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
		0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
		0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
		0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
		0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
		0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
		0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
		0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
		0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
		0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
		0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
	};
	GLfloat skyboxVertices[] = {
		// Positions          
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		-1.0f,  1.0f, -1.0f,
		1.0f,  1.0f, -1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		1.0f, -1.0f,  1.0f
	};
	

	// Setup cube VAO
	GLuint cubeVAO, cubeVBO;
	glGenVertexArrays(1, &cubeVAO);
	glGenBuffers(1, &cubeVBO);
	glBindVertexArray(cubeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), &cubeVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glBindVertexArray(0);
	// Setup skybox VAO
	GLuint skyboxVAO, skyboxVBO;
	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glBindVertexArray(0);
	


	


	// Cubemap (Skybox)
	//loads the cubemap 'faces' into a vector and loads them into a cubemap format in skyboxTexture
	vector<const GLchar*> faces;
	faces.push_back("U:/Physics/VanyaOpenGL/Stuff/skybox/clouds/stormydays_ft.png");
	faces.push_back("U:/Physics/VanyaOpenGL/Stuff/skybox/clouds/stormydays_bk.png");
	faces.push_back("U:/Physics/VanyaOpenGL/Stuff/skybox/clouds/stormydays_up.png"); 
	faces.push_back("U:/Physics/VanyaOpenGL/Stuff/skybox/clouds/stormydays_dn.png"); 
	faces.push_back("U:/Physics/VanyaOpenGL/Stuff/skybox/clouds/stormydays_rt.png");
	faces.push_back("U:/Physics/VanyaOpenGL/Stuff/skybox/clouds/stormydays_lf.png");
	GLuint skyboxTexture = loadCubemap(faces);
	
#pragma endregion


	DistanceChecker dChecker;


	




	// Game loop
	while (!glfwWindowShouldClose(window))
	{

		

		// Set frame time
		GLfloat currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		// Check and call events
		glfwPollEvents();
		Do_Movement();
	

		// Clear buffers
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glfwGetCursorPos(window, &xPos, &yPos);

		clock++;
		






		//Change cursor position to window coordinates
		xPos = 3 *  xPos / screenWidth - 1;
		yPos = -3 * yPos / screenHeight + 1;
		//std::cout << xPos << " : " << yPos << std::endl;


		//Setup triangle
		GLfloat tX1 = -0.5f + trianglePosX1, tY1 = -0.5f, tZ1 = 0.0f +trianglePosZ1;
		GLfloat tX2 = 0.5f + trianglePosX, tY2 = -0.5f, tZ2 = 0.0f +trianglePosZ2;
		GLfloat tX3 = 0.0f + trianglePosX, tY3 = 0.5f, tZ3 = 0.0f +trianglePosZ3;

		GLfloat triangleVertices[] = {
			tX1, tY1, tZ1, // Left  
			tX2, tY2, tZ2, // Right 
			tX3, tY3, tZ3  // Top   
		};

		//Triangle vertex points
		glm::vec3 tri1 = glm::vec3 (tX1, tY1, tZ1);
		glm::vec3 tri2 = glm::vec3(tX2, tY2, tZ2);
		glm::vec3 tri3 = glm::vec3(tX3, tY3, tZ3);

		glm::vec3 mousePos = glm::vec3(xPos, yPos, zPos);


		//Setup Triangle VAO
		GLuint tVBO, tVAO;
		glGenVertexArrays(1, &tVAO);
		glGenBuffers(1, &tVBO);
		glBindVertexArray(tVAO);
		glBindBuffer(GL_ARRAY_BUFFER, tVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(triangleVertices), triangleVertices, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

		

		//Mouse Line setup
		GLfloat lineVertices[] =
		{
			xPos, yPos, zPos,
			foundPos1, foundPos2, foundPos3
		};
		
		GLuint lVBO, lVAO;
		glGenVertexArrays(1, &lVAO);
		glGenBuffers(1, &lVBO);
		glBindVertexArray(lVAO);
		glBindBuffer(GL_ARRAY_BUFFER, lVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(lineVertices), lineVertices, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);


		// Found edge line setup
		GLfloat closeLineVertices[] =
		{
			pFound.x, pFound.y, pFound.z,
			pFound2.x, pFound2.y, pFound2.z
		};
		
		GLuint clVBO, clVAO;
		glGenVertexArrays(1, &clVAO);
		glGenBuffers(1, &clVBO);
		glBindVertexArray(clVAO);
		glBindBuffer(GL_ARRAY_BUFFER, clVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(closeLineVertices), closeLineVertices, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
		

		//glBufferSubData



		// Draw scene, create transformations
		glm::mat4 model;
		glm::mat4 view = camera.GetViewMatrix();
		glm::mat4 projection = glm::perspective(camera.Zoom, (float)screenWidth / (float)screenHeight, 0.1f, 100.0f);
		
		




		
		

		//Draw
		// wireframe mode
		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		drawShader.Use();
		glUniform3f(glGetUniformLocation(drawShader.Program, "cameraPos"), camera.Position.x, camera.Position.y, camera.Position.z);
		glUniformMatrix4fv(glGetUniformLocation(drawShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(drawShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
		model = glm::mat4();
		glUniformMatrix4fv(glGetUniformLocation(drawShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
		//cube.Draw(drawShader);
		glBindVertexArray(tVAO);
		glDrawArrays(GL_TRIANGLES, 0, 3);
		glBindVertexArray(0);

		glBindVertexArray(lVAO);
		glDrawArrays(GL_LINES, 0, 2);
		glBindVertexArray(0);

		model = glm::mat4();
		model = glm::translate(model, mousePos);
		glUniformMatrix4fv(glGetUniformLocation(drawShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
		sphere.Draw(drawShader);

		

		

		

		//whiteShader.Use();
		////draw a 'line' 
		//glUniform3f(glGetUniformLocation(whiteShader.Program, "cameraPos"), camera.Position.x, camera.Position.y, camera.Position.z);
		//glUniformMatrix4fv(glGetUniformLocation(whiteShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
		//glUniformMatrix4fv(glGetUniformLocation(whiteShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
		//model = glm::mat4();
		//model = glm::translate(model, glm::vec3(xPos, yPos, 0.0f));
		//glUniformMatrix4fv(glGetUniformLocation(whiteShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
		////sphere.Draw(whiteShader);
		
		
		

		

		int region = dChecker.voronoiSingleTriangle(mousePos, tri1, tri2, tri3);

		foundPos1 = dChecker.closestPoint.x;
		foundPos2 = dChecker.closestPoint.y;
		foundPos3 = dChecker.closestPoint.z;

		

		//Draw based on found region
		highlightShader.Use();
		glUniform3f(glGetUniformLocation(highlightShader.Program, "cameraPos"), camera.Position.x, camera.Position.y, camera.Position.z);
		glUniformMatrix4fv(glGetUniformLocation(highlightShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(highlightShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
		

		model = glm::mat4();
		

		switch (region)
		{
		case 1: // v1
			std::cout << "v1" << std::endl;
			model = glm::translate(model, tri1);
			glUniformMatrix4fv(glGetUniformLocation(highlightShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
			sphere.Draw(highlightShader);

			break;
		case 2: // v2
			std::cout << "v2" << std::endl;
			model = glm::translate(model, tri2);
			glUniformMatrix4fv(glGetUniformLocation(highlightShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
			sphere.Draw(highlightShader);

			break;
		case 3: //v3
			std::cout << "v3" << std::endl;
			model = glm::translate(model, tri3);
			glUniformMatrix4fv(glGetUniformLocation(highlightShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
			sphere.Draw(highlightShader);

			break;
		case 4: // v12
			std::cout << "v12" << std::endl;
			pFound.x = tX1; // tri1.x;
			pFound.y = tY1; // tri1.y;
			pFound.z = tZ1; // tri1.z;
			pFound2.x = tX2; // tri2.x;
			pFound2.y = tY2; // tri2.y;
			pFound2.z = tZ2; // tri2.z;
			
			glUniformMatrix4fv(glGetUniformLocation(highlightShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
			glBindVertexArray(clVAO);
			glDrawArrays(GL_LINES, 0, 2);
			glBindVertexArray(0);
			break;
		case 5: //v23
			std::cout << "v23" << std::endl;
			pFound.x = tri3.x;
			pFound.y = tri3.y;
			pFound.z = tri3.z;
			pFound2.x = tri2.x;
			pFound2.y = tri2.y;
			pFound2.z = tri2.z;
			//model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.1f));
			glUniformMatrix4fv(glGetUniformLocation(highlightShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
			glBindVertexArray(clVAO);
			glDrawArrays(GL_LINES, 0, 2);
			glBindVertexArray(0);

			break;
		case 6: //v31
			std::cout << "v31" << std::endl;
			pFound.x = tri3.x;
			pFound.y = tri3.y;
			pFound.z = tri3.z;
			pFound2.x = tri1.x;
			pFound2.y = tri1.y;
			pFound2.z = tri1.z;
			//model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.1f));
			glUniformMatrix4fv(glGetUniformLocation(highlightShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
			glBindVertexArray(clVAO);
			glDrawArrays(GL_LINES, 0, 2);
			glBindVertexArray(0);

			break;
		case 7: //face
			std::cout << "face" << std::endl;
			model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.001f));
			glUniformMatrix4fv(glGetUniformLocation(drawShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
			glBindVertexArray(tVAO);
			glDrawArrays(GL_TRIANGLES, 0, 3);
			glBindVertexArray(0);
			model = glm::mat4();
			model = glm::translate(model, glm::vec3(0.0f, 0.0f, -0.001f));
			glUniformMatrix4fv(glGetUniformLocation(drawShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
			glBindVertexArray(tVAO);
			glDrawArrays(GL_TRIANGLES, 0, 3);
			glBindVertexArray(0);


			break;
		default:
			std::cout << "Nothing Found!" << std::endl;
		}

		std::cout << "Mouse Pos:  X: " << xPos << "Y: " << yPos << "Z: " << zPos << std::endl;
		std::cout << "Found Position:  X: " << dChecker.closestPoint.x << " Y: " << dChecker.closestPoint.y << " Z: " << dChecker.closestPoint.z << std::endl;
		std::cout << "Distance: " << dChecker.distance << std::endl;



		//Removes wireframe for skybox
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        // DRAW SKYBOX
		glDepthFunc(GL_LEQUAL);  // Change depth function so depth test passes when values are equal to depth buffer's content
		skyboxShader.Use(); //skybox shader
		view = glm::mat4(glm::mat3(camera.GetViewMatrix()));	// Remove any translation component of the view matrix, movement doesn't affect skybox position vectors
		glUniformMatrix4fv(glGetUniformLocation(skyboxShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(skyboxShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
		// skybox cube
		glBindVertexArray(skyboxVAO);
		glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);
		glDepthFunc(GL_LESS); // Set depth function back to default
		
		

		// Swap the buffers
		glfwSwapBuffers(window);
	}

	glfwTerminate();
	return 0;
}





// Loads a cubemap texture from 6 individual texture faces loaded in the 'faces' vector
GLuint loadCubemap(vector<const GLchar*> faces)
{
	GLuint textureID;
	glGenTextures(1, &textureID);
	glActiveTexture(GL_TEXTURE0);

	int width, height;
	unsigned char* image;

	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
	for (GLuint i = 0; i < faces.size(); i++)
	{
		image = SOIL_load_image(faces[i], &width, &height, 0, SOIL_LOAD_RGB);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	return textureID;
}






// General texture loading 
#pragma region "Texture Loading"
// This function loads a texture from file. 
GLuint loadTexture(GLchar* path)
{
	//Generate texture ID and load texture data 
	GLuint textureID;
	glGenTextures(1, &textureID);
	int width, height;
	unsigned char* image = SOIL_load_image(path, &width, &height, 0, SOIL_LOAD_RGB);
	// Assign texture to ID
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	glGenerateMipmap(GL_TEXTURE_2D);

	// Parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);
	SOIL_free_image_data(image);
	return textureID;
}

#pragma endregion


// User input for camera-related transformations
#pragma region "User input"

// Moves/alters the camera positions based on user input
void Do_Movement()
{
	// Camera controls
	if (keys[GLFW_KEY_W])
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (keys[GLFW_KEY_S])
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (keys[GLFW_KEY_A])
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (keys[GLFW_KEY_D])
		camera.ProcessKeyboard(RIGHT, deltaTime);
}


// Is called whenever a key is pressed/released via GLFW
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);


	if (key == GLFW_KEY_T && action == GLFW_PRESS)
		trianglePosZ1 += 1.0f;
	if (key == GLFW_KEY_G && action == GLFW_PRESS)
		trianglePosZ1 -= 1.0f;
	if (key == GLFW_KEY_Y && action == GLFW_PRESS)
		trianglePosZ2 += 1.0f;
	if (key == GLFW_KEY_H && action == GLFW_PRESS)
		trianglePosZ2 -= 1.0f;
	if (key == GLFW_KEY_U && action == GLFW_PRESS)
		trianglePosZ3 += 1.0f;
	if (key == GLFW_KEY_J && action == GLFW_PRESS)
		trianglePosZ3 -= 1.0f;

	if (key == GLFW_KEY_I && action == GLFW_PRESS)
		trianglePosX += 1.0f;
	if (key == GLFW_KEY_K && action == GLFW_PRESS)
		trianglePosX -= 1.0f;
	if (key == GLFW_KEY_O && action == GLFW_PRESS)
		trianglePosX1 += 1.0f;
	if (key == GLFW_KEY_L && action == GLFW_PRESS)
		trianglePosX1 -= 1.0f;

	//Reset
	if (key == GLFW_KEY_R && action == GLFW_PRESS)
	{
		//playForces = !playForces;
		trianglePosX = 0.0f;
		trianglePosX1 = 0.0f;
		trianglePosZ1 = 0.0f;
		trianglePosZ2 = 0.0f;
		trianglePosZ3 = 0.0f;
	}
		


	if (action == GLFW_PRESS)
		keys[key] = true;
	else if (action == GLFW_RELEASE)
		keys[key] = false;

	
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	GLfloat xoffset = xpos - lastX;
	GLfloat yoffset = lastY - ypos;

	lastX = xpos;
	lastY = ypos;

	//camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}


void cursorPositionCallback(GLFWwindow *window, double xPos, double yPos)
{
	//std::cout << xPos << " : " << yPos << std::endl;


}

void cursorEnterCallback(GLFWwindow *window, int entered)
{
	if (entered)
	{

	}
}

void mouseButtonCallback(GLFWwindow * window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
	{
		//std::cout << "Right button pressed" << std::endl;
		zPos = -5.0f;
	}

	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE)
	{
		//std::cout << "Right button released" << std::endl;
		zPos = 0.0f;
	}
}



#pragma endregion






