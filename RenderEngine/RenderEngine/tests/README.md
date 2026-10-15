# tests/

Not part of the Visual Studio build. Run from the project root (where `Engine.cpp` lives) on Linux/WSL with g++ 11+.

| File | What it runs |
|---|---|
| `test_transform.cpp` | `TransformUtil` round trips; shows the original gizmo rotation maths was wrong. Needs only GLM. |
| `test_core.cpp` | The real `Object`, `Scene`, `Geometry` with GL calls replaced by tracking stubs. |
| `test_sequencer.cpp` | The real `AnimationSequencer` linked with the real ImGui sources. |
| `test_commands.cpp` | Undo/redo commands, history trimming, settings round trip. |
| `test_engine_e2e.cpp` | The whole engine: real window, real GLSL, real ImGui frames, scripted session, `glGetError()` every frame, frame dumps. Needs system GLFW + GLEW and a display (e.g. `xvfb-run` with `LIBGL_ALWAYS_SOFTWARE=1`). Uses `#define private public` (test only). |

Example (unit tests that stub GL):

    INC="-I. -Iimgui -Ilibraries/glm -Ilibraries/glew/include -Ilibraries/glfw/include -Ilibraries/json-develop/single_include -Ilibraries/stb"
    g++ -std=c++17 -w -DGLEW_NO_GLU $INC tests/test_core.cpp Object.cpp Scene.cpp Geometry.cpp Texture.cpp Shader.cpp \
        -Wl,--unresolved-symbols=ignore-all -o /tmp/test_core && /tmp/test_core

Example (end to end; links the system GLFW/GLEW instead of the vendored Windows libs):

    SRCS="Engine.cpp Interface.cpp AnimationSequencer.cpp Object.cpp Scene.cpp Camera.cpp InputManager.cpp KeybindManager.cpp Shader.cpp Texture.cpp Geometry.cpp \
          imgui/imgui.cpp imgui/imgui_draw.cpp imgui/imgui_tables.cpp imgui/imgui_widgets.cpp imgui/imgui_impl_glfw.cpp imgui/imgui_impl_opengl3.cpp \
          imgui/ImGuiFileDialog-master/ImGuiFileDialog.cpp imgui/ImGuizmo-master/ImGuizmo.cpp imgui/ImGuizmo-master/ImCurveEdit.cpp imgui/ImGuizmo-master/ImSequencer.cpp"
    g++ -std=c++17 -O1 -w -DGLEW_NO_GLU -I. -Iimgui -Ilibraries/glm -Ilibraries/json-develop/single_include -Ilibraries/stb \
        tests/test_engine_e2e.cpp $SRCS -o /tmp/e2e -lglfw -lGLEW -lGL -ldl -lpthread
    LIBGL_ALWAYS_SOFTWARE=1 xvfb-run -a -s "-screen 0 1600x900x24" /tmp/e2e
