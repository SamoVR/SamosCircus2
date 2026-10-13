# RIFT / RenderEngine — rework report

Everything here drops into your existing project folder (same flat layout as the original).
Copy the files over the originals, open `RenderEngine.vcxproj`, build **Release | x64**.
`tests/` is not part of the build (see "Tests").

---

## 1. Files

**New (6 headers, header-only, no project changes needed beyond the `.vcxproj` already updated)**

| File | Purpose |
|---|---|
| `Logger.h` | Levelled log to `RenderEngine_log.txt`, the VS Output window and stderr. Replaces every invisible `std::cerr`. |
| `Command.h`, `CommandHistory.h` | Undo/redo core (200-step cap, redo cleared on a new action). |
| `Commands.h` | Add / Remove / Modify / Reparent / Composite commands. |
| `Settings.h` | `settings.json` persistence (toggles, camera, gizmo, UI scale, rebound keys). |
| `TransformUtil.h` | Matrix ⇄ position/rotation/scale that matches the engine's own X·Y·Z rotation order. |

**Rewritten / changed**

`AnimationSequencer.h/.cpp` · `Camera.h/.cpp` · `Engine.h/.cpp` · `Geometry.cpp` · `InputManager.h/.cpp` ·
`Interface.h/.cpp` · `KeybindManager.h/.cpp` · `main.cpp` · `Object.h/.cpp` · `Scene.h/.cpp` ·
`Shader.h/.cpp` · `Texture.h/.cpp` · `Vertex.h` · `vertex.glsl` · `fragment.glsl` · `line_fragment.glsl` ·
`RenderEngine.vcxproj` + `.filters` (additive only: new headers, Release|x64 post-build staging step)

**Untouched:** `Geometry.h`, `KeyCombo.h`, `KeyComboHash.h`, `resource.h`, the other `.glsl` files, all vendored libraries.

> Line endings: all `.h/.cpp/.glsl` are CRLF to match your project.

---

## 2. The original nine findings

| # | Finding | Fix |
|---|---|---|
| RE-01 | Use-after-free when deleting a parent with a selected child | `Scene` is the single owner (`unique_ptr`); one removal path that always detaches the hierarchy first; selection is pruned every frame; deletes are undoable commands that keep ownership. |
| RE-02 | Parenting had no effect at render time | `Object::draw` uses `getGlobalTransform()`. The hand-written `propagateTransform` workaround (and its "grandchildren NOT FIXED" bug) is **removed** — it would now double-move children. |
| RE-03 | Sequencer type-count (5) vs name table (2) | Count derived from the table; name lookup bounds-checked; `static_assert` on the mode table. |
| RE-04 | First-frame `.at()` crash | Safe `find()`; input is also now sampled *before* use. |
| RE-05 | `%s` given an `Object*` | Prints the parent's name / "None". |
| RE-06 | Multi-selection property editing broken | Per-object ImGui ID (pointer), no shared `static` state. |
| RE-07 | Duplicate shared GPU buffers | Explicit copy constructor allocates its own VAO/VBO; implicit copy of GPU handles is gone. |
| RE-08a | Default curves could zero an object's Scale | Per-channel curves; seeded flat at the object's *current* transform. |
| RE-08b | `removeObject` overloads disagreed on ownership | One overload, one contract. |

## 3. Additional bugs found during the rework (not in the original report)

1. **Animations only played while the timeline row was expanded**, and each item's start/end frame was ignored (the logic lived in `CustomDraw`, which the widget only calls for expanded rows). Now `applyToObjects()` runs every frame, independent of the UI, respects the frame range, and doesn't fight manual gizmo edits while paused.
2. **Shutdown order:** `cleanup()` ran twice (`run()` and the destructor) and GL objects were freed *after* the context was destroyed. Now idempotent, GL resources freed first.
3. **Measurement lines were wrong:** the model matrix already contained the scale and the length was multiplied by it again (a 3× object got a 9-unit line labelled "3"); rotation/parenting ignored; the line shader hard-coded white so the red/green/blue colours never showed; the labels' screen offsets were computed but never applied (labels overlapped).
4. **Shader program leak between draws:** `Object::draw` never bound its shader, but the line shader is bound in between — uniforms for the next object went to the wrong program. `draw` now binds its own shader.
5. **Gizmo rotation maths was wrong:** the Euler extraction doesn't invert the engine's rotation order. Rebuilt the transform incorrectly in **2000 / 2000** random test rotations.
6. **Colour picker used a `static glm::vec3`** shared by every object.
7. **Timeline couldn't scroll** (`firstFrame` was a per-frame local).
8. **Keybinds fired while typing** (typing "R" in the name box switched the gizmo); Esc quit instantly with no prompt.
9. **Camera/picking input:** right-drag started over a panel orbited the camera, scrolling a panel zoomed it, clicking a gizmo handle also picked and deselected, mouse coords used framebuffer instead of window size.
10. **Window:** minimised window → division by zero in aspect ratio; first frame's `deltaTime` was huge; `glfwSwapInterval` never set.
11. **Scene loading:** a malformed file could crash or wipe the scene; duplicate names silently overwrote each other (**your own `scene.json` has five objects all named "Cube"**).
12. **Saved scenes stored absolute paths** (`C:\Users\Samuel\...`), so textures couldn't load on any other machine.
13. **Failures were silent:** shader compile/link failures, texture failures and file errors only went to a console that doesn't exist in the Windows-subsystem build.
14. **Dead/duplicated:** `Camera::update()` was an uncalled stub; `Scene::fromJSONToVertices` duplicated `Object`'s; invalid drag-and-drop **unparented** the dropped object; `ImGui "#version 130"` in a 3.3-core context.

## 4. New features

- **Undo / Redo** (Ctrl+Z, Ctrl+Y / Ctrl+Shift+Z): add, delete (restores hierarchy, list position and animation items), duplicate, gizmo drags (one step per drag, whole selection), property edits, reparenting, reset transform.
- **Real File menu:** New, Open, Save, Save As (overwrite confirmation), Exit — with an **unsaved-changes prompt** on New / Open / close, and `*` in the window title.
- **One gizmo for the whole selection**, local/world space, optional snapping (Ctrl inverts).
- **Lighting:** directional diffuse + ambient (normals are now generated; objects with no normals stay unlit as before).
- **Camera smoothing** (toggle + speed), Focus Selected (`.`), Reset Camera.
- **Accurate picking** (ray vs. the mesh's real bounding box in local space — follows rotation, scale and parents).
- **Settings panel is now real** (it was empty headings): view, camera, gizmo, UI scale, **rebindable keybinds with conflict warning**, persisted to `settings.json`.
- **Timeline:** play/pause/stop/loop, FPS, last frame, item delete, curves drawn linear (what you see is what drives the object).
- **UI:** new theme, stats overlay (FPS / object count / selection), status toasts, About + Shortcuts, context menu in the object tree, unique auto-naming, new objects appear at the camera's focus point.
- **Rendering:** 4× MSAA, wireframe and grid toggles.
- **Start-up errors now show a dialog** instead of the app silently vanishing.

## 5. Behaviour changes you should know about

- **Esc no longer quits.** Close via the window button / Alt+F4 / File ▸ Exit (prompts if unsaved).
- Keybind names changed: the old "Open Save Dialog" (Ctrl+S) is now **Save** (Ctrl+S) and **Save As** (Ctrl+Shift+S). T / R / F gizmo keys unchanged.
- Multi-select modifier is **Ctrl or Shift** everywhere (the tree used Ctrl, the viewport used Shift).
- Re-parenting **keeps the object where it is in the world** (its local transform is recomputed). Duplicates likewise.
- Scene files gain `normal` per vertex and `textureOwned`; **old files still load** (tested with your `scene.json`). Texture paths are saved relative to `assets/`.
- Removing a texture keeps the object's last colour instead of forcing white.
- Scale in the Properties panel now allows 0.001–1000 (was clamped to 0.1–10); rotation is unclamped.

## 6. Verification — what was actually run

Environment: Ubuntu, GCC 13, the project's own vendored GLM / ImGui / ImGuizmo / ImGuiFileDialog / nlohmann / stb, system GLFW 3.3 + GLEW 2.2, Mesa software OpenGL under a virtual display.

| Check | Result |
|---|---|
| All 12 project `.cpp` files, `-Wall -Wextra` | **0 warnings** |
| `tests/test_core.cpp` — the *real* `Object`/`Scene`/`Geometry` with GL calls stubbed to track buffer lifetimes (copy independence, no double-free/leak, hierarchy, scene remove, save/load round trip, malformed & legacy files) | **27 / 27** |
| `tests/test_sequencer.cpp` — real `AnimationSequencer` linked with real ImGui | **15 / 15** |
| `tests/test_commands.cpp` — undo/redo incl. delete-parent-with-children-and-animation, history trimming, settings round trip | **18 / 18** |
| `tests/test_transform.cpp` — 2000 random transforms, gimbal lock, mirrored | new: error ~1e-5; **original gizmo maths wrong 2000/2000** |
| **Full engine, end-to-end:** real `Engine::init/update/render` (real GLFW window, real GLSL compile of the new lighting shaders, real ImGui frames) driven through a scripted session: select, gizmo, add/duplicate/undo/redo, parenting visible at render time, delete-parent, measurement lines, wireframe, animation with the row collapsed, save/load, **real ray picking**, close prompt, **your real `scene.json`**, shutdown with live objects | **31 / 31 checks, 0 GL errors in every frame, `cleanup()` twice is safe** |

Frames from that run were inspected visually (lit textured cube, gizmo, docked panels, prompt modal, toasts).

## 7. What I could NOT verify

- **No MSVC / Windows build.** Everything above is GCC/Linux. The `#ifdef _WIN32` parts (`Windows.h` in `Logger.h`/`main.cpp`, `MessageBox`, `wWinMain`) were reasoned about, not compiled. If MSVC complains, it will most likely be something small in those.
- **No real mouse/keyboard/GPU/DPI testing.** Input was driven by calling the engine's own functions, not by physical devices; software GL, not your graphics card.
- **The post-build step in the `.vcxproj` is untested** (it's Release|x64 only; delete the `<PostBuildEvent>` block if it misbehaves).
- Not done: installer/packaging, code signing, crash-dump reporting.

## 8. Known limitations / recommended next steps

- Not undoable: texture changes, measurement toggles, animation item add/remove, timeline curve edits.
- Picking uses the mesh's bounding box (much better than before, but a sphere's corners still count as a hit).
- Scene files reference parents **by name**; the loader now disambiguates duplicates, but unique IDs would be cleaner.
- Lighting is a fixed single light with no on/off toggle yet.
- **Dead dependency:** nothing in your code references the Discord SDK, yet the project compiles 14 of its `.cpp` files and links `discord_game_sdk.lib`, which makes the `.exe` require `discord_game_sdk.dll` at start-up. Removing them (and `stb_vorbis.c`, `imgui_demo.cpp`, `GraphEditor.cpp`, `ImGradient.cpp`) shrinks the build and removes a runtime DLL dependency. I left this alone because I can't build on Windows to confirm.
- **Hard-coded library path** in the `.vcxproj` (`C:\Users\Samuel\Documents\Visual Studio 2022\libraries\...`) means it won't build on another machine as-is; the `.lib` files in the project root already cover it, so it can likely be deleted.

## 9. Tests

`tests/` holds the four suites and the transform test. They compile with plain g++ against the project's vendored headers; the GL-dependent ones replace GLEW's function pointers with counting stubs (see the top of each file). They are not wired into the Visual Studio project.
