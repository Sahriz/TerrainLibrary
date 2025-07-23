#include "App.h"


	App::App() {
		// Constructor code here
		// Initialize GLFW
		if (!glfwInit()) {
			// handle error
		}

		// Set OpenGL version
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
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
    GLuint App::compileShader(GLenum type, const std::string& source) {
        GLuint shader = glCreateShader(type);
        const char* src = source.c_str();
        glShaderSource(shader, 1, &src, nullptr);
        glCompileShader(shader);

        GLint success;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            GLint logLength;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
            std::vector<char> log(logLength);
            glGetShaderInfoLog(shader, logLength, nullptr, log.data());
            std::cerr << "Shader compile error:\n" << log.data() << "\n";
            glDeleteShader(shader);
            return 0;
        }

        return shader;
    }

    GLuint App::createShaderProgram(const std::string& vertexPath, const std::string& fragmentPath) {
        std::string vertexSource = readFile(vertexPath);
        std::string fragmentSource = readFile(fragmentPath);

        GLuint vertShader = compileShader(GL_VERTEX_SHADER, vertexSource);
        if (vertShader == 0) return 0;

        GLuint fragShader = compileShader(GL_FRAGMENT_SHADER, fragmentSource);
        if (fragShader == 0) {
            glDeleteShader(vertShader);
            return 0;
        }

        GLuint program = glCreateProgram();
        glAttachShader(program, vertShader);
        glAttachShader(program, fragShader);
        glLinkProgram(program);

        GLint success;
        glGetProgramiv(program, GL_LINK_STATUS, &success);
        if (!success) {
            GLint logLength;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);
            std::vector<char> log(logLength);
            glGetProgramInfoLog(program, logLength, nullptr, log.data());
            std::cerr << "Shader program link error:\n" << log.data() << "\n";
            glDeleteProgram(program);
            program = 0;
        }

        glDeleteShader(vertShader);
        glDeleteShader(fragShader);
        return program;
    }

    void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
        Camera* cam = static_cast<Camera*>(glfwGetWindowUserPointer(window));
        if (cam) {
            cam->ProcessMouseMovement(window, xpos, ypos);
        }
    }

    void App::createPerspectiveMatrix(float fov, float aspect, float near, float far, float right, float left, float top, float bottom) {
        float f = 1.0f / std::tan(fov / 2.0f);


        _perspectiveMat = glm::mat4(0.0f);

        _perspectiveMat[0][0] = 2.0f*near/(right-left);
		_perspectiveMat[2][0] = (right + left) / (right - left);
        _perspectiveMat[1][1] = 2.0f*near/(top-bottom);
		_perspectiveMat[2][1] = (top + bottom) / (top - bottom);
        _perspectiveMat[2][2] = -(far + near) / (far - near);
        _perspectiveMat[3][2] = -(2 * far * near) / (far - near);
        _perspectiveMat[2][3] = -1.0f;
    }

    void App::Run() {
       
        std::vector<glm::vec3> cubeVertices = {
    {-0.5f, -0.5f, -0.5f}, // 0
    { 0.5f, -0.5f, -0.5f}, // 1
    { 0.5f,  0.5f, -0.5f}, // 2
    {-0.5f,  0.5f, -0.5f}, // 3
    {-0.5f, -0.5f,  0.5f}, // 4
    { 0.5f, -0.5f,  0.5f}, // 5
    { 0.5f,  0.5f,  0.5f}, // 6
    {-0.5f,  0.5f,  0.5f}  // 7
        };

        std::vector<int> cubeIndices = {
            // Front face
            4, 5, 6,
            6, 7, 4,

            // Back face
            1, 0, 3,
            3, 2, 1,

            // Left face
            0, 4, 7,
            7, 3, 0,

            // Right face
            5, 1, 2,
            2, 6, 5,

            // Top face
            3, 7, 6,
            6, 2, 3,

            // Bottom face
            0, 1, 5,
            5, 4, 0
        };

        int width = 1280, height = 720;
        // Vertex data
        //createPerspectiveMatrix(glm::radians(80.0f), width/height, 0.1f, 1000.0f, 0.5f, -0.5f, 0.5f, -0.5f);
		_perspectiveMat = glm::perspective(glm::radians(80.0f), static_cast<float>(width) / static_cast<float>(height), 0.1f, 1000.0f);
		
        // Create GLFW window
        GLFWwindow* window = glfwCreateWindow(width, height, "ImGui Window", nullptr, nullptr);
        if (!window) {
            glfwTerminate();
            return;
        }

        glfwMakeContextCurrent(window);
        gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

        Camera camera = Camera(window);
        glfwSetWindowUserPointer(window, &camera);
        glfwSetCursorPosCallback(window, mouse_callback);
        glfwWindowHint(GLFW_DEPTH_BITS, 24);
        
        const GLubyte* version = glGetString(GL_VERSION);
        //std::cout << "OpenGL Version: " << version << std::endl;
        
        Core::PlaneMesh planeMesh = Core::CreatePlaneMesh(100, 100);
        Core::ApplyHeightMapCPU(planeMesh, 100, 100);
        
        GLuint VAO, VBO, VBONormals, EBO;

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &VBONormals);
        glGenBuffers(1, &EBO);
        
        
        glBindVertexArray(VAO);
        
		//Vertex buffer setup
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, planeMesh.vertices.size() * sizeof(glm::fvec3), planeMesh.vertices.data(), GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::fvec3), (void*)0);
		//Normal buffer setup
        
        glBindBuffer(GL_ARRAY_BUFFER, VBONormals);
		glBufferData(GL_ARRAY_BUFFER, planeMesh.normals.size() * sizeof(glm::fvec3), planeMesh.normals.data(), GL_STATIC_DRAW);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::fvec3), (void*)0);
		//Index buffer setup
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, planeMesh.indices.size() * sizeof(int), planeMesh.indices.data(), GL_STATIC_DRAW);
        

        glBindVertexArray(0);
   
       
        

	

        GLuint shaderProgram = createShaderProgram("Shaders/shader.vert", "Shaders/shader.frag");
        
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
        ImGui_ImplOpenGL3_Init("#version 430");

        glm::mat4 identity = glm::mat4(0.0f);
        identity[0][0] = 1.0f;
        identity[1][1] = 1.0f;
        identity[2][2] = 1.0f;
        identity[3][3] = 1.0f;

        glm::mat4 view = glm::translate(identity, glm::vec3(0.0f, 5.0f, 0.0f));
        glm::mat4 model = glm::translate(identity, glm::vec3(0.0f, -2.0f, 0.0f));
		glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(model)));
        GLenum err;
        glUseProgram(shaderProgram);
        while ((err = glGetError()) != GL_NO_ERROR) {
            std::cerr << "OpenGL error: " << std::hex << err << "\n";
        }
        GLint widthLocation = glGetUniformLocation(shaderProgram, "Width");
        GLint heightLocation = glGetUniformLocation(shaderProgram, "Height");
        GLint timeLocation = glGetUniformLocation(shaderProgram, "Time");
        GLint projMLocation = glGetUniformLocation(shaderProgram, "projM");
		GLuint modelMLocation = glGetUniformLocation(shaderProgram, "uModel");
        GLint viewLoc = glGetUniformLocation(shaderProgram, "uView");
		GLint normalMatrixLocation = glGetUniformLocation(shaderProgram, "normalMatrix");
        
        
        glUniform1f(widthLocation, width);
        glUniform1f(heightLocation, height);
        glUniformMatrix4fv(projMLocation, 1, GL_FALSE, glm::value_ptr(_perspectiveMat));
		glUniformMatrix4fv(modelMLocation, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix3fv(normalMatrixLocation, 1, GL_FALSE, glm::value_ptr(normalMatrix));
        

        // Main loop
        float scaleStartValue = 1.0f;
        float frequencyStartValue = 1.0f;
        int octaveStartValue = 1;
		float lacunarityStartValue = 2.0f;
		float persistanceStartValue = 0.5f;
		float prevTime = 0.0f;
        glEnable(GL_CULL_FACE);
        glEnable(GL_DEPTH_TEST);
        while (!glfwWindowShouldClose(window)) {

            glfwPollEvents();
			
			
            float timeValue = glfwGetTime();
			float deltaTime = timeValue - prevTime;
			prevTime = timeValue;
            float fps = 1 / deltaTime;
            camera.HandleKeyboardInput(deltaTime, window);
            view = camera.GetViewMatrix();
            glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		    //std::cout << "\rDelta Time: " << deltaTime << "s" << " | FPS: " << fps << std::flush;
			//std::cout << "FPS: " << fps << std::endl << std::flush;
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
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            //glUseProgram(shaderProgram);

            glBindVertexArray(VAO);
            
            glDrawElements(GL_TRIANGLES, planeMesh.indices.size(), GL_UNSIGNED_INT, 0);


            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            glfwSwapBuffers(window);
        }

        // Cleanup
        glDeleteProgram(shaderProgram);
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
		glDeleteBuffers(1, &VBONormals);
        glDeleteBuffers(1, &EBO);
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        glfwDestroyWindow(window);
        glfwTerminate();
    }



