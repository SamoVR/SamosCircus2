#pragma once
//
// Settings.h -- user settings persisted to settings.json next to the .exe.
// Previously nothing survived a restart (rebound keys, toggles, last scene).
//
#include <cstdint>
#include <string>
#include <vector>
#include <fstream>
#include <nlohmann/json.hpp>
#include "Logger.h"

struct SavedKeybind {
    std::string name;
    int key = 0;
    bool ctrl = false, shift = false, alt = false;
};

struct AppSettings {
    // View
    bool showGrid = true;
    bool wireframe = false;
    bool vsync = true;
    bool showStats = true;
    // Camera
    bool cameraSmoothing = true;
    float cameraSmoothingSpeed = 12.0f;
    // Gizmo
    bool gizmoLocalSpace = false;
    bool snapEnabled = false;
    float snapTranslate = 0.5f, snapRotate = 15.0f, snapScale = 0.1f;
    // UI: multiplier applied on top of the monitor's DPI scale
    float uiScale = 1.0f;
    int layoutVersion = 0; // see Interface::update
    // Discord Rich Presence
    bool discordRichPresence = true;
    std::int64_t discordClientId = 0; // 0 = use RichPresence::kClientId
    // Files
    std::string lastScenePath;
    std::vector<SavedKeybind> keybinds;

    static constexpr const char* kFile = "settings.json";

    nlohmann::json toJson() const {
        nlohmann::json j;
        j["showGrid"] = showGrid;               j["wireframe"] = wireframe;
        j["vsync"] = vsync;                     j["showStats"] = showStats;
        j["cameraSmoothing"] = cameraSmoothing; j["cameraSmoothingSpeed"] = cameraSmoothingSpeed;
        j["gizmoLocalSpace"] = gizmoLocalSpace; j["snapEnabled"] = snapEnabled;
        j["snapTranslate"] = snapTranslate;     j["snapRotate"] = snapRotate; j["snapScale"] = snapScale;
        j["uiScale"] = uiScale;                 j["lastScenePath"] = lastScenePath;
        j["discordRichPresence"] = discordRichPresence; j["discordClientId"] = discordClientId; j["layoutVersion"] = layoutVersion;
        j["keybinds"] = nlohmann::json::array();
        for (const auto& k : keybinds) {
            j["keybinds"].push_back({ {"name", k.name}, {"key", k.key}, {"ctrl", k.ctrl}, {"shift", k.shift}, {"alt", k.alt} });
        }
        return j;
    }

    // Tolerant: a missing or wrongly-typed field keeps its default.
    void fromJson(const nlohmann::json& j) {
        auto get = [&j](const char* key, auto& out) {
            try { if (j.contains(key)) out = j.at(key).get<std::decay_t<decltype(out)>>(); }
            catch (const nlohmann::json::exception&) {}
        };
        get("showGrid", showGrid);   get("wireframe", wireframe);
        get("vsync", vsync);         get("showStats", showStats);
        get("cameraSmoothing", cameraSmoothing); get("cameraSmoothingSpeed", cameraSmoothingSpeed);
        get("gizmoLocalSpace", gizmoLocalSpace); get("snapEnabled", snapEnabled);
        get("snapTranslate", snapTranslate); get("snapRotate", snapRotate); get("snapScale", snapScale);
        get("uiScale", uiScale);     get("lastScenePath", lastScenePath);
        get("discordRichPresence", discordRichPresence); get("discordClientId", discordClientId); get("layoutVersion", layoutVersion);
        uiScale = uiScale < 0.5f ? 0.5f : (uiScale > 3.0f ? 3.0f : uiScale);

        keybinds.clear();
        try {
            if (j.contains("keybinds") && j["keybinds"].is_array()) {
                for (const auto& e : j["keybinds"]) {
                    SavedKeybind k;
                    k.name = e.at("name").get<std::string>();
                    k.key = e.at("key").get<int>();
                    k.ctrl = e.value("ctrl", false); k.shift = e.value("shift", false); k.alt = e.value("alt", false);
                    keybinds.push_back(k);
                }
            }
        } catch (const nlohmann::json::exception&) { keybinds.clear(); }
    }

    bool load() {
        std::ifstream in(kFile);
        if (!in.is_open()) return false; // first run: defaults
        try {
            nlohmann::json j; in >> j;
            fromJson(j);
            return true;
        } catch (const nlohmann::json::exception& e) {
            LOG_WARN("Settings", std::string("Ignoring unreadable settings.json: ") + e.what());
            return false;
        }
    }

    bool save() const {
        std::ofstream out(kFile);
        if (!out.is_open()) { LOG_WARN("Settings", "Could not write settings.json"); return false; }
        out << toJson().dump(2);
        return true;
    }
};
