//
//  main.cpp
//  OpenGL Advances Lighting
//
//  Created by CGIS on 28/11/16.
//  Copyright � 2016 CGIS. All rights reserved.
//

#if defined (_APPLE_)
#define GLFW_INCLUDE_GLCOREARB
#define GL_SILENCE_DEPRECATION
#else
#define GLEW_STATIC
#include <GL/glew.h>
#endif

#include <GLFW/glfw3.h>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/matrix_inverse.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "Shader.hpp"
#include "Model3D.hpp"
#include "Camera.hpp"
#include "SkyBox.hpp"
#include "RainSystem.hpp"

#include <iostream>

int glWindowWidth = 800;
int glWindowHeight = 600;
int retina_width, retina_height;
GLFWwindow* glWindow = NULL;

const unsigned int SHADOW_WIDTH = 4096;
const unsigned int SHADOW_HEIGHT = 4096;

glm::mat4 model;
GLuint modelLoc;
glm::mat4 view;
GLuint viewLoc;
glm::mat4 projection;
GLuint projectionLoc;
glm::mat3 normalMatrix;
GLuint normalMatrixLoc;
glm::mat4 lightRotation;

glm::vec3 sunDir;
GLuint sunDirLoc;
glm::vec3 sunColor;
GLuint sunColorLoc;
GLfloat sunAngle = 0.0f;

glm::vec3 lightDir;
GLuint lightDirLoc;
glm::vec3 lightColor;
GLuint lightColorLoc;

glm::vec3 upPoint = glm::vec3(-147.897f, 196.0f, 150.8463f);
glm::vec3 downPoint = glm::vec3(-171.902f, 1.2639f, 4.13482f);

bool lightOn = true;
GLuint lightOnLoc;

GLfloat fogDensity = 0.0f;
GLuint fogDensityLoc;
glm::vec3 fogColor = glm::vec3(0.5f, 0.5f, 0.5f);
GLuint fogColorLoc;

gps::Shader rainShader;
gps::RainSystem* rainSystem;
bool isRaining = false;
float lastFrame = 0.0f;

glm::vec3 birdPosition = glm::vec3(-150.0f, 85.0f, 0.0f);
float birdAngle = 0.0f;

bool firstMouse = true;
float lastX = glWindowWidth / 2.0f;
float lastY = glWindowHeight / 2.0f;
float yaw = -90.0f;
float pitch = 0.0f;

bool inIntroAnimation = true;
std::vector<glm::vec3> introPoints;
std::vector<glm::vec3> lookingPoints;
int currentIntroPoint = 0;
float introLerpFactor = 0.0f;
const float INTRO_SPEED = 0.0025f;

gps::Camera myCamera(
	glm::vec3(0.0f, 2.0f, 5.5f),
	glm::vec3(0.0f, 0.0f, 0.0f),
	glm::vec3(0.0f, 1.0f, 0.0f));
float cameraSpeed = 1.25f;

bool pressedKeys[1024];
float angleY = 0.0f;
GLfloat lightAngle;

gps::Model3D proiect;
gps::Model3D ground;
gps::Model3D lightCube;
gps::Model3D screenQuad;
gps::Model3D bird;

gps::Shader myCustomShader;
gps::Shader lightShader;
gps::Shader screenQuadShader;
gps::Shader depthMapShader;

gps::Shader skyboxShader;
gps::SkyBox mySkyBox;

GLuint shadowMapFBO;
GLuint depthMapTexture;

bool showDepthMap;

GLenum glCheckError_(const char* file, int line) {
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR)
	{
		std::string error;
		switch (errorCode)
		{
		case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
		case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
		case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
		case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
		case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
		}
		std::cout << error << " | " << file << " (" << line << ")" << std::endl;
	}
	return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)

void windowResizeCallback(GLFWwindow* window, int width, int height) {
	fprintf(stdout, "window resized to width: %d , and height: %d\n", width, height);
	//TODO	
}

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (key == GLFW_KEY_M && action == GLFW_PRESS)
		showDepthMap = !showDepthMap;

	if (key == GLFW_KEY_O && action == GLFW_PRESS) {
		lightOn = !lightOn;
	}

	if (key == GLFW_KEY_R && action == GLFW_PRESS) {
		isRaining = !isRaining;
	}

	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
			pressedKeys[key] = true;
		else if (action == GLFW_RELEASE)
			pressedKeys[key] = false;
	}
}

void mouseCallback(GLFWwindow* window, double xposIn, double yposIn) {
	float xpos = static_cast<float>(xposIn);
	float ypos = static_cast<float>(yposIn);

	if (firstMouse) {
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; 
	lastX = xpos;
	lastY = ypos;

	const float sensitivity = 0.1f;
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	yaw += xoffset;
	pitch += yoffset;

	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;

	glm::vec3 direction;
	direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	direction.y = sin(glm::radians(pitch));
	direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

	myCamera.rotate(direction);
}

void initSkybox() {
	std::vector<const GLchar*> faces;
	faces.push_back("skybox/right.tga");
	faces.push_back("skybox/left.tga");
	
	faces.push_back("skybox/top.tga");
	faces.push_back("skybox/bottom.tga");

	faces.push_back("skybox/back.tga");
	faces.push_back("skybox/front.tga");
	
	mySkyBox.Load(faces);
}

void processMovement()
{
	/*if (pressedKeys[GLFW_KEY_Q]) {
		angleY -= 1.0f;
	}

	if (pressedKeys[GLFW_KEY_E]) {
		angleY += 1.0f;
	}*/

	if (pressedKeys[GLFW_KEY_J]) {
		sunAngle -= 1.0f;
	}

	if (pressedKeys[GLFW_KEY_L]) {
		sunAngle += 1.0f;
	}

	if (!inIntroAnimation) {
		if (pressedKeys[GLFW_KEY_W]) {
			myCamera.move(gps::MOVE_FORWARD, cameraSpeed);
		}
		if (pressedKeys[GLFW_KEY_S]) {
			myCamera.move(gps::MOVE_BACKWARD, cameraSpeed);
		}
		if (pressedKeys[GLFW_KEY_A]) {
			myCamera.move(gps::MOVE_LEFT, cameraSpeed);
		}
		if (pressedKeys[GLFW_KEY_D]) {
			myCamera.move(gps::MOVE_RIGHT, cameraSpeed);
		}
		if (pressedKeys[GLFW_KEY_Z]) {
			myCamera.move(gps::MOVE_DOWN, cameraSpeed);
		}
		if (pressedKeys[GLFW_KEY_X]) {
			myCamera.move(gps::MOVE_UP, cameraSpeed);
		}
	}

	if (pressedKeys[GLFW_KEY_C])
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	if (pressedKeys[GLFW_KEY_V])
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}

	if (pressedKeys[GLFW_KEY_B])
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
	}

	if (pressedKeys[GLFW_KEY_N])
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glEnable(GL_POLYGON_SMOOTH);
	}

	if (pressedKeys[GLFW_KEY_K]) {
		fogDensity -= 0.001f;
		if (fogDensity < 0.0f) fogDensity = 0.0f;
		myCustomShader.useShaderProgram();
		glUniform1f(fogDensityLoc, fogDensity);
	}

	if (pressedKeys[GLFW_KEY_I]) {
		fogDensity += 0.001f;
		myCustomShader.useShaderProgram();
		glUniform1f(fogDensityLoc, fogDensity);
	}

	float radius = 50.0f;
	float speed = 0.5f;  

	birdAngle -= speed;
	birdPosition.x = -150.0f + radius * cos(glm::radians(-birdAngle));
	birdPosition.z = radius * sin(glm::radians(-birdAngle));

	if (inIntroAnimation) {
		introLerpFactor += INTRO_SPEED;

		if (introLerpFactor >= 1.0f) {
			introLerpFactor = 0.0f;
			currentIntroPoint++;

			if (currentIntroPoint >= introPoints.size() - 1) {
				inIntroAnimation = false;
				return;
			}
		}

		glm::vec3 currentPos = glm::mix(
			introPoints[currentIntroPoint],
			introPoints[currentIntroPoint + 1],
			introLerpFactor
		);

		myCamera = gps::Camera(
			currentPos,
			lookingPoints[currentIntroPoint],
			glm::vec3(0.0f, 1.0f, 0.0f)
		);
	}

}

bool initOpenGLWindow()
{
	if (!glfwInit()) {
		fprintf(stderr, "ERROR: could not start GLFW3\n");
		return false;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


	//window scaling for HiDPI displays
	glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);

	//for sRBG framebuffer
	glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_TRUE);

	//for antialising
	glfwWindowHint(GLFW_SAMPLES, 4);

	glWindow = glfwCreateWindow(glWindowWidth, glWindowHeight, "OpenGL Shader Example", NULL, NULL);
	if (!glWindow) {
		fprintf(stderr, "ERROR: could not open window with GLFW3\n");
		glfwTerminate();
		return false;
	}

	glfwSetWindowSizeCallback(glWindow, windowResizeCallback);
	glfwSetKeyCallback(glWindow, keyboardCallback);
	glfwSetCursorPosCallback(glWindow, mouseCallback);
	glfwSetInputMode(glWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glfwMakeContextCurrent(glWindow);

	glfwSwapInterval(1);

#if not defined (_APPLE_)
	// start GLEW extension handler
	glewExperimental = GL_TRUE;
	glewInit();
#endif

	// get version info
	const GLubyte* renderer = glGetString(GL_RENDERER); // get renderer string
	const GLubyte* version = glGetString(GL_VERSION); // version as a string
	printf("Renderer: %s\n", renderer);
	printf("OpenGL version supported %s\n", version);

	//for RETINA display
	glfwGetFramebufferSize(glWindow, &retina_width, &retina_height);

	return true;
}

void initOpenGLState()
{
	glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
	glViewport(0, 0, retina_width, retina_height);

	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glEnable(GL_CULL_FACE); // cull face
	glCullFace(GL_BACK); // cull back face
	glFrontFace(GL_CCW); // GL_CCW for counter clock-wise
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_FRAMEBUFFER_SRGB);
}

void initObjects() {
	proiect.LoadModel("objects/proiect/pg.obj");
	//ground.LoadModel("objects/ground/ground.obj");
	lightCube.LoadModel("objects/cube/cube.obj");
	//screenQuad.LoadModel("objects/quad/quad.obj");
	rainSystem = new gps::RainSystem(100000);
	bird.LoadModel("objects/proiect/cioara.obj");
}

void initShaders() {
	myCustomShader.loadShader("shaders/shaderStart.vert", "shaders/shaderStart.frag");
	myCustomShader.useShaderProgram();
	lightShader.loadShader("shaders/lightCube.vert", "shaders/lightCube.frag");
	lightShader.useShaderProgram();
	screenQuadShader.loadShader("shaders/screenQuad.vert", "shaders/screenQuad.frag");
	screenQuadShader.useShaderProgram();
	depthMapShader.loadShader("shaders/depthMap.vert", "shaders/depthMap.frag");
	depthMapShader.useShaderProgram();
	skyboxShader.loadShader("shaders/skyboxShader.vert", "shaders/skyboxShader.frag");
	skyboxShader.useShaderProgram();
	rainShader.loadShader("shaders/rain.vert", "shaders/rain.frag");
}

glm::mat4 computeLightSpaceTrMatrix() {
	glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(sunAngle), glm::vec3(0.0f, 1.0f, 0.0f));

	glm::vec4 rotatedUpPoint = rotationMatrix * glm::vec4(upPoint, 1.0f);
	glm::vec4 rotatedDownPoint = rotationMatrix * glm::vec4(downPoint, 1.0f);

	glm::vec3 lightPos = glm::vec3(rotatedUpPoint);
	glm::vec3 lightTarget = glm::vec3(rotatedDownPoint);

	glm::mat4 lightView = glm::lookAt(lightPos,
		lightTarget,
		glm::vec3(0.0f, 1.0f, 0.0f));

	const float orthoSize = 200.0f;
	const float nearPlane = 1.0f;
	const float farPlane = 600.0f;

	glm::mat4 lightProjection = glm::ortho(-orthoSize, orthoSize,
		-orthoSize, orthoSize,
		nearPlane, farPlane);

	return lightProjection * lightView;
}

void initUniforms() {
	myCustomShader.useShaderProgram();

	model = glm::mat4(1.0f);
	modelLoc = glGetUniformLocation(myCustomShader.shaderProgram, "model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	view = myCamera.getViewMatrix();
	viewLoc = glGetUniformLocation(myCustomShader.shaderProgram, "view");
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	normalMatrixLoc = glGetUniformLocation(myCustomShader.shaderProgram, "normalMatrix");
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

	projection = glm::perspective(glm::radians(45.0f), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
	projectionLoc = glGetUniformLocation(myCustomShader.shaderProgram, "projection");
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

	//set the light direction (direction towards the light)
	lightDir = glm::vec3(-173.933f, 42.133f, -49.276f);
	lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));
	lightDirLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightDir");
	glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(lightRotation)) * lightDir));

	//set light color
	lightColor = glm::vec3(1.0f, 1.0f, 1.0f); 
	lightColorLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightColor");
	glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));

	lightOnLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightOn");
	glUniform1i(lightOnLoc, lightOn);

	sunDir = upPoint - downPoint;
	sunColor = glm::vec3(1.0f, 0.95f, 0.8f);

	sunDirLoc = glGetUniformLocation(myCustomShader.shaderProgram, "sunDir");
	sunColorLoc = glGetUniformLocation(myCustomShader.shaderProgram, "sunColor");
	
	glUniform3fv(sunDirLoc, 1, glm::value_ptr(sunDir));
	glUniform3fv(sunColorLoc, 1, glm::value_ptr(sunColor));

	myCustomShader.useShaderProgram();
	fogDensityLoc = glGetUniformLocation(myCustomShader.shaderProgram, "fogDensity");
	fogColorLoc = glGetUniformLocation(myCustomShader.shaderProgram, "fogColor");
	glUniform1f(fogDensityLoc, fogDensity);
	glUniform3fv(fogColorLoc, 1, glm::value_ptr(fogColor));

	lightShader.useShaderProgram();
	glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

	glUseProgram(depthMapShader.shaderProgram);
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "lightSpaceTrMatrix"), 1, GL_FALSE, glm::value_ptr(computeLightSpaceTrMatrix()));
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model)); 
}

void initIntro() {
	introPoints.push_back(glm::vec3(-173.9f, 107.94f, -162.69f));
	introPoints.push_back(glm::vec3(-91.316f, 104.13f, -8.58f));
	introPoints.push_back(glm::vec3(-158.32f, 95.129f, 119.42f));
	introPoints.push_back(glm::vec3(-245.88, 71.811f, 61.073f));
	introPoints.push_back(glm::vec3(-164.88f, 42.811f, -4.927f));
	introPoints.push_back(glm::vec3(-167.88f, 42.811f, -4.927f));

	lookingPoints.push_back(downPoint);
	lookingPoints.push_back(glm::vec3(-124.0f, 3.0f, 172.0f));
	lookingPoints.push_back(glm::vec3(-337.0f, 47.0f, -41.15));
	lookingPoints.push_back(glm::vec3(-25.5f, 25.0f, -41.26f));
	lookingPoints.push_back(glm::vec3(-173.4f, 54.0f, -118.0f));

	myCamera = gps::Camera(
		introPoints[0],
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f)
	);
}

void initFBO() {
	//TODO - Create the FBO, the depth texture and attach the depth texture to the FBO
	//generate FBO ID
	glGenFramebuffers(1, &shadowMapFBO);
	//create depth texture for FBO
	glGenTextures(1, &depthMapTexture);
	glBindTexture(GL_TEXTURE_2D, depthMapTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
		SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	//attach texture to FBO
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTexture,
		0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void drawObjects(gps::Shader shader, bool depthPass) {

	shader.useShaderProgram();

	model = glm::rotate(glm::mat4(1.0f), glm::radians(angleY), glm::vec3(0.0f, 1.0f, 0.0f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	// do not send the normal matrix if we are rendering in the depth map
	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}

	proiect.Draw(shader);

	model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
	model = glm::scale(model, glm::vec3(0.5f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	// do not send the normal matrix if we are rendering in the depth map
	if (!depthPass) {
		glm::mat4 birdModel = glm::mat4(1.0f);
		birdModel = glm::rotate(birdModel, glm::radians(angleY), glm::vec3(0.0f, 1.0f, 0.0f));
		birdModel = glm::translate(birdModel, birdPosition);
		birdModel = glm::rotate(birdModel, glm::radians(birdAngle), glm::vec3(0.0f, 1.0f, 0.0f));
		birdModel = glm::scale(birdModel, glm::vec3(0.5f));

		glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(birdModel));

		normalMatrix = glm::mat3(glm::inverseTranspose(view * birdModel));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

		bird.Draw(shader);
	}

	ground.Draw(shader);
	mySkyBox.Draw(skyboxShader, view, projection);
}

void renderScene() {

	// depth maps creation pass
	//TODO - Send the light-space transformation matrix to the depth map creation shader and
	//		 render the scene in the depth map

	depthMapShader.useShaderProgram();
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "lightSpaceTrMatrix"),
		1,
		GL_FALSE,
		glm::value_ptr(computeLightSpaceTrMatrix()));
	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);
	drawObjects(depthMapShader, showDepthMap);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// render depth map on screen - toggled with the M key

	if (showDepthMap) {
		glViewport(0, 0, retina_width, retina_height);

		glClear(GL_COLOR_BUFFER_BIT);

		screenQuadShader.useShaderProgram();

		//bind the depth map
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, depthMapTexture);
		glUniform1i(glGetUniformLocation(screenQuadShader.shaderProgram, "depthMap"), 0);

		glDisable(GL_DEPTH_TEST);
		screenQuad.Draw(screenQuadShader);
		glEnable(GL_DEPTH_TEST);
	}
	else {

		// final scene rendering pass (with shadows)

		glViewport(0, 0, retina_width, retina_height);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		myCustomShader.useShaderProgram();

		view = myCamera.getViewMatrix();
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

		lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));
		lightRotation = glm::rotate(lightRotation, glm::radians(angleY), glm::vec3(0.0f, 1.0f, 0.0f));
		glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(lightRotation)) * lightDir));

		glUniform1i(lightOnLoc, lightOn);

		//bind the shadow map
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, depthMapTexture);
		glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "shadowMap"), 3);

		glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "lightSpaceTrMatrix"),
			1,
			GL_FALSE,
			glm::value_ptr(computeLightSpaceTrMatrix()));

		drawObjects(myCustomShader, false);
		glm::mat4 birdModel = glm::mat4(1.0f);
		birdModel = glm::rotate(birdModel, glm::radians(angleY), glm::vec3(0.0f, 1.0f, 0.0f));
		birdModel = glm::translate(birdModel, birdPosition);
		birdModel = glm::rotate(birdModel, glm::radians(birdAngle), glm::vec3(0.0f, 1.0f, 0.0f));
		birdModel = glm::scale(birdModel, glm::vec3(0.5f));

		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(birdModel));
		normalMatrix = glm::mat3(glm::inverseTranspose(view * birdModel));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

		bird.Draw(myCustomShader);

		//draw a white cube around the light

		if (lightOn) {
			lightShader.useShaderProgram();

			glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));

			model = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));
			model = glm::rotate(model, glm::radians(angleY), glm::vec3(0.0f, 1.0f, 0.0f));
			model = glm::translate(model, 1.0f * lightDir);
			model = glm::scale(model, glm::vec3(2.5, 2.5, 2.5));
			glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

			lightCube.Draw(lightShader);
		}

		if (isRaining) {
			float currentFrame = glfwGetTime();
			float deltaTime = currentFrame - lastFrame;
			lastFrame = currentFrame;

			rainSystem->update(deltaTime);

			rainShader.useShaderProgram();
			glUniformMatrix4fv(glGetUniformLocation(rainShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
			glUniformMatrix4fv(glGetUniformLocation(rainShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
			glUniform1f(glGetUniformLocation(rainShader.shaderProgram, "deltaTime"), deltaTime);

			rainSystem->draw(rainShader);
		}

		lightShader.useShaderProgram();

		glm::mat4 sunRotation = glm::rotate(glm::mat4(1.0f), glm::radians(sunAngle), glm::vec3(0.0f, 1.0f, 0.0f));
		glm::vec4 rotatedUpPoint = sunRotation * glm::vec4(upPoint, 1.0f);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(rotatedUpPoint));
		model = glm::rotate(model, glm::radians(angleY), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(5.0f)); // Make the sun cube larger

		glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

		// Draw the sun cube
		lightCube.Draw(lightShader);

		myCustomShader.useShaderProgram();
		glUniform3fv(sunDirLoc, 1, glm::value_ptr(rotatedUpPoint));
	}
}
void cleanup() {
	glDeleteTextures(1, &depthMapTexture);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDeleteFramebuffers(1, &shadowMapFBO);
	glfwDestroyWindow(glWindow);
	//close GL context and any other GLFW resources
	glfwTerminate();
}

int main(int argc, const char* argv[]) {

	if (!initOpenGLWindow()) {
		glfwTerminate();
		return 1;
	}

	initOpenGLState();
	initObjects();
	initShaders();
	initUniforms();
	initSkybox();
	initIntro();
	initFBO();

	glCheckError();

	while (!glfwWindowShouldClose(glWindow)) {
		processMovement();
		renderScene();

		glfwPollEvents();
		glfwSwapBuffers(glWindow);
	}

	cleanup();

	return 0;
}