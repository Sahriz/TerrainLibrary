#include "App.h"


	App::App() {
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

    std::string App::readFile(const std::string& filePath) {
        std::ifstream file(filePath);
        std::stringstream buffer;
        if (file) {
            buffer << file.rdbuf();
        }
        else {
            std::cerr << "Failed to open file: " << filePath << "\n";
        }
        return buffer.str();
    }
    // Shader loading helpers
    GLuint App::compileShader(GLenum type, const char* src) {
        GLuint shader = glCreateShader(type);
        glShaderSource(shader, 1, &src, nullptr);
        glCompileShader(shader);
        return shader;
    }

    GLuint App::createShaderProgramFromFiles(const std::string& vertPath, const std::string& fragPath) {
        std::string vertSource = readFile(vertPath);
        std::string fragSource = readFile(fragPath);
        const char* vSrc = vertSource.c_str();
        const char* fSrc = fragSource.c_str();

        GLuint vs = compileShader(GL_VERTEX_SHADER, vSrc);
        GLuint fs = compileShader(GL_FRAGMENT_SHADER, fSrc);
        GLuint program = glCreateProgram();
        glAttachShader(program, vs);
        glAttachShader(program, fs);
        glLinkProgram(program);

        // Optional: check for linking errors here

        glDeleteShader(vs);
        glDeleteShader(fs);
        return program;
    }

    void App::Run() {
       
        int width = 1280, height = 720;
        // Vertex data
        float vertices[] = {
             0.0f,  1.0f, 0.0f,
            -1.0f, -1.0f, 0.0f,
             1.0f, -1.0f, 0.0f
        };
        // Create GLFW window
        GLFWwindow* window = glfwCreateWindow(width, height, "ImGui Window", nullptr, nullptr);
        if (!window) {
            Core::PrintHelloWorld();
            glfwTerminate();
            return;
        }

        glfwMakeContextCurrent(window);
        gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

        GLuint VAO, VBO;
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        GLuint shaderProgram = createShaderProgramFromFiles("Shaders/shader.vert", "Shaders/shader.frag");

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
        ImGui_ImplOpenGL3_Init("#version 460");

        GLint widthLocation = glGetUniformLocation(shaderProgram, "Width");
        GLint heightLocation = glGetUniformLocation(shaderProgram, "Height");
        GLint timeLocation = glGetUniformLocation(shaderProgram, "Time");
        glUseProgram(shaderProgram);
        glUniform1f(widthLocation, width);
        glUniform1f(heightLocation, height);


        // Main loop
        float scaleStartValue = 1.0f;
        float frequencyStartValue = 1.0f;
        int octaveStartValue = 1;
		float lacunarityStartValue = 2.0f;
		float persistanceStartValue = 0.5f;
        while (!glfwWindowShouldClose(window)) {

            glfwPollEvents();

            float timeValue = glfwGetTime();
            glUniform1f(timeLocation, timeValue);
            
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            ImGui::Begin("Hello, ImGui!");
            ImGui::Text("This is a text widget.");
            ImGui::SliderFloat("NoiseScale", &scaleStartValue, 0.001f, 5.0f);
            ImGui::SliderFloat("FrequencyScale", &frequencyStartValue, 0.001f, 5.0f);
            ImGui::SliderInt("Octaves", &octaveStartValue, 0, 10);
            ImGui::SliderFloat("Persistance", &persistanceStartValue, 0.1f, 4.0f);
			ImGui::SliderFloat("Lacunarity", &lacunarityStartValue, 0.1f, 4.0f);
            if (ImGui::Button("Click Me")) {
                std::cout << "Button clicked!" << std::endl;
                std::cout << scaleStartValue << std::endl;
                UpdateStartValues(scaleStartValue, frequencyStartValue, octaveStartValue);
            }
            
            ImGui::End();
            
            ImGui::Render();

            int display_w, display_h;
            glfwGetFramebufferSize(window, &display_w, &display_h);
            glViewport(0, 0, display_w, display_h);
            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            glUseProgram(shaderProgram);
            glBindVertexArray(VAO);
            glDrawArrays(GL_TRIANGLES, 0, 3);


            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            glfwSwapBuffers(window);
        }

        // Cleanup
        glDeleteProgram(shaderProgram);
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        glfwDestroyWindow(window);
        glfwTerminate();
    }



