#include "Engine.h"

Engine::Engine()
    : window(nullptr), width(1600), height(800), windowTitle("RIFT Engine"), UI(nullptr), camera(nullptr), cube(nullptr), 
    backgroundShader(nullptr), fullScreenQuadVAO(0), fullScreenQuadVBO(0), gridShader(nullptr), gridVAO(0),
    gridVBO(0), inputManager(nullptr), keybindManager(nullptr), lastFrame(0), shader(nullptr), targetVisualizer(nullptr)
{

}

Engine::~Engine()
{
    cleanup();
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);

    // Update camera aspect ratio
    Engine* engine = static_cast<Engine*>(glfwGetWindowUserPointer(window));
    if (engine && engine->camera) {
        engine->width = width;
        engine->height = height;
        engine->camera->setAspectRatio(static_cast<float>(width) / height);
    }
}

void Engine::scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    // Get the engine instance back from the window's user pointer
    Engine* engine = static_cast<Engine*>(glfwGetWindowUserPointer(window));
    if (engine && engine->camera)
        engine->camera->handleScrollInput((float)yoffset);
}

void setAppIcon(GLFWwindow* window)
{
    GLFWimage icon;
    int channels;
    icon.pixels = stbi_load("assets/images/icon.png", &icon.width, &icon.height, &channels, 4);
    if (icon.pixels) {
        glfwSetWindowIcon(window, 1, &icon);
        stbi_image_free(icon.pixels);
    }
    else {
        std::cerr << "Failed to load window icon\n";
    }
}

void Engine::initGrid() {
    const int gridSize = 10;
    std::vector<glm::vec3> gridVertices;

    for (int i = -gridSize; i <= gridSize; ++i) {
        // Lines parallel to X axis
        gridVertices.emplace_back(-gridSize, 0.0f, i);
        gridVertices.emplace_back(gridSize, 0.0f, i);

        // Lines parallel to Z axis
        gridVertices.emplace_back(i, 0.0f, -gridSize);
        gridVertices.emplace_back(i, 0.0f, gridSize);
    }

    gridLineCount = gridVertices.size();

    glGenVertexArrays(1, &gridVAO);
    glGenBuffers(1, &gridVBO);

    glBindVertexArray(gridVAO);
    glBindBuffer(GL_ARRAY_BUFFER, gridVBO);
    glBufferData(GL_ARRAY_BUFFER, gridVertices.size() * sizeof(glm::vec3), gridVertices.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glBindVertexArray(0);
}

void Engine::initFullScreenQuad() {
    std::vector<Vertex> quadVertices = createFullScreenQuadVertices();

    // Create the VAO and VBO for the full-screen quad
    glGenVertexArrays(1, &fullScreenQuadVAO);
    glGenBuffers(1, &fullScreenQuadVBO);

    glBindVertexArray(fullScreenQuadVAO);

    glBindBuffer(GL_ARRAY_BUFFER, fullScreenQuadVBO);
    glBufferData(GL_ARRAY_BUFFER, quadVertices.size() * sizeof(Vertex), &quadVertices[0], GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);

    // Texture coordinate attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(glm::vec3)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);  // Unbind VAO
}

void Engine::renderBackground() {
    glDisable(GL_DEPTH_TEST);  // Disable depth testing to render background behind other objects

    backgroundShader->use();  // Use the shader for the background gradient
    backgroundShader->setMat4("view", glm::mat4(1.0f));  // Identity view matrix (no camera)
    backgroundShader->setMat4("projection", glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f));  // 2D orthographic projection

    // Bind the full-screen quad VAO and draw it
    glBindVertexArray(fullScreenQuadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);  // Draw the quad with the gradient

    glEnable(GL_DEPTH_TEST);  // Re-enable depth testing for 3D rendering
}

void Engine::renderGrid() {

    gridShader->use();
    gridShader->setMat4("view", camera->getViewMatrix());
    gridShader->setMat4("projection", camera->getProjectionMatrix());
    gridShader->setMat4("model", glm::mat4(1.0f));

    glBindVertexArray(gridVAO);
    glDrawArrays(GL_LINES, 0, gridLineCount);
    glBindVertexArray(0);

}

void Engine::performObjectPicking(double mouseX, double mouseY)
{
    // Normalize mouse to [-1, 1]
    float x = (2.0f * mouseX) / width - 1.0f;
    float y = 1.0f - (2.0f * mouseY) / height;
    glm::vec4 rayClip(x, y, -1.0f, 1.0f);

    // Inverse Projection * Clip -> Eye space
    glm::mat4 proj = camera->getProjectionMatrix();
    glm::mat4 view = camera->getViewMatrix();
    glm::mat4 invVP = glm::inverse(proj * view);

    // Create ray start and end in NDC
    glm::vec4 rayStartNDC(x, y, -1.0f, 1.0f);
    glm::vec4 rayEndNDC(x, y, 0.0f, 1.0f);

    // Unproject to world space
    glm::vec4 rayStartWorld = invVP * rayStartNDC;
    glm::vec4 rayEndWorld = invVP * rayEndNDC;
    rayStartWorld /= rayStartWorld.w;
    rayEndWorld /= rayEndWorld.w;

    glm::vec3 rayOrigin = glm::vec3(rayStartWorld);
    glm::vec3 rayDirection = glm::normalize(glm::vec3(rayEndWorld - rayStartWorld));

    Object* closestObject = nullptr;
    float closestDistance = std::numeric_limits<float>::max();

    for (auto* obj : scene.getObjects()) {
        float distance;

        // Basic sphere bounds
        float radius = glm::length(obj->scale) * 0.5f;
        glm::vec3 center = obj->position;

        glm::vec3 oc = rayOrigin - center;
        float b = glm::dot(oc, rayDirection);
        float c = glm::dot(oc, oc) - radius * radius;
        float h = b * b - c;
        if (h < 0.0f) continue;

        h = sqrt(h);
        distance = -b - h;

        if (distance > 0.0f && distance < closestDistance) {
            closestDistance = distance;
            closestObject = obj;
        }
    }

    if (closestObject) {
        bool shiftPressed = ImGui::GetIO().KeyShift;
        UI->selectObject(closestObject, shiftPressed);
    }
    else {
        UI->clearSelection();
    }
}

void Engine::setupDefaultKeybinds() {
    keybindManager->registerKeybind("Open Save Dialog", { GLFW_KEY_S, true, false, false }, [this]() {
        UI->openSaveDialog();
        });

    keybindManager->registerKeybind("Duplicate Object", { GLFW_KEY_D, true, false, false }, [this]() {
        UI->duplicateObject();
        });

    keybindManager->registerKeybind("Translate Gizmo", { GLFW_KEY_T, false, false, false }, [this]() {
        UI->setGizmoOperation(ImGuizmo::OPERATION::TRANSLATE);
        });

    keybindManager->registerKeybind("Rotate Gizmo", { GLFW_KEY_R, false, false, false }, [this]() {
        UI->setGizmoOperation(ImGuizmo::OPERATION::ROTATE);
        });

    keybindManager->registerKeybind("Scale Gizmo", { GLFW_KEY_F, false, false, false }, [this]() {
        UI->setGizmoOperation(ImGuizmo::OPERATION::SCALE);
        });
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

    glfwSetWindowUserPointer(window, this); //
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetScrollCallback(window, scroll_callback);

    glfwMakeContextCurrent(window);
    glewExperimental = GL_TRUE;

    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW\n";
        return false;
    }

    setAppIcon(window);

    glViewport(0, 0, width, height);
    glEnable(GL_DEPTH_TEST);

    shader = new Shader("vertex.glsl", "fragment.glsl");
    backgroundShader = new Shader("background_vertex.glsl", "background_fragment.glsl");
    gridShader = new Shader("grid_vertex.glsl", "grid_fragment.glsl");

    Texture* cubeTexture = new Texture("assets/images/icon.png");
    cube = new Object("Starting Cube", createCubeVertices(), cubeTexture);
    scene.addObject(cube);

    targetVisualizer = new Object("target_marker", createCubeVertices(), nullptr);
    targetVisualizer->setScale(glm::vec3(0.1f)); // Make it small
    scene.addInternalObject(targetVisualizer);

    camera = new Camera(width / static_cast<float>(height));
    keybindManager = new KeybindManager();
    inputManager = new InputManager();
    setupDefaultKeybinds();

    UI = new Interface(window, scene, camera, keybindManager, inputManager);

    initGrid();
    initFullScreenQuad();

    return true;
}

void Engine::run()
{
    if (!init())
        return;

    while (!glfwWindowShouldClose(window)) {
        float currentFrame = glfwGetTime();
        float deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        update(deltaTime);
        render(deltaTime);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    cleanup();
}

void Engine::processInput() {
    if (inputManager->isKeyPressed(GLFW_KEY_ESCAPE)) {
        glfwSetWindowShouldClose(window, true);
    }

    if (inputManager->isMouseButtonPressed(GLFW_MOUSE_BUTTON_RIGHT)) {
        glm::vec2 delta = inputManager->getMouseDelta();
        camera->handleMouseInput(delta.x, -delta.y, true, false);
    }

    if (inputManager->isMouseButtonPressed(GLFW_MOUSE_BUTTON_MIDDLE)) {
        glm::vec2 delta = inputManager->getMouseDelta();
        camera->handleMouseInput(delta.x, -delta.y, false, true);
    }

    if (inputManager->isMouseButtonJustPressed(GLFW_MOUSE_BUTTON_LEFT) && !ImGui::GetIO().WantCaptureMouse) {
        glm::vec2 mousePos = inputManager->getMousePosition();
        performObjectPicking(mousePos.x, mousePos.y);
    }

}

void Engine::update(float deltaTime)
{
    processInput();

    inputManager->update(window);
    keybindManager->update(inputManager);

    if (targetVisualizer && camera) {
        targetVisualizer->position = camera->target;
    }

}

void Engine::render(float deltaTime)
{
    glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    renderBackground();

    renderGrid();

    shader->use();
    shader->setMat4("view", camera->getViewMatrix());
    shader->setMat4("projection", camera->getProjectionMatrix());

    for (auto* obj : scene.getInternalObjects()) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // Same handling for internal objects (checking texture or color)
        if (!obj->texture)
            shader->setVec3("color", obj->color);
            
        obj->draw(*shader, *camera);  // Use the color instead

        glDisable(GL_BLEND);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    for (auto* obj : scene.getObjects()) {
        // Check if the object has a texture
        if (!obj->texture)
            shader->setVec3("color", obj->color);
        obj->draw(*shader, *camera);
    }
    
    UI->update(deltaTime);

}

void Engine::cleanup()
{
    glfwDestroyWindow(window);
    glfwTerminate();
}
