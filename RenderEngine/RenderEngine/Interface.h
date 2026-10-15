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
#include "CommandHistory.h"
#include "Commands.h"
#include "Settings.h"
#include "TransformUtil.h"
#include "Logger.h"

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
#include <unordered_map>
#include <vector>
#include <memory>
#include <string>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

class Interface {
public:
    Interface(GLFWwindow* window, Scene& scene, Camera* camera, KeybindManager* keybindManager,
              InputManager* inputManager, AppSettings& settings);
    ~Interface();

    Interface(const Interface&) = delete;
    Interface& operator=(const Interface&) = delete;

    // Per frame. tickAnimation runs from Engine::update (independent of any
    // window being visible); update builds and renders the UI.
    void tickAnimation(float deltaTime);
    void update(float deltaTime);

    // ---- selection
    const std::unordered_set<Object*>& getSelectedObjects() const { return selectedObjects; }
    void clearSelection();
    // appendSelection: Ctrl/Shift-click semantics -- toggles the object.
    void selectObject(Object* object, bool appendSelection);
    void selectAll();

    // ---- actions (menus and keybinds)
    void newScene();
    void openLoadDialog();
    void openSaveDialog();   // "Save As"
    void saveScene();        // "Save": Save As if the scene has no file yet
    void duplicateObject();
    void deleteSelected();
    void undo();
    void redo();
    void setGizmoOperation(ImGuizmo::OPERATION operation);
    void toggleGizmoSpace();
    void focusSelection();

    // ---- queries / hooks for Engine
    bool isGizmoActive() const;        // pointer is over / dragging the gizmo
    bool wantsTextInput() const;       // typing in a text field: keybinds must not fire
    bool isModalUiOpen() const;        // any popup or file dialog is open
    bool isCapturingKey() const { return !waitingForRebind.empty(); } // keybind rebind popup is open
    bool hasUnsavedChanges() const;
    // The Engine calls this when the window close button / Esc was used.
    // Returns true if the window may close right now; otherwise a
    // "unsaved changes" prompt is shown and the close is postponed.
    bool requestClose();
    void applySavedKeybinds();         // settings.json -> KeybindManager
    void showStatus(const std::string& message, bool isError = false);

    struct DebugText {
        glm::vec3 worldPosition;
        std::string text;
        ImVec4 color;
        ImVec2 screenOffset;
    };
    std::vector<DebugText> debugTexts;

private:
    GLFWwindow* window;
    Scene& scene;
    Camera* camera;
    KeybindManager* keybindManager;
    InputManager* inputManager;
    AppSettings& settings;

    ImFont* headingFont = nullptr;
    ImFont* iconFont = nullptr;

    std::unique_ptr<Texture> defaultTexture;             // shared by every primitive created from the UI
    std::unique_ptr<AnimationSequencer> animationSequencer;
    CommandHistory history;

    std::unordered_set<Object*> selectedObjects;
    Object* primaryObject = nullptr;                     // the one the gizmo is attached to
    void pruneSelection();                               // drops pointers no longer in the scene

    // ---- gizmo
    ImGuizmo::OPERATION gizmoOperation = ImGuizmo::TRANSLATE;
    bool gizmoWasUsing = false;
    std::vector<std::pair<Object*, ObjectState>> gizmoStartStates;
    void renderGizmo();

    // ---- property-edit undo tracking (state captured when a widget is
    // activated, committed as one undo step when it is released)
    std::unordered_map<Object*, ObjectState> editBefore;
    void beginEdit(Object* object);
    void endEdit(Object* object, const char* what);
    void trackEdit(Object* object, const char* what); // call right after the widget

    // ---- scene / file state
    Object* textureTargetObject = nullptr;
    Object* panelDeleteRequest = nullptr;               // set by the Properties panel, executed after it is drawn
    std::string currentScenePath;
    std::string savedSnapshot;
    bool dirtyCached = false;
    float dirtyTimer = 0.0f;
    std::string shownTitle;
    void markClean();
    void refreshDirtyState(float deltaTime);
    void updateWindowTitle();
    bool doSave(const std::string& path);
    bool doLoad(const std::string& path);
    void resetSceneState();                              // clears selection / animation / history

    enum class PendingAction { None, NewScene, OpenScene, Exit };
    PendingAction pendingAction = PendingAction::None;
    bool openUnsavedPopup = false;
    bool exitConfirmed = false;
    void guardUnsaved(PendingAction action);
    void runPending(PendingAction action);

    // ---- helpers
    std::string makeUniqueName(const std::string& base) const;
    void addPrimitive(const std::string& baseName, const std::vector<Vertex>& vertices);
    void deleteObjects(const std::vector<Object*>& objects);
    void reparent(Object* object, Object* newParent);
    ObjectState localStateFor(const Object& object, const Object* newParent) const;
    void pushHeading(const char* text);

    // ---- animation playback
    bool playing = false;
    bool loopPlayback = true;
    float playbackFps = 30.0f;
    float frameAccumulator = 0.0f;
    bool timelineExpanded = true;
    int timelineSelected = -1;
    int timelineFirstFrame = 0;                          // was a per-frame local, which made scrolling impossible

    // ---- status / overlays
    std::string statusText;
    bool statusIsError = false;
    float statusTimer = 0.0f;
    float smoothedFps = 60.0f;
    bool showAbout = false;
    bool showShortcuts = false;
    void statusOverlay();
    void statsOverlay();
    void aboutPopups();
    void unsavedPopup();

    // ---- settings / keybinds
    std::string waitingForRebind;
    std::string rebindWarning;
    std::vector<NamedKeybind> defaultKeybinds;
    bool settingsDirty = false;
    void saveKeybindsToSettings();
    void applyUiScale();
    void applyTheme();

    // ---- panels
    void renderText();
    void mainMenuBarUI();
    void sceneControlsUI();
    void objectListUI();
    void renderObjectNode(Object* object);
    void propertiesUI();
    void displayObjectProperties(Object* object);
    void settingsUI();
    void timelineUI();
    void updateFileBrowsers();
    void showShapeSelectionPopup();
};
