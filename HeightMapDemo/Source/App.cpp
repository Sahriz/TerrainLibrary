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

std::string App::ReadFile(const std::string& filePath) {
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
GLuint App::CompileShader(GLenum type, const std::string& source) {
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

GLuint App::CreateShaderProgram(const std::string& vertexPath, const std::string& fragmentPath) {
	std::string vertexSource = ReadFile(vertexPath);
	std::string fragmentSource = ReadFile(fragmentPath);

	GLuint vertShader = CompileShader(GL_VERTEX_SHADER, vertexSource);
	if (vertShader == 0) return 0;

	GLuint fragShader = CompileShader(GL_FRAGMENT_SHADER, fragmentSource);
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

void App::ResetToStartValues() {
	_scale = 0.1f;
	_amplitude = 1.0f;
	_frequency = 1.0f;
	_octave = 5;
	_lacunarity = 2.0f;
	_persistance = 0.5f;
	_width = 1000;
	_height = 1000;
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
	Camera* cam = static_cast<Camera*>(glfwGetWindowUserPointer(window));
	if (cam) {
		cam->ProcessMouseMovement(window, xpos, ypos);
	}
}

void App::UpdatePlaneMesh(Core::PlaneMesh& planeData, GLuint& VAO, GLuint& VBOVertex, GLuint& VBONormals, GLuint& EBO) {
	
	planeData = std::move(Core::CreateHeightMapPlaneMeshGPU(_width,_height, glm::ivec2(0,0), _scale, _amplitude, _frequency, _octave, _persistance, _lacunarity, true));
	std::cout << planeData.vertices.size() << " " << planeData.normals.size() << " " << planeData.indices.size() << std::endl;
	ProgramSetup(planeData, VAO, VBOVertex, VBONormals, EBO);
}

void App::ProgramSetup(Core::PlaneMesh& planeData, GLuint& VAO, GLuint& VBOVertex, GLuint& VBONormals, GLuint& EBO) {
	GLenum err;

	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBOVertex);
	glDeleteBuffers(1, &VBONormals);
	glDeleteBuffers(1, &EBO);

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBOVertex);
	glGenBuffers(1, &VBONormals);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);
	
	//Vertex buffer setup
	glBindBuffer(GL_ARRAY_BUFFER, VBOVertex);
	glBufferData(GL_ARRAY_BUFFER, planeData.vertices.size() * sizeof(glm::fvec3), planeData.vertices.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::fvec3), (void*)0);

	//Normal buffer setup
	glBindBuffer(GL_ARRAY_BUFFER, VBONormals);
	glBufferData(GL_ARRAY_BUFFER, planeData.normals.size() * sizeof(glm::fvec3), planeData.normals.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::fvec3), (void*)0);

	//Index buffer setup
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, planeData.indices.size() * sizeof(int), planeData.indices.data(), GL_STATIC_DRAW);

	glBindVertexArray(0);
	
}

void App::ProgramInit(Core::PlaneMesh& planeData, GLuint& VAO, GLuint& VBOVertex, GLuint& VBONormals, GLuint& EBO) {
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBOVertex);
	glGenBuffers(1, &VBONormals);
	glGenBuffers(1, &EBO);


	glBindVertexArray(VAO);

	//Vertex buffer setup
	glBindBuffer(GL_ARRAY_BUFFER, VBOVertex);
	glBufferData(GL_ARRAY_BUFFER, planeData.vertices.size() * sizeof(glm::fvec3), planeData.vertices.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::fvec3), (void*)0);
	//Normal buffer setup

	glBindBuffer(GL_ARRAY_BUFFER, VBONormals);
	glBufferData(GL_ARRAY_BUFFER, planeData.normals.size() * sizeof(glm::fvec3), planeData.normals.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::fvec3), (void*)0);
	//Index buffer setup
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, planeData.indices.size() * sizeof(int), planeData.indices.data(), GL_STATIC_DRAW);


	glBindVertexArray(0);
}

void App::Cleanup(GLuint& VAO, GLuint& VBOVertex, GLuint& VBONormals, GLuint& EBO, GLFWwindow* window, GLuint& shaderProgram) {
	glDeleteProgram(shaderProgram);
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBOVertex);
	glDeleteBuffers(1, &VBONormals);
	glDeleteBuffers(1, &EBO);
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	glfwDestroyWindow(window);
	glfwTerminate();
}

void App::Run() {
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

	Core::PlaneMesh planeData = Core::CreateHeightMapPlaneMeshGPU(_width, _height);
	std::cout << planeData.vertices.size() << " " << planeData.normals.size() << " " << planeData.indices.size() << std::endl;

	GLuint VAO, VBOVertex, VBONormals, EBO;
	ProgramInit(planeData, VAO, VBOVertex, VBONormals, EBO);

	GLuint shaderProgram = CreateShaderProgram("Shaders/shader.vert", "Shaders/shader.frag");

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

	glm::mat4 identity = glm::mat4(1.0f);

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
	
	float prevTime = 0.0f;
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
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
		ImGui::SetNextWindowSize(ImVec2(380,300), 0);
		ImGui::Begin("Settings Panel  |  Press E to access");
		ImGui::Text("Mesh Settings");
		ImGui::SliderInt("Width", &_width, 100, 2000);
		ImGui::SliderInt("Height", &_height, 100, 2000);
		ImGui::SliderFloat("NoiseScale", &_scale, 0.001f, 1.0f);
		ImGui::SliderFloat("Amplitude", &_amplitude, 0.01f, 5.0f);
		ImGui::SliderFloat("FrequencyScale", &_frequency, 0.01f, 5.0f);
		ImGui::SliderInt("Octaves", &_octave, 0, 10);
		ImGui::SliderFloat("Persistance", &_persistance, 0.1f, 4.0f);
		ImGui::SliderFloat("Lacunarity", &_lacunarity, 0.1f, 4.0f);
		
		if (ImGui::Button("Regenerate Mesh")) {
			UpdatePlaneMesh(planeData, VAO, VBOVertex, VBONormals, EBO);
		}
		if (ImGui::Button("Reset Settings")) {
			ResetToStartValues();
		}
		ImGui::Text("WASD to move  |  Space to ascend and ctrl to descend");

		ImGui::End();

		ImGui::Render();

		int display_w, display_h;
		glfwGetFramebufferSize(window, &display_w, &display_h);
		glViewport(0, 0, display_w, display_h);
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram(shaderProgram);
		glBindVertexArray(VAO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO); 
		glDrawElements(GL_TRIANGLES, planeData.indices.size(), GL_UNSIGNED_INT, 0);
		

		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(window);
	}

	Cleanup(VAO, VBOVertex, VBONormals, EBO, window, shaderProgram);
}



