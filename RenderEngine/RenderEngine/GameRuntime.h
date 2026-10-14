#pragma once
//
// GameRuntime.h -- play mode ("Play" / "Stop" in the toolbar, like Roblox
// Studio's Play button). While running it
//   * spawns a player character (WASD / Space, the camera follows it),
//   * simulates physics for parts with Anchored = off (gravity, collisions,
//     bounciness),
//   * runs every object's RiftScript (on start / on update / on touch).
// On Stop everything the game changed is put back exactly as it was, so the
// edited scene is never affected by a test run.
//
#include "Scene.h"
#include "Camera.h"
#include "InputManager.h"
#include "RiftScript.h"

#include <glm/glm.hpp>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

class GameRuntime : public rs::Host {
public:
    GameRuntime(Scene& scene, Camera& camera, InputManager& input);
    ~GameRuntime() override;

    void start();
    void stop();
    bool isRunning() const { return running_; }
    void togglePause() { paused_ = !paused_; }
    bool isPaused() const { return paused_; }

    // allowPlayerInput: false while the user types in a UI field.
    void update(float deltaTime, bool allowPlayerInput);

    // ---- output for the UI
    struct LogLine { std::string text; bool isError; float time; };
    const std::vector<LogLine>& log() const { return log_; }
    void clearLog() { log_.clear(); }
    const std::string& message() const { return message_; }
    float messageTimeLeft() const { return messageTimer_; }
    enum class Outcome { None, Won, Lost };
    Outcome outcome() const { return outcome_; }
    float elapsed() const { return time_; }
    const Object* player() const { return player_.get(); }

    // Checks a script for syntax errors (used by the script editor).
    static std::string checkSyntax(const std::string& source);

    // ---- tuning (shown in Settings > Game)
    float gravity = -30.0f;
    float walkSpeed = 8.0f;
    float jumpSpeed = 12.0f;
    float killPlaneY = -60.0f;

    // ---- rs::Host
    bool command(const std::string& name, const std::vector<rs::Value>& args, std::string& error) override;
    bool getField(const std::string& object, const std::string& field, rs::Value& out, std::string& error) override;
    bool callFunction(const std::string& name, const std::vector<rs::Value>& args, rs::Value& out, std::string& error) override;
    void print(const std::string& text) override;

    // World-space axis-aligned bounds (also used by the editor to drop new parts onto surfaces).
    struct Box { glm::vec3 lo, hi; };
    static Box worldBox(const Object& o);
    static bool overlaps(const Box& a, const Box& b, float margin = 0.0f);
    struct Obb { glm::vec3 center; glm::vec3 axis[3]; glm::vec3 half; };
    static Obb worldObb(const Object& o);
    // Separating-axis test; on overlap `push` moves `box` out of `obb`.
    static bool separate(const Box& box, const Obb& obb, glm::vec3& push, float margin = 0.0f);

private:
    Scene& scene_;
    Camera& camera_;
    InputManager& input_;

    bool running_ = false;
    bool paused_ = false;
    float time_ = 0.0f;

    // Everything play mode may change, captured on Play and restored on Stop.
    struct Snapshot {
        glm::vec3 position, rotation, scale, color;
        Object::GameProps game;
    };
    std::unordered_map<Object*, Snapshot> snapshot_;
    Camera::Orbit savedOrbit_{};

    std::unordered_map<Object*, std::unique_ptr<rs::Script>> scripts_;
    std::set<std::pair<const Object*, const Object*>> touching_; // contacts last frame (for "on touch" edges)

    // Player character: owned here (not in the Explorer, never saved); Engine draws it.
    std::unique_ptr<Object> player_;
    glm::vec3 playerVelocity_{ 0.0f };
    glm::vec3 spawnPoint_{ 0.0f, 3.0f, 0.0f };
    bool playerGrounded_ = false;

    std::vector<LogLine> log_;
    std::string message_;
    float messageTimer_ = 0.0f;
    Outcome outcome_ = Outcome::None;

    // Script execution context (valid only inside a script call)
    Object* self_ = nullptr;
    const Object* other_ = nullptr;


    void spawnPlayer();
    void updatePlayer(float dt, bool allowInput);
    void stepPhysics(float dt);
    // Pushes `box` (moving by `velocity`) out of every solid part except `self`.
    // Returns true if it ended up standing on something.
    bool resolveCollisions(Box& box, glm::vec3& velocity, const Object* self, float bounciness);
    void detectTouches();
    void fire(Object* object, const std::string& event, const Object* other = nullptr);
    void logLine(const std::string& text, bool isError);
    Object* resolveObject(const std::string& name) const;
};
