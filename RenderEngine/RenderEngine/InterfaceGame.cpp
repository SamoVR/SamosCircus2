// InterfaceGame.cpp -- the game-engine side of the editor UI: Play/Stop,
// Insert templates, per-object game properties, the Script Editor and the
// Output console. (The scene-editing UI is in Interface.cpp.)
#include "Interface.h"
#include "misc/cpp/imgui_stdlib.h"
#include <algorithm>
#include <cfloat>

void Interface::togglePlay() {
    if (!runtime) return;
    if (runtime->isRunning()) {
        runtime->stop();
        showStatus("Stopped - scene restored to its state before Play");
        return;
    }
    clearSelection();
    playing = false; // timeline playback would fight the simulation
    showOutput = true;
    runtime->start();
}

namespace {
    // Scripts used by Insert > Scripted templates. They double as examples
    // of RiftScript for people writing their first script.
    const char* kKillBrickScript =
        "-- Sends the player back to the spawn point\n"
        "on touch\n"
        "    if other == \"Player\" then\n"
        "        message \"Ouch! Back to spawn.\", 2\n"
        "        respawn\n"
        "    end\n"
        "end\n";
    const char* kSpinnerScript =
        "var speed = 90  -- degrees per second\n"
        "\n"
        "on update\n"
        "    rotate 0, speed * dt, 0\n"
        "end\n";
    const char* kCoinScript =
        "var startY = 0\n"
        "\n"
        "on start\n"
        "    startY = self.y\n"
        "end\n"
        "\n"
        "on update\n"
        "    rotate 0, 120 * dt, 0\n"
        "    setpos self.x, startY + sin(time * 180) * 0.25, self.z\n"
        "end\n"
        "\n"
        "on touch\n"
        "    if other == \"Player\" then\n"
        "        print \"Coin collected!\"\n"
        "        hide\n"
        "    end\n"
        "end\n";
    const char* kJumpPadScript =
        "var power = 28\n"
        "\n"
        "on touch\n"
        "    if other == \"Player\" then\n"
        "        launch 0, power, 0\n"
        "    end\n"
        "end\n";
    const char* kFinishScript =
        "on touch\n"
        "    if other == \"Player\" then\n"
        "        win \"Finished in \" .. round(time) .. \" s!\"\n"
        "    end\n"
        "end\n";
    const char* kEmptyScript =
        "-- RiftScript: runs while the game is playing (Game > Play / F5)\n"
        "\n"
        "on start\n"
        "    print \"Hello from \" .. self.name\n"
        "end\n"
        "\n"
        "on update\n"
        "    -- runs every frame, dt = seconds since the last frame\n"
        "end\n"
        "\n"
        "on touch\n"
        "    -- other = name of what touched this part (\"Player\" for the player)\n"
        "end\n";
}

void Interface::insertTemplate(const std::string& kind) {
    std::vector<Vertex> mesh = createCubeVertices();
    if (kind == "Sphere" || kind == "Coin") mesh = createSphereVertices();
    else if (kind == "Wedge") mesh = createTriangle3DVertices();

    Texture* texture = defaultTexture->isValid() ? defaultTexture.get() : nullptr;
    const bool plain = kind == "Part" || kind == "Sphere" || kind == "Wedge";
    auto object = std::make_unique<Object>(makeUniqueName(kind), mesh, plain ? texture : nullptr);
    object->position = camera->target;

    auto& g = object->game;
    if (kind == "Baseplate") {
        object->position = glm::vec3(0.0f, -0.5f, 0.0f);
        object->scale = glm::vec3(60.0f, 1.0f, 60.0f);
        object->color = glm::vec3(0.36f, 0.38f, 0.40f);
    } else if (kind == "SpawnPoint") {
        object->scale = glm::vec3(3.0f, 0.4f, 3.0f);
        object->color = glm::vec3(0.30f, 0.80f, 0.45f);
    } else if (kind == "KillBrick") {
        object->scale = glm::vec3(4.0f, 0.4f, 4.0f);
        object->color = glm::vec3(0.90f, 0.15f, 0.15f);
        g.script = kKillBrickScript;
    } else if (kind == "Spinner") {
        object->scale = glm::vec3(8.0f, 0.5f, 0.8f);
        object->color = glm::vec3(0.95f, 0.55f, 0.10f);
        g.script = kSpinnerScript;
    } else if (kind == "Coin") {
        object->scale = glm::vec3(0.7f);
        object->color = glm::vec3(1.0f, 0.85f, 0.10f);
        g.canCollide = false;
        g.script = kCoinScript;
    } else if (kind == "JumpPad") {
        object->scale = glm::vec3(2.0f, 0.3f, 2.0f);
        object->color = glm::vec3(0.20f, 0.85f, 0.95f);
        g.script = kJumpPadScript;
    } else if (kind == "FallingBlock") {
        object->position += glm::vec3(0.0f, 5.0f, 0.0f);
        object->color = glm::vec3(0.60f, 0.35f, 0.90f);
        g.anchored = false;
        g.bounciness = 0.3f;
    } else if (kind == "Finish") {
        object->scale = glm::vec3(4.0f, 0.3f, 4.0f);
        object->color = glm::vec3(1.0f, 0.80f, 0.20f);
        g.script = kFinishScript;
    }

    // Like Roblox Studio: a new part lands ON whatever is under the camera
    // pivot instead of inside it (all inserts used to pile up at one spot).
    if (kind != "Baseplate" && kind != "FallingBlock") {
        GameRuntime::Box box = GameRuntime::worldBox(*object);
        float floorY = -1e30f;
        for (Object* other : scene.getObjects()) {
            const GameRuntime::Box ob = GameRuntime::worldBox(*other);
            const bool underneath = box.lo.x < ob.hi.x && box.hi.x > ob.lo.x && box.lo.z < ob.hi.z && box.hi.z > ob.lo.z;
            if (underneath) floorY = std::max(floorY, ob.hi.y);
        }
        if (floorY < -1e29f) floorY = 0.0f; // nothing below: stand on the ground plane
        object->position.y += floorY - box.lo.y;
    }

    Object* raw = object.get();
    history.execute(std::make_unique<AddObjectCommand>(scene, std::move(object), "Insert " + kind));
    selectObject(raw, false);
    showStatus("Inserted " + raw->name);
}

void Interface::gamePropertiesUI(Object* object) {
    pushHeading("Game");
    auto& g = object->game;
    ImGui::Checkbox("Anchored", &g.anchored);
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Anchored parts are never moved by physics.\nTurn off to make the part fall and collide.");
    ImGui::SameLine();
    ImGui::Checkbox("Can Collide", &g.canCollide);
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Off: the player and other parts pass through (touch events still fire).");
    ImGui::SameLine();
    ImGui::Checkbox("Visible", &g.visible);
    if (!g.anchored) ImGui::SliderFloat("Bounciness", &g.bounciness, 0.0f, 1.0f, "%.2f");

    const bool hasScript = g.script.find_first_not_of(" \t\r\n") != std::string::npos;
    if (hasScript) {
        const long lines = static_cast<long>(std::count(g.script.begin(), g.script.end(), '\n')) + 1;
        ImGui::Text(ICON_FA_CODE " Script (%ld lines)", lines);
        ImGui::SameLine();
        if (ImGui::SmallButton("Edit")) {
            scriptTarget = object;
            showScriptEditor = true;
            scriptCheckResult = GameRuntime::checkSyntax(g.script);
        }
        ImGui::SameLine();
        if (ImGui::SmallButton("Remove")) g.script.clear();
    } else if (ImGui::Button(ICON_FA_CODE " Add Script")) {
        g.script = kEmptyScript;
        scriptTarget = object;
        showScriptEditor = true;
        scriptCheckResult.clear();
    }
}

void Interface::scriptEditorUI() {
    if (!showScriptEditor) return;
    ImGui::SetNextWindowSize(ImVec2(620.0f, 520.0f), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("Script Editor", &showScriptEditor)) { ImGui::End(); return; }

    if (!scriptTarget) {
        ImGui::TextDisabled("No script open.");
        ImGui::TextDisabled("Select an object and use Properties > Game > Add Script / Edit.");
        ImGui::End();
        return;
    }

    pushHeading(scriptTarget->name.c_str());
    ImGui::SameLine();
    if (scriptCheckResult.empty()) ImGui::TextColored(ImVec4(0.4f, 0.9f, 0.5f, 1.0f), ICON_FA_CHECK " no syntax errors");
    else ImGui::TextColored(ImVec4(1.0f, 0.45f, 0.40f, 1.0f), ICON_FA_EXCLAMATION_TRIANGLE " %s", scriptCheckResult.c_str());

    const bool locked = isPlaying();
    if (locked) {
        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.3f, 1.0f), "Stop the game to edit scripts (changes would be lost on Stop).");
        ImGui::BeginDisabled();
    }

    if (ImGui::CollapsingHeader("Language reference")) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.75f, 0.82f, 0.95f, 1.0f));
        ImGui::TextUnformatted(rs::languageReference());
        ImGui::PopStyleColor();
    }

    const ImVec2 size(-FLT_MIN, std::max(120.0f, ImGui::GetContentRegionAvail().y));
    if (ImGui::InputTextMultiline("##source", &scriptTarget->game.script, size, ImGuiInputTextFlags_AllowTabInput)) {
        scriptCheckResult = GameRuntime::checkSyntax(scriptTarget->game.script); // live: compiling is cheap
    }

    if (locked) ImGui::EndDisabled();
    ImGui::End();
}

void Interface::outputUI() {
    if (!showOutput || !runtime) return;
    if (!ImGui::Begin("Output", &showOutput)) { ImGui::End(); return; }

    if (ImGui::SmallButton("Clear")) runtime->clearLog();
    ImGui::Separator();

    ImGui::BeginChild("##log", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);
    for (const auto& line : runtime->log()) {
        ImGui::TextDisabled("%6.2f", line.time);
        ImGui::SameLine();
        if (line.isError) ImGui::TextColored(ImVec4(1.0f, 0.45f, 0.40f, 1.0f), "%s", line.text.c_str());
        else ImGui::TextUnformatted(line.text.c_str());
    }
    if (runtime->log().size() != lastLogSize) { // follow new output
        lastLogSize = runtime->log().size();
        ImGui::SetScrollHereY(1.0f);
    }
    ImGui::EndChild();
    ImGui::End();
}

void Interface::playOverlay() {
    if (!isPlaying()) return;
    const ViewArea* vp = &viewArea;
    ImDrawList* draw = ImGui::GetForegroundDrawList();

    // Coloured frame around the window = "you are in play mode".
    draw->AddRect(vp->WorkPos, ImVec2(vp->WorkPos.x + vp->WorkSize.x, vp->WorkPos.y + vp->WorkSize.y),
                  IM_COL32(60, 200, 110, 200), 0.0f, 0, 3.0f);

    const std::string& message = runtime->message();
    if (!message.empty()) {
        ImFont* font = headingFont ? headingFont : ImGui::GetFont();
        const float fontSize = font->FontSize * 2.0f * settings.uiScale;
        const ImVec2 textSize = font->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, message.c_str());
        const ImVec2 center(vp->WorkPos.x + vp->WorkSize.x * 0.5f, vp->WorkPos.y + vp->WorkSize.y * 0.22f);
        const ImVec2 pos(center.x - textSize.x * 0.5f, center.y - textSize.y * 0.5f);
        draw->AddRectFilled(ImVec2(pos.x - 18, pos.y - 10), ImVec2(pos.x + textSize.x + 18, pos.y + textSize.y + 10), IM_COL32(10, 12, 20, 190), 8.0f);
        ImU32 color = IM_COL32(255, 255, 255, 255);
        if (runtime->outcome() == GameRuntime::Outcome::Won) color = IM_COL32(255, 215, 70, 255);
        if (runtime->outcome() == GameRuntime::Outcome::Lost) color = IM_COL32(255, 95, 85, 255);
        draw->AddText(font, fontSize, pos, color, message.c_str());
    }

    const char* hint = runtime->outcome() != GameRuntime::Outcome::None ? "Game over - press F5 to stop and return to the editor"
                     : runtime->isPaused() ? "PAUSED"
                     : "WASD move  |  Space jump  |  Shift sprint  |  Right-drag look  |  F5 stop";
    const ImVec2 hintSize = ImGui::CalcTextSize(hint);
    const ImVec2 hintPos(vp->WorkPos.x + (vp->WorkSize.x - hintSize.x) * 0.5f, vp->WorkPos.y + vp->WorkSize.y - hintSize.y - 14.0f);
    draw->AddRectFilled(ImVec2(hintPos.x - 10, hintPos.y - 5), ImVec2(hintPos.x + hintSize.x + 10, hintPos.y + hintSize.y + 5), IM_COL32(10, 12, 20, 160), 6.0f);
    draw->AddText(hintPos, IM_COL32(220, 230, 245, 255), hint);
}

// Roblox-Studio-like arrangement: toolbar on top, Explorer over Properties on
// the right, Script Editor / Settings on the left, Output / Timeline below,
// the 3D view in the middle.
void Interface::buildDefaultLayout(ImGuiID dockspaceId) {
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::DockBuilderRemoveNode(dockspaceId);
    ImGui::DockBuilderAddNode(dockspaceId, ImGuiDockNodeFlags_DockSpace | ImGuiDockNodeFlags_PassthruCentralNode);
    ImGui::DockBuilderSetNodeSize(dockspaceId, viewport->WorkSize);

    ImGuiID center = dockspaceId;
    const ImGuiID top = ImGui::DockBuilderSplitNode(center, ImGuiDir_Up, 0.08f, nullptr, &center);
    ImGuiID right = ImGui::DockBuilderSplitNode(center, ImGuiDir_Right, 0.22f, nullptr, &center);
    const ImGuiID rightBottom = ImGui::DockBuilderSplitNode(right, ImGuiDir_Down, 0.55f, nullptr, &right);
    const ImGuiID left = ImGui::DockBuilderSplitNode(center, ImGuiDir_Left, 0.26f, nullptr, &center);
    const ImGuiID bottom = ImGui::DockBuilderSplitNode(center, ImGuiDir_Down, 0.26f, nullptr, &center);

    ImGui::DockBuilderDockWindow("Toolbar###Scene Controls", top);
    ImGui::DockBuilderDockWindow("Explorer###Object List", right);
    ImGui::DockBuilderDockWindow("Properties", rightBottom);
    ImGui::DockBuilderDockWindow("Script Editor", left);
    ImGui::DockBuilderDockWindow("Settings", left);
    ImGui::DockBuilderDockWindow("Output", bottom);
    ImGui::DockBuilderDockWindow("Timeline", bottom);
    if (ImGuiDockNode* toolbarNode = ImGui::DockBuilderGetNode(top)) {
        toolbarNode->LocalFlags |= ImGuiDockNodeFlags_NoTabBar | ImGuiDockNodeFlags_NoResize;
    }
    ImGui::DockBuilderFinish(dockspaceId);
}
