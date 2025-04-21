#include "Interface.h"

Interface::Interface(GLFWwindow* window, Scene& scene, Camera* camera, KeybindManager* keybindManager, InputManager* inputManager)
    : window(window), scene(scene), camera(camera), keybindManager(keybindManager), inputManager(inputManager), defaultTexture(new Texture("assets/textures/texture_08.png")) {

    // ImGui Setup
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigDebugHighlightIdConflicts = false;

    // Get the monitor's DPI scaling factor
    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    float xscale, yscale;
    glfwGetMonitorContentScale(monitor, &xscale, &yscale);

    // Adjust font scaling based on DPI scale (average scale factor for both axes)
    io.FontGlobalScale = (xscale + yscale) / 2.0f;

    // Load font
    io.Fonts->AddFontDefault();
    headingFont = io.Fonts->AddFontFromFileTTF("assets/fonts/ProggyVector-Regular.ttf", 18.0f);
    if (!headingFont) {
        std::cerr << "Failed to load heading font." << std::endl;
    }

    static const ImWchar icons_ranges[] = { 0xf000, 0xf3ff, 0 };  // Adjust range if using newer FA sets
    ImFontConfig icons_config;
    icons_config.MergeMode = true;
    icons_config.PixelSnapH = true;
    icons_config.GlyphMinAdvanceX = 16.0f; // Optional

    iconFont = io.Fonts->AddFontFromFileTTF("assets/fonts/fa-solid-900.ttf", 16.0f, &icons_config, icons_ranges);

    ImGui_ImplOpenGL3_CreateFontsTexture();
}

Interface::~Interface() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void Interface::update() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // DockSpace
    ImGuiWindowFlags windowFlags =
        ImGuiWindowFlags_MenuBar |
        ImGuiWindowFlags_NoDocking |
        ImGuiWindowFlags_NoBackground |
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoBringToFrontOnFocus |
        ImGuiWindowFlags_NoNavFocus;

    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);
    ImGui::SetNextWindowViewport(viewport->ID);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);


    ImGui::Begin("MainDockspace", nullptr, windowFlags);
    ImGui::PopStyleVar(1);

    ImGuiID dockspace_id = ImGui::GetID("MyDockspace");

    ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode; // | ImGuiDockNodeFlags_NoDockingInCentralNode

    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
    ImGui::End();

    debugUI();
    settingsUI();
    sceneControlsUI();
    objectListUI();
    propertiesUI();

    updateFileBrowsers();

    renderGizmo();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Interface::updateFileBrowsers()
{
    if (ImGuiFileDialog::Instance()->Display("ChooseTex")) {
        if (ImGuiFileDialog::Instance()->IsOk() && textureTargetObject) {
            std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
            textureTargetObject->setTexture(new Texture(filePathName));
        }
        ImGuiFileDialog::Instance()->Close();
        textureTargetObject = nullptr;
    }
    
    if (ImGuiFileDialog::Instance()->Display("SaveSceneDialog")) {
        if (ImGuiFileDialog::Instance()->IsOk()) {
            std::string filePath = ImGuiFileDialog::Instance()->GetFilePathName();
            scene.saveToFile(filePath);
        }
        ImGuiFileDialog::Instance()->Close();
    }

    if (ImGuiFileDialog::Instance()->Display("LoadSceneDialog")) {
        if (ImGuiFileDialog::Instance()->IsOk()) {
            std::string filePath = ImGuiFileDialog::Instance()->GetFilePathName();
            selectedObjects = {};
            scene.loadFromFile(filePath);
        }
        ImGuiFileDialog::Instance()->Close();
    }

}

void Interface::settingsUI() {
    ImGui::Begin("Settings");

    ImGui::PushFont(headingFont);
    ImGui::Text("Camera");
    ImGui::PopFont();

    ImGui::Separator();

    ImGui::PushFont(headingFont);
    ImGui::Text("UI");
    ImGui::PopFont();

    ImGui::Separator();

    ImGui::PushFont(headingFont);
    ImGui::Text("Keybinds");
    ImGui::PopFont();

    static std::string waitingForRebind = "";
    ImGui::Separator();

    for (auto& keybind : keybindManager->getKeybinds()) {
        ImGui::Text("%s", keybind.name.c_str());
        ImGui::SameLine();

        std::string comboStr = "";
        if (keybind.combo.ctrl) comboStr += "Ctrl + ";
        if (keybind.combo.shift) comboStr += "Shift + ";
        if (keybind.combo.alt) comboStr += "Alt + ";
        const char* keyName = glfwGetKeyName(keybind.combo.key, 0);
        if (keyName)
            comboStr += keyName;
        else
            comboStr += "Key " + std::to_string(keybind.combo.key); // fallback


        if (ImGui::Button((comboStr + "##" + keybind.name).c_str())) {
            waitingForRebind = keybind.name;
        }
    }

    if (!waitingForRebind.empty()) {
        ImGui::OpenPopup("RebindKeyPopup");
    }

    if (ImGui::BeginPopup("RebindKeyPopup")) {
        ImGui::Text("Press new key for: %s", waitingForRebind.c_str());
        ImGui::Text("Hold modifiers (Ctrl, Shift, Alt)");

        for (int key = GLFW_KEY_SPACE; key <= GLFW_KEY_LAST; ++key) {
            if (inputManager->isKeyJustPressed(key)) {
                // Skip modifier-only keys
                if (key == GLFW_KEY_LEFT_CONTROL || key == GLFW_KEY_RIGHT_CONTROL ||
                    key == GLFW_KEY_LEFT_SHIFT || key == GLFW_KEY_RIGHT_SHIFT ||
                    key == GLFW_KEY_LEFT_ALT || key == GLFW_KEY_RIGHT_ALT)
                    continue;

                // Now we process a "real" key while modifiers are held
                KeyCombo newCombo;
                newCombo.key = key;
                newCombo.ctrl = ImGui::GetIO().KeyCtrl;
                newCombo.shift = ImGui::GetIO().KeyShift;
                newCombo.alt = ImGui::GetIO().KeyAlt;

                keybindManager->rebindKey(waitingForRebind, newCombo);
                waitingForRebind = "";
                ImGui::CloseCurrentPopup();
                break;
            }
        }


        if (ImGui::Button("Cancel")) {
            waitingForRebind = "";
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    ImGui::End();
}


void Interface::propertiesUI() {
    if (selectedObjects.empty()) return;

    ImGui::Begin("Properties");

    ImGui::PushFont(headingFont);
    ImGui::Text("Object Properties");
    ImGui::PopFont();

    ImGui::Separator();

    // We use a temporary list of objects to prevent modifying the selectedObjects set
    // while iterating over it.
    std::vector<Object*> objectsToDisplay(selectedObjects.begin(), selectedObjects.end());

    for (Object* object : objectsToDisplay) {
        if (object == nullptr) {
            // Skip the object if it's nullptr (safety check)
            continue;
        }

        // Make sure the object is still in the scene
        if (std::find(scene.getObjects().begin(), scene.getObjects().end(), object) == scene.getObjects().end()) {
            // If the object has been removed from the scene, we should not attempt to display it.
            continue;
        }

        displayObjectProperties(object, 0);
    }

    ImGui::End();
}

void Interface::objectListUI() {
    ImGui::Begin("Object List");

    ImGui::PushFont(headingFont);
    ImGui::Text("Objects:");
    ImGui::PopFont();

    ImGui::Separator();

    // Root "Scene" node
    ImGuiTreeNodeFlags rootFlags = ImGuiTreeNodeFlags_DefaultOpen |
        ImGuiTreeNodeFlags_OpenOnArrow |
        ImGuiTreeNodeFlags_OpenOnDoubleClick |
        ImGuiTreeNodeFlags_SpanAvailWidth;

    bool open = ImGui::TreeNodeEx("Scene", rootFlags);

    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("OBJECT")) {
            Object* droppedObject = *(Object**)payload->Data;

            if (droppedObject && droppedObject->parent) {
                droppedObject->removeParent(); // unparent it = move to root
            }
        }
        ImGui::EndDragDropTarget();
    }

    if (open) {
        // Only render top-level objects (objects with no parent)
        for (Object* object : scene.getObjects()) {
            if (!object->parent) {
                renderObjectNode(object);
            }
        }
        ImGui::TreePop();
    }

    ImGui::End();
}

void Interface::renderObjectNode(Object* object) {
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow |
        ImGuiTreeNodeFlags_OpenOnDoubleClick |
        ImGuiTreeNodeFlags_SpanAvailWidth;

    if (selectedObjects.count(object))
        flags |= ImGuiTreeNodeFlags_Selected;

    bool open = ImGui::TreeNodeEx(object->name.c_str(), flags);

    // Select on click with optional Shift for multi-select
    if (ImGui::IsItemClicked()) {
        bool ctrlPressed = ImGui::GetIO().KeyCtrl;
        selectObject(object, ctrlPressed);
    }

    // Drag source
    if (ImGui::BeginDragDropSource()) {
        ImGui::SetDragDropPayload("OBJECT", &object, sizeof(Object*));
        ImGui::Text("Move: %s", object->name.c_str());
        ImGui::EndDragDropSource();
    }

    // Drop target
    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("OBJECT")) {
            Object* droppedObject = *(Object**)payload->Data;

            if (droppedObject != object && !object->isChildOf(droppedObject)) {
                droppedObject->setParent(object);
            }
            else
            {
                droppedObject->removeParent();
            }
        }
        ImGui::EndDragDropTarget();
    }

    if (open) {
        for (Object* child : object->children) {
            renderObjectNode(child);
        }
        ImGui::TreePop();
    }
}

void Interface::sceneControlsUI() {
    ImGui::Begin("Scene Controls");

    if (ImGui::Button("Add Object")) {
        showShapePopup = true;
        ImGui::OpenPopup("Select Object Shape");
    }

    showShapeSelectionPopup();

    ImGui::SameLine();

    if (ImGui::Button("Save Scene")) {
        openSaveDialog();
    }

    ImGui::SameLine();

    if (ImGui::Button("Load Scene")) {
        openLoadDialog();
    }

    ImGui::Separator();
    //ImGui::Text("Gizmo Mode:");

    float buttonSize = 40.0f;
    ImVec4 activeColor = ImVec4(0.2f, 0.6f, 0.95f, 1.0f);  // Highlight color
    ImVec4 hoveredColor = ImVec4(0.3f, 0.7f, 1.0f, 1.0f);   // Hover color
    ImVec4 normalColor = ImGui::GetStyleColorVec4(ImGuiCol_Button);

    auto squareIconButton = [&](const char* icon, ImGuizmo::OPERATION op) {
        bool isActive = (gizmoOperation == op);
        if (isActive) ImGui::PushStyleColor(ImGuiCol_Button, activeColor);
        else ImGui::PushStyleColor(ImGuiCol_Button, normalColor);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, hoveredColor);

        ImGui::PushFont(iconFont);  // Switch to icon font

        if (ImGui::Button(icon, ImVec2(buttonSize, buttonSize))) {
            setGizmoOperation(op);
        }

        ImGui::PopFont();           // Back to previous font
        ImGui::PopStyleColor(2);
        ImGui::SameLine();
    };


    // Font Awesome icons: (Translate), (Rotate), (Scale)
    squareIconButton(ICON_FA_ARROWS_ALT, ImGuizmo::TRANSLATE);  // Translate
    squareIconButton(ICON_FA_SYNC_ALT, ImGuizmo::ROTATE);       // Rotate
    squareIconButton(ICON_FA_EXPAND_ARROWS_ALT, ImGuizmo::SCALE); // Scale

    ImGui::End();
}

void Interface::openSaveDialog() {
    IGFD::FileDialogConfig config;
    config.path = "assets/scenes";

    ImGuiFileDialog::Instance()->OpenDialog("SaveSceneDialog", "Save Scene", ".json", config);
}

void Interface::openLoadDialog() {
    IGFD::FileDialogConfig config;
    config.path = "assets/scenes";

    ImGuiFileDialog::Instance()->OpenDialog("LoadSceneDialog", "Load Scene", ".json", config);
}

void Interface::showShapeSelectionPopup() {
    if (showShapePopup) {
        if (ImGui::BeginPopupModal("Select Object Shape", NULL)) {
            ImGui::Text("Choose Shape:");

            if (ImGui::Button("Cube")) {
                scene.addObject(new Object("Cube", createCubeVertices(), defaultTexture));
                ImGui::CloseCurrentPopup();
            }

            if (ImGui::Button("Triangle")) {
                scene.addObject(new Object("Triangle", createTriangle3DVertices(), defaultTexture));
                ImGui::CloseCurrentPopup();
            }

            if (ImGui::Button("Sphere")) {
                scene.addObject(new Object("Sphere", createSphereVertices(), defaultTexture));
                ImGui::CloseCurrentPopup();
            }

            ImGui::Separator();

            if (ImGui::Button("Cancel")) {
                ImGui::CloseCurrentPopup(); 
            }

            ImGui::EndPopup();
        }
    }
}

void Interface::displayObjectProperties(Object* object, int index) {

    //
    // ISSUES:
    // when multiple objects selected, 2nd, 3rd, etc.. objects properties cant be changed, 
    // only 1st objects properties can.; -- NOT FIXED
    // 

    ImGui::PushID(index);

    ImGui::PushFont(headingFont);
    ImGui::Text("General");
    ImGui::PopFont();

    static char tempName[128] = {};
    static bool nameInitialized = false;
    if (!nameInitialized) {
        strncpy_s(tempName, object->name.c_str(), sizeof(tempName) - 1);
        nameInitialized = true;
    }

    ImGui::InputText("Object Name##", tempName, IM_ARRAYSIZE(tempName));

    if (ImGui::Button("Apply##"))
        object->setName(std::string(tempName));

    ImGui::Text("Object's Parent: %s", object->parent);

    ImGui::Separator();

    ImGui::PushFont(headingFont);
    ImGui::Text("Transform");
    ImGui::PopFont();

    // Display Position, Rotation, and Scale
    ImGui::DragFloat3("Position", glm::value_ptr(object->position), 0.1f);
    ImGui::DragFloat3("Rotation", glm::value_ptr(object->rotation), 1.0f, -180.0f, 180.0f);
    ImGui::DragFloat3("Scale", glm::value_ptr(object->scale), 0.01f, 0.1f, 10.0f);

    ImGui::Separator();

    ImGui::PushFont(headingFont);
    ImGui::Text("Texture/Color");
    ImGui::PopFont();

    if (object->texture) {
        ImGui::Text("Current Texture: %s", object->texture->getFilePath().c_str());
    }
    else {
        ImGui::Text("No texture applied");
    }

    if (ImGui::Button("Choose Texture")) {
        IGFD::FileDialogConfig config;
        config.path = "assets/";
        ImGuiFileDialog::Instance()->OpenDialog(
            "ChooseTex",
            "Select Texture",
            "All Supported{.png,.jpg,.jpeg,.bmp},.png,.jpg,.jpeg,.bmp",
            config
        );
        textureTargetObject = object;
    }

    ImGui::SameLine();

    if (ImGui::Button("Remove Texture")) {
        object->removeTexture();
        object->setColor(glm::vec3(1.0f, 1.0f, 1.0f));
    }

    if (!object->texture) {
        static glm::vec3 color = object->color;
        ImGui::ColorEdit3("Object Color", &color[0]);
        if (ImGui::Button("Apply Color")) {
            object->setColor(color);
        }
    }

    ImGui::Separator();

    if (ImGui::Button("Duplicate Object")) {
        duplicateObject();
    }

    ImGui::SameLine();

    if (ImGui::Button("Remove Object")) {
        // First, find the index of the object in the scene's object list.
        auto it = std::find(scene.getObjects().begin(), scene.getObjects().end(), object);

        if (it != scene.getObjects().end()) {
            size_t index = std::distance(scene.getObjects().begin(), it);

            selectedObjects.erase(object);

            if (!object->children.empty()) // if object has children, remove children
            {
                for (int i = 0; i < object->children.size(); i++)
                {
                    scene.removeObject(0,object->children[i]);
                }
            }
            scene.removeObject(index); // remove object

            // Remove the object from the selection list as 

            // Clear the textureTargetObject if it was pointing to this object
            if (textureTargetObject == object) {
                textureTargetObject = nullptr;
            }
        }
    }



    ImGui::PopID();
}

void Interface::duplicateObject() {
    if (!selectedObjects.empty()) {
        // We'll just duplicate the first selected object for now
        Object* original = *selectedObjects.begin();
        if (original) {
            // Clone the object (make sure you have a proper copy/clone constructor or method)
            Object* duplicate = new Object(*original);  // Assuming copy constructor

            // Rename it to avoid name conflicts
            duplicate->name += " (Copy)";

            // Ensure it's not parented (or keep the same parent if that's intended)
            duplicate->parent = nullptr;
            duplicate->children.clear(); // Clear children if you're only duplicating the base

            // Add to scene
            scene.addObject(duplicate);
            selectedObjects = { duplicate };
        }
    }
}

void Interface::clearSelection() {
    selectedObjects.clear();
    lastSelectedObject = nullptr;
}

void Interface::selectObject(Object* object, bool appendSelection) {
    if (!appendSelection) {
        selectedObjects.clear();
    }

    selectedObjects.insert(object);
    lastSelectedObject = object;
}

void Interface::propagateTransform(Object* parent, const glm::mat4& delta) {
    for (Object* child : parent->children) {
        glm::mat4 childModel = glm::mat4(1.0f);
        childModel = glm::translate(childModel, child->position);
        childModel = glm::rotate(childModel, glm::radians(child->rotation.x), glm::vec3(1, 0, 0));
        childModel = glm::rotate(childModel, glm::radians(child->rotation.y), glm::vec3(0, 1, 0));
        childModel = glm::rotate(childModel, glm::radians(child->rotation.z), glm::vec3(0, 0, 1));
        childModel = glm::scale(childModel, child->scale);

        glm::mat4 newChildModel = delta * childModel;

        // Decompose the new transform matrix
        glm::vec3 skew;
        glm::vec4 perspective;
        glm::quat rotationQuat;
        glm::vec3 translation, scale;

        glm::decompose(newChildModel, scale, rotationQuat, translation, skew, perspective);
        glm::vec3 eulerAngles = glm::eulerAngles(rotationQuat);

        child->position = translation;
        child->rotation = glm::degrees(eulerAngles);
        child->scale = scale;

        if(!child->children.empty())
            propagateTransform(child,delta);
    }
}

void Interface::renderGizmo() {
    if (selectedObjects.empty()) return;
    if (!camera) {
        std::cerr << "Camera is nullptr!" << std::endl;
        return;
    }

    ImGuizmo::BeginFrame();
    ImGuizmo::SetRect(0, 0, ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y);

    glm::mat4 view = camera->getViewMatrix();
    glm::mat4 projection = camera->getProjectionMatrix();

    for (Object* object : selectedObjects) {
        glm::mat4 originalGlobal = object->getGlobalTransform();
        glm::mat4 model = object->getLocalTransform();

        bool manipulated = ImGuizmo::Manipulate(
            glm::value_ptr(view),
            glm::value_ptr(projection),
            gizmoOperation,
            gizmoMode,
            glm::value_ptr(model)
        );

        if (manipulated) {
            // Calculate delta matrix between original and new
            glm::mat4 deltaMatrix = glm::inverse(originalGlobal) * model;

            // Update parent object transform
            object->position = glm::vec3(model[3][0], model[3][1], model[3][2]);
            object->rotation.x = glm::degrees(atan2(model[1][2], model[1][1]));
            object->rotation.y = glm::degrees(atan2(model[2][0], model[0][0]));
            object->rotation.z = glm::degrees(atan2(model[1][0], model[0][0]));
            object->scale = glm::vec3(glm::length(model[0]), glm::length(model[1]), glm::length(model[2]));

            //
            // ISSUES:
            // If object has a children and that children does have another 
            // children it fucks everything up; -- NOT FIXED
            //
            
            if(!object->children.empty())
                propagateTransform(object, deltaMatrix);

        }
    }
}

void Interface::debugUI() {    
    ImGui::Begin("Debug Controls");

    ImGui::PushFont(headingFont);
    ImGui::Text("Debug Controls");
    ImGui::PopFont();

    ImGui::Separator();

    ImGui::PushFont(headingFont);
    ImGui::Text("Objects");
    ImGui::PopFont();
    
    std::unordered_set<Object*> uniqueObjects;

    for (Object* object : scene.getObjects()) {
        uniqueObjects.insert(object);
    }
    for (Object* object : scene.getInternalObjects()) {
        uniqueObjects.insert(object);
    }

    int c = static_cast<int>(uniqueObjects.size());
    ImGui::Text("Total Objects: %i", c);

    for (Object* object : scene.getObjects()) {
        ImGui::Text("[OBJECT]: %s", object->name.c_str());
    }
    for (Object* object : scene.getInternalObjects()) {
        ImGui::Text("[INTERNAL OBJECT]: %s", object->name.c_str());
    }

    

    ImGui::Separator();

    ImGui::End();
}

void Interface::setGizmoOperation(ImGuizmo::OPERATION operation)
{
    gizmoOperation = operation;
}
