#pragma once

#include "Scene.h"
#include "Object.h"
#include "Vertex.h"
#include "Geometry.h"
#include "Texture.h"
#include "Camera.h"
#include "KeybindManager.h"
#include "InputManager.h"
#include "AnimationSequencer.h"

#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "ImGuiFileDialog-master/ImGuiFileDialog.h"
#include "ImGuizmo-master/ImGuizmo.h"
#include "ImGuizmo-master/ImSequencer.h"
#include "ImGuizmo-master/ImCurveEdit.h"
#include "FontAwesome-main/font_awesome.h"

#include <unordered_set>
#include <vector>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <iostream>
#include <sstream>
#include <string>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>

class Interface {
public:
    Interface(GLFWwindow* window, Scene& scene, Camera* camera, KeybindManager* keybindManager, InputManager* inputManager);
    ~Interface();

    void update();
    const std::unordered_set<Object*>& getSelectedObjects() const {
        return selectedObjects;
    }
    void clearSelection();
    void selectObject(Object* object, bool appendSelection);
    void propagateTransform(Object* parent, const glm::mat4& delta);

    void duplicateObject();
    void openSaveDialog();
    void openLoadDialog();
    void setGizmoOperation(ImGuizmo::OPERATION operation);

private:
    GLFWwindow* window;
    ImFont* headingFont;
    ImFont* iconFont;
    Scene& scene;
    Camera* camera;
    KeybindManager* keybindManager;
    InputManager* inputManager;
    Texture* defaultTexture;
    Object* textureTargetObject = nullptr;
    AnimationSequencer* animationSequencer = nullptr;

    std::unordered_set<Object*> selectedObjects;
    Object* lastSelectedObject = nullptr;

    void renderGizmo();

    ImGuizmo::OPERATION gizmoOperation = ImGuizmo::TRANSLATE;
    ImGuizmo::MODE gizmoMode = ImGuizmo::WORLD;

    bool showShapePopup = false;

    void displayObjectProperties(Object* object, int index);
    void renderObjectNode(Object* object);

    void debugUI();
    void mainMenuBarUI();
    void sceneControlsUI();
    void objectListUI();
    void settingsUI();
    void propertiesUI();
    void timelineUI();
   
    void updateFileBrowsers();
    void showShapeSelectionPopup();
    std::string selectedTexturePath = "assets/textures/texture_08.png";

};
