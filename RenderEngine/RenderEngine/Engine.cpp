#include "Engine.h"
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdio>
#include <limits>
#include <stdexcept>

Engine::Engine() {}

Engine::~Engine() {
    cleanup();
}

// ----------------------------------------------------------------------------
//  GLFW callbacks
// ----------------------------------------------------------------------------

void Engine::framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    // A minimised window reports 0x0; calling glViewport/aspect with that
    // divided by zero in the original.
    if (width <= 0 || height <= 0) return;
    glViewport(0, 0, width, height);

    Engine* engine = static_cast<Engine*>(glfwGetWindowUserPointer(window));
    if (engine && engine->camera) {
        engine->width = width;
        engine->height = height;
        engine->camera->setAspectRatio(static_cast<float>(width) / static_cast<float>(height));
    }
}

void Engine::scrollCallback(GLFWwindow* window, double /*xoffset*/, double yoffset) {
    Engine* engine = static_cast<Engine*>(glfwGetWindowUserPointer(window));
    if (!engine || !engine->camera) return;
    // Scrolling a panel (object list, timeline, ...) must not also zoom the camera.
    if (ImGui::GetCurrentContext() && ImGui::GetIO().WantCaptureMouse) return;
    engine->camera->handleScrollInput(static_cast<float>(yoffset));
}

static void setAppIcon(GLFWwindow* window) {
    GLFWimage icon;
    int channels;
    icon.pixels = stbi_load("assets/images/icon.png", &icon.width, &icon.height, &channels, 4);
    if (icon.pixels) {
        glfwSetWindowIcon(window, 1, &icon);
        stbi_image_free(icon.pixels);
    } else {
        LOG_WARN("Engine", "Failed to load window icon assets/images/icon.png");
    }
}

// ----------------------------------------------------------------------------
//  GL resources
// ----------------------------------------------------------------------------

void Engine::initLines() {
    glGenVertexArrays(1, &lineVAO);
    glGenBuffers(1, &lineVBO);

    glBindVertexArray(lineVAO);
    glBindBuffer(GL_ARRAY_BUFFER, lineVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * 2, nullptr, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glBindVertexArray(0);
}

void Engine::initGrid() {
    const int gridSize = 10;
    std::vector<glm::vec3> gridVertices;
    for (int i = -gridSize; i <= gridSize; ++i) {
        gridVertices.emplace_back(-gridSize, 0.0f, i); // parallel to X
        gridVertices.emplace_back(gridSize, 0.0f, i);
        gridVertices.emplace_back(i, 0.0f, -gridSize); // parallel to Z
        gridVertices.emplace_back(i, 0.0f, gridSize);
    }
    gridLineCount = static_cast<int>(gridVertices.size());

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
    const std::vector<Vertex> quadVertices = createFullScreenQuadVertices();

    glGenVertexArrays(1, &fullScreenQuadVAO);
    glGenBuffers(1, &fullScreenQuadVBO);
    glBindVertexArray(fullScreenQuadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, fullScreenQuadVBO);
    glBufferData(GL_ARRAY_BUFFER, quadVertices.size() * sizeof(Vertex), quadVertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoord));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);
}

void Engine::renderBackground() {
    if (!backgroundShader || !backgroundShader->isValid()) return;
    glDisable(GL_DEPTH_TEST); // the gradient sits behind everything

    backgroundShader->use();
    backgroundShader->setMat4("view", glm::mat4(1.0f));
    backgroundShader->setMat4("projection", glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f));

    glBindVertexArray(fullScreenQuadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    glEnable(GL_DEPTH_TEST);
}

void Engine::renderGrid() {
    if (!settings.showGrid || !gridShader || !gridShader->isValid()) return;

    gridShader->use();
    gridShader->setMat4("view", camera->getViewMatrix());
    gridShader->setMat4("projection", camera->getProjectionMatrix());
    gridShader->setMat4("model", glm::mat4(1.0f));

    glBindVertexArray(gridVAO);
    glDrawArrays(GL_LINES, 0, gridLineCount);
    glBindVertexArray(0);
}

void Engine::drawLine(const glm::vec3& start, const glm::vec3& end, const ImVec4& color) {
    if (!lineShader || !lineShader->isValid()) return;
    const glm::vec3 vertices[2] = { start, end };

    lineShader->use();
    lineShader->setMat4("view", camera->getViewMatrix());
    lineShader->setMat4("projection", camera->getProjectionMatrix());
    lineShader->setMat4("model", glm::mat4(1.0f));
    lineShader->setVec3("lineColor", glm::vec3(color.x, color.y, color.z)); // was ignored (shader was hardcoded white)

    glBindVertexArray(lineVAO);
    glBindBuffer(GL_ARRAY_BUFFER, lineVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
    glDrawArrays(GL_LINES, 0, 2);
    glBindVertexArray(0);
}

// Measurement lines now follow the object's real transform. The original built
// the line from `position` and `scale` only, so (a) it ignored rotation and
// parenting, and (b) the model matrix already contained the scale while the
// line length was multiplied by it again -- a 3x object got a 9-unit line
// labelled "3". The lines are now drawn along the mesh's own bounding-box
// edges transformed by the global matrix, and the label is the true world length.
void Engine::renderMeasurementLines(Object* object) {
    if (!object->showMeasureX && !object->showMeasureY && !object->showMeasureZ) return;

    const glm::mat4 world = object->getGlobalTransform();
    const glm::vec3 lo = object->getBoundsMin();
    const glm::vec3 hi = object->getBoundsMax();
    auto toWorld = [&world](const glm::vec3& p) { return glm::vec3(world * glm::vec4(p, 1.0f)); };

    const ImVec4 colorX(1.0f, 0.15f, 0.15f, 1.0f);
    const ImVec4 colorY(0.2f, 1.0f, 0.2f, 1.0f);
    const ImVec4 colorZ(0.3f, 0.5f, 1.0f, 1.0f);

    auto measure = [&](const glm::vec3& a, const glm::vec3& b, const ImVec4& color, ImVec2 labelOffset) {
        const glm::vec3 start = toWorld(a);
        const glm::vec3 end = toWorld(b);
        glDisable(GL_DEPTH_TEST); // visible through the object
        drawLine(start, end, color);
        glEnable(GL_DEPTH_TEST);
        char text[48];
        std::snprintf(text, sizeof(text), "%.2f m", glm::length(end - start));
        UI->debugTexts.push_back({ (start + end) * 0.5f, text, color, labelOffset });
    };

    if (object->showMeasureX) measure({ lo.x, lo.y, hi.z }, { hi.x, lo.y, hi.z }, colorX, ImVec2(0.0f, -15.0f));
    if (object->showMeasureY) measure({ lo.x, lo.y, hi.z }, { lo.x, hi.y, hi.z }, colorY, ImVec2(-45.0f, 0.0f));
    if (object->showMeasureZ) measure({ lo.x, lo.y, lo.z }, { lo.x, lo.y, hi.z }, colorZ, ImVec2(0.0f, 15.0f));
}

// ----------------------------------------------------------------------------
//  Picking
// ----------------------------------------------------------------------------

namespace {
    // Ray vs. axis-aligned box (slab method). Returns the distance along the ray.
    bool rayAabb(const glm::vec3& origin, const glm::vec3& dir, glm::vec3 lo, glm::vec3 hi, float& tHit) {
        lo -= glm::vec3(1e-3f); // flat meshes (zero thickness) must still be hittable
        hi += glm::vec3(1e-3f);
        float tMin = 0.0f, tMax = std::numeric_limits<float>::max();
        for (int axis = 0; axis < 3; ++axis) {
            if (std::abs(dir[axis]) < 1e-8f) {
                if (origin[axis] < lo[axis] || origin[axis] > hi[axis]) return false;
            } else {
                float t1 = (lo[axis] - origin[axis]) / dir[axis];
                float t2 = (hi[axis] - origin[axis]) / dir[axis];
                if (t1 > t2) std::swap(t1, t2);
                tMin = std::max(tMin, t1);
                tMax = std::min(tMax, t2);
                if (tMin > tMax) return false;
            }
        }
        tHit = tMin;
        return true;
    }
}

// Picking tests the ray against each object's real bounding box in its own
// local space (so it follows rotation, non-uniform scale and parenting). The
// original tested a sphere of radius |scale|/2 around `position`, which
// misses the corners of stretched objects and hits empty space around them.
void Engine::performObjectPicking(double mouseX, double mouseY) {
    // Mouse coordinates are in WINDOW units; the framebuffer can be larger on
    // scaled displays, so normalise by the window size, not the framebuffer's.
    int windowW = 0, windowH = 0;
    glfwGetWindowSize(window, &windowW, &windowH);
    if (windowW <= 0 || windowH <= 0) return;

    const float x = (2.0f * static_cast<float>(mouseX)) / static_cast<float>(windowW) - 1.0f;
    const float y = 1.0f - (2.0f * static_cast<float>(mouseY)) / static_cast<float>(windowH);

    const glm::mat4 invVP = glm::inverse(camera->getProjectionMatrix() * camera->getViewMatrix());
    glm::vec4 nearPoint = invVP * glm::vec4(x, y, -1.0f, 1.0f);
    glm::vec4 farPoint = invVP * glm::vec4(x, y, 1.0f, 1.0f);
    nearPoint /= nearPoint.w;
    farPoint /= farPoint.w;

    const glm::vec3 rayOrigin(nearPoint);
    const glm::vec3 rayDir = glm::normalize(glm::vec3(farPoint - nearPoint));

    Object* closest = nullptr;
    float closestDistance = std::numeric_limits<float>::max();

    for (Object* obj : scene.getObjects()) {
        const glm::mat4 world = obj->getGlobalTransform();
        if (std::abs(glm::determinant(world)) < 1e-12f) continue; // collapsed object: nothing to hit
        const glm::mat4 inv = glm::inverse(world);

        // Not re-normalised: t then measures the same distance along the world ray.
        const glm::vec3 localOrigin = glm::vec3(inv * glm::vec4(rayOrigin, 1.0f));
        const glm::vec3 localDir = glm::vec3(inv * glm::vec4(rayDir, 0.0f));

        float t;
        if (rayAabb(localOrigin, localDir, obj->getBoundsMin(), obj->getBoundsMax(), t) && t < closestDistance) {
            closestDistance = t;
            closest = obj;
        }
    }

    const ImGuiIO& io = ImGui::GetIO();
    const bool additive = io.KeyShift || io.KeyCtrl;
    if (closest) UI->selectObject(closest, additive);
    else if (!additive) UI->clearSelection(); // a missed shift-click no longer wipes the selection
}

// ----------------------------------------------------------------------------
//  Keybinds
// ----------------------------------------------------------------------------

void Engine::setupDefaultKeybinds() {
    // The names are used by Interface to show shortcuts in its menus.
    auto bind = [this](const char* name, int key, bool ctrl, bool shift, bool alt, std::function<void()> action) {
        keybindManager->registerKeybind(name, { key, ctrl, shift, alt }, action);
    };

    bind("New Scene",          GLFW_KEY_N, true,  false, false, [this]() { UI->newScene(); });
    bind("Open Scene",         GLFW_KEY_O, true,  false, false, [this]() { UI->openLoadDialog(); });
    bind("Save",               GLFW_KEY_S, true,  false, false, [this]() { UI->saveScene(); });
    bind("Save As",            GLFW_KEY_S, true,  true,  false, [this]() { UI->openSaveDialog(); });
    bind("Undo",               GLFW_KEY_Z, true,  false, false, [this]() { UI->undo(); });
    bind("Redo",               GLFW_KEY_Y, true,  false, false, [this]() { UI->redo(); });
    bind("Redo (Alt)",         GLFW_KEY_Z, true,  true,  false, [this]() { UI->redo(); });
    bind("Duplicate Object",   GLFW_KEY_D, true,  false, false, [this]() { UI->duplicateObject(); });
    bind("Delete Object",      GLFW_KEY_DELETE, false, false, false, [this]() { UI->deleteSelected(); });
    bind("Select All",         GLFW_KEY_A, true,  false, false, [this]() { UI->selectAll(); });
    bind("Focus Selected",     GLFW_KEY_PERIOD, false, false, false, [this]() { UI->focusSelection(); });
    bind("Translate Gizmo",    GLFW_KEY_T, false, false, false, [this]() { UI->setGizmoOperation(ImGuizmo::OPERATION::TRANSLATE); });
    bind("Rotate Gizmo",       GLFW_KEY_R, false, false, false, [this]() { UI->setGizmoOperation(ImGuizmo::OPERATION::ROTATE); });
    bind("Scale Gizmo",        GLFW_KEY_F, false, false, false, [this]() { UI->setGizmoOperation(ImGuizmo::OPERATION::SCALE); });
    bind("Toggle Gizmo Space", GLFW_KEY_X, false, false, false, [this]() { UI->toggleGizmoSpace(); });
    bind("Play / Stop",        GLFW_KEY_F5, false, false, false, [this]() { UI->togglePlay(); });
}

// ----------------------------------------------------------------------------
//  Init / loop / shutdown
// ----------------------------------------------------------------------------

bool Engine::init() {
    settings.load(); // first run: defaults

    if (!glfwInit()) {
        throw std::runtime_error("Failed to initialise GLFW.");
    }
    glfwStarted = true;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4); // MSAA: smooth object edges and grid lines

    window = glfwCreateWindow(width, height, windowTitle.c_str(), nullptr, nullptr);
    if (!window) {
        throw std::runtime_error("Failed to create the OpenGL 3.3 window. Update your graphics driver, or check that the GPU supports OpenGL 3.3.");
    }

    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
    glfwSetScrollCallback(window, scrollCallback); // ImGui's backend chains to this when it installs its own

    glfwMakeContextCurrent(window);
    glfwSwapInterval(settings.vsync ? 1 : 0); // was never set, leaving the frame rate to driver defaults

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        throw std::runtime_error("Failed to initialise GLEW.");
    }

    setAppIcon(window);

    int fbWidth = width, fbHeight = height;
    glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
    glViewport(0, 0, fbWidth, fbHeight);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);

    shader = std::make_unique<Shader>("vertex.glsl", "fragment.glsl");
    backgroundShader = std::make_unique<Shader>("background_vertex.glsl", "background_fragment.glsl");
    gridShader = std::make_unique<Shader>("grid_vertex.glsl", "grid_fragment.glsl");
    lineShader = std::make_unique<Shader>("line_vertex.glsl", "line_fragment.glsl");

    // Without the main shader nothing can be drawn. This used to fail
    // silently, leaving an empty-looking window with no explanation.
    if (!shader->isValid()) {
        throw std::runtime_error("The main shader (vertex.glsl / fragment.glsl) failed to load or compile.\nSee RenderEngine_log.txt for the GLSL error. The shader files must sit next to the executable.");
    }
    if (!backgroundShader->isValid() || !gridShader->isValid() || !lineShader->isValid()) {
        LOG_WARN("Engine", "One of the helper shaders (background / grid / line) failed to load; those elements will not be drawn.");
    }

    // Starting content. The cube owns its texture; with the scene being the
    // sole owner of objects, nothing here can leak.
    auto cubeTexture = std::make_unique<Texture>("assets/images/icon.png");
    scene.addObject(std::make_unique<Object>("Starting Cube", createCubeVertices(), std::move(cubeTexture)));

    auto marker = std::make_unique<Object>("target_marker", createCubeVertices(), static_cast<Texture*>(nullptr));
    marker->setScale(glm::vec3(0.1f));
    targetVisualizer = marker.get();
    scene.addInternalObject(std::move(marker));

    camera = std::make_unique<Camera>(static_cast<float>(fbWidth) / static_cast<float>(std::max(fbHeight, 1)));
    keybindManager = std::make_unique<KeybindManager>();
    inputManager = std::make_unique<InputManager>();
    setupDefaultKeybinds();

    UI = std::make_unique<Interface>(window, scene, camera.get(), keybindManager.get(), inputManager.get(), settings);
    UI->applySavedKeybinds(); // user rebinds from settings.json override the defaults

    runtime = std::make_unique<GameRuntime>(scene, *camera, *inputManager);
    UI->setRuntime(runtime.get());

    if (settings.discordRichPresence) discord.init(settings.discordClientId);

    initLines();
    initGrid();
    initFullScreenQuad();

    lastFrame = glfwGetTime(); // the first dt was measured from GLFW start-up (a huge value) before
    return true;
}

int Engine::run(const std::string& startupScene) {
    init();
    if (!startupScene.empty()) UI->openScene(startupScene);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents(); // first, so this frame already sees the latest input and close request

        // Close button / Alt+F4 / File > Exit: ask before discarding unsaved work.
        // A running game is stopped first, so the scene is back in its edited state.
        if (glfwWindowShouldClose(window) && UI->isPlaying()) UI->togglePlay();
        if (glfwWindowShouldClose(window) && !UI->requestClose()) {
            glfwSetWindowShouldClose(window, GLFW_FALSE);
        }

        int fbWidth = 0, fbHeight = 0;
        glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
        if (fbWidth == 0 || fbHeight == 0) { // minimised: don't burn a core drawing nothing
            glfwWaitEventsTimeout(0.1);
            lastFrame = glfwGetTime();
            continue;
        }

        const double now = glfwGetTime();
        const float deltaTime = std::min(static_cast<float>(now - lastFrame), 0.1f); // clamp stalls (window drag, breakpoint)
        lastFrame = now;

        update(deltaTime);
        render(deltaTime);
        glfwSwapBuffers(window);
    }

    cleanup();
    return 0;
}

void Engine::processInput() {
    const ImGuiIO& io = ImGui::GetIO();
    const bool overUI = io.WantCaptureMouse;

    // Esc no longer quits the application: it also cancels dialogs and key
    // rebinding, and closing is handled (with an unsaved-changes prompt) via
    // the window close button, Alt+F4 or File > Exit.

    // Orbit / pan only start in the 3D view, and continue until release.
    if (inputManager->isMouseButtonJustPressed(GLFW_MOUSE_BUTTON_RIGHT) && !overUI) orbiting = true;
    if (!inputManager->isMouseButtonPressed(GLFW_MOUSE_BUTTON_RIGHT)) orbiting = false;
    if (inputManager->isMouseButtonJustPressed(GLFW_MOUSE_BUTTON_MIDDLE) && !overUI) panning = true;
    if (!inputManager->isMouseButtonPressed(GLFW_MOUSE_BUTTON_MIDDLE)) panning = false;

    if (orbiting) {
        const glm::vec2 delta = inputManager->getMouseDelta();
        camera->handleMouseInput(delta.x, -delta.y, true, false);
    }
    if (panning) {
        const glm::vec2 delta = inputManager->getMouseDelta();
        camera->handleMouseInput(delta.x, -delta.y, false, true);
    }

    // Clicking a gizmo handle must not also pick (and deselect) what is behind it.
    // No picking while the game runs: the scene is not editable then.
    if (inputManager->isMouseButtonJustPressed(GLFW_MOUSE_BUTTON_LEFT) && !overUI && !UI->isGizmoActive() && !UI->isPlaying()) {
        const glm::vec2 mousePos = inputManager->getMousePosition();
        performObjectPicking(mousePos.x, mousePos.y);
    }
}

void Engine::update(float deltaTime) {
    // Input is sampled BEFORE it is used. The original processed input first, so
    // everything ran one frame late, and on frame 1 it queried "previous"
    // button state that did not exist yet (the first-frame crash).
    inputManager->update(window);
    processInput();

    // Shortcuts must not fire while typing in a text field, rebinding a key,
    // or with a dialog open (typing "R" in the name box switched the gizmo).
    const bool typing = UI->wantsTextInput() || UI->isCapturingKey() || UI->isModalUiOpen();
    if (UI->isPlaying()) {
        // In play mode WASD etc. belong to the game, so editor shortcuts are
        // off; only Play/Stop (F5) and Esc (stop) work.
        if (!typing && (inputManager->isKeyJustPressed(GLFW_KEY_F5) || inputManager->isKeyJustPressed(GLFW_KEY_ESCAPE))) {
            UI->togglePlay();
        }
    } else if (!typing) {
        keybindManager->update(inputManager.get());
    }

    if (UI->isPlaying()) runtime->update(deltaTime, !typing && !ImGui::GetIO().WantCaptureKeyboard);
    else UI->tickAnimation(deltaTime);
    camera->update(deltaTime);
    updatePresence(deltaTime);

    if (targetVisualizer) targetVisualizer->position = camera->target;
}

void Engine::render(float deltaTime) {
    glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    renderBackground();
    renderGrid();

    shader->use();
    shader->setMat4("view", camera->getViewMatrix());
    shader->setMat4("projection", camera->getProjectionMatrix());

    for (Object* obj : scene.getInternalObjects()) {
        if (UI->isPlaying()) break; // the orbit-target marker is an editor aid
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        obj->draw(*shader, *camera);
        glDisable(GL_BLEND);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    glPolygonMode(GL_FRONT_AND_BACK, settings.wireframe ? GL_LINE : GL_FILL);
    for (Object* obj : scene.getObjects()) {
        obj->draw(*shader, *camera);
    }
    if (runtime && runtime->player()) runtime->player()->draw(*shader, *camera);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    // Measurement lines last, in a separate pass: they switch shader programs,
    // which used to leave the line program bound for the next object's draw.
    for (Object* obj : scene.getObjects()) {
        renderMeasurementLines(obj);
    }

    UI->update(deltaTime);
}

void Engine::cleanup() {
    if (cleanedUp) return; // run() and ~Engine() both used to call this; the second window destroy was UB
    cleanedUp = true;

    // Order matters: everything that owns GL objects goes while the context is alive.
    if (window) glfwMakeContextCurrent(window);

    if (runtime) runtime->stop(); // restores the scene and frees the player's GL buffers
    runtime.reset();
    discord.shutdown();
    UI.reset();                 // ImGui backends + the UI's textures
    targetVisualizer = nullptr; // about to be destroyed with the scene
    scene.clearAll();           // every Object's VAO/VBO (user and internal)
    shader.reset();
    backgroundShader.reset();
    gridShader.reset();
    lineShader.reset();

    if (window) { // only valid with a live context (e.g. not if init() failed early)
        if (lineVAO) glDeleteVertexArrays(1, &lineVAO);
        if (lineVBO) glDeleteBuffers(1, &lineVBO);
        if (gridVAO) glDeleteVertexArrays(1, &gridVAO);
        if (gridVBO) glDeleteBuffers(1, &gridVBO);
        if (fullScreenQuadVAO) glDeleteVertexArrays(1, &fullScreenQuadVAO);
        if (fullScreenQuadVBO) glDeleteBuffers(1, &fullScreenQuadVBO);
    }
    lineVAO = lineVBO = gridVAO = gridVBO = fullScreenQuadVAO = fullScreenQuadVBO = 0;

    camera.reset();
    keybindManager.reset();
    inputManager.reset();

    if (window) {
        glfwDestroyWindow(window);
        window = nullptr;
    }
    if (glfwStarted) {
        glfwTerminate();
        glfwStarted = false;
    }
}

// ----------------------------------------------------------------------------
//  Discord Rich Presence
// ----------------------------------------------------------------------------

void Engine::updatePresence(float deltaTime) {
    presenceTimer -= deltaTime;
    if (presenceTimer <= 0.0f) {
        presenceTimer = 1.0f; // building the strings every frame is pointless

        std::string sceneName = "Untitled scene";
        const std::string& path = UI->scenePath();
        if (!path.empty()) {
            const size_t slash = path.find_last_of("/\\");
            sceneName = slash == std::string::npos ? path : path.substr(slash + 1);
        }
        const std::string details = (UI->isPlaying() ? "Playtesting " : "Building ") + sceneName;
        const std::string state = UI->isPlaying() ? "In play mode"
                                                  : std::to_string(scene.objectCount()) + " objects in scene";
        discord.setActivity(details, state);
    }
    discord.update(deltaTime);
}
