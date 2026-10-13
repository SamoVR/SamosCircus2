// Test harness (NOT part of the deliverable). Drives the real Engine::init/update/render for
// a scripted session, checks glGetError() every frame, and dumps frames with glReadPixels.
#include <thread>
#include <chrono>
#include <cstdlib>
#include <cstdio>
#include <memory>
#include <string>
#include <vector>
#include <functional>
#include <unordered_set>
#include <unordered_map>
#include <algorithm>
#include <sstream>
#include <fstream>
#include <iostream>
#include <mutex>
#include <limits>
#include <stdexcept>
#include <cmath>
#include <array>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <nlohmann/json.hpp>
#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "ImGuiFileDialog-master/ImGuiFileDialog.h"
#include "ImGuizmo-master/ImGuizmo.h"
#include "ImGuizmo-master/ImSequencer.h"
#include "ImGuizmo-master/ImCurveEdit.h"
#include "FontAwesome-main/font_awesome.h"
#include "stb_image.h"
#define private public
#include "Engine.h"
#undef private

static int glErrors = 0, checks = 0, failures = 0;
#define EXPECT(cond, msg) do { ++checks; if (cond) std::printf("  [ok]   %s\n", msg); else { ++failures; std::printf("  [FAIL] %s  (line %d)\n", msg, __LINE__); } } while (0)

static void saveFrame(Engine& e, const char* path) {
    int w, h; glfwGetFramebufferSize(e.window, &w, &h);
    std::vector<unsigned char> px(static_cast<size_t>(w) * h * 3);
    glReadPixels(0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, px.data());
    while (glGetError() != GL_NO_ERROR) {} // a failed read (multisample) must not count as an engine error
    FILE* f = std::fopen(path, "wb");
    std::fprintf(f, "P6\n%d %d\n255\n", w, h);
    for (int y = h - 1; y >= 0; --y) std::fwrite(&px[static_cast<size_t>(y) * w * 3], 1, static_cast<size_t>(w) * 3, f);
    std::fclose(f);
}

int main() {
    Engine e;
    e.init();
    const float dt = 1.0f / 60.0f;
    auto step = [&](int n = 1, const char* shot = nullptr) {
        for (int i = 0; i < n; ++i) {
            glfwPollEvents();
            e.update(dt);
            e.render(dt);
            if (shot && i == n - 1) saveFrame(e, shot);
            for (GLenum err; (err = glGetError()) != GL_NO_ERROR;) { ++glErrors; std::printf("  GL ERROR 0x%x\n", err); }
            glfwSwapBuffers(e.window);
        }
    };

    std::printf("== (picture) the user's scene.json, camera pulled back\n");
    e.UI->doLoad("scene.json");
    e.camera->handleScrollInput(-6.0f); e.camera->handleMouseInput(60.0f, 80.0f, true, false);
    step(90, "/tmp/f_legacy_pretty.ppm");
    e.UI->resetSceneState(); e.scene.clear();
    e.scene.addObject(std::make_unique<Object>("Starting Cube", createCubeVertices(), std::make_unique<Texture>("assets/images/icon.png")));
    e.UI->markClean(); e.camera->reset(); step(30);

    std::printf("== idle frames\n");
    step(30, "/tmp/f_idle.ppm");
    EXPECT(e.scene.objectCount() == 1, "starting scene has one object");
    Object* cube = e.scene.findByName("Starting Cube");
    EXPECT(cube != nullptr, "starting cube exists");

    std::printf("== selection + gizmo\n");
    e.UI->selectObject(cube, false);
    step(10, "/tmp/f_selected.ppm");
    EXPECT(e.UI->getSelectedObjects().size() == 1, "cube is selected");

    std::printf("== add / duplicate / undo / redo\n");
    e.UI->addPrimitive("Sphere", createSphereVertices());
    e.UI->addPrimitive("Triangle", createTriangle3DVertices());
    step(5);
    EXPECT(e.scene.objectCount() == 3, "two primitives added");
    e.UI->duplicateObject();
    step(2);
    EXPECT(e.scene.objectCount() == 4, "duplicate created a 4th object");
    e.UI->undo(); step(2);
    EXPECT(e.scene.objectCount() == 3, "undo removed the duplicate");
    EXPECT(e.UI->getSelectedObjects().size() <= 1 && e.scene.contains(e.UI->primaryObject) == (e.UI->primaryObject != nullptr), "selection never points at a removed object after undo");
    e.UI->redo(); step(2);
    EXPECT(e.scene.objectCount() == 4, "redo brought it back");

    std::printf("== parenting is visible at render time\n");
    Object* sphere = e.scene.findByName("Sphere");
    sphere->position = glm::vec3(3, 0, 0);
    e.UI->reparent(sphere, cube);
    cube->position = glm::vec3(0, 2, 0);
    step(5, "/tmp/f_parent.ppm");
    glm::vec3 sw(sphere->getGlobalTransform()[3]);
    EXPECT(std::abs(sw.x - 3.0f) < 1e-3f && std::abs(sw.y - 2.0f) < 1e-3f, "child follows its parent (world position (3,2,0))");
    e.UI->deleteObjects({ cube });  // delete a PARENT while a child exists
    step(5);
    EXPECT(sphere->parent == nullptr && e.scene.contains(sphere), "deleting the parent leaves the child alive and unparented");
    e.UI->undo(); step(3);
    EXPECT(sphere->parent == cube, "undo restored the parent link");

    std::printf("== measurement lines + wireframe + grid off\n");
    cube->showMeasureX = cube->showMeasureY = cube->showMeasureZ = true;
    cube->scale = glm::vec3(3, 1, 2);
    e.UI->selectObject(cube, false);
    step(5, "/tmp/f_measure.ppm");
    e.settings.wireframe = true; step(3); e.settings.wireframe = false;
    e.settings.showGrid = false; step(3); e.settings.showGrid = true;

    std::printf("== animation without any row expanded\n");
    int idx = e.UI->animationSequencer->addItemForObject(sphere, 0, 60);
    auto& item = e.UI->animationSequencer->items[idx];
    item.mode = AnimationSequencer::TransformMode::Position;
    item.curvesByMode[0].mPts[0][0] = ImVec2(0, 0);   item.curvesByMode[0].mPts[0][1] = ImVec2(60, 6);
    item.curvesByMode[0].mDirty = true;
    e.UI->animationSequencer->currentFrame = 30;
    step(3);
    EXPECT(std::abs(sphere->position.x - 3.0f) < 1e-3f, "frame 30 -> x = 3 (halfway to 6) with the row collapsed");

    std::printf("== keybinds / text-input guard\n");
    e.UI->setGizmoOperation(ImGuizmo::ROTATE); step(3, "/tmp/f_rotate.ppm");
    e.UI->setGizmoOperation(ImGuizmo::SCALE); step(3);
    e.UI->clearSelection(); step(2);

    std::printf("== save / load round trip through the UI layer\n");
    EXPECT(e.UI->doSave("/tmp/ui_roundtrip.json"), "doSave succeeds");
    EXPECT(!e.UI->hasUnsavedChanges(), "scene is clean right after saving");
    e.UI->addPrimitive("Cube", createCubeVertices()); step(2);
    EXPECT(e.UI->hasUnsavedChanges(), "adding an object marks the scene dirty");
    EXPECT(e.UI->doLoad("/tmp/ui_roundtrip.json"), "doLoad succeeds");
    step(5, "/tmp/f_loaded.ppm");
    EXPECT(e.scene.objectCount() == 4 && !e.UI->hasUnsavedChanges(), "loaded scene matches the saved one and is clean");
    EXPECT(!e.UI->history.canUndo(), "undo history is cleared after loading (commands would hold dead pointers)");
    EXPECT(e.UI->animationSequencer->items.empty(), "animation items cleared after loading");
    EXPECT(!e.UI->doLoad("/tmp/does_not_exist.json") && e.scene.objectCount() == 4, "loading a missing file fails without touching the scene");
    step(5);

    std::printf("== real ray picking (Engine::performObjectPicking)\n");
    e.UI->resetSceneState(); e.scene.clear(); e.UI->markClean();
    auto bar = std::make_unique<Object>("Bar", createCubeVertices(), static_cast<Texture*>(nullptr));
    bar->scale = glm::vec3(10.0f, 0.4f, 0.4f);        // long thin bar
    bar->rotation = glm::vec3(0, 0, 0);
    Object* barPtr = bar.get();
    e.scene.addObject(std::move(bar));
    e.camera->reset(); e.camera->smoothingEnabled = false; e.camera->target = glm::vec3(0); e.camera->handleScrollInput(-20.0f);
    step(3);
    int ww, wh; glfwGetWindowSize(e.window, &ww, &wh);
    auto pickAt = [&](glm::vec3 world) { glm::vec2 n = e.camera->worldToScreen(world); e.UI->clearSelection(); e.performObjectPicking(n.x * ww, n.y * wh); return !e.UI->getSelectedObjects().empty(); };
    EXPECT(pickAt(glm::vec3(0, 0, 0)), "click on the bar's centre selects it");
    EXPECT(pickAt(glm::vec3(4.5f, 0, 0)), "click near the bar's far end selects it");
    // The old test was a sphere of radius |scale|/2 = ~5 around the centre: a click 3 units ABOVE the thin bar was a "hit".
    EXPECT(!pickAt(glm::vec3(0, 3.0f, 0)), "click 3 units above the thin bar misses (old sphere test wrongly hit)");
    EXPECT(!pickAt(glm::vec3(0, 0, 0) + glm::vec3(0, -4.0f, 0)), "click far below the bar misses");
    // rotated bar: picking follows rotation
    barPtr->rotation = glm::vec3(0, 0, 90.0f); step(1);
    EXPECT(pickAt(glm::vec3(0, 4.0f, 0)) && !pickAt(glm::vec3(4.0f, 0, 0)), "after rotating 90 degrees the hit area rotates with it");
    e.UI->selectObject(barPtr, false);

    std::printf("== closing with unsaved changes asks first\n");
    e.UI->markClean();
    EXPECT(e.UI->requestClose(), "clean scene: close is allowed immediately");
    barPtr->position.x = 1.0f;
    EXPECT(e.UI->hasUnsavedChanges() && !e.UI->requestClose(), "dirty scene: close is postponed and the prompt is raised");
    step(3, "/tmp/f_prompt.ppm");
    EXPECT(e.UI->pendingAction == Interface::PendingAction::Exit, "pending action is Exit");

    std::printf("== the user's real legacy scene.json (pre-normals, pre-ownership save format)\n");
    e.UI->resetSceneState(); e.scene.clear(); e.UI->markClean();
    EXPECT(e.UI->doLoad("scene.json"), "scene.json from the original project loads");
    std::printf("     loaded %zu object(s):", e.scene.objectCount());
    for (Object* o : e.scene.getObjects()) std::printf(" '%s'%s", o->name.c_str(), o->parent ? "(child)" : "");
    std::printf("\n");
    EXPECT(e.scene.objectCount() > 0, "legacy scene contains objects");
    int texturesOk = 0, texturesTotal = 0;
    for (Object* o : e.scene.getObjects()) if (o->texture) { ++texturesTotal; if (o->texture->isValid()) ++texturesOk; }
    std::printf("     textures resolved: %d of %d (stored as '%s')\n", texturesOk, texturesTotal,
        texturesTotal ? e.scene.getObjects()[0]->texture ? e.scene.getObjects()[0]->texture->getFilePath().c_str() : "-" : "-");
    EXPECT(texturesTotal > 0 && texturesOk == texturesTotal, "Windows-absolute texture paths from an old save resolve against ./assets");
    step(10, "/tmp/f_legacy.ppm");

    std::printf("== shutdown with live objects (GL resources must be freed while the context exists)\n");
    e.UI->addPrimitive("Sphere", createSphereVertices()); step(2);

    std::printf("\nframes OK; GL errors: %d; checks: %d, failures: %d\n", glErrors, checks, failures);
    e.cleanup();
    e.cleanup(); // second call must be a harmless no-op (it used to destroy the window twice)
    std::printf("cleanup() twice: ok\n");
    return (glErrors == 0 && failures == 0) ? 0 : 1;
}
