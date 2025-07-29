#include "Renderer.h"

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
	Camera* cam = static_cast<Camera*>(glfwGetWindowUserPointer(window));
	if (cam) {
		cam->ProcessMouseMovement(window, xpos, ypos);
	}
}

Renderer::Renderer() {
	_chunkRenderer = ChunkRenderer(_width, _height, _viewDistance);

	if (!glfwInit()) {
		// handle error
	}

	// Set OpenGL version
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	_window = glfwCreateWindow(_screenWidth, _screenHeight, "ChunkDemo", nullptr, nullptr);
	_camera = Camera(_window);
	if (!_window) {
		glfwTerminate();
		return;
	}

	glfwMakeContextCurrent(_window);
	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

	_perspectiveMat = glm::perspective(glm::radians(80.0f), static_cast<float>(_screenWidth) / static_cast<float>(_screenHeight), 0.1f, 1000.0f);



	glfwSetWindowUserPointer(_window, &_camera);
	glfwSetCursorPosCallback(_window, mouse_callback);
	glfwWindowHint(GLFW_DEPTH_BITS, 24);

	_shaderProgram = CreateShaderProgram("Shaders/shader.vert", "Shaders/shader.frag");

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
	ImGui_ImplGlfw_InitForOpenGL(_window, true);
	ImGui_ImplOpenGL3_Init("#version 430");

	_identity = glm::mat4(1.0f);
	_view = glm::translate(_identity, glm::vec3(0.0f, 5.0f, 0.0f));
	_model = glm::translate(_identity, glm::vec3(0.0f, -2.0f, 0.0f));
	_normalMatrix = glm::transpose(glm::inverse(glm::mat3(_model)));

	glUseProgram(_shaderProgram);

	_widthLocation = glGetUniformLocation(_shaderProgram, "Width");
	_heightLocation = glGetUniformLocation(_shaderProgram, "Height");
	_timeLocation = glGetUniformLocation(_shaderProgram, "Time");
	_projMLocation = glGetUniformLocation(_shaderProgram, "projM");
	_modelMLocation = glGetUniformLocation(_shaderProgram, "uModel");
	_viewLoc = glGetUniformLocation(_shaderProgram, "uView");
	_normalMatrixLocation = glGetUniformLocation(_shaderProgram, "normalMatrix");


	glUniform1f(_widthLocation, _screenWidth);
	glUniform1f(_heightLocation, _screenHeight);
	glUniformMatrix4fv(_projMLocation, 1, GL_FALSE, glm::value_ptr(_perspectiveMat));
	glUniformMatrix4fv(_modelMLocation, 1, GL_FALSE, glm::value_ptr(_model));
	glUniformMatrix4fv(_viewLoc, 1, GL_FALSE, glm::value_ptr(_view));
	glUniformMatrix3fv(_normalMatrixLocation, 1, GL_FALSE, glm::value_ptr(_normalMatrix));


	// Main loop

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glEnable(GL_DEPTH_TEST);
	//Init();
}



void Renderer::Init() {
	
}

std::string Renderer::ReadFile(const std::string& filePath) {
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
GLuint Renderer::CompileShader(GLenum type, const std::string& source) {
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

GLuint Renderer::CreateShaderProgram(const std::string& vertexPath, const std::string& fragmentPath) {
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

void Renderer::DrawChunks(ChunkManager& chunkManager) {
	std::unordered_map<ChunkCoord, Core::PlaneMesh>& chunkMap = chunkManager.GetChunkMap();
	_chunkRenderer.UpdateActiveChunk(GetCameraPosition(), chunkManager);
	for (const glm::ivec2& coord : _chunkRenderer.GetActiveChunkSet()) {
		Core::PlaneMesh& planeData = chunkMap[coord];
		glUseProgram(_shaderProgram);
		glBindVertexArray(planeData.vao);
		glDrawElements(GL_TRIANGLES, planeData.indices.size(), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	}
}

void Renderer::ResetToStartValues() {
	_scale = 0.1f;
	_amplitude = 1.0f;
	_frequency = 1.0f;
	_octave = 5;
	_lacunarity = 2.0f;
	_persistance = 0.5f;
	_width = 1000;
	_height = 1000;
}

void Renderer::Render(ChunkManager& chunkManager) {
	glfwPollEvents();

	float timeValue = glfwGetTime();
	float deltaTime = timeValue - _prevTime;
	_prevTime = timeValue;
	float fps = 1 / deltaTime;
	_camera.HandleKeyboardInput(deltaTime, _window);
	_view = _camera.GetViewMatrix();
	glUniformMatrix4fv(_viewLoc, 1, GL_FALSE, glm::value_ptr(_view));
	//std::cout << "\rDelta Time: " << deltaTime << "s" << " | FPS: " << fps << std::flush;
	//std::cout << "FPS: " << fps << std::endl << std::flush;
	glUniform1f(_timeLocation, timeValue);

	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	ImGui::SetNextWindowSize(ImVec2(380, 300), 0);
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
	ImGui::SliderInt("ViewDistance", &_viewDistance, 0, 5);

	if (ImGui::Button("Regenerate Mesh")) {
		chunkManager.DestroyChunks();
		chunkManager.UpdateSettings(_scale, _amplitude, _frequency, _octave, _lacunarity, _persistance, _width, _height, _viewDistance);
	}
	if (ImGui::Button("Reset Settings")) {
		ResetToStartValues();
	}
	ImGui::Text("WASD to move  |  Space to ascend and ctrl to descend");

	ImGui::End();

	ImGui::Render();

	int display_w, display_h;
	glfwGetFramebufferSize(_window, &display_w, &display_h);
	glViewport(0, 0, display_w, display_h);
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	DrawChunks(chunkManager);

	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	glfwSwapBuffers(_window);
}

void Renderer::Cleanup(ChunkManager& chunkManager) {
	chunkManager.DestroyChunks();
	glDeleteProgram(_shaderProgram);
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	glfwDestroyWindow(_window);
	glfwTerminate();
}