#include "Game.h"

Game::Game()
	: window(nullptr), width(1600), height(800), windowTitle("Control Simulator"), core(core)
{

}

Game::~Game() {
	cleanup();
}

///////////

bool Game::init() 
{
	if (!glfwInit()) {
		std::cerr << "Failed to init glfw!\n";
		return false;
	}

	window = glfwCreateWindow(width, height, windowTitle.c_str(), nullptr, nullptr);
	if (!window) {
		std::cerr << "Failed to create window!\n";
		glfwTerminate();
		return false;
	}

	glfwSetWindowUserPointer(window, this);
	//
	//

	core = new Core(0.0f,0.0f,0.1f,glm::vec3(0.1f,0.1f,0.1f));

	glfwMakeContextCurrent(window);
	if (glewInit() != GLEW_OK) {
		std::cerr << "Failed to init glew!\n";
		return false;
	}

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	// Optional style
	ImGui::StyleColorsDark();

	// Init ImGui for GLFW + OpenGL
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 130");

	return true;
}

void Game::run() {
	if (!init())
		return;

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		core->update();
		core->render();

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGui::Begin("Controls");

		ImGui::Text("Temperature: %.2f C",core->temperature);

		ImGui::End();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(window);
	}
	cleanup();
}

void Game::cleanup() {
	glfwDestroyWindow(window);
	glfwTerminate();
}
