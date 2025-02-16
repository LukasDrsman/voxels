#pragma once
#include "irenderable.hpp"
#include <GLFW/glfw3.h>
#include <GL/glew.h>
#include "shaders.hpp"
#include <stdexcept>
#include <iostream>

namespace ctx
{
	class Window
	{
		GLFWwindow *window;

	public:
		Window(int w, int h);
		~Window();

		void run(IRenderable& obj) const;
	};

	inline Window::Window(const int w, const int h)
	{
		glfwWindowHint(GLFW_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		glfwWindowHint(GLFW_RESIZABLE, 1);
		glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, 1);


		if (!glfwInit())
		{
			throw std::runtime_error("[GLFW] Did not initialize");
		}

		window = glfwCreateWindow(w, h, "Window", nullptr, nullptr);

		if (!window)
		{
			glfwTerminate();
			throw std::runtime_error("[GLFW] Window not created");
		}

		glfwMakeContextCurrent(window);
	}

	inline Window::~Window()
	{
		std::cout << "[GLFW] Terminated" << std::endl;
		glfwTerminate();
	}

	inline void Window::run(IRenderable& obj) const
	{
		glewInit();
		obj.pre_render();
		while(!glfwWindowShouldClose(window))
		{
			glfwPollEvents();
			obj.pre_render_cleanup();
			obj.render();
			glfwSwapBuffers(window);
		}
	}
}

