#include "GameRuntime.h"
#include "Geometry.h"
#include "Logger.h"

#include <GLFW/glfw3.h>
#include <algorithm>
#include <cctype>
#include <cmath>

namespace {
    constexpr const char* kPlayerName = "Player";
    const glm::vec3 kPlayerSize(0.8f, 1.8f, 0.8f);

    std::string lower(std::string s) {
        for (char& c : s) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        return s;
    }

    // key("W"), key("space"), key("left") ... -> GLFW key code, or -1.
    int keyCodeFor(const std::string& nameIn) {
        const std::string name = lower(nameIn);
        if (name.size() == 1) {
            const char c = name[0];
            if (c >= 'a' && c <= 'z') return GLFW_KEY_A + (c - 'a');
            if (c >= '0' && c <= '9') return GLFW_KEY_0 + (c - '0');
        }
        if (name == "space") return GLFW_KEY_SPACE;
        if (name == "shift") return GLFW_KEY_LEFT_SHIFT;
        if (name == "ctrl") return GLFW_KEY_LEFT_CONTROL;
        if (name == "enter") return GLFW_KEY_ENTER;
        if (name == "up") return GLFW_KEY_UP;
        if (name == "down") return GLFW_KEY_DOWN;
        if (name == "left") return GLFW_KEY_LEFT;
        if (name == "right") return GLFW_KEY_RIGHT;
        if (name == "tab") return GLFW_KEY_TAB;
        return -1;
    }

    bool needArgs(const std::string& name, const std::vector<rs::Value>& args, size_t n, std::string& error) {
        if (args.size() >= n) return true;
        error = name + " needs " + std::to_string(n) + " value(s)";
        return false;
    }

    float arg(const std::vector<rs::Value>& args, size_t i) {
        return i < args.size() && !args[i].isString ? static_cast<float>(args[i].number) : 0.0f;
    }
    glm::vec3 vec(const std::vector<rs::Value>& args) { return { arg(args, 0), arg(args, 1), arg(args, 2) }; }
}

GameRuntime::GameRuntime(Scene& scene, Camera& camera, InputManager& input)
    : scene_(scene), camera_(camera), input_(input) {}

GameRuntime::~GameRuntime() {
    if (running_) stop();
}

// ============================================================================
//  Start / stop
// ============================================================================

void GameRuntime::start() {
    if (running_) return;
    running_ = true;
    paused_ = false;
    time_ = 0.0f;
    outcome_ = Outcome::None;
    message_.clear();
    messageTimer_ = 0.0f;
    touching_.clear();
    log_.clear();
    savedOrbit_ = camera_.getOrbit();

    snapshot_.clear();
    for (Object* o : scene_.getObjects()) {
        snapshot_[o] = { o->position, o->rotation, o->scale, o->color, o->game };
        o->game.velocity = glm::vec3(0.0f);
    }

    spawnPlayer();
    // Third-person view behind/above the player, keeping the editor's heading.
    camera_.setOrbit({ 14.0f, savedOrbit_.yaw, 22.0f, player_->position });
    logLine("Game started. WASD / arrows = move, Space = jump, right-drag = look around, F5 = stop.", false);

    // Compile everything first so a broken script is reported before anything runs.
    scripts_.clear();
    for (Object* o : scene_.getObjects()) {
        if (o->game.script.find_first_not_of(" \t\r\n") == std::string::npos) continue;
        auto script = std::make_unique<rs::Script>();
        if (!script->compile(o->game.script)) {
            logLine("[" + o->name + "] syntax error, " + script->error(), true);
            continue;
        }
        scripts_[o] = std::move(script);
    }
    for (auto& [object, script] : scripts_) {
        self_ = object;
        script->setVariable("dt", rs::Value(0.0));
        script->setVariable("time", rs::Value(0.0));
        script->setVariable("other", rs::Value(std::string()));
        if (!script->runInit(*this)) logLine("[" + object->name + "] " + script->error(), true);
        self_ = nullptr;
    }
    for (auto& entry : scripts_) fire(entry.first, "start");
    LOG_INFO("Game", "Play mode started (" + std::to_string(scripts_.size()) + " script(s))");
}

void GameRuntime::stop() {
    if (!running_) return;
    running_ = false;
    scripts_.clear();
    touching_.clear();
    player_.reset();

    // Objects can't be added or removed while playing (the editor is locked),
    // but check anyway so a stale pointer is never written through.
    for (auto& [object, s] : snapshot_) {
        if (!scene_.contains(object)) continue;
        object->position = s.position;
        object->rotation = s.rotation;
        object->scale = s.scale;
        object->color = s.color;
        object->game = s.game;
        object->game.velocity = glm::vec3(0.0f);
    }
    snapshot_.clear();
    camera_.setOrbit(savedOrbit_); // back to the exact editor view
    message_.clear();
    messageTimer_ = 0.0f;
    LOG_INFO("Game", "Play mode stopped, scene restored");
}

void GameRuntime::spawnPlayer() {
    // A part named "SpawnPoint" (or "Spawn") decides where the player appears.
    spawnPoint_ = glm::vec3(0.0f, 3.0f, 0.0f);
    for (Object* o : scene_.getObjects()) {
        const std::string n = lower(o->name);
        if (n == "spawnpoint" || n == "spawn" || n == "spawnlocation") {
            const Box b = worldBox(*o);
            spawnPoint_ = glm::vec3((b.lo.x + b.hi.x) * 0.5f, b.hi.y + kPlayerSize.y * 0.5f + 0.05f, (b.lo.z + b.hi.z) * 0.5f);
            break;
        }
    }
    player_ = std::make_unique<Object>(kPlayerName, createCubeVertices(), static_cast<Texture*>(nullptr));
    player_->scale = kPlayerSize;
    player_->color = glm::vec3(0.25f, 0.55f, 1.0f);
    player_->position = spawnPoint_;
    playerVelocity_ = glm::vec3(0.0f);
    playerGrounded_ = false;
}

// ============================================================================
//  Frame
// ============================================================================

void GameRuntime::update(float dt, bool allowPlayerInput) {
    if (!running_) return;
    if (messageTimer_ > 0.0f) {
        messageTimer_ -= dt;
        if (messageTimer_ <= 0.0f && outcome_ == Outcome::None) message_.clear();
    }
    if (paused_ || outcome_ != Outcome::None) {
        if (player_) camera_.focusOn(player_->position + glm::vec3(0.0f, 0.6f, 0.0f));
        return;
    }
    time_ += dt;

    for (auto& [object, script] : scripts_) {
        script->setVariable("dt", rs::Value(static_cast<double>(dt)));
        script->setVariable("time", rs::Value(static_cast<double>(time_)));
    }
    for (auto& entry : scripts_) fire(entry.first, "update");

    stepPhysics(dt);
    updatePlayer(dt, allowPlayerInput);
    detectTouches();

    if (player_) camera_.focusOn(player_->position + glm::vec3(0.0f, 0.6f, 0.0f));
}

void GameRuntime::updatePlayer(float dt, bool allowInput) {
    if (!player_) return;

    glm::vec3 wish(0.0f);
    if (allowInput) {
        // Camera-relative movement on the ground plane.
        glm::vec3 forward = camera_.target - camera_.position;
        forward.y = 0.0f;
        forward = glm::length(forward) > 1e-4f ? glm::normalize(forward) : glm::vec3(0.0f, 0.0f, -1.0f);
        const glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0.0f, 1.0f, 0.0f)));

        if (input_.isKeyPressed(GLFW_KEY_W) || input_.isKeyPressed(GLFW_KEY_UP)) wish += forward;
        if (input_.isKeyPressed(GLFW_KEY_S) || input_.isKeyPressed(GLFW_KEY_DOWN)) wish -= forward;
        if (input_.isKeyPressed(GLFW_KEY_D) || input_.isKeyPressed(GLFW_KEY_RIGHT)) wish += right;
        if (input_.isKeyPressed(GLFW_KEY_A) || input_.isKeyPressed(GLFW_KEY_LEFT)) wish -= right;
        if (glm::length(wish) > 1e-4f) {
            wish = glm::normalize(wish);
            // Face the walking direction (yaw only).
            player_->rotation.y = glm::degrees(std::atan2(wish.x, wish.z));
        }
        if (playerGrounded_ && input_.isKeyPressed(GLFW_KEY_SPACE)) {
            playerVelocity_.y = jumpSpeed;
            playerGrounded_ = false;
        }
    }

    const float sprint = allowInput && input_.isKeyPressed(GLFW_KEY_LEFT_SHIFT) ? 1.6f : 1.0f;
    playerVelocity_.x = wish.x * walkSpeed * sprint;
    playerVelocity_.z = wish.z * walkSpeed * sprint;
    playerVelocity_.y += gravity * dt;

    player_->position += playerVelocity_ * dt;
    Box box{ player_->position - kPlayerSize * 0.5f, player_->position + kPlayerSize * 0.5f };
    playerGrounded_ = resolveCollisions(box, playerVelocity_, nullptr, 0.0f);
    player_->position = (box.lo + box.hi) * 0.5f;

    if (player_->position.y < killPlaneY) {
        logLine("Player fell out of the world, respawning.", false);
        player_->position = spawnPoint_;
        playerVelocity_ = glm::vec3(0.0f);
    }
}

// Simple, predictable physics: unanchored ROOT parts fall and are pushed out
// of solid parts along the axis of least penetration. Children of a moving
// part simply move with it (that's how the hierarchy already works).
void GameRuntime::stepPhysics(float dt) {
    for (Object* o : scene_.getObjects()) {
        if (o->game.anchored || o->parent) continue;

        glm::vec3& v = o->game.velocity;
        v.y += gravity * dt;
        o->position += v * dt;

        Box box = worldBox(*o);
        const glm::vec3 centerBefore = (box.lo + box.hi) * 0.5f;
        if (o->game.canCollide) {
            if (resolveCollisions(box, v, o, o->game.bounciness)) {
                v.x *= std::max(0.0f, 1.0f - 6.0f * dt); // ground friction
                v.z *= std::max(0.0f, 1.0f - 6.0f * dt);
            }
            o->position += (box.lo + box.hi) * 0.5f - centerBefore;
        }

        if (o->position.y < killPlaneY) { // fell out of the world: Roblox "destroys" it
            o->game.visible = false;
            o->game.anchored = true;
            v = glm::vec3(0.0f);
        }
    }
}

// Collision against each part's ORIENTED box. Using the axis-aligned box
// made rotated parts far bigger than they look (a spinning 8 x 0.8 bar
// blocked an 8 x 8 square), so walls/ramps/spinners felt invisible.
bool GameRuntime::resolveCollisions(Box& box, glm::vec3& velocity, const Object* self, float bounciness) {
    bool grounded = false;
    for (Object* other : scene_.getObjects()) {
        if (other == self || !other->game.canCollide || !other->game.visible) continue;
        if (self && (other->isChildOf(const_cast<Object*>(self)) || self->isChildOf(other))) continue;
        const Obb ob = worldObb(*other);
        glm::vec3 push;
        if (!separate(box, ob, push)) continue;
        float depth = glm::length(push);
        if (depth < 1e-6f) continue;
        glm::vec3 normal = push / depth;

        // Small ledges are stepped onto instead of blocking the walk.
        const float top = ob.center.y + std::fabs(ob.axis[0].y) * ob.half.x + std::fabs(ob.axis[1].y) * ob.half.y + std::fabs(ob.axis[2].y) * ob.half.z;
        const float stepUp = top - box.lo.y;
        if (normal.y < 0.7f && stepUp > 0.0f && stepUp < 0.35f && velocity.y <= 0.0f) {
            normal = glm::vec3(0.0f, 1.0f, 0.0f);
            depth = stepUp;
        }

        box.lo += normal * depth;
        box.hi += normal * depth;
        const float into = glm::dot(velocity, normal);
        if (into < 0.0f) {
            velocity -= (1.0f + bounciness) * into * normal;
            if (std::fabs(glm::dot(velocity, normal)) < 0.5f) velocity -= glm::dot(velocity, normal) * normal; // no micro-bounces
        }
        if (normal.y > 0.7f) grounded = true;
    }
    return grounded;
}

void GameRuntime::detectTouches() {
    std::set<std::pair<const Object*, const Object*>> now;
    const std::vector<Object*> objects = scene_.getObjects();

    auto contact = [&](Object* a, const Object* b) {
        const std::pair<const Object*, const Object*> key(a, b);
        now.insert(key);
        if (!touching_.count(key)) fire(a, "touch", b); // fires once when the contact begins
    };

    std::vector<std::pair<Object*, Box>> boxes;
    for (Object* o : objects) {
        if (o->game.visible) boxes.emplace_back(o, worldBox(*o));
    }
    glm::vec3 scratch;
    const Box playerBox = player_ ? Box{ player_->position - kPlayerSize * 0.5f, player_->position + kPlayerSize * 0.5f } : Box{};

    for (size_t i = 0; i < boxes.size(); ++i) {
        Object* a = boxes[i].first;
        const bool aListens = scripts_.count(a) && scripts_[a]->hasHandler("touch");
        if (player_ && aListens && separate(playerBox, worldObb(*a), scratch, 0.05f)) contact(a, player_.get());
        if (!aListens) continue;
        for (size_t j = 0; j < boxes.size(); ++j) {
            if (i == j) continue;
            Object* b = boxes[j].first;
            if (a->isChildOf(b) || b->isChildOf(a)) continue;
            if (separate(boxes[i].second, worldObb(*b), scratch, 0.02f)) contact(a, b);
        }
    }
    touching_ = std::move(now);
}

void GameRuntime::fire(Object* object, const std::string& event, const Object* other) {
    auto it = scripts_.find(object);
    if (it == scripts_.end() || it->second->failed() || !it->second->hasHandler(event)) return;
    self_ = object;
    other_ = other;
    it->second->setVariable("other", rs::Value(other ? other->name : std::string()));
    if (!it->second->fire(event, *this)) {
        logLine("[" + object->name + "] error, " + it->second->error() + " (script stopped)", true);
    }
    self_ = nullptr;
    other_ = nullptr;
}

// ============================================================================
//  Helpers
// ============================================================================

GameRuntime::Box GameRuntime::worldBox(const Object& o) {
    const glm::mat4 m = o.getGlobalTransform();
    const glm::vec3 lo = o.getBoundsMin(), hi = o.getBoundsMax();
    Box b{ glm::vec3(1e30f), glm::vec3(-1e30f) };
    for (int i = 0; i < 8; ++i) {
        const glm::vec3 corner((i & 1) ? hi.x : lo.x, (i & 2) ? hi.y : lo.y, (i & 4) ? hi.z : lo.z);
        const glm::vec3 w = glm::vec3(m * glm::vec4(corner, 1.0f));
        b.lo = glm::min(b.lo, w);
        b.hi = glm::max(b.hi, w);
    }
    return b;
}

bool GameRuntime::overlaps(const Box& a, const Box& b, float margin) {
    return a.lo.x < b.hi.x + margin && a.hi.x > b.lo.x - margin &&
           a.lo.y < b.hi.y + margin && a.hi.y > b.lo.y - margin &&
           a.lo.z < b.hi.z + margin && a.hi.z > b.lo.z - margin;
}

void GameRuntime::logLine(const std::string& text, bool isError) {
    log_.push_back({ text, isError, time_ });
    if (log_.size() > 500) log_.erase(log_.begin(), log_.begin() + 100);
    if (isError) LOG_WARN("Script", text);
}

Object* GameRuntime::resolveObject(const std::string& name) const {
    if (name == "self") return self_;
    if (name == "other") return const_cast<Object*>(other_);
    if (name == "player") return player_.get();
    return scene_.findByName(name);
}

std::string GameRuntime::checkSyntax(const std::string& source) {
    rs::Script script;
    return script.compile(source) ? std::string() : script.error();
}

// ============================================================================
//  rs::Host -- what scripts can do
// ============================================================================

void GameRuntime::print(const std::string& text) {
    logLine(self_ ? "[" + self_->name + "] " + text : text, false);
}

bool GameRuntime::command(const std::string& name, const std::vector<rs::Value>& args, std::string& error) {
    Object* o = self_;
    if (!o) { error = "no object"; return false; }

    if (name == "move")     { if (!needArgs(name, args, 3, error)) return false; o->position += vec(args); return true; }
    if (name == "rotate")   { if (!needArgs(name, args, 3, error)) return false; o->rotation += vec(args); return true; }
    if (name == "setpos")   { if (!needArgs(name, args, 3, error)) return false; o->position = vec(args); return true; }
    if (name == "setrot")   { if (!needArgs(name, args, 3, error)) return false; o->rotation = vec(args); return true; }
    if (name == "setscale") { if (!needArgs(name, args, 3, error)) return false; o->scale = glm::max(vec(args), glm::vec3(0.001f)); return true; }
    if (name == "color")    { if (!needArgs(name, args, 3, error)) return false; o->color = glm::clamp(vec(args), 0.0f, 1.0f); return true; }
    if (name == "velocity") { if (!needArgs(name, args, 3, error)) return false; o->game.velocity = vec(args); return true; }
    if (name == "push")     { if (!needArgs(name, args, 3, error)) return false; o->game.velocity += vec(args); return true; }
    if (name == "anchored") { o->game.anchored = args.empty() || args[0].truthy(); if (o->game.anchored) o->game.velocity = glm::vec3(0.0f); return true; }
    if (name == "collide")  { o->game.canCollide = args.empty() || args[0].truthy(); return true; }
    if (name == "hide")     { o->game.visible = false; return true; }
    if (name == "show")     { o->game.visible = true; return true; }
    if (name == "message") {
        if (!needArgs(name, args, 1, error)) return false;
        message_ = args[0].toString();
        messageTimer_ = args.size() > 1 ? arg(args, 1) : 3.0f;
        return true;
    }
    if (name == "teleport") {
        if (!needArgs(name, args, 3, error)) return false;
        if (player_) { player_->position = vec(args); playerVelocity_ = glm::vec3(0.0f); }
        return true;
    }
    if (name == "respawn") {
        if (player_) { player_->position = spawnPoint_; playerVelocity_ = glm::vec3(0.0f); }
        return true;
    }
    if (name == "launch") {
        if (!needArgs(name, args, 3, error)) return false;
        playerVelocity_ = vec(args);
        playerGrounded_ = false;
        return true;
    }
    if (name == "win" || name == "lose") {
        outcome_ = name == "win" ? Outcome::Won : Outcome::Lost;
        message_ = args.empty() ? (name == "win" ? "You win!" : "Game over") : args[0].toString();
        messageTimer_ = 1e9f;
        logLine(message_, false);
        return true;
    }
    error = "unknown command '" + name + "'";
    return false;
}

bool GameRuntime::getField(const std::string& objectName, const std::string& field, rs::Value& out, std::string& error) {
    const Object* o = resolveObject(objectName);
    if (!o) {
        error = objectName == "other" ? "'other' only exists inside 'on touch'" : "no object named '" + objectName + "'";
        return false;
    }
    const glm::vec3 vel = o == player_.get() ? playerVelocity_ : o->game.velocity;
    if (field == "name") { out = rs::Value(o->name); return true; }
    if (field == "x") { out = rs::Value(o->position.x); return true; }
    if (field == "y") { out = rs::Value(o->position.y); return true; }
    if (field == "z") { out = rs::Value(o->position.z); return true; }
    if (field == "rx") { out = rs::Value(o->rotation.x); return true; }
    if (field == "ry") { out = rs::Value(o->rotation.y); return true; }
    if (field == "rz") { out = rs::Value(o->rotation.z); return true; }
    if (field == "sx") { out = rs::Value(o->scale.x); return true; }
    if (field == "sy") { out = rs::Value(o->scale.y); return true; }
    if (field == "sz") { out = rs::Value(o->scale.z); return true; }
    if (field == "vx") { out = rs::Value(vel.x); return true; }
    if (field == "vy") { out = rs::Value(vel.y); return true; }
    if (field == "vz") { out = rs::Value(vel.z); return true; }
    if (field == "anchored") { out = rs::Value(o->game.anchored ? 1.0 : 0.0); return true; }
    if (field == "visible") { out = rs::Value(o->game.visible ? 1.0 : 0.0); return true; }
    error = "unknown field '" + field + "'";
    return false;
}

bool GameRuntime::callFunction(const std::string& name, const std::vector<rs::Value>& args, rs::Value& out, std::string& error) {
    if (name == "key") {
        if (args.empty()) { error = "key() needs a key name, e.g. key(\"E\")"; return false; }
        const int code = keyCodeFor(args[0].toString());
        if (code < 0) { error = "unknown key '" + args[0].toString() + "'"; return false; }
        out = rs::Value(input_.isKeyPressed(code) ? 1.0 : 0.0);
        return true;
    }
    if (name == "distance") {
        // distance("Part") -> from self to that part; distance() -> from self to the player
        const Object* a = self_;
        const Object* b = args.empty() ? player_.get() : resolveObject(args[0].toString());
        if (!a || !b) { error = "distance(): object not found"; return false; }
        out = rs::Value(glm::length(glm::vec3(a->getGlobalTransform()[3]) - glm::vec3(b->getGlobalTransform()[3])));
        return true;
    }
    if (name == "exists") {
        const Object* o = args.empty() ? nullptr : resolveObject(args[0].toString());
        out = rs::Value(o && o->game.visible ? 1.0 : 0.0);
        return true;
    }
    error = "unknown function '" + name + "'";
    return false;
}

GameRuntime::Obb GameRuntime::worldObb(const Object& o) {
    const glm::mat4 m = o.getGlobalTransform();
    const glm::vec3 lo = o.getBoundsMin(), hi = o.getBoundsMax();
    Obb b;
    b.center = glm::vec3(m * glm::vec4((lo + hi) * 0.5f, 1.0f));
    for (int i = 0; i < 3; ++i) {
        const glm::vec3 column(m[i]);
        const float length = glm::length(column);
        b.axis[i] = length > 1e-6f ? column / length : glm::vec3(i == 0, i == 1, i == 2);
        b.half[i] = (hi[i] - lo[i]) * 0.5f * length;
    }
    return b;
}

// Separating-axis test of an axis-aligned box against an oriented box (the 3
// world axes, the 3 box axes and their 9 cross products). On overlap `push`
// is the smallest move that takes `box` out of `obb`.
bool GameRuntime::separate(const Box& box, const Obb& obb, glm::vec3& push, float margin) {
    const glm::vec3 centerA = (box.lo + box.hi) * 0.5f;
    const glm::vec3 halfA = (box.hi - box.lo) * 0.5f;
    const glm::vec3 d = obb.center - centerA;
    float best = 1e30f;
    bool found = false;

    auto test = [&](glm::vec3 axis, float preference) {
        const float length = glm::length(axis);
        if (length < 1e-6f) return true; // parallel edges: no information
        axis /= length;
        const float rA = halfA.x * std::fabs(axis.x) + halfA.y * std::fabs(axis.y) + halfA.z * std::fabs(axis.z);
        float rB = 0.0f;
        for (int i = 0; i < 3; ++i) rB += obb.half[i] * std::fabs(glm::dot(obb.axis[i], axis));
        const float dist = glm::dot(d, axis);
        const float overlap = rA + rB - std::fabs(dist);
        if (overlap <= -margin) return false;
        // Face axes are preferred over edge axes of similar depth (steadier contacts).
        if (overlap * preference < best) {
            best = overlap * preference;
            push = (dist > 0.0f ? -axis : axis) * overlap;
            found = true;
        }
        return true;
    };

    const glm::vec3 world[3] = { {1, 0, 0}, {0, 1, 0}, {0, 0, 1} };
    for (const auto& a : world) if (!test(a, 1.0f)) return false;
    for (const auto& a : obb.axis) if (!test(a, 1.0f)) return false;
    for (const auto& a : world)
        for (const auto& b : obb.axis)
            if (!test(glm::cross(a, b), 1.15f)) return false;
    return found;
}
