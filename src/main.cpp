# include <Geode/Geode.hpp>

# include <Geode/Result.hpp>

# include <Geode/modify/Modify.hpp>


# include <Geode/modify/PlayLayer.hpp>

# include <Geode/modify/PlayerObject.hpp>

# include <Geode/modify/PauseLayer.hpp>


# include <Geode/ui/BasedButtonSprite.hpp>


# include <matjson.hpp>


# include <windows.h>

# include <minwindef.h>

# include <winerror.h>

# include <winreg.h>


# include <cstddef>

# include <zmq.hpp>


using namespace geode::prelude;


struct Settings {

    bool enable = Mod::get()->getSettingValue<bool>("enable");

    bool undeafen = Mod::get()->getSettingValue<bool>("undeafen");

    bool pause_toggle = Mod::get()->getSettingValue<bool>("pause_toggle");

    bool startpos = Mod::get()->getSettingValue<bool>("startpos");

    bool practise = Mod::get()->getSettingValue<bool>("practise");


    int deafen_percentage = Mod::get()->getSettingValue<int>("deafen_percentage");

    int undeafen_percentage = Mod::get()->getSettingValue<int>("undeafen_percentage");

};


struct LevelConfig {

    bool enable;

    int deafen_percentage;

    int undeafen_percentage;


    LevelConfig() : enable(false), deafen_percentage(40), undeafen_percentage(95) {}

    LevelConfig(bool e, int d, int u) : enable(e), deafen_percentage(d), undeafen_percentage(u) {}

};


template<>

struct matjson::Serialize<LevelConfig> {

    static Result<LevelConfig> fromJson(matjson::Value const& value) {

        GEODE_UNWRAP_INTO(bool _e, value["_e"].asBool());

        GEODE_UNWRAP_INTO(bool _d, value["_d"].asInt());

        GEODE_UNWRAP_INTO(bool _u, value["_u"].asInt());

        return Ok(LevelConfig(_e, _d, _u));

    }

    static matjson::Value toJson(LevelConfig const& value) {

        Value obj = matjson::Value();
        
        obj["_e"] = value.enable;

        obj["_d"] = value.deafen_percentage;

        obj["_u"] = value.undeafen_percentage;

        return obj;

    }

};


int user_platform;


Settings settings;


bool active = false;

bool flag_must_undeafen = false;

bool flag_ignore_update = false;


int current_level_id;


$on_mod(Loaded) {

    listenForSettingChanges<bool>("enable", [](bool value) { settings.enable = value; });

    listenForSettingChanges<bool>("undeafen", [](bool value) { settings.undeafen = value; });

    listenForSettingChanges<bool>("pause_toggle", [](int value) { settings.pause_toggle = value; });

    listenForSettingChanges<int>("deafen_percentage", [](int value) { settings.deafen_percentage = value; });

    listenForSettingChanges<int>("undeafen_percentage", [](int value) { settings.undeafen_percentage = value; });

}


$on_game(Loaded) {

    if ((void *)GetProcAddress(GetModuleHandle("ntdll.dll"), "wine_get_host_version")) {

        geode::log::info("Detected Linux (wine) environment");

        // geode::log::info("Using cmpzmq version {}", zmq::version());

        user_platform = 1;

        HKEY environment_key;

        RegOpenKeyExA(

            HKEY_LOCAL_MACHINE,

            "System\\CurrentControlSet\\Control\\Session Manager\\Environment",

            NULL, KEY_ALL_ACCESS, &environment_key

        );

        unsigned long length = 128;

        char* pathext = new char[length];

        LSTATUS result = RegGetValueA(

            environment_key, NULL, "PATHEXT", 

            RRF_RT_REG_SZ, NULL, pathext, &length

        );

        if (result == ERROR_MORE_DATA) {

            delete[] pathext;

            pathext = new char[length];

            RegGetValueA(

                environment_key, NULL, "PATHEXT", 

                RRF_RT_REG_SZ, NULL, pathext, &length

            );

        }

        geode::log::info("Found PATHEXT: {}", pathext);

        delete[] pathext;

        RegCloseKey(environment_key);

    } else {

        geode::log::info("Detected Windows environment");

        user_platform = 0;

    }

}


const void press_keys(const std::string key_combo) {

    switch (user_platform) {

        case 1:

            // std::system("cmd /c start /unix $(which ydotool)");

        case 0:

        default:

            geode::log::error("Invalid platform: {}", user_platform);

    }

}


class $modify(PlayLayer) {

    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {

        if (!PlayLayer::init(level, useReplay, dontCreateObjects)) return false;

        if (active) {

            active = false;

            geode::log::info("Disabled auto deafen");

        }

        flag_must_undeafen = false;

        return true;

    }

    virtual void postUpdate(float dt) {

        PlayLayer::postUpdate(dt);

        if (!settings.enable) { return; }

        if (flag_ignore_update) { return; }

        if (flag_must_undeafen) {

            if (active) {

                active = false;

                geode::log::info("Disabled auto deafen");

            }

            flag_must_undeafen = false;

            return;

        }

        int current_percentage = PlayLayer::getCurrentPercentInt();

        if (active && ((current_percentage < settings.deafen_percentage) || (current_percentage < settings.undeafen_percentage))) {

            active = false;

            geode::log::info("Disabled auto deafen");

        }

        if (active && !settings.undeafen) { return; }

        if (settings.undeafen && !active && (current_percentage > settings.deafen_percentage)) { return; }

        if (current_percentage > settings.deafen_percentage) {

            if (settings.undeafen && (current_percentage > settings.undeafen_percentage) && (settings.undeafen_percentage > settings.deafen_percentage)) {

                active = false;

                geode::log::info("Disabled auto deafen");

                return;

            }

            active = true;

            geode::log::info("Enabled auto deafen");

            return;

        }

    }

    void pauseGame(bool unfocused) {

        PlayLayer::pauseGame(unfocused);

        if (settings.enable && settings.pause_toggle && active) {

            active = false;

            geode::log::info("Disabled auto deafen");

        }

    }

    void resume() {

        PlayLayer::resume();

        if (!settings.enable || !settings.pause_toggle || active) { return; }

        int current_percentage = PlayLayer::getCurrentPercentInt();

        if (current_percentage > settings.deafen_percentage) {

            if (settings.undeafen && (current_percentage > settings.undeafen_percentage) && (settings.undeafen_percentage > settings.deafen_percentage)) {

                active = false;

                geode::log::info("Disabled auto deafen");

            }

            active = true;

            geode::log::info("Enabled auto deafen");

        }

    }

    void playEndAnimationToPos(CCPoint position) {

        PlayLayer::playEndAnimationToPos(position);

        if (!settings.enable || !active) { return; }

        active = false;

        geode::log::info("Disabled auto deafen");

        flag_ignore_update = true;

    }

};


class $modify(PlayerObject) {

    void playerDestroyed(bool noEffects) {

        PlayerObject::playerDestroyed(noEffects);

        if (!settings.enable || !active) { return; }

        flag_must_undeafen = true;

    }

};


class $modify(PauseLayer) {

    void customSetup() {

        PauseLayer::customSetup();

        CCNode* menu = this->getChildByID("right-button-menu");

        CCMenuItemSpriteExtra* settings_button = CCMenuItemSpriteExtra::create(

            CCSprite::createWithSpriteFrameName("diffIcon_02_btn_001.png"), 

            this, 

            nullptr
        
        );

        menu->addChild(settings_button);

    }

};