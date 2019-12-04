#pragma once

#include <glad\glad.h>
#include <GLFW\glfw3.h>

#include "Shader.h"
#include "CShader.h"

#include "stb_image.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <random>

#include <omp.h>

class Graphics {

public:

	GLFWwindow *window;
	const GLFWvidmode* mode;

	unsigned int screenQuadVAO, screenQuadVBO;
	unsigned int displayScreenTexture;

	bool firstMouse = true;
	bool firstClick = true;

	float lastX = 1000 / 2.0f;
	float lastY = 800 / 2.0f;

	// timing
	float deltaTime = 0.0f;	// time between current frame and last frame
	float lastFrame = 0.0f;

	Shader screenShader, clearShader;
	CShader flameShader;

	unsigned int flameBuffer;
	unsigned int clearFBO;

	float time = 0;

	float parA[10], parB[10], parC[10], parD[10], parE[10], parF[10];
	float weights[11];

	~Graphics() {
		delete this;
	}

	Graphics() {

		glfwInit();
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
		//glfwWindowHint(GLFW_SAMPLES, 4);

		GLFWmonitor *monitor = glfwGetPrimaryMonitor();
		mode = glfwGetVideoMode(monitor);
		window = glfwCreateWindow(mode->width, mode->height, "My Title", NULL, NULL);

		//cout << "width: " << mode->width << " | height: " << mode->height << endl;

		if (window == NULL)
		{
			std::cout << "Failed to create GLFW window" << std::endl;
			glfwTerminate();
		}
		glfwMakeContextCurrent(window);

		glfwSetWindowPos(window, 0, 0);

		//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
		{
			std::cout << "Failed to initialize GLAD" << std::endl;

		}

		//glEnable(GL_MULTISAMPLE);
		glEnable(GL_DEPTH_TEST);
		glViewport(0, 0, mode->width, mode->height);

		flameShader = CShader("flameShader.glslcs");
		displayScreenTexture = flameShader.createFrameBufferTexture(mode->width,mode->height);

		screenShader = Shader("screenShader.vs", "screenShader.fs");
		screenShader.use();
		screenShader.setInt("displayScreenTexture", 0);

		clearShader = Shader("clearShader.vs", "clearShader.fs");

		glGenFramebuffers(1, &clearFBO);
		glBindFramebuffer(GL_FRAMEBUFFER, clearFBO);
		glBindTexture(GL_TEXTURE_2D, displayScreenTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, mode->width, mode->height, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, displayScreenTexture, 0);
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			std::cout << "SSAO Blur Framebuffer not complete!" << std::endl;
		glBindFramebuffer(GL_FRAMEBUFFER, 0);


		for (int i = 0; i < 10;i++) {
			parA[i] = getRandomFloat(-1, 1);
			parB[i] = getRandomFloat(-1, 1);
			parC[i] = getRandomFloat(-1, 1);
			parD[i] = getRandomFloat(-1, 1);
			parE[i] = getRandomFloat(-1, 1);
			parF[i] = getRandomFloat(-1, 1);
		}

		for (int i = 0; i < 11; i++) { weights[i] = getRandomFloat(-1, 1); }
	}

	float getRandomFloat(float min, float max) {

		std::random_device rd; // obtain a random number from hardware
		std::mt19937 eng(rd()); // seed the generator
		std::uniform_real_distribution<> distr(min, max);

		return distr(eng);
	}

	int getRandomInt(int min, int max) {
		std::random_device rd; // obtain a random number from hardware
		std::mt19937 eng(rd()); // seed the generator
		std::uniform_int_distribution<> distr(min, max);

		return distr(eng);
	}

	void mainLoop() {
		int frameCount = 0;
		while (!glfwWindowShouldClose(window))
		{

			float currentFrame = glfwGetTime();
			deltaTime = currentFrame - lastFrame;
			lastFrame = currentFrame;
			//cout <<"deltaTime: " << deltaTime << endl;
			//frameCount++;

			//if (deltaTime >= 1.0) {
			//cout << "FPS: " << frameCount << endl;
			//frameCount = 0;
			//lastFrame = currentFrame;
			//}

			//Input
			processInput();

			//Render
			render();

			glfwSwapBuffers(window);
			glfwPollEvents();

			time += 0.1;
		}
	}

	int nextPowerOfTwo(int x) {
		x--;
		x |= x >> 1; // handle 2 bit numbers
		x |= x >> 2; // handle 4 bit numbers
		x |= x >> 4; // handle 8 bit numbers
		x |= x >> 8; // handle 16 bit numbers
		x |= x >> 16; // handle 32 bit numbers
		x++;
		return x;
	}

	void render() {


		glViewport(0, 0, mode->width, mode->height);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//clear texture
		glBindFramebuffer(GL_FRAMEBUFFER, clearFBO);

		clearShader.use();
		renderScreenQuad();

		glBindFramebuffer(GL_FRAMEBUFFER, 0);


		//render flame fractal

		flameShader.use();
		flameShader.setFloat("width", mode->width);
		flameShader.setFloat("height", mode->height);
		flameShader.setFloat("time", time);
		flameShader.setFloatArray("a", parA, 10); flameShader.setFloatArray("b", parB,10); flameShader.setFloatArray("c", parC,10);
		flameShader.setFloatArray("d", parD, 10); flameShader.setFloatArray("e", parE,10); flameShader.setFloatArray("f", parF,10);
		flameShader.setFloatArray("w", weights,11);

		glBindImageTexture(0, displayScreenTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

		unsigned int worksizeX = mode->width;// nextPowerOfTwo(mode->width);
		unsigned int worksizeY = mode->height;// nextPowerOfTwo(mode->height);

		glDispatchCompute(worksizeX / 32, worksizeY / 32, 1);

		/* Reset image binding. */
		glBindImageTexture(0, 0, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

		screenShader.use();
		
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, displayScreenTexture);

		renderScreenQuad();
	}

	unsigned int squadVAO = 0;
	unsigned int squadVBO;
	void renderScreenQuad()
	{
		if (squadVAO == 0)
		{
			float squadVertices[] = {
				// positions        // texture Coords
				-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
				-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
				1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
				1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
			};
			// setup plane VAO
			glGenVertexArrays(1, &squadVAO);
			glGenBuffers(1, &squadVBO);
			glBindVertexArray(squadVAO);
			glBindBuffer(GL_ARRAY_BUFFER, squadVBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(squadVertices), &squadVertices, GL_STATIC_DRAW);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
		}
		glBindVertexArray(squadVAO);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		glBindVertexArray(0);
	}

	void processInput()
	{

	}

	void framebuffer_resize_callback(GLFWwindow* window, int width, int height)
	{
		glViewport(0, 0, width, height);
	}

	void mouse_callback(GLFWwindow* window, double xpos, double ypos)
	{

		if (firstMouse)
		{
			lastX = xpos;
			lastY = ypos;
			firstMouse = false;
		}

		float xoffset = xpos - lastX;
		float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

		lastX = xpos;
		lastY = ypos;
	}

	// glfw: whenever the mouse scroll wheel scrolls, this callback is called
	// ----------------------------------------------------------------------
	void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
	{
	}

	void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
		//action: GLFW_PRESS / GLFW_RELEASE / GLFW_REPEAT
		//string keyName = string(glfwGetKeyName(key, scancode));


		switch (action) {
		case GLFW_PRESS:
			//cout << key << endl;

			keyAction(key);
			break;
		case GLFW_RELEASE:
			break;
		case GLFW_REPEAT:
			keyAction(key);
			break;
		default:
			break;
		};
	}

	void keyAction(int key) {

		switch (key) {

		case 256:
			//esc button
			glfwSetWindowShouldClose(window, true);
			break;
		default:
			break;
		};
	}

	void terminate() {
		glfwTerminate();
	}

	// utility function for loading a 2D texture from file
	// ---------------------------------------------------
	unsigned int loadTexture(char const * path)
	{

		//stbi_set_flip_vertically_on_load(1);
		unsigned int textureID;
		glGenTextures(1, &textureID);

		int width, height, nrComponents;
		unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);

		if (data)
		{
			GLenum format;
			GLenum dataFormat;
			if (nrComponents == 1) {
				format = GL_RED;
				dataFormat = GL_RED;
			}
			else if (nrComponents == 3) {
				format = GL_SRGB;
				dataFormat = GL_RGB;
			}
			else if (nrComponents == 4) {
				format = GL_SRGB_ALPHA;
				dataFormat = GL_RGBA;
			}

			glBindTexture(GL_TEXTURE_2D, textureID);
			glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, dataFormat, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			stbi_image_free(data);
		}
		else
		{
			std::cout << "Texture failed to load at path: " << path << std::endl;
			stbi_image_free(data);
		}

		return textureID;
	}

};