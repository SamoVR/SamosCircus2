#include "Engine.h"

Engine::Engine()
    : window(nullptr), width(1600), height(1000), windowTitle("RIFT Engine")
{

}

Engine::~Engine()
{
    cleanup();
}

std::vector<glm::vec3> createCubeVertices() {
    return {
        // Front face
        {-0.5f, -0.5f,  0.5f},
        { 0.5f, -0.5f,  0.5f},
        { 0.5f,  0.5f,  0.5f},
        {-0.5f, -0.5f,  0.5f},
        { 0.5f,  0.5f,  0.5f},
        {-0.5f,  0.5f,  0.5f},

        // Back face
        {-0.5f, -0.5f, -0.5f},
        {-0.5f,  0.5f, -0.5f},
        { 0.5f,  0.5f, -0.5f},
        {-0.5f, -0.5f, -0.5f},
        { 0.5f,  0.5f, -0.5f},
        { 0.5f, -0.5f, -0.5f},

        // Left face
        {-0.5f, -0.5f, -0.5f},
        {-0.5f, -0.5f,  0.5f},
        {-0.5f,  0.5f,  0.5f},
        {-0.5f, -0.5f, -0.5f},
        {-0.5f,  0.5f,  0.5f},
        {-0.5f,  0.5f, -0.5f},

        // Right face
        {0.5f, -0.5f, -0.5f},
        {0.5f,  0.5f, -0.5f},
        {0.5f,  0.5f,  0.5f},
        {0.5f, -0.5f, -0.5f},
        {0.5f,  0.5f,  0.5f},
        {0.5f, -0.5f,  0.5f},

        // Top face
        {-0.5f,  0.5f, -0.5f},
        {-0.5f,  0.5f,  0.5f},
        { 0.5f,  0.5f,  0.5f},
        {-0.5f,  0.5f, -0.5f},
        { 0.5f,  0.5f,  0.5f},
        { 0.5f,  0.5f, -0.5f},

        // Bottom face
        {-0.5f, -0.5f, -0.5f},
        { 0.5f, -0.5f, -0.5f},
        { 0.5f, -0.5f,  0.5f},
        {-0.5f, -0.5f, -0.5f},
        { 0.5f, -0.5f,  0.5f},
        {-0.5f, -0.5f,  0.5f}
    };
}


void Engine::scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    // Get the engine instance back from the window's user pointer
    Engine* engine = static_cast<Engine*>(glfwGetWindowUserPointer(window));
    if (engine && engine->camera)
        engine->camera->handleScrollInput((float)yoffset);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

bool Engine::init()
{
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return false;
    }

    // Set OpenGL version and profile
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create window
    window = glfwCreateWindow(width, height, windowTitle.c_str(), nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return false;
    }

    glfwSetWindowUserPointer(window, this);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    glfwMakeContextCurrent(window);
    glewExperimental = GL_TRUE;

    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW\n";
        return false;
    }

    shader = new Shader("vertex.glsl", "fragment.glsl");
    camera = new Camera(width / (float)height);
    cube = new Object(createCubeVertices());

    glViewport(0, 0, width, height);
    glEnable(GL_DEPTH_TEST);

    return true;
}

void Engine::run()
{
    if (!init())
        return;

    while (!glfwWindowShouldClose(window)) {
        processInput();

        update();
        render();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    cleanup();
}

void Engine::processInput()
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);

        static double lastX = xpos, lastY = ypos;
        float deltaX = xpos - lastX;
        float deltaY = ypos - lastY;
        camera->handleMouseInput(deltaX, -deltaY, true); // Invert Y if needed
        lastX = xpos;
        lastY = ypos;
    }


}

void Engine::update()
{
    // Game logic, timing, animation etc
}

void Engine::render()
{
    glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    shader->use();
    shader->setMat4("view", camera->getViewMatrix());
    shader->setMat4("projection", camera->getProjectionMatrix());

    cube->draw(*shader);
}

void Engine::cleanup()
{
    glfwDestroyWindow(window);
    glfwTerminate();
}
