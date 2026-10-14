#include "RichPresence.h"
#include "Logger.h"
#include "discord.h"

RichPresence::~RichPresence() {
    shutdown();
}

void RichPresence::init(std::int64_t clientId) {
    clientId_ = clientId != 0 ? clientId : kClientId;
    if (clientId_ == 0) {
        LOG_INFO("Discord", "Rich Presence disabled: no Discord Application ID set (RichPresence::kClientId / settings.json).");
        return;
    }
    tryConnect();
}

void RichPresence::tryConnect() {
    discord::Core* core = nullptr;
    // NoRequireDiscord: if Discord isn't running, fail quietly instead of
    // trying to launch it / closing the process.
    const discord::Result result = discord::Core::Create(clientId_, DiscordCreateFlags_NoRequireDiscord, &core);
    if (result != discord::Result::Ok || !core) {
        retryTimer_ = 15.0f; // Discord may be started later
        return;
    }
    core_ = core;
    core_->SetLogHook(discord::LogLevel::Warn, [](discord::LogLevel, const char* message) {
        LOG_WARN("Discord", message);
    });
    LOG_INFO("Discord", "Connected to Discord, Rich Presence active.");
    dirty_ = true;
}

void RichPresence::shutdown() {
    if (!core_) return;
    core_->ActivityManager().ClearActivity([](discord::Result) {});
    core_->RunCallbacks();
    delete core_;
    core_ = nullptr;
}

void RichPresence::setActivity(const std::string& details, const std::string& state) {
    if (details == details_ && state == state_) return;
    details_ = details;
    state_ = state;
    dirty_ = true;
}

void RichPresence::update(float deltaTime) {
    if (clientId_ == 0) return;
    if (!core_) {
        retryTimer_ -= deltaTime;
        if (retryTimer_ <= 0.0f) tryConnect();
        return;
    }

    cooldown_ -= deltaTime;
    if (dirty_ && cooldown_ <= 0.0f) push();

    if (core_->RunCallbacks() != discord::Result::Ok) {
        // Discord was closed: drop the connection and retry later.
        delete core_;
        core_ = nullptr;
        retryTimer_ = 15.0f;
    }
}

void RichPresence::push() {
    discord::Activity activity{};
    activity.SetType(discord::ActivityType::Playing);
    activity.SetDetails(details_.c_str());
    activity.SetState(state_.c_str());
    activity.GetTimestamps().SetStart(startTime_);
    activity.GetAssets().SetLargeImage("rift_logo");
    activity.GetAssets().SetLargeText("RIFT Engine");
    core_->ActivityManager().UpdateActivity(activity, [](discord::Result result) {
        if (result != discord::Result::Ok) LOG_WARN("Discord", "UpdateActivity failed (" + std::to_string(static_cast<int>(result)) + ")");
    });
    dirty_ = false;
    cooldown_ = 4.0f; // Discord allows ~5 updates per 20 s
}
