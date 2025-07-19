#include <GLFW/glfw3.h>      // must come before glad
#include <glad/glad.h>

#include "Core/Core.h"
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include <iostream>

class App {
	public:
	App() {
		// Constructor code here
		// Initialize GLFW
		if (!glfwInit()) {
			// handle error
		}

		// Set OpenGL version
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	}
    void Run() {
       

        // Create GLFW window
        GLFWwindow* window = glfwCreateWindow(1280, 720, "ImGui Window", nullptr, nullptr);
        if (!window) {
            Core::PrintHelloWorld();
            glfwTerminate();
            return;
        }

        glfwMakeContextCurrent(window);
        glfwSwapInterval(1); // Enable vsync

        // Load OpenGL with glad
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            // handle error
            std::cerr << "Failed to initialize GLAD\n";
            return;
        }

        // Setup ImGui
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        ImGui::StyleColorsDark();
        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init("#version 410");

        // Main loop
        while (!glfwWindowShouldClose(window)) {

            glfwPollEvents();

            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            ImGui::Begin("Hello, ImGui!");
            ImGui::Text("This is a text widget.");
            ImGui::End();

            ImGui::Render();
            int display_w, display_h;
            glfwGetFramebufferSize(window, &display_w, &display_h);
            glViewport(0, 0, display_w, display_h);
            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            glfwSwapBuffers(window);
        }

        // Cleanup
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        glfwDestroyWindow(window);
        glfwTerminate();
    }

    ~App() {}

	private:

};