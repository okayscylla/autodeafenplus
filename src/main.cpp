# include <Geode/Geode.hpp>

# include <Geode/Result.hpp>

# include <Geode/modify/Modify.hpp>


# include <Geode/modify/PlayLayer.hpp>

# include <Geode/modify/PlayerObject.hpp>

# include <Geode/modify/PauseLayer.hpp>


# include <Geode/binding/CCMenuItemSpriteExtra.hpp>


# include <Geode/ui/BasedButtonSprite.hpp>

# include <Geode/ui/Popup.hpp>


# include <matjson.hpp>


# include <windows.h>

# include <minwindef.h>

# include <winerror.h>

# include <winreg.h>


# include <cstddef>

# include <zmq.hpp>


using namespace geode::prelude;


struct Settings {

    bool enable;

    bool undeafen;

    bool pause_toggle;

    bool startpos;

    bool practise;

    int deafen_percentage;

    int undeafen_percentage;

    Settings (

        bool _e, bool _u, bool _pt, bool _s, bool _ps, int _d, int _up

    ) :

        enable(_e), undeafen(_u), pause_toggle(_pt), startpos(_s), practise(_ps),

        deafen_percentage(_d), undeafen_percentage(_up)

    {}

    Settings () :

        enable(Mod::get()->getSettingValue<bool>("enable")),

        undeafen(Mod::get()->getSettingValue<bool>("undeafen")),

        pause_toggle(Mod::get()->getSettingValue<bool>("pause_toggle")),

        startpos(Mod::get()->getSettingValue<bool>("startpos")),

        practise(Mod::get()->getSettingValue<bool>("practise")),

        deafen_percentage(Mod::get()->getSettingValue<int>("deafen_percentage")),

        undeafen_percentage(Mod::get()->getSettingValue<int>("undeafen_percentage")) 

    {}

};


struct LevelConfig {

    bool enable;

    int deafen_percentage;

    int undeafen_percentage;

    LevelConfig() :

        enable(false),

        deafen_percentage(Mod::get()->getSettingValue<int>("deafen_percentage")),

        undeafen_percentage(Mod::get()->getSettingValue<int>("undeafen_percentage"))

    {}

    LevelConfig(bool _e, int _d, int _u) : enable(_e), deafen_percentage(_d), undeafen_percentage(_u) {}

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


class ADPSettingsLayer : public geode::Popup {

    protected:

    bool init(LevelConfig* config) {

        if (!Popup::init(360.f, 240.f)) {

            return false;

        }

        this->setTitle("AutoDeafen+ Settings");

        return true;

    }

    public:

    static ADPSettingsLayer* create(LevelConfig* config) {

        ADPSettingsLayer* pp = new ADPSettingsLayer();

        if (pp->init(config)) {

            pp->autorelease();

            return pp;

        }

        delete pp;

        return nullptr;

    }

};


int user_platform;


Settings settings;


bool active = false;


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


class $modify(ADPPlayLayer, PlayLayer) {

    struct Fields {

        bool m_flagIgnoreUpdates = false;

    };

    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {

        if (!PlayLayer::init(level, useReplay, dontCreateObjects)) return false;

        if (active) {

            active = false;

            geode::log::info("Disabled auto deafen");

        }

        return true;

    }

    virtual void postUpdate(float dt) {

        PlayLayer::postUpdate(dt);

        if (!settings.enable) { return; }

        if (settings.undeafen_percentage <= settings.deafen_percentage) { return; }

        if (this->m_playerDied) {

            if (active) {

                active = false;

                geode::log::info("Disabled auto deafen (player died)");

            }

            return;

        }

        if (m_fields->m_flagIgnoreUpdates) { return; }

        int current_percentage = PlayLayer::getCurrentPercentInt();

        if (active && (current_percentage < settings.deafen_percentage)) {

            active = false;

            geode::log::info("Disabled auto deafen (before deafen percent)");

            return;

        }

        if (active && !settings.undeafen) { return; }

        if (!active && settings.undeafen && (current_percentage >= settings.undeafen_percentage)) { return; }

        if (current_percentage >= settings.deafen_percentage) {

            if (settings.undeafen && (current_percentage >= settings.undeafen_percentage)) {

                active = false;

                geode::log::info("Disabled auto deafen (passed undeafen percent)");

                return;

            }

            active = true;

            geode::log::info("Enabled auto deafen (passed deafen percent)");

        }

    }

    void pauseGame(bool unfocused) {

        PlayLayer::pauseGame(unfocused);

        if (settings.enable && settings.pause_toggle && active) {

            active = false;

            geode::log::info("Disabled auto deafen (paused)");

        }

    }

    void resume() {

        PlayLayer::resume();

        if (!settings.enable || !settings.pause_toggle || active) { return; }

        int current_percentage = PlayLayer::getCurrentPercentInt();

        if (current_percentage > settings.deafen_percentage) {

            if (settings.undeafen && (current_percentage > settings.undeafen_percentage)) {

                active = false;

                geode::log::info("Disabled auto deafen (past undeafen percentage)");

                return;

            }

            active = true;

            geode::log::info("Enabled auto deafen (unpaused)");

        }

    }

    void playEndAnimationToPos(CCPoint position) {

        PlayLayer::playEndAnimationToPos(position);

        if (!settings.enable || !active) { return; }

        active = false;

        geode::log::info("Disabled auto deafen (end animation)");

        m_fields->m_flagIgnoreUpdates = true;

    }

};


class $modify(ADPPauseLayer, PauseLayer) {

    void customSetup() {

        PauseLayer::customSetup();

        CCNode* menu = this->getChildByID("right-button-menu");

        CCMenuItemSpriteExtra* settings_button = CCMenuItemSpriteExtra::create(

            CCSprite::createWithSpriteFrameName("diffIcon_02_btn_001.png"), 

            this, 

            menu_selector(ADPPauseLayer::onADPSettingsToggle)

        );

        menu->addChild(settings_button);

        menu->updateLayout();

    }

    void onADPSettingsToggle(CCObject* sendor) {

        geode::log::info("Opening settings menu");

        ADPSettingsLayer* settings = ADPSettingsLayer::create(nullptr);

        settings->show();

    }

};
