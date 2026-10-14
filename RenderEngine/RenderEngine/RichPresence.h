#pragma once
//
// RichPresence.h -- Discord Rich Presence ("Playing RIFT Engine") through
// the bundled Discord Game SDK (libraries/discord-rpc + discord_game_sdk.dll).
//
// Completely optional at runtime: if Discord is not running, the DLL is
// missing callbacks fail, or the user turned it off in Settings, every call is
// a cheap no-op and the editor keeps working.
//
// SETUP: create an application at https://discord.com/developers/applications,
// name it "RIFT Engine" (that name is what Discord shows as "Playing ..."),
// upload the logo under Rich Presence > Art Assets with the key "rift_logo",
// and put the Application ID into kClientId below (or into settings.json as
// "discordClientId").
//
#include <cstdint>
#include <ctime>
#include <memory>
#include <string>

namespace discord { class Core; }

class RichPresence {
public:
    static constexpr std::int64_t kClientId = 1363649635603972227; // <-- Application ID from the Discord developer portal

    RichPresence() = default;
    ~RichPresence();
    RichPresence(const RichPresence&) = delete;
    RichPresence& operator=(const RichPresence&) = delete;

    // clientId == 0 -> uses kClientId; still 0 -> presence stays disabled.
    void init(std::int64_t clientId);
    void shutdown();

    // Call once per frame. Pushes a new activity only when something changed
    // (Discord rate-limits updates) and pumps the SDK callbacks.
    void update(float deltaTime);

    // details = first line ("Editing house.json"), state = second line ("12 objects").
    void setActivity(const std::string& details, const std::string& state);

    bool isConnected() const { return core_ != nullptr; }

private:
    discord::Core* core_ = nullptr;
    std::string details_, state_;
    bool dirty_ = false;
    float cooldown_ = 0.0f;
    float retryTimer_ = 0.0f;
    std::int64_t clientId_ = 0;
    std::int64_t startTime_ = static_cast<std::int64_t>(std::time(nullptr));
    void tryConnect();
    void push();
};
