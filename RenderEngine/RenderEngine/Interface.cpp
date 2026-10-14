#include "Interface.h"
#include <algorithm>
#include <cstdio>
#include <cstring>

// ============================================================================
//  Construction / theme / frame
// ============================================================================

Interface::Interface(GLFWwindow* window, Scene& scene, Camera* camera, KeybindManager* keybindManager,
                     InputManager* inputManager, AppSettings& settings)
    : window(window), scene(scene), camera(camera), keybindManager(keybindManager),
    inputManager(inputManager), settings(settings) {

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigDebugHighlightIdConflicts = false;

    ImGui::StyleColorsDark();
    applyTheme();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    // The context is OpenGL 3.3 core (see Engine::init), so ask for GLSL 330 core.
    ImGui_ImplOpenGL3_Init("#version 330 core");

    // Font Awesome is merged into BOTH fonts, so icons work in any widget
    // (it used to be merged only into the heading font: icons in normal
    // buttons rendered as '?').
    static const ImWchar icons_ranges[] = { 0xf000, 0xf3ff, 0 };
    ImFontConfig icons_config;
    icons_config.MergeMode = true;
    icons_config.PixelSnapH = true;
    auto mergeIcons = [&](float size) {
        icons_config.GlyphMinAdvanceX = size;
        return io.Fonts->AddFontFromFileTTF("assets/fonts/fa-solid-900.ttf", size, &icons_config, icons_ranges);
    };

    io.Fonts->AddFontDefault();
    iconFont = mergeIcons(13.0f);
    headingFont = io.Fonts->AddFontFromFileTTF("assets/fonts/ProggyVector-Regular.ttf", 18.0f);
    if (!headingFont) {
        LOG_WARN("Interface", "Failed to load assets/fonts/ProggyVector-Regular.ttf; using the default font.");
    } else {
        mergeIcons(16.0f);
    }
    if (!iconFont) {
        LOG_WARN("Interface", "Failed to load assets/fonts/fa-solid-900.ttf; toolbar icons will not display.");
    }
    ImGui_ImplOpenGL3_CreateFontsTexture();
    applyUiScale();

    defaultTexture = std::make_unique<Texture>("assets/textures/texture_08.png");
    if (!defaultTexture->isValid()) {
        LOG_WARN("Interface", "Default texture assets/textures/texture_08.png is missing; new primitives will use a flat colour.");
    }
    animationSequencer = std::make_unique<AnimationSequencer>();

    defaultKeybinds = keybindManager->getKeybinds(); // remembered for "Reset keybinds"
    currentScenePath.clear();
    markClean();
}

Interface::~Interface() {
    if (settingsDirty) settings.save();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void Interface::applyUiScale() {
    float dpi = 1.0f;
    if (GLFWmonitor* monitor = glfwGetPrimaryMonitor()) {
        float xscale = 1.0f, yscale = 1.0f;
        glfwGetMonitorContentScale(monitor, &xscale, &yscale);
        dpi = (xscale + yscale) / 2.0f;
    }
    ImGui::GetIO().FontGlobalScale = dpi * settings.uiScale;
}

void Interface::applyTheme() {
    ImGuiStyle& s = ImGui::GetStyle();
    s.WindowRounding = 4.0f;  s.ChildRounding = 4.0f;  s.FrameRounding = 3.0f;
    s.PopupRounding = 4.0f;   s.ScrollbarRounding = 6.0f; s.GrabRounding = 3.0f; s.TabRounding = 3.0f;
    s.WindowPadding = ImVec2(10, 10);  s.FramePadding = ImVec2(8, 4);
    s.ItemSpacing = ImVec2(8, 6);      s.ItemInnerSpacing = ImVec2(6, 4);
    s.WindowBorderSize = 1.0f;         s.FrameBorderSize = 0.0f;

    const ImVec4 accent(0.25f, 0.60f, 1.00f, 1.00f);
    const ImVec4 accentHover(0.35f, 0.68f, 1.00f, 1.00f);
    const ImVec4 accentDim(0.20f, 0.45f, 0.78f, 1.00f);
    ImVec4* c = s.Colors;
    c[ImGuiCol_WindowBg]         = ImVec4(0.105f, 0.115f, 0.135f, 0.97f);
    c[ImGuiCol_ChildBg]          = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    c[ImGuiCol_PopupBg]          = ImVec4(0.12f, 0.13f, 0.15f, 0.98f);
    c[ImGuiCol_Border]           = ImVec4(0.22f, 0.24f, 0.28f, 1.00f);
    c[ImGuiCol_FrameBg]          = ImVec4(0.16f, 0.18f, 0.21f, 1.00f);
    c[ImGuiCol_FrameBgHovered]   = ImVec4(0.21f, 0.24f, 0.28f, 1.00f);
    c[ImGuiCol_FrameBgActive]    = ImVec4(0.25f, 0.29f, 0.34f, 1.00f);
    c[ImGuiCol_TitleBg]          = ImVec4(0.09f, 0.10f, 0.12f, 1.00f);
    c[ImGuiCol_TitleBgActive]    = ImVec4(0.12f, 0.14f, 0.17f, 1.00f);
    c[ImGuiCol_MenuBarBg]        = ImVec4(0.09f, 0.10f, 0.12f, 1.00f);
    c[ImGuiCol_Header]           = ImVec4(accent.x, accent.y, accent.z, 0.28f);
    c[ImGuiCol_HeaderHovered]    = ImVec4(accent.x, accent.y, accent.z, 0.45f);
    c[ImGuiCol_HeaderActive]     = ImVec4(accent.x, accent.y, accent.z, 0.60f);
    c[ImGuiCol_Button]           = ImVec4(0.20f, 0.23f, 0.28f, 1.00f);
    c[ImGuiCol_ButtonHovered]    = accentDim;
    c[ImGuiCol_ButtonActive]     = accent;
    c[ImGuiCol_CheckMark]        = accent;
    c[ImGuiCol_SliderGrab]       = accentDim;
    c[ImGuiCol_SliderGrabActive] = accentHover;
    c[ImGuiCol_Separator]        = ImVec4(0.22f, 0.24f, 0.28f, 1.00f);
    c[ImGuiCol_ResizeGrip]       = ImVec4(accent.x, accent.y, accent.z, 0.25f);
    c[ImGuiCol_ResizeGripHovered]= ImVec4(accent.x, accent.y, accent.z, 0.60f);
    c[ImGuiCol_Tab]              = ImVec4(0.13f, 0.15f, 0.18f, 1.00f);
    c[ImGuiCol_TabHovered]       = accentDim;
    c[ImGuiCol_TextSelectedBg]   = ImVec4(accent.x, accent.y, accent.z, 0.35f);
    c[ImGuiCol_DragDropTarget]   = accentHover;
    c[ImGuiCol_NavHighlight]     = accent;
}

void Interface::pushHeading(const char* text) {
    ImGui::PushFont(headingFont); // NULL is allowed and means "default font"
    ImGui::TextUnformatted(text);
    ImGui::PopFont();
}

void Interface::tickAnimation(float deltaTime) {
    if (playing) {
        // Cap dt so a long stall (window drag, breakpoint) doesn't skip the whole timeline.
        frameAccumulator += std::min(deltaTime, 0.1f) * playbackFps;
        const int framesToAdvance = static_cast<int>(frameAccumulator);
        if (framesToAdvance > 0) {
            frameAccumulator -= static_cast<float>(framesToAdvance);
            animationSequencer->currentFrame += framesToAdvance;
            if (animationSequencer->currentFrame > animationSequencer->frameMax) {
                if (loopPlayback) {
                    animationSequencer->currentFrame = animationSequencer->frameMin;
                } else {
                    animationSequencer->currentFrame = animationSequencer->frameMax;
                    playing = false;
                }
            }
        }
    }
    animationSequencer->applyToObjects();
}

void Interface::update(float deltaTime) {
    pruneSelection();

    camera->smoothingEnabled = settings.cameraSmoothing;
    camera->smoothingSpeed = settings.cameraSmoothingSpeed;
    smoothedFps += ((deltaTime > 0.0f ? 1.0f / deltaTime : smoothedFps) - smoothedFps) * 0.05f;
    if (statusTimer > 0.0f) statusTimer -= deltaTime;
    refreshDirtyState(deltaTime);

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // Full-window dockspace; the centre stays transparent so the 3D view shows through.
    ImGuiWindowFlags windowFlags =
        ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos); // below the menu bar
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("MainDockspace", nullptr, windowFlags);
    ImGui::PopStyleVar(3);
    const ImGuiID dockspaceId = ImGui::GetID("MyDockspace");
    const ImGuiDockNode* rootNode = ImGui::DockBuilderGetNode(dockspaceId);
    // Rebuilt on first run, when imgui.ini has no layout, or when the panel set
    // changed since the saved layout (it would leave new panels floating).
    constexpr int kLayoutVersion = 2;
    if (resetLayoutRequested || !rootNode || rootNode->IsLeafNode() || settings.layoutVersion != kLayoutVersion) {
        settings.layoutVersion = kLayoutVersion;
        settingsDirty = true;
        buildDefaultLayout(dockspaceId);
        resetLayoutRequested = false;
    }
    ImGui::DockSpace(dockspaceId, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);
    if (const ImGuiDockNode* central = ImGui::DockBuilderGetCentralNode(dockspaceId)) viewArea = { central->Pos, central->Size };
    else viewArea = { viewport->WorkPos, viewport->WorkSize };
    ImGui::End();

    mainMenuBarUI();
    settingsUI();
    sceneControlsUI();

    // Play mode: the scene is being simulated and will be restored on Stop,
    // so editing it would be lost -- the editing panels are read-only.
    const bool playMode = isPlaying();
    if (playMode) ImGui::BeginDisabled();
    objectListUI();
    propertiesUI();
    timelineUI();
    if (playMode) ImGui::EndDisabled();
    scriptEditorUI();
    outputUI();

    updateFileBrowsers();
    unsavedPopup();
    aboutPopups();

    renderText();
    if (!playMode) renderGizmo();
    statsOverlay();
    statusOverlay();
    playOverlay();

    // Persist settings once the mouse is released (not on every slider tick).
    if (settingsDirty && !ImGui::IsMouseDown(0)) {
        settings.save();
        settingsDirty = false;
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

// ============================================================================
//  Status, dirty tracking, window title
// ============================================================================

void Interface::showStatus(const std::string& message, bool isError) {
    statusText = message;
    statusIsError = isError;
    statusTimer = isError ? 6.0f : 3.5f;
    if (isError) LOG_WARN("Interface", message); else LOG_INFO("Interface", message);
}

void Interface::markClean() {
    savedSnapshot = scene.toJsonString();
    dirtyCached = false;
    updateWindowTitle();
}

bool Interface::hasUnsavedChanges() const {
    return scene.toJsonString() != savedSnapshot;
}

void Interface::refreshDirtyState(float deltaTime) {
    if (isPlaying()) return; // the simulation moves things; that is not an unsaved edit
    dirtyTimer += deltaTime;
    if (dirtyTimer < 0.75f) return;
    dirtyTimer = 0.0f;
    // Serialising a huge scene every tick would stall the frame; above this
    // many objects the check only runs when actually needed (close/new/open).
    if (scene.objectCount() > 2000) return;
    const bool dirty = hasUnsavedChanges();
    if (dirty != dirtyCached) {
        dirtyCached = dirty;
        updateWindowTitle();
    }
}

void Interface::updateWindowTitle() {
    std::string name = "Untitled";
    if (!currentScenePath.empty()) {
        const size_t slash = currentScenePath.find_last_of("/\\");
        name = (slash == std::string::npos) ? currentScenePath : currentScenePath.substr(slash + 1);
    }
    const std::string title = "RIFT Engine - " + name + (dirtyCached ? "*" : "");
    if (title != shownTitle) {
        shownTitle = title;
        glfwSetWindowTitle(window, title.c_str());
    }
}

// ============================================================================
//  Scene file operations
// ============================================================================

void Interface::resetSceneState() {
    selectedObjects.clear();
    primaryObject = nullptr;
    textureTargetObject = nullptr;
    editBefore.clear();
    gizmoStartStates.clear();
    gizmoWasUsing = false;
    animationSequencer->clearItems();
    history.clear(); // commands may point at objects that no longer exist
}

bool Interface::doSave(const std::string& path) {
    if (!scene.saveToFile(path)) {
        showStatus("Could not save to " + path + " (see RenderEngine_log.txt)", true);
        return false;
    }
    currentScenePath = path;
    settings.lastScenePath = path;
    settingsDirty = true;
    markClean();
    showStatus("Saved " + path);
    return true;
}

bool Interface::doLoad(const std::string& path) {
    // loadFromFile leaves the current scene untouched on failure, so selection,
    // animation and undo history are only reset once it has succeeded.
    if (!scene.loadFromFile(path, defaultTexture.get())) {
        showStatus("Could not load " + path + " (see RenderEngine_log.txt)", true);
        return false;
    }
    resetSceneState();
    currentScenePath = path;
    settings.lastScenePath = path;
    settingsDirty = true;
    markClean();
    showStatus("Loaded " + path);
    return true;
}

void Interface::guardUnsaved(PendingAction action) {
    if (!hasUnsavedChanges()) { runPending(action); return; }
    pendingAction = action;
    openUnsavedPopup = true;
}

void Interface::runPending(PendingAction action) {
    switch (action) {
    case PendingAction::NewScene:
        scene.clear();
        resetSceneState();
        currentScenePath.clear();
        markClean();
        showStatus("New scene");
        break;
    case PendingAction::OpenScene: {
        IGFD::FileDialogConfig config;
        config.path = "assets/scenes";
        ImGuiFileDialog::Instance()->OpenDialog("LoadSceneDialog", "Open Scene", ".json", config);
        break;
    }
    case PendingAction::Exit:
        exitConfirmed = true;
        glfwSetWindowShouldClose(window, GLFW_TRUE);
        break;
    case PendingAction::None:
        break;
    }
}

void Interface::newScene()          { guardUnsaved(PendingAction::NewScene); }
void Interface::openLoadDialog()    { guardUnsaved(PendingAction::OpenScene); }

void Interface::openSaveDialog() {
    IGFD::FileDialogConfig config;
    config.path = "assets/scenes";
    config.flags = ImGuiFileDialogFlags_ConfirmOverwrite;
    ImGuiFileDialog::Instance()->OpenDialog("SaveSceneDialog", "Save Scene As", ".json", config);
}

void Interface::saveScene() {
    if (currentScenePath.empty()) openSaveDialog();
    else doSave(currentScenePath);
}

bool Interface::requestClose() {
    if (exitConfirmed || !hasUnsavedChanges()) return true;
    pendingAction = PendingAction::Exit;
    openUnsavedPopup = true;
    return false;
}

void Interface::updateFileBrowsers() {
    if (ImGuiFileDialog::Instance()->Display("ChooseTex")) {
        if (ImGuiFileDialog::Instance()->IsOk() && textureTargetObject && scene.contains(textureTargetObject)) {
            const std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
            auto texture = std::make_unique<Texture>(filePathName);
            if (texture->isValid()) {
                textureTargetObject->setOwnedTexture(std::move(texture)); // frees the previous owned texture
            } else {
                // The original replaced the texture with a dead one and leaked the old.
                showStatus("Could not load image: " + filePathName, true);
            }
        }
        ImGuiFileDialog::Instance()->Close();
        textureTargetObject = nullptr;
    }

    if (ImGuiFileDialog::Instance()->Display("SaveSceneDialog")) {
        if (ImGuiFileDialog::Instance()->IsOk()) {
            std::string filePath = ImGuiFileDialog::Instance()->GetFilePathName();
            if (filePath.size() < 5 || filePath.compare(filePath.size() - 5, 5, ".json") != 0) filePath += ".json";
            if (doSave(filePath) && pendingAction != PendingAction::None) {
                const PendingAction action = pendingAction; // "Save" from the unsaved-changes prompt
                pendingAction = PendingAction::None;
                runPending(action);
            }
        } else if (pendingAction != PendingAction::None) {
            pendingAction = PendingAction::None; // user cancelled the dialog -> cancel the pending action too
        }
        ImGuiFileDialog::Instance()->Close();
    }

    if (ImGuiFileDialog::Instance()->Display("LoadSceneDialog")) {
        if (ImGuiFileDialog::Instance()->IsOk()) {
            doLoad(ImGuiFileDialog::Instance()->GetFilePathName());
        }
        ImGuiFileDialog::Instance()->Close();
    }
}

void Interface::unsavedPopup() {
    if (openUnsavedPopup) {
        ImGui::OpenPopup("Unsaved changes");
        openUnsavedPopup = false;
    }
    ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    if (ImGui::BeginPopupModal("Unsaved changes", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("The scene has changes that have not been saved.");
        ImGui::Spacing();
        if (ImGui::Button("Save", ImVec2(110, 0))) {
            ImGui::CloseCurrentPopup();
            if (currentScenePath.empty()) {
                openSaveDialog(); // pendingAction resumes after the file name is chosen
            } else if (doSave(currentScenePath)) {
                const PendingAction action = pendingAction;
                pendingAction = PendingAction::None;
                runPending(action);
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Don't save", ImVec2(110, 0))) {
            ImGui::CloseCurrentPopup();
            const PendingAction action = pendingAction;
            pendingAction = PendingAction::None;
            if (action == PendingAction::NewScene) {
                scene.clear(); resetSceneState(); currentScenePath.clear(); markClean(); showStatus("New scene");
            } else {
                // Open / Exit: skip the dirty check we just answered.
                if (action == PendingAction::Exit) { exitConfirmed = true; glfwSetWindowShouldClose(window, GLFW_TRUE); }
                else runPending(action == PendingAction::OpenScene ? PendingAction::OpenScene : PendingAction::None);
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(110, 0))) {
            pendingAction = PendingAction::None;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

// ============================================================================
//  Selection
// ============================================================================

void Interface::clearSelection() {
    selectedObjects.clear();
    primaryObject = nullptr;
}

void Interface::selectObject(Object* object, bool appendSelection) {
    if (!object) return;
    if (!appendSelection) {
        selectedObjects.clear();
        selectedObjects.insert(object);
        primaryObject = object;
        return;
    }
    auto it = selectedObjects.find(object); // Ctrl/Shift-click toggles
    if (it != selectedObjects.end()) {
        selectedObjects.erase(it);
        if (primaryObject == object) primaryObject = selectedObjects.empty() ? nullptr : *selectedObjects.begin();
    } else {
        selectedObjects.insert(object);
        primaryObject = object;
    }
}

void Interface::selectAll() {
    selectedObjects.clear();
    for (Object* o : scene.getObjects()) selectedObjects.insert(o);
    primaryObject = selectedObjects.empty() ? nullptr : *selectedObjects.begin();
}

void Interface::pruneSelection() {
    // Defence in depth: whatever removed an object (delete, undo of an add,
    // loading a scene), nothing here may keep a pointer to it. The original
    // kept deleted children in the selection and the gizmo then dereferenced them.
    const std::vector<Object*> objects = scene.getObjects();
    const std::unordered_set<Object*> alive(objects.begin(), objects.end());
    for (auto it = selectedObjects.begin(); it != selectedObjects.end();) {
        if (!alive.count(*it)) it = selectedObjects.erase(it); else ++it;
    }
    if (primaryObject && !selectedObjects.count(primaryObject)) {
        primaryObject = selectedObjects.empty() ? nullptr : *selectedObjects.begin();
    }
    if (textureTargetObject && !alive.count(textureTargetObject)) textureTargetObject = nullptr;
    if (scriptTarget && !alive.count(scriptTarget)) scriptTarget = nullptr;
    for (auto it = editBefore.begin(); it != editBefore.end();) {
        if (!alive.count(it->first)) it = editBefore.erase(it); else ++it;
    }
}

void Interface::focusSelection() {
    if (!primaryObject) return;
    camera->focusOn(glm::vec3(primaryObject->getGlobalTransform()[3]));
}

// ============================================================================
//  Undoable actions
// ============================================================================

std::string Interface::makeUniqueName(const std::string& base) const {
    if (!scene.findByName(base)) return base;
    for (int n = 2; ; ++n) {
        const std::string candidate = base + " (" + std::to_string(n) + ")";
        if (!scene.findByName(candidate)) return candidate;
    }
}

void Interface::addPrimitive(const std::string& baseName, const std::vector<Vertex>& vertices) {
    auto object = std::make_unique<Object>(makeUniqueName(baseName), vertices, defaultTexture->isValid() ? defaultTexture.get() : nullptr);
    object->position = camera->target; // appears where the camera is looking, not buried at the origin
    Object* raw = object.get();
    history.execute(std::make_unique<AddObjectCommand>(scene, std::move(object), "Add " + baseName));
    selectObject(raw, false);
    showStatus("Added " + raw->name);
}

void Interface::duplicateObject() {
    if (selectedObjects.empty()) return;

    std::unordered_set<std::string> used;
    for (Object* o : scene.getObjects()) used.insert(o->name);

    std::vector<std::unique_ptr<Command>> commands;
    std::vector<Object*> created;
    for (Object* original : scene.getObjects()) { // scene order, so the copies keep it
        if (!selectedObjects.count(original)) continue;

        auto copy = std::make_unique<Object>(*original); // own GPU buffers; unparented, no children
        std::string name = original->name + " (Copy)";
        for (int n = 2; used.count(name); ++n) name = original->name + " (Copy " + std::to_string(n) + ")";
        used.insert(name);
        copy->setName(name);

        // The copy is not parented, so bake the original's world transform
        // into its local one or it would jump if the original had a parent.
        TransformUtil::decompose(original->getGlobalTransform(), copy->position, copy->rotation, copy->scale);

        created.push_back(copy.get());
        commands.push_back(std::make_unique<AddObjectCommand>(scene, std::move(copy), "Add " + name));
    }
    if (commands.empty()) return;

    const size_t count = created.size();
    history.execute(std::make_unique<CompositeCommand>(count == 1 ? "Duplicate " + created[0]->name : "Duplicate objects", std::move(commands)));
    selectedObjects.clear();
    for (Object* c : created) selectedObjects.insert(c);
    primaryObject = created.back();
    showStatus(count == 1 ? "Duplicated " + created[0]->name : "Duplicated " + std::to_string(count) + " objects");
}

void Interface::deleteObjects(const std::vector<Object*>& objects) {
    std::vector<std::unique_ptr<Command>> commands;
    std::string firstName;
    for (Object* o : objects) {
        if (!scene.contains(o)) continue;
        if (firstName.empty()) firstName = o->name;
        commands.push_back(std::make_unique<RemoveObjectCommand>(scene, *animationSequencer, o));
    }
    if (commands.empty()) return;

    const size_t count = commands.size();
    for (Object* o : objects) selectedObjects.erase(o);
    if (primaryObject && !selectedObjects.count(primaryObject)) {
        primaryObject = selectedObjects.empty() ? nullptr : *selectedObjects.begin();
    }
    // One command: children are detached, the objects (and their animation
    // items) stay owned by the command so Undo can bring everything back.
    history.execute(std::make_unique<CompositeCommand>(count == 1 ? "Delete " + firstName : "Delete objects", std::move(commands)));
    showStatus(count == 1 ? "Deleted " + firstName : "Deleted " + std::to_string(count) + " objects");
}

void Interface::deleteSelected() {
    std::vector<Object*> targets;
    for (Object* o : scene.getObjects()) {
        if (selectedObjects.count(o)) targets.push_back(o);
    }
    deleteObjects(targets);
}

void Interface::undo() {
    if (!history.canUndo()) { showStatus("Nothing to undo"); return; }
    const std::string what = history.peekUndoDescription();
    history.undo();
    showStatus("Undo: " + what);
}

void Interface::redo() {
    if (!history.canRedo()) { showStatus("Nothing to redo"); return; }
    const std::string what = history.peekRedoDescription();
    history.redo();
    showStatus("Redo: " + what);
}

ObjectState Interface::localStateFor(const Object& object, const Object* newParent) const {
    ObjectState state = ObjectState::capture(object);
    const glm::mat4 parentWorld = newParent ? newParent->getGlobalTransform() : glm::mat4(1.0f);
    if (std::abs(glm::determinant(parentWorld)) < 1e-12f) return state; // degenerate parent: keep the local values
    const glm::mat4 local = glm::inverse(parentWorld) * object.getGlobalTransform();
    TransformUtil::decompose(local, state.position, state.rotation, state.scale);
    return state;
}

void Interface::reparent(Object* object, Object* newParent) {
    if (!object || object == newParent || object->parent == newParent) return;
    if (newParent && newParent->isChildOf(object)) {
        showStatus("Cannot parent an object under one of its own children", true);
        return;
    }
    // The object stays where it is in the world (its local transform is
    // recomputed for the new parent) -- now that parenting is real at render
    // time, keeping the local values would make it jump.
    ObjectState before = ObjectState::capture(*object);
    ObjectState after = localStateFor(*object, newParent);
    history.execute(std::make_unique<ReparentCommand>(object, object->parent, newParent, before, after));
}

// ----------------------------------------------------------------------------
//  Property-edit undo tracking
// ----------------------------------------------------------------------------

void Interface::beginEdit(Object* object) {
    if (!editBefore.count(object)) editBefore[object] = ObjectState::capture(*object);
}

void Interface::endEdit(Object* object, const char* what) {
    auto it = editBefore.find(object);
    if (it == editBefore.end()) return;
    if (object->name.empty()) object->setName("Object"); // an empty tree label would be unselectable
    const ObjectState after = ObjectState::capture(*object);
    if (after != it->second) {
        history.execute(std::make_unique<ModifyObjectCommand>(object, it->second, after, std::string(what) + " " + object->name));
    }
    editBefore.erase(it);
}

void Interface::trackEdit(Object* object, const char* what) {
    if (ImGui::IsItemActivated()) beginEdit(object);
    if (ImGui::IsItemDeactivated()) endEdit(object, what);
}

// ============================================================================
//  Gizmo
// ============================================================================

void Interface::setGizmoOperation(ImGuizmo::OPERATION operation) { gizmoOperation = operation; }

void Interface::toggleGizmoSpace() {
    settings.gizmoLocalSpace = !settings.gizmoLocalSpace;
    settingsDirty = true;
    showStatus(settings.gizmoLocalSpace ? "Gizmo: local space" : "Gizmo: world space");
}

bool Interface::isGizmoActive() const {
    return primaryObject != nullptr && (ImGuizmo::IsOver() || ImGuizmo::IsUsing());
}

bool Interface::wantsTextInput() const { return ImGui::GetIO().WantTextInput; }

bool Interface::isModalUiOpen() const {
    return ImGui::IsPopupOpen(nullptr, ImGuiPopupFlags_AnyPopupId | ImGuiPopupFlags_AnyPopupLevel) || ImGuiFileDialog::Instance()->IsOpened();
}

void Interface::renderGizmo() {
    ImGuiIO& io = ImGui::GetIO();
    ImGuizmo::BeginFrame();
    ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);

    if (!primaryObject || selectedObjects.empty()) {
        gizmoWasUsing = false;
        gizmoStartStates.clear();
        return;
    }

    const glm::mat4 view = camera->getViewMatrix();
    const glm::mat4 projection = camera->getProjectionMatrix();

    // The gizmo sits on the primary object's GLOBAL transform. The original
    // gave every selected object its own overlapping gizmo and tried to push
    // changes down to children by hand (propagateTransform) because parenting
    // did nothing at render time. Hierarchy is now handled by the render path,
    // so one gizmo moves the whole selection and children simply follow.
    const glm::mat4 globalBefore = primaryObject->getGlobalTransform();
    glm::mat4 manipulated = globalBefore;

    float snap[3] = { 0, 0, 0 };
    const bool snapActive = settings.snapEnabled != io.KeyCtrl; // holding Ctrl inverts the snap setting
    if (gizmoOperation == ImGuizmo::TRANSLATE)   snap[0] = snap[1] = snap[2] = settings.snapTranslate;
    else if (gizmoOperation == ImGuizmo::ROTATE) snap[0] = snap[1] = snap[2] = settings.snapRotate;
    else                                         snap[0] = snap[1] = snap[2] = settings.snapScale;

    const bool changed = ImGuizmo::Manipulate(
        glm::value_ptr(view), glm::value_ptr(projection), gizmoOperation,
        settings.gizmoLocalSpace ? ImGuizmo::LOCAL : ImGuizmo::WORLD,
        glm::value_ptr(manipulated), nullptr, snapActive ? snap : nullptr);
    const bool usingNow = ImGuizmo::IsUsing();

    // Selected objects that have no selected ancestor; their descendants follow through the hierarchy.
    std::vector<Object*> roots;
    for (Object* o : selectedObjects) {
        bool ancestorSelected = false;
        for (const Object* p = o->parent; p; p = p->parent) {
            if (selectedObjects.count(const_cast<Object*>(p))) { ancestorSelected = true; break; }
        }
        if (!ancestorSelected) roots.push_back(o);
    }

    if (usingNow && !gizmoWasUsing) { // drag started: remember the "before" state for undo
        gizmoStartStates.clear();
        for (Object* o : roots) gizmoStartStates.emplace_back(o, ObjectState::capture(*o));
    }

    if (changed && std::abs(glm::determinant(globalBefore)) > 1e-12f) {
        const glm::mat4 delta = manipulated * glm::inverse(globalBefore); // change in world space
        for (Object* o : roots) {
            const glm::mat4 newGlobal = delta * o->getGlobalTransform();
            const glm::mat4 parentGlobal = o->parent ? o->parent->getGlobalTransform() : glm::mat4(1.0f);
            if (std::abs(glm::determinant(parentGlobal)) < 1e-12f) continue;
            TransformUtil::decompose(glm::inverse(parentGlobal) * newGlobal, o->position, o->rotation, o->scale);
        }
    }

    if (!usingNow && gizmoWasUsing) { // drag finished: one undo step for the whole drag
        std::vector<std::unique_ptr<Command>> commands;
        for (auto& entry : gizmoStartStates) {
            Object* o = entry.first;
            if (!scene.contains(o)) continue;
            const ObjectState after = ObjectState::capture(*o);
            if (after != entry.second) {
                commands.push_back(std::make_unique<ModifyObjectCommand>(o, entry.second, after, "Transform " + o->name));
            }
        }
        if (commands.size() == 1) history.execute(std::move(commands[0]));
        else if (commands.size() > 1) history.execute(std::make_unique<CompositeCommand>("Transform objects", std::move(commands)));
        gizmoStartStates.clear();
    }
    gizmoWasUsing = usingNow;
}

// ============================================================================
//  Panels
// ============================================================================

namespace {
    std::string keyName(int key) {
        switch (key) {
        case GLFW_KEY_DELETE: return "Del";   case GLFW_KEY_ESCAPE: return "Esc";
        case GLFW_KEY_ENTER: return "Enter";  case GLFW_KEY_TAB: return "Tab";
        case GLFW_KEY_SPACE: return "Space";  case GLFW_KEY_BACKSPACE: return "Backspace";
        case GLFW_KEY_UP: return "Up";        case GLFW_KEY_DOWN: return "Down";
        case GLFW_KEY_LEFT: return "Left";    case GLFW_KEY_RIGHT: return "Right";
        default: break;
        }
        if (key >= GLFW_KEY_F1 && key <= GLFW_KEY_F12) return "F" + std::to_string(key - GLFW_KEY_F1 + 1);
        if (const char* n = glfwGetKeyName(key, 0)) {
            std::string s = n;
            for (char& c : s) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
            return s;
        }
        return "Key " + std::to_string(key);
    }

    std::string comboToString(const KeyCombo& c) {
        std::string s;
        if (c.ctrl) s += "Ctrl+";
        if (c.shift) s += "Shift+";
        if (c.alt) s += "Alt+";
        return s + keyName(c.key);
    }

    // Shortcut text for a named keybind, so menus follow user rebinding.
    std::string shortcutFor(const KeybindManager* manager, const std::string& name) {
        for (const auto& kb : manager->getKeybinds()) {
            if (kb.name == name) return comboToString(kb.combo);
        }
        return "";
    }
}

void Interface::mainMenuBarUI() {
    if (!ImGui::BeginMainMenuBar()) return;

    if (ImGui::BeginMenu("File")) {
        if (ImGui::MenuItem("New", shortcutFor(keybindManager, "New Scene").c_str())) newScene();
        if (ImGui::MenuItem("Open...", shortcutFor(keybindManager, "Open Scene").c_str())) openLoadDialog();
        ImGui::Separator();
        if (ImGui::MenuItem("Save", shortcutFor(keybindManager, "Save").c_str())) saveScene();
        if (ImGui::MenuItem("Save As...", shortcutFor(keybindManager, "Save As").c_str())) openSaveDialog();
        ImGui::Separator();
        if (ImGui::MenuItem("Exit")) {
            if (requestClose()) glfwSetWindowShouldClose(window, GLFW_TRUE);
        }
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Edit")) {
        const std::string undoLabel = history.canUndo() ? "Undo " + history.peekUndoDescription() : "Undo";
        const std::string redoLabel = history.canRedo() ? "Redo " + history.peekRedoDescription() : "Redo";
        if (ImGui::MenuItem(undoLabel.c_str(), shortcutFor(keybindManager, "Undo").c_str(), false, history.canUndo())) undo();
        if (ImGui::MenuItem(redoLabel.c_str(), shortcutFor(keybindManager, "Redo").c_str(), false, history.canRedo())) redo();
        ImGui::Separator();
        const bool hasSelection = !selectedObjects.empty();
        if (ImGui::MenuItem("Duplicate", shortcutFor(keybindManager, "Duplicate Object").c_str(), false, hasSelection)) duplicateObject();
        if (ImGui::MenuItem("Delete", shortcutFor(keybindManager, "Delete Object").c_str(), false, hasSelection)) deleteSelected();
        if (ImGui::MenuItem("Select All", shortcutFor(keybindManager, "Select All").c_str())) selectAll();
        if (ImGui::MenuItem("Focus Selected", shortcutFor(keybindManager, "Focus Selected").c_str(), false, hasSelection)) focusSelection();
        ImGui::EndMenu();
    }

    const bool playMode = isPlaying();
    if (ImGui::BeginMenu("Insert", !playMode)) {
        if (ImGui::MenuItem("Part (Cube)"))  insertTemplate("Part");
        if (ImGui::MenuItem("Sphere"))       insertTemplate("Sphere");
        if (ImGui::MenuItem("Wedge (Triangle)")) insertTemplate("Wedge");
        ImGui::Separator();
        if (ImGui::MenuItem("Baseplate"))    insertTemplate("Baseplate");
        if (ImGui::MenuItem("Spawn Point"))  insertTemplate("SpawnPoint");
        ImGui::Separator();
        ImGui::TextDisabled("Scripted templates");
        if (ImGui::MenuItem("Kill Brick"))   insertTemplate("KillBrick");
        if (ImGui::MenuItem("Spinner"))      insertTemplate("Spinner");
        if (ImGui::MenuItem("Coin"))         insertTemplate("Coin");
        if (ImGui::MenuItem("Jump Pad"))     insertTemplate("JumpPad");
        if (ImGui::MenuItem("Falling Block")) insertTemplate("FallingBlock");
        if (ImGui::MenuItem("Finish Line"))  insertTemplate("Finish");
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Game")) {
        if (ImGui::MenuItem(playMode ? "Stop" : "Play", shortcutFor(keybindManager, "Play / Stop").c_str())) togglePlay();
        if (ImGui::MenuItem("Pause", nullptr, runtime && runtime->isPaused(), playMode)) runtime->togglePause();
        ImGui::Separator();
        ImGui::MenuItem("Output", nullptr, &showOutput);
        ImGui::MenuItem("Script Editor", nullptr, &showScriptEditor);
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("View")) {
        if (ImGui::MenuItem("Show Grid", nullptr, &settings.showGrid)) settingsDirty = true;
        if (ImGui::MenuItem("Wireframe", nullptr, &settings.wireframe)) settingsDirty = true;
        if (ImGui::MenuItem("Stats Overlay", nullptr, &settings.showStats)) settingsDirty = true;
        if (ImGui::MenuItem("Smooth Camera", nullptr, &settings.cameraSmoothing)) settingsDirty = true;
        if (ImGui::MenuItem("Reset Camera")) camera->reset();
        if (ImGui::MenuItem("Reset Layout")) resetLayoutRequested = true;
        ImGui::Separator();
        if (ImGui::MenuItem("Gizmo: Local Space", shortcutFor(keybindManager, "Toggle Gizmo Space").c_str(), &settings.gizmoLocalSpace)) settingsDirty = true;
        if (ImGui::MenuItem("Gizmo: Snapping", nullptr, &settings.snapEnabled)) settingsDirty = true;
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Help")) {
        if (ImGui::MenuItem("Keyboard Shortcuts")) showShortcuts = true;
        if (ImGui::MenuItem("About RIFT Engine")) showAbout = true;
        ImGui::EndMenu();
    }
    ImGui::EndMainMenuBar();
}

void Interface::aboutPopups() {
    if (showAbout) { ImGui::OpenPopup("About RIFT Engine"); showAbout = false; }
    if (showShortcuts) { ImGui::OpenPopup("Keyboard Shortcuts"); showShortcuts = false; }
    const ImVec2 center = ImGui::GetMainViewport()->GetCenter();

    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    if (ImGui::BeginPopupModal("About RIFT Engine", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        pushHeading("RIFT Engine");
        ImGui::Text("A small OpenGL scene editor: scene graph, gizmos, animation curves.");
        ImGui::Separator();
        ImGui::TextDisabled("Built with GLFW, GLEW, GLM, Dear ImGui, ImGuizmo, ImGuiFileDialog,");
        ImGui::TextDisabled("nlohmann/json and stb_image.");
        ImGui::Spacing();
        if (ImGui::Button("Close", ImVec2(100, 0))) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }

    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    if (ImGui::BeginPopupModal("Keyboard Shortcuts", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        pushHeading("Viewport");
        ImGui::BulletText("Right mouse drag: orbit        Middle mouse drag: pan");
        ImGui::BulletText("Mouse wheel: zoom              Left click: select");
        ImGui::BulletText("Ctrl/Shift + click: add to or remove from the selection");
        ImGui::Spacing();
        pushHeading("Keybinds (rebind them in Settings)");
        for (const auto& kb : keybindManager->getKeybinds()) {
            ImGui::BulletText("%-20s %s", kb.name.c_str(), comboToString(kb.combo).c_str());
        }
        ImGui::Spacing();
        if (ImGui::Button("Close", ImVec2(100, 0))) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }
}

void Interface::sceneControlsUI() {
    ImGui::Begin("Toolbar###Scene Controls");
    const bool toolbarLocked = isPlaying(); // captured: Play/Stop below changes isPlaying() mid-frame

    // Play / Stop -- the green/red button of the engine.
    {
        const bool playMode = isPlaying();
        const ImVec4 tint = playMode ? ImVec4(0.80f, 0.22f, 0.22f, 1.0f) : ImVec4(0.18f, 0.62f, 0.30f, 1.0f);
        ImGui::PushStyleColor(ImGuiCol_Button, tint);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(tint.x + 0.1f, tint.y + 0.1f, tint.z + 0.1f, 1.0f));
        if (ImGui::Button(playMode ? ICON_FA_STOP "  Stop" : ICON_FA_PLAY "  Play")) togglePlay();
        ImGui::PopStyleColor(2);
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s (%s)", playMode ? "Stop the game and restore the scene" : "Test the game: WASD + Space",
                                                      shortcutFor(keybindManager, "Play / Stop").c_str());
        if (playMode) {
            ImGui::SameLine();
            if (ImGui::Button(runtime->isPaused() ? ICON_FA_PLAY "  Resume" : ICON_FA_PAUSE "  Pause")) runtime->togglePause();
            ImGui::SameLine();
            ImGui::TextDisabled("Playing  %.1f s", runtime->elapsed());
        }
        if (toolbarLocked) ImGui::BeginDisabled();
        ImGui::SameLine();
    }

    if (ImGui::Button("Add Object")) ImGui::OpenPopup("Select Object Shape");
    showShapeSelectionPopup();
    ImGui::SameLine();
    if (ImGui::Button("Save")) saveScene();
    ImGui::SameLine();
    if (ImGui::Button("Open")) openLoadDialog();
    ImGui::SameLine();
    if (!history.canUndo()) ImGui::BeginDisabled();
    if (ImGui::Button("Undo")) undo();
    if (!history.canUndo()) ImGui::EndDisabled();
    ImGui::SameLine();
    if (!history.canRedo()) ImGui::BeginDisabled();
    if (ImGui::Button("Redo")) redo();
    if (!history.canRedo()) ImGui::EndDisabled();

    // One row: the toolbar is docked as a thin strip (see buildDefaultLayout).
    ImGui::SameLine(0.0f, 24.0f);

    const float buttonSize = ImGui::GetFrameHeight();
    const ImVec4 activeColor(0.25f, 0.60f, 1.00f, 1.00f);
    const ImVec4 hoveredColor(0.35f, 0.68f, 1.00f, 1.00f);
    const ImVec4 normalColor = ImGui::GetStyleColorVec4(ImGuiCol_Button);

    auto gizmoButton = [&](const char* icon, const char* tooltip, ImGuizmo::OPERATION op) {
        ImGui::PushStyleColor(ImGuiCol_Button, gizmoOperation == op ? activeColor : normalColor);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, hoveredColor);
        if (ImGui::Button(icon, ImVec2(buttonSize, buttonSize))) setGizmoOperation(op);
        ImGui::PopStyleColor(2);
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", tooltip);
        ImGui::SameLine();
    };
    gizmoButton(ICON_FA_ARROWS_ALT, "Translate", ImGuizmo::TRANSLATE);
    gizmoButton(ICON_FA_SYNC_ALT, "Rotate", ImGuizmo::ROTATE);
    gizmoButton(ICON_FA_EXPAND_ARROWS_ALT, "Scale", ImGuizmo::SCALE);

    // Same row as the gizmo buttons (the toolbar is often docked as a short strip), vertically centred.
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + (buttonSize - ImGui::GetFrameHeight()) * 0.5f);
    if (ImGui::Checkbox("Local", &settings.gizmoLocalSpace)) settingsDirty = true;
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Gizmo works in the object's local axes instead of world axes");
    ImGui::SameLine();
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + (buttonSize - ImGui::GetFrameHeight()) * 0.5f);
    if (ImGui::Checkbox("Snap", &settings.snapEnabled)) settingsDirty = true;
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Snap gizmo movement (hold Ctrl to invert). Step sizes are in Settings.");

    if (toolbarLocked) ImGui::EndDisabled();
    ImGui::End();
}

void Interface::showShapeSelectionPopup() {
    if (ImGui::BeginPopup("Select Object Shape")) {
        pushHeading("Choose shape");
        if (ImGui::Selectable("Cube"))     addPrimitive("Cube", createCubeVertices());
        if (ImGui::Selectable("Triangle")) addPrimitive("Triangle", createTriangle3DVertices());
        if (ImGui::Selectable("Sphere"))   addPrimitive("Sphere", createSphereVertices());
        ImGui::EndPopup();
    }
}

void Interface::objectListUI() {
    ImGui::Begin("Explorer###Object List");

    pushHeading("Objects");
    ImGui::SameLine();
    ImGui::TextDisabled("(%zu)", scene.objectCount());
    ImGui::Separator();

    const ImGuiTreeNodeFlags rootFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnArrow |
        ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;
    const bool open = ImGui::TreeNodeEx("Scene", rootFlags);

    // Dropping on "Scene" moves the object back to the root.
    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("OBJECT")) {
            Object* dropped = *static_cast<Object**>(payload->Data);
            if (scene.contains(dropped)) reparent(dropped, nullptr);
        }
        ImGui::EndDragDropTarget();
    }

    if (open) {
        for (Object* object : scene.getObjects()) { // snapshot: safe even if the tree edits the scene
            if (!object->parent) renderObjectNode(object);
        }
        ImGui::TreePop();
    }

    // Click on empty space deselects.
    if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(0) && !ImGui::IsAnyItemHovered()) clearSelection();

    ImGui::End();
}

void Interface::renderObjectNode(Object* object) {
    ImGui::PushID(object); // ids come from the pointer, not the (non-unique) name

    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick |
        ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_DefaultOpen;
    if (object->children.empty()) flags |= ImGuiTreeNodeFlags_Leaf;
    if (selectedObjects.count(object)) flags |= ImGuiTreeNodeFlags_Selected;

    const bool open = ImGui::TreeNodeEx(object->name.c_str(), flags);
    const ImGuiIO& io = ImGui::GetIO();

    if (ImGui::IsItemClicked(0) && !ImGui::IsItemToggledOpen()) {
        selectObject(object, io.KeyCtrl || io.KeyShift); // same modifiers as viewport picking
    }
    if (ImGui::IsItemClicked(1) && !selectedObjects.count(object)) selectObject(object, false);

    if (ImGui::BeginPopupContextItem("object_context")) {
        if (ImGui::MenuItem("Focus")) { selectObject(object, false); focusSelection(); }
        if (ImGui::MenuItem("Duplicate")) duplicateObject();
        if (ImGui::MenuItem("Unparent", nullptr, false, object->parent != nullptr)) reparent(object, nullptr);
        ImGui::Separator();
        if (ImGui::MenuItem("Delete")) deleteSelected();
        ImGui::EndPopup();
    }

    if (ImGui::BeginDragDropSource()) {
        ImGui::SetDragDropPayload("OBJECT", &object, sizeof(Object*));
        ImGui::Text("Move: %s", object->name.c_str());
        ImGui::EndDragDropSource();
    }

    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("OBJECT")) {
            Object* dropped = *static_cast<Object**>(payload->Data);
            // Invalid drops (onto itself / its own descendant) used to UNPARENT the
            // dropped object as a side effect; they are now simply refused.
            if (scene.contains(dropped)) {
                if (dropped == object || object->isChildOf(dropped)) showStatus("Cannot parent an object under itself or its own child", true);
                else reparent(dropped, object);
            }
        }
        ImGui::EndDragDropTarget();
    }

    if (open) {
        const std::vector<Object*> kids = object->children; // copy: a drop inside the loop edits this list
        for (Object* child : kids) renderObjectNode(child);
        ImGui::TreePop();
    }
    ImGui::PopID();
}

void Interface::propertiesUI() {
    ImGui::Begin("Properties");

    if (selectedObjects.empty()) {
        ImGui::TextDisabled("Nothing selected.");
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
        ImGui::TextWrapped("Click an object in the viewport or in the Explorer.");
        ImGui::PopStyleColor();
        ImGui::End();
        return;
    }

    if (selectedObjects.size() > 1) {
        ImGui::Text("%zu objects selected", selectedObjects.size());
        ImGui::Separator();
    }

    // Scene order; a snapshot, because the panel can trigger a delete.
    std::vector<Object*> ordered;
    for (Object* o : scene.getObjects()) {
        if (selectedObjects.count(o)) ordered.push_back(o);
    }
    for (Object* object : ordered) displayObjectProperties(object);

    ImGui::End();

    if (panelDeleteRequest) { // deferred so nothing above touches a removed object
        Object* target = panelDeleteRequest;
        panelDeleteRequest = nullptr;
        deleteObjects({ target });
    }
}

void Interface::displayObjectProperties(Object* object) {
    // THE multi-selection bug: this used to be called with index 0 for every
    // object (so all of them shared one ImGui ID scope) and kept the name in a
    // function-level `static` buffer shared by all objects. IDs now come from
    // the object pointer and there is no shared mutable state.
    ImGui::PushID(object);

    const bool single = selectedObjects.size() == 1;
    const bool expanded = single || ImGui::CollapsingHeader((object->name + "###header").c_str(), ImGuiTreeNodeFlags_DefaultOpen);

    if (expanded) {
        pushHeading("General");

        char nameBuf[128];
        std::snprintf(nameBuf, sizeof(nameBuf), "%s", object->name.c_str());
        if (ImGui::InputText("Name", nameBuf, sizeof(nameBuf))) object->setName(nameBuf);
        trackEdit(object, "Rename");

        // This line used to pass the parent Object* to a %s format (undefined behaviour).
        ImGui::Text("Parent: %s", object->parent ? object->parent->name.c_str() : "None");
        if (object->parent) {
            ImGui::SameLine();
            if (ImGui::SmallButton("Unparent")) reparent(object, nullptr);
        }

        ImGui::Separator();
        pushHeading("Transform");

        ImGui::DragFloat3("Position", glm::value_ptr(object->position), 0.05f);
        trackEdit(object, "Move");
        ImGui::DragFloat3("Rotation", glm::value_ptr(object->rotation), 0.5f, 0.0f, 0.0f, "%.1f");
        trackEdit(object, "Rotate");
        ImGui::DragFloat3("Scale", glm::value_ptr(object->scale), 0.01f, 0.001f, 1000.0f);
        trackEdit(object, "Scale");

        if (ImGui::SmallButton("Reset Transform")) {
            const ObjectState before = ObjectState::capture(*object);
            ObjectState after = before;
            after.position = glm::vec3(0.0f); after.rotation = glm::vec3(0.0f); after.scale = glm::vec3(1.0f);
            if (after != before) history.execute(std::make_unique<ModifyObjectCommand>(object, before, after, "Reset " + object->name));
        }

        const glm::mat4 world = object->getGlobalTransform();
        const glm::vec3 extent = object->getBoundsMax() - object->getBoundsMin();
        ImGui::TextDisabled("Size: %.2f x %.2f x %.2f", extent.x * glm::length(glm::vec3(world[0])),
            extent.y * glm::length(glm::vec3(world[1])), extent.z * glm::length(glm::vec3(world[2])));

        ImGui::Separator();
        pushHeading("Texture / Color");

        if (object->texture) ImGui::TextWrapped("Texture: %s", object->texture->getFilePath().c_str());
        else ImGui::TextDisabled("No texture (flat colour)");

        if (ImGui::Button("Choose Texture")) {
            IGFD::FileDialogConfig config;
            config.path = "assets/";
            ImGuiFileDialog::Instance()->OpenDialog("ChooseTex", "Select Texture",
                "All Supported{.png,.jpg,.jpeg,.bmp},.png,.jpg,.jpeg,.bmp", config);
            textureTargetObject = object;
        }
        if (object->texture) {
            ImGui::SameLine();
            if (ImGui::Button("Remove Texture")) object->removeTexture();
        } else {
            // This used a function-level `static glm::vec3` shared by every object, so
            // object B started from object A's colour. It edits the object directly now.
            ImGui::ColorEdit3("Color", glm::value_ptr(object->color));
            trackEdit(object, "Recolor");
        }

        ImGui::Separator();
        pushHeading("Actions");
        if (ImGui::Button("Animate Object")) {
            animationSequencer->addItemForObject(object, animationSequencer->frameMin, animationSequencer->frameMin + 30);
            showStatus("Added " + object->name + " to the timeline");
        }
        ImGui::SameLine();
        if (ImGui::Button("Duplicate")) duplicateObject();
        ImGui::SameLine();
        if (ImGui::Button("Delete")) panelDeleteRequest = object;

        ImGui::Separator();
        gamePropertiesUI(object);

        ImGui::Separator();
        pushHeading("Measurement Lines");
        ImGui::Checkbox("X-Axis", &object->showMeasureX);
        ImGui::SameLine();
        ImGui::Checkbox("Y-Axis", &object->showMeasureY);
        ImGui::SameLine();
        ImGui::Checkbox("Z-Axis", &object->showMeasureZ);
    }

    ImGui::PopID();
    if (!single) ImGui::Spacing();
}

void Interface::saveKeybindsToSettings() {
    settings.keybinds.clear();
    for (const auto& kb : keybindManager->getKeybinds()) {
        settings.keybinds.push_back({ kb.name, kb.combo.key, kb.combo.ctrl, kb.combo.shift, kb.combo.alt });
    }
    settingsDirty = true;
}

void Interface::applySavedKeybinds() {
    for (const auto& saved : settings.keybinds) {
        keybindManager->rebindKey(saved.name, KeyCombo{ saved.key, saved.ctrl, saved.shift, saved.alt }); // unknown names are ignored
    }
}

void Interface::settingsUI() {
    ImGui::Begin("Settings");

    pushHeading("View");
    if (ImGui::Checkbox("Show grid", &settings.showGrid)) settingsDirty = true;
    if (ImGui::Checkbox("Wireframe", &settings.wireframe)) settingsDirty = true;
    if (ImGui::Checkbox("Stats overlay", &settings.showStats)) settingsDirty = true;
    if (ImGui::Checkbox("VSync", &settings.vsync)) {
        glfwSwapInterval(settings.vsync ? 1 : 0);
        settingsDirty = true;
    }

    ImGui::Separator();
    pushHeading("Camera");
    if (ImGui::Checkbox("Smooth movement", &settings.cameraSmoothing)) settingsDirty = true;
    if (!settings.cameraSmoothing) ImGui::BeginDisabled();
    if (ImGui::SliderFloat("Smoothing speed", &settings.cameraSmoothingSpeed, 3.0f, 30.0f, "%.1f")) settingsDirty = true;
    if (!settings.cameraSmoothing) ImGui::EndDisabled();
    if (ImGui::Button("Reset camera")) camera->reset();

    ImGui::Separator();
    pushHeading("Gizmo");
    if (ImGui::Checkbox("Local space", &settings.gizmoLocalSpace)) settingsDirty = true;
    if (ImGui::Checkbox("Snapping", &settings.snapEnabled)) settingsDirty = true;
    if (ImGui::DragFloat("Move step", &settings.snapTranslate, 0.05f, 0.01f, 10.0f, "%.2f")) settingsDirty = true;
    if (ImGui::DragFloat("Rotate step (deg)", &settings.snapRotate, 0.5f, 1.0f, 90.0f, "%.0f")) settingsDirty = true;
    if (ImGui::DragFloat("Scale step", &settings.snapScale, 0.01f, 0.01f, 5.0f, "%.2f")) settingsDirty = true;

    ImGui::Separator();
    pushHeading("UI");
    if (ImGui::SliderFloat("UI scale", &settings.uiScale, 0.75f, 2.0f, "%.2f")) {
        applyUiScale();
        settingsDirty = true;
    }

    ImGui::Separator();
    pushHeading("Keybinds");
    ImGui::TextDisabled("Click a shortcut, then press the new keys (Esc cancels).");

    for (const auto& keybind : keybindManager->getKeybinds()) {
        ImGui::PushID(keybind.name.c_str());
        ImGui::Text("%s", keybind.name.c_str());
        ImGui::SameLine(ImGui::GetContentRegionAvail().x > 260.0f ? 190.0f * ImGui::GetIO().FontGlobalScale : 0.0f);
        if (ImGui::Button((comboToString(keybind.combo) + "###combo").c_str(), ImVec2(130, 0))) {
            waitingForRebind = keybind.name;
            rebindWarning.clear();
        }
        ImGui::PopID();
    }
    if (!rebindWarning.empty()) ImGui::TextColored(ImVec4(1.0f, 0.75f, 0.2f, 1.0f), "%s", rebindWarning.c_str());

    if (ImGui::Button("Reset keybinds")) {
        for (const auto& d : defaultKeybinds) keybindManager->rebindKey(d.name, d.combo);
        rebindWarning.clear();
        saveKeybindsToSettings();
    }

    if (!waitingForRebind.empty() && !ImGui::IsPopupOpen("RebindKeyPopup")) ImGui::OpenPopup("RebindKeyPopup");
    if (ImGui::BeginPopupModal("RebindKeyPopup", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Press the new shortcut for: %s", waitingForRebind.c_str());
        ImGui::TextDisabled("Hold Ctrl / Shift / Alt first if you want modifiers. Esc cancels.");

        if (inputManager->isKeyJustPressed(GLFW_KEY_ESCAPE)) {
            waitingForRebind.clear();
            ImGui::CloseCurrentPopup();
        } else {
            for (int key = GLFW_KEY_SPACE; key <= GLFW_KEY_LAST; ++key) {
                if (!inputManager->isKeyJustPressed(key)) continue;
                if (key == GLFW_KEY_LEFT_CONTROL || key == GLFW_KEY_RIGHT_CONTROL || key == GLFW_KEY_LEFT_SHIFT ||
                    key == GLFW_KEY_RIGHT_SHIFT || key == GLFW_KEY_LEFT_ALT || key == GLFW_KEY_RIGHT_ALT) continue;

                KeyCombo combo{ key, ImGui::GetIO().KeyCtrl, ImGui::GetIO().KeyShift, ImGui::GetIO().KeyAlt };
                // Two actions on one shortcut used to be silently allowed (both fired).
                const std::string conflict = keybindManager->rebindKey(waitingForRebind, combo);
                rebindWarning = conflict.empty() ? "" : "\"" + waitingForRebind + "\" now shares its shortcut with \"" + conflict + "\" -- both will fire.";
                saveKeybindsToSettings();
                waitingForRebind.clear();
                ImGui::CloseCurrentPopup();
                break;
            }
        }
        if (ImGui::Button("Cancel")) { waitingForRebind.clear(); ImGui::CloseCurrentPopup(); }
        ImGui::EndPopup();
    }

    ImGui::End();
}

void Interface::timelineUI() {
    ImGui::Begin("Timeline");

    if (ImGui::Button(playing ? "Pause" : "Play")) playing = !playing;
    ImGui::SameLine();
    if (ImGui::Button("Stop")) {
        playing = false;
        frameAccumulator = 0.0f;
        animationSequencer->currentFrame = animationSequencer->frameMin;
    }
    ImGui::SameLine();
    ImGui::Checkbox("Loop", &loopPlayback);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(70.0f);
    ImGui::DragFloat("FPS", &playbackFps, 0.5f, 1.0f, 120.0f, "%.0f");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(80.0f);
    ImGui::DragInt("Last frame", &animationSequencer->frameMax, 1.0f, animationSequencer->frameMin + 1, 100000);
    ImGui::SameLine();
    ImGui::Text("Frame %d / %d", animationSequencer->currentFrame, animationSequencer->frameMax);

    // Delete is enabled so items can be removed from the UI (there was no way
    // to remove one short of deleting its object). `firstFrame` is persistent
    // now -- as a per-frame local it reset the horizontal scroll every frame.
    ImSequencer::Sequencer(
        &animationSequencer->sequencer,
        &animationSequencer->currentFrame,
        &timelineExpanded, &timelineSelected, &timelineFirstFrame,
        ImSequencer::SEQUENCER_EDIT_STARTEND | ImSequencer::SEQUENCER_CHANGE_FRAME | ImSequencer::SEQUENCER_DEL);

    if (animationSequencer->items.empty()) {
        ImGui::TextDisabled("Select an object and press \"Animate Object\" in Properties to add it here.");
    } else {
        ImGui::TextDisabled("Double-click a row to edit its curves. Curves start flat at the object's current transform.");
    }
    ImGui::End();
}

void Interface::renderText() {
    const glm::mat4 viewProjection = camera->getProjectionMatrix() * camera->getViewMatrix();
    const ImVec2 displaySize = ImGui::GetIO().DisplaySize;
    for (const auto& dbg : debugTexts) {
        glm::vec4 clip = viewProjection * glm::vec4(dbg.worldPosition, 1.0f);
        if (clip.w <= 0.0f) continue; // behind the camera
        clip /= clip.w;
        if (clip.x < -1.0f || clip.x > 1.0f || clip.y < -1.0f || clip.y > 1.0f || clip.z < 0.0f || clip.z > 1.0f) continue;

        // screenOffset was computed by the engine but never applied, so the
        // three axis labels were drawn on top of each other.
        const float x = (clip.x + 1.0f) * 0.5f * displaySize.x + dbg.screenOffset.x;
        const float y = (1.0f - clip.y) * 0.5f * displaySize.y + dbg.screenOffset.y;
        ImGui::GetBackgroundDrawList()->AddText(ImVec2(x, y), ImColor(dbg.color), dbg.text.c_str());
    }
    debugTexts.clear();
}

void Interface::statsOverlay() {
    if (!settings.showStats) return;
    const ViewArea* vp = &viewArea;
    ImGui::SetNextWindowPos(ImVec2(vp->WorkPos.x + vp->WorkSize.x - 12.0f, vp->WorkPos.y + 12.0f), ImGuiCond_Always, ImVec2(1.0f, 0.0f));
    ImGui::SetNextWindowBgAlpha(0.55f);
    const ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoDocking;
    if (ImGui::Begin("##stats", nullptr, flags)) {
        ImGui::Text("%.0f FPS", smoothedFps);
        ImGui::Text("Objects: %zu", scene.objectCount());
        ImGui::Text("Selected: %zu", selectedObjects.size());
    }
    ImGui::End();
}

void Interface::statusOverlay() {
    if (statusTimer <= 0.0f || statusText.empty()) return;
    const ViewArea* vp = &viewArea;
    ImGui::SetNextWindowPos(ImVec2(vp->WorkPos.x + 12.0f, vp->WorkPos.y + vp->WorkSize.y - 12.0f), ImGuiCond_Always, ImVec2(0.0f, 1.0f));
    ImGui::SetNextWindowBgAlpha(0.70f * std::min(1.0f, statusTimer)); // fades during the last second
    const ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoDocking;
    if (ImGui::Begin("##status", nullptr, flags)) {
        const ImVec4 color = statusIsError ? ImVec4(1.0f, 0.45f, 0.40f, 1.0f) : ImVec4(0.85f, 0.92f, 1.0f, 1.0f);
        ImGui::TextColored(color, "%s", statusText.c_str());
    }
    ImGui::End();
}
