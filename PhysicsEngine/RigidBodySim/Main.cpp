// Adapted from the online tuturials by Joey de Vries found at 'http://learnopengl.com/'
// Also from Anton's OpenGL 4 Tutorials https://capnramses.itch.io/antons-opengl-4-tutorials


/*
     *
         *
	              *
Rigid Body Motion 

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
#include "Force.h"

#define PI 3.141592653589793238462643383279502

#pragma endregion




#pragma region "Function Prototypes and Variables"

//Function prototypes
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void Do_Movement();

//Texture stuff
GLuint loadCubemap(vector<const GLchar*> faces);
GLuint loadTexture(GLchar* path);

//Drawing
void RenderQuad();



//Physics 
void EulerSolver(ParticleSystem p);
void ParticleDerivativeSolver(ParticleSystem p, float timestep);





// Camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
bool keys[1024];
GLfloat lastX = 400, lastY = 300;
bool firstMouse = true;

//Demo controls
bool testBasics = false;
bool testVelOmega = false;
bool movePos = false;
bool playForces = false;
bool rotateRT = true;
bool playTorque = false;

//Forces
bool gravityBool = false;
bool leftFan = false;
bool rightFan = false;



GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;
GLuint screenWidth = 1600, screenHeight = 900; // Properties


float timestep = 0.01f;
float epsilon = 0.000001f;
float ninety = PI / 2;


int i, j;
int clock;


glm::vec3 posVec(0.0f, 0.0f, 0.0f);
glm::vec3 velVec(0.0f, 0.0f, 0.0f);
glm::vec3 angVec(0.0f, 0.0f, 0.0f);

glm::vec3 forceVec(0.0f, 0.0f, 0.0f);
glm::vec3 torqueVec(0.0f, 0.0f, 0.0f);








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

	GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "Real-Time Rendering Assignment 2", nullptr, nullptr); // Windowed
	glfwMakeContextCurrent(window);

	// Set the required callback functions
	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	// Input Options
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// Initialize GLEW to setup the OpenGL Function pointers
	glewExperimental = GL_TRUE;
	glewInit();

	// Define the viewport dimensions, as above
	glViewport(0, 0, screenWidth, screenHeight);

#pragma endregion

	// Setup some OpenGL options
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);



#pragma region "Shaders"



	// Setup and compile our shaders
	Shader skyboxShader("U:/Physics/VanyaOpenGL/Stuff/Shaders/skybox/skybox.vert", "U:/Physics/VanyaOpenGL/Stuff/Shaders/skybox/skybox.frag");
	
	Shader reflectShader("U:/Physics/VanyaOpenGL/Stuff/Shaders/reflect/reflect.vert", "U:/Physics/VanyaOpenGL/Stuff/Shaders/reflect/reflect.frag");

	Shader whiteShader("U:/Physics/Phys2/Stuff/Shaders/plaincolour/white.vert", "U:/Physics/VanyaOpenGL/Phys2/Shaders/plaincolour/white.frag");

	Shader drawShader("U:/Physics/VanyaOpenGL/Stuff/Shaders/plaincolour/blue.vert", "U:/Physics/VanyaOpenGL/Stuff/Shaders/plaincolour/blue.frag");
	
#pragma endregion



#pragma region "Models and Textures"

	// Load models
	//Model cube("U:/Physics/Phys2/Stuff/Models/cube.obj");
	//Model modelCopyMeshes("U:/Physics/Phys2/Stuff/Models/cube.obj");

	Model sphere("U:/Physics/Phys2/Stuff/Models/sphere/sphere.obj");
	//Mesh cube1 = cube.returnVertices();

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





	glm::mat3 newRotationMatrix;
	newRotationMatrix[0] = glm::vec3(1.0f, 0.0f, 0.0f);
	newRotationMatrix[1] = glm::vec3(0.0f, glm::cos(PI/4), glm::sin(PI / 4));
	newRotationMatrix[2] = glm::vec3(0.0f, -glm::sin(PI / 4), glm::cos(PI / 4));


	// Make a Rigid Body object
	RigidBody rb;
	rb.Update(timestep);



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

		clock++;
		

		// Draw scene, create transformations
		glm::mat4 model;
		glm::mat4 view = camera.GetViewMatrix();
		glm::mat4 projection = glm::perspective(camera.Zoom, (float)screenWidth / (float)screenHeight, 0.1f, 100.0f);




		//Draw inner box
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		drawShader.Use();
		glUniform3f(glGetUniformLocation(drawShader.Program, "cameraPos"), camera.Position.x, camera.Position.y, camera.Position.z);
		glUniformMatrix4fv(glGetUniformLocation(drawShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(drawShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
		model = glm::mat4();
		model = glm::translate(model, rb.position);
		glUniformMatrix4fv(glGetUniformLocation(drawShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
		rb.model.Draw(drawShader);



		//Draw line box
		whiteShader.Use();
		glUniform3f(glGetUniformLocation(whiteShader.Program, "cameraPos"), camera.Position.x, camera.Position.y, camera.Position.z);
		glUniformMatrix4fv(glGetUniformLocation(whiteShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(whiteShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
		//model = glm::mat4();
		//model = glm::translate(model, bodies[i].position);
		model = glm::scale(model, glm::vec3(1.001f, 1.001f, 1.001f));
		glUniformMatrix4fv(glGetUniformLocation(whiteShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		rb.model.Draw(whiteShader);



		

		//Update position and orientation
		rb.Update(timestep);




		if (testBasics) // press T
		{
				if (rotateRT)
				{
					glm::vec3 posVec(0.0f, 1.0f, 0.0f);

					rb.position = posVec;
					rb.orientationMatrix = rb.orientationMatrix * newRotationMatrix;
					rotateRT = false; // only applies the rotation matrix once
				}
		}
		


		if (testVelOmega) // Press Y
		{
			rb.angVelocity = angVec; // Pos/Neg to velocity - Press K/L
			rb.velocity = velVec; // Pos/Neg to angular velocity - Press M/N
		}


		

		if (playTorque)  // press R
		{
			model = glm::mat4();
			model = glm::translate(model, rb.modelVertices[0]);
			glUniformMatrix4fv(glGetUniformLocation(whiteShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
			sphere.Draw(whiteShader);


			glm::vec3 springForce(0.0f, 0.0f, 10.0f);
			glm::vec3 springTorque = glm::cross((rb.modelVertices[0] - rb.position), springForce);


			//rb.applyForce(springForce, timestep);
			rb.applyTorque(springTorque, timestep);

		}


		if (playForces) // Press F
		{
			rb.gravityForce = true;
			rb.dragForce = true;

			glm::vec3 pushForce(0.0f, 9.8f, -1.0f);

			glm::vec3 pushTorque = glm::cross((rb.modelVertices[2] - rb.position), pushForce);

			rb.applyForce(pushForce, timestep);
			rb.applyTorque(pushForce, timestep * 5);

			//playForces = false;
		}
		




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


#pragma region "Makes a quad"

// RenderQuad() Renders a 1x1 quad in NDC
// Used for tangent calculations in normal mapping

GLuint quadVAO = 0;
GLuint quadVBO;
void RenderQuad()
{
	if (quadVAO == 0)
	{
		// normal vector
		glm::vec3 nm(0.0, 0.0, 1.0);


		// positions
		glm::vec3 pos1(-1.0, 1.0, 0.0);
		glm::vec3 pos2(-1.0, -1.0, 0.0);
		glm::vec3 pos3(1.0, -1.0, 0.0);
		glm::vec3 pos4(1.0, 1.0, 0.0);


		// texture coordinates
		glm::vec2 uv1(0.0, 1.0);
		glm::vec2 uv2(0.0, 0.0);
		glm::vec2 uv3(1.0, 0.0);
		glm::vec2 uv4(1.0, 1.0);




		// calculate tangent/bitangent vectors of both triangles
		glm::vec3 tangent1, bitangent1;
		glm::vec3 tangent2, bitangent2;

		// TRIANGLE 1
		glm::vec3 edge1 = pos2 - pos1;
		glm::vec3 edge2 = pos3 - pos1;
		glm::vec2 deltaUV1 = uv2 - uv1;
		glm::vec2 deltaUV2 = uv3 - uv1;

		GLfloat f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

		tangent1.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
		tangent1.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
		tangent1.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
		tangent1 = glm::normalize(tangent1);

		bitangent1.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
		bitangent1.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
		bitangent1.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
		bitangent1 = glm::normalize(bitangent1);

		// TRIANGLE 2
		edge1 = pos3 - pos1;
		edge2 = pos4 - pos1;
		deltaUV1 = uv3 - uv1;
		deltaUV2 = uv4 - uv1;

		f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

		tangent2.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
		tangent2.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
		tangent2.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
		tangent2 = glm::normalize(tangent2);


		bitangent2.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
		bitangent2.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
		bitangent2.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
		bitangent2 = glm::normalize(bitangent2);

		//Vertex data for rendering quads, used in walls

		GLfloat quadVertices[] = {
			// Positions            // normal         // TexCoords  // Tangent                          // Bitangent
			pos1.x, pos1.y, pos1.z, nm.x, nm.y, nm.z, uv1.x, uv1.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,
			pos2.x, pos2.y, pos2.z, nm.x, nm.y, nm.z, uv2.x, uv2.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,
			pos3.x, pos3.y, pos3.z, nm.x, nm.y, nm.z, uv3.x, uv3.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,

			pos1.x, pos1.y, pos1.z, nm.x, nm.y, nm.z, uv1.x, uv1.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z,
			pos3.x, pos3.y, pos3.z, nm.x, nm.y, nm.z, uv3.x, uv3.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z,
			pos4.x, pos4.y, pos4.z, nm.x, nm.y, nm.z, uv4.x, uv4.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z
		};

		// Plane VAO
		glGenVertexArrays(1, &quadVAO);
		glGenBuffers(1, &quadVBO);
		glBindVertexArray(quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 14 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(GLfloat), (GLvoid*)(8 * sizeof(GLfloat)));
		glEnableVertexAttribArray(4);
		glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(GLfloat), (GLvoid*)(11 * sizeof(GLfloat)));
	}
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}
#pragma endregion


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
		testBasics = !testBasics;
	if (key == GLFW_KEY_Y && action == GLFW_PRESS)
		testVelOmega = !testVelOmega;
	
	if (key == GLFW_KEY_F && action == GLFW_PRESS)
		playForces = !playForces;
	if (key == GLFW_KEY_R && action == GLFW_PRESS)
		playTorque = !playTorque;
	if (key == GLFW_KEY_G && action == GLFW_PRESS)
		gravityBool = !gravityBool;


	if (key == GLFW_KEY_O && action == GLFW_PRESS)
	{
		movePos = !movePos;
		posVec += glm::vec3(0.0f, 1.0f, 0.0f);
	}
	if (key == GLFW_KEY_P && action == GLFW_PRESS)
	{
		posVec = glm::vec3(0.0f, 0.0f, 0.0f);
		movePos = !movePos;
	}
		

	if (key == GLFW_KEY_K && action == GLFW_PRESS)
		velVec += glm::vec3(0.01f, 0.0f, 0.0f);
	if (key == GLFW_KEY_L && action == GLFW_PRESS)
		velVec -= glm::vec3(0.01f, 0.0f, 0.0f);

	if (key == GLFW_KEY_N && action == GLFW_PRESS)
		angVec += glm::vec3(0.5f, 0.5f, 0.0f);
	if (key == GLFW_KEY_M && action == GLFW_PRESS)
		angVec -= glm::vec3(0.5f, 0.5f, 0.0f);





	if (key == GLFW_KEY_V && action == GLFW_PRESS)
		forceVec += glm::vec3(0.008f, 0.0f, 0.0f);
	if (key == GLFW_KEY_C && action == GLFW_PRESS)
		forceVec -= glm::vec3(0.008f, 0.0f, 0.0f);

	if (key == GLFW_KEY_Z && action == GLFW_PRESS)
		torqueVec += glm::vec3(0.5f, 1.0f, 0.0f);
	if (key == GLFW_KEY_X && action == GLFW_PRESS)
		torqueVec -= glm::vec3(0.5f, 1.0f, 0.0f);




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

	camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}

#pragma endregion






