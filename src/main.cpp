# include <Geode/Geode.hpp>

# include <Geode/Result.hpp>

# include <Geode/utils/async.hpp>

# include <Geode/utils/Keyboard.hpp>

# include <Geode/modify/Modify.hpp>


# include <Geode/modify/PlayLayer.hpp>

# include <Geode/modify/PauseLayer.hpp>


# include <Geode/binding/CCMenuItemSpriteExtra.hpp>

# include <Geode/binding/CCMenuItemToggler.hpp>


# include <Geode/ui/BasedButtonSprite.hpp>

# include <Geode/ui/TextInput.hpp>

# include <Geode/ui/Popup.hpp>


# include <matjson.hpp>


# include <windows.h>

# include <minwindef.h>

# include <winerror.h>

# include <winreg.h>

# include <winuser.h>


# include <cstddef>

# include <cstring>

# include <cctype>

# include <sstream>

# include <string>

# include <vector>

# include <algorithm>


# include <zmq.hpp>

# include "keycodes.h"


using namespace geode::prelude;


struct Settings {

    bool enable;

    bool undeafen;

    bool pause_toggle;

    bool startpos;

    bool practise;

    int deafen_percentage;

    int undeafen_percentage;

    std::vector<int> discord_keybind;

    Settings (

        bool _e, bool _u, bool _pt, bool _s, bool _ps, int _d, int _up, std::vector<int> _k

    ) :

        enable(_e), undeafen(_u), pause_toggle(_pt), startpos(_s), practise(_ps),

        deafen_percentage(_d), undeafen_percentage(_up), discord_keybind(_k)

    {}

    Settings () :

        enable(Mod::get()->getSettingValue<bool>("enable")),

        undeafen(Mod::get()->getSettingValue<bool>("undeafen")),

        pause_toggle(Mod::get()->getSettingValue<bool>("pause_toggle")),

        startpos(Mod::get()->getSettingValue<bool>("startpos")),

        practise(Mod::get()->getSettingValue<bool>("practise")),

        deafen_percentage(Mod::get()->getSettingValue<int>("deafen_percentage")),

        undeafen_percentage(Mod::get()->getSettingValue<int>("undeafen_percentage")),

        discord_keybind([]() {

            std::stringstream _s(

                Mod::get()->getSettingValue<std::vector<geode::Keybind>>("discord_keybind")[0].toString()

            );

            std::string _i;

            std::vector<int> codes;

            while(std::getline(_s, _i, '+')) {

                try {

                    std::transform(

                        _i.begin(), _i.end(), _i.begin(),

                        [](unsigned char c){ return std::toupper(c); }

                    );

                    // TODO: clean this up
                    if ((void *)GetProcAddress(GetModuleHandle("ntdll.dll"), "wine_get_host_version")) {

                        codes.insert(

                            codes.end(),

                            LINUX_KEYCODES.at(_i)

                        );

                    } else {

                        codes.insert(

                            codes.end(),

                            WINDOWS_KEYCODES.at(_i)

                        );

                    }

                } catch (std::out_of_range) {

                    geode::log::error("Failed to find keycode for key \"{}\"", _i);

                }

            }

            geode::log::info("Found discord keybind: {}", codes);

            return codes;

        }())

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

        GEODE_UNWRAP_INTO(int _d, value["_d"].asInt());

        GEODE_UNWRAP_INTO(int _u, value["_u"].asInt());

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


Settings settings;

LevelConfig current_level;

int level_id;


int user_platform;

bool active = false;


zmq::context_t b_context;

zmq::socket_t b_socket(b_context, zmq::socket_type::push);


$on_game(Loaded) {

    if ((void *)GetProcAddress(GetModuleHandle("ntdll.dll"), "wine_get_host_version")) {

        geode::log::info("Detected Linux (wine) environment");

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

        std::string _str = (std::string)pathext;

        delete[] pathext;

        geode::log::info("Found PATHEXT: {}", _str);

        if (_str.contains(";.;") || _str.ends_with(";.")) {

            geode::log::info("PATHEXT already configured");

        } else {

            geode::log::info("PATHEXT not configured, updating");

            _str.append(";.");

            RegSetKeyValueA(

                environment_key,

                NULL,

                "PATHEXT",

                REG_SZ,

                _str.c_str(),

                std::strlen(_str.c_str())

            );

        }

        RegCloseKey(environment_key);

        // shutdown existing bridge if already running

        zmq::socket_t _s(b_context, zmq::socket_type::push);

        _s.bind("tcp://localhost:6767");

        matjson::Value _shutdown_req;

        _shutdown_req["type"] = "shutdown";

        _shutdown_req["keys"] = std::vector<int>(); // for clarity

        _s.send(

            zmq::buffer(_shutdown_req.dump(matjson::NO_INDENTATION)),

            zmq::send_flags::dontwait

        );

        _s.close();

        // startup new bridge

        std::system("geode/resources/okayscylla.autodeafenplus/bridge"); // FIXME: unsafe system call

        // reconnect and get ready for input

        b_socket.bind("tcp://localhost:6767");

    } else {

        geode::log::info("Detected Windows environment");

        user_platform = 0;

    }

}


$on_game(Exiting) {

    matjson::Value _shutdown_req;

    _shutdown_req["type"] = "shutdown";

    _shutdown_req["keys"] = std::vector<int>(); // for clarity

    b_socket.send(

        zmq::buffer(_shutdown_req.dump(matjson::NO_INDENTATION)),

        zmq::send_flags::dontwait

    );

    b_socket.close();

    b_context.shutdown();

    b_socket.close();

}


$on_mod(Loaded) {

    listenForSettingChanges<bool>(

        "enable",

        [](bool value) { settings.enable = value; }

    );

    listenForSettingChanges<bool>(

        "undeafen",

        [](bool value) { settings.undeafen = value; }

    );

    listenForSettingChanges<bool>(

        "pause_toggle",

        [](int value) { settings.pause_toggle = value; }

    );

    listenForSettingChanges<bool>(

        "startpos",

        [](int value) { settings.startpos = value; }

    );

    listenForSettingChanges<bool>(

        "practise",

        [](int value) { settings.practise = value; }

    );

    listenForSettingChanges<int>(

        "deafen_percentage",

        [](int value) { settings.deafen_percentage = value; }

    );

    listenForSettingChanges<int>(

        "undeafen_percentage",

        [](int value) { settings.undeafen_percentage = value; }

    );

    listenForSettingChanges<std::vector<geode::Keybind>>(

        "discord-keybind",

        [](std::vector<geode::Keybind> value) {

            std::stringstream _s(value[0].toString());

            std::string _i;

            std::vector<int> codes;

            while(std::getline(_s, _i, '+')) {

                try {

                    std::transform(

                        _i.begin(), _i.end(), _i.begin(),

                        [](unsigned char c){ return std::toupper(c); }

                    );

                    switch (user_platform) {

                        case 0:

                            codes.insert(

                                codes.end(),

                                WINDOWS_KEYCODES.at(_i)

                            );

                            break;

                        case 1:

                            codes.insert(

                                codes.end(),

                                LINUX_KEYCODES.at(_i)

                            );

                            break;

                        default:

                            geode::log::error("No keycode table for platform {}", user_platform);

                    }

                } catch (std::out_of_range) {

                    geode::log::error("Failed to find keycode for key \"{}\"", _i);

                }

            }

            settings.discord_keybind = codes;

            geode::log::info("Updated discord keybind: {}", codes);

        }

    );

}


const void press_keys(const std::vector<int>* keycodes) {

    if (keycodes->size() == 0) { return; }

    INPUT keycombo[keycodes->size() * 2]; // fuck C99

    matjson::Value _input_req;

    switch (user_platform) {

        case 0: // windows, on main thread as while SendInput is blocking, has very minimal overhead

            for (int i = 0; i < keycodes->size(); i++) {

                keycombo[i].type = INPUT_KEYBOARD;

                keycombo[i].ki.wScan = (*keycodes)[i];

            }

            for (int i = keycodes->size(); i > 0; i--) {

                keycombo[(keycodes->size() * 2) - i].type = INPUT_KEYBOARD;

                keycombo[(keycodes->size() * 2) - i].ki.wScan = (*keycodes)[i - 1];

                keycombo[(keycodes->size() * 2) - i].ki.dwFlags = KEYEVENTF_KEYUP;

            }

            SendInput((keycodes->size() * 2), keycombo, sizeof(INPUT));

            break;

        case 1: // linux, on main thread as zmq::send is non blocking

            _input_req["type"] = "input";

            _input_req["keys"] = *keycodes;

            b_socket.send(

                zmq::buffer(_input_req.dump(matjson::NO_INDENTATION)),

                zmq::send_flags::dontwait

            );

            break;

        default:

            geode::log::error("Invalid platform: {}", user_platform);

        return;

    }

}


class ADPSettingsLayer : public geode::Popup {

    protected:

    bool init() override {

        if (!Popup::init(360.f, 240.f)) {

            return false;

        }

        // basic layout setup

        this->setTitle("AutoDeafen+ Settings");

        CCMenu* col1 = CCMenu::create();

        col1->setContentSize(CCSize(360.f,240.f));

        col1->setLayout(ColumnLayout::create());

        col1->setID("col1");

        // row 1

        CCMenu* row1 = CCMenu::create();

        row1->setContentSize(CCSize(360.f,40.f));

        row1->setLayout(AnchorLayout::create());

        row1->setID("row1");

        CCLabelBMFont* enable_text = CCLabelBMFont::create("Enable for level:", "bigFont.fnt");

        enable_text->setAnchorPoint(ccp(0.f,0.5f));

        enable_text->setScale(0.6f);

        row1->addChildAtPosition(enable_text, Anchor::Left, ccp(10,0));

        CCMenuItemToggler* enable_box = CCMenuItemToggler::create(

            CCSprite::createWithSpriteFrameName("GJ_checkOff_001.png"),

            CCSprite::createWithSpriteFrameName("GJ_checkOn_001.png"),

            this,

            menu_selector(ADPSettingsLayer::onADPLayerEnableToggle)

        );

        enable_box->toggle(current_level.enable);

        enable_box->setAnchorPoint(ccp(1.f,0.5f));

        enable_box->setScale(0.8f);

        enable_box->setID("enable-box");

        row1->addChildAtPosition(enable_box, Anchor::Right, ccp(-10.f,0.f));

        row1->updateLayout();

        // row 2

        CCMenu* row2 = CCMenu::create();

        row2->setContentSize(CCSize(360.f,40.f));

        row2->setLayout(AnchorLayout::create());

        row2->setID("row2");

        CCLabelBMFont* deafen_text = CCLabelBMFont::create("Deafen percentage:", "bigFont.fnt");

        deafen_text->setAnchorPoint(ccp(0.f,0.5f));

        deafen_text->setScale(0.6f);

        row2->addChildAtPosition(deafen_text, Anchor::Left, ccp(10.f,0.f));

        TextInput* deafen_input = TextInput::create(

            40.f,

            std::to_string(current_level.deafen_percentage)

        );

        deafen_input->setCommonFilter(CommonFilter::Int);

        deafen_input->setMaxCharCount(2);

        deafen_input->setAnchorPoint(ccp(1.f,0.5f));

        deafen_input->setScale(0.9f);

        deafen_input->setID("deafen-input");

        row2->addChildAtPosition(deafen_input, Anchor::Right, ccp(-10.f,0.f));

        row2->updateLayout();

        // row 3

        CCMenu* row3 = CCMenu::create();

        row3->setContentSize(CCSize(360.f, 40.f));

        row3->setLayout(AnchorLayout::create());

        row3->setID("row3");

        CCLabelBMFont* undeafen_text = CCLabelBMFont::create("Undeafen percentage:", "bigFont.fnt");

        undeafen_text->setAnchorPoint(ccp(0.f,0.5f));

        undeafen_text->setScale(0.6f);

        row3->addChildAtPosition(undeafen_text, Anchor::Left, ccp(10.f,0.f));

        TextInput* undeafen_input = TextInput::create(

            40.f,

            std::to_string(current_level.undeafen_percentage)

        );

        undeafen_input->setCommonFilter(CommonFilter::Int);

        undeafen_input->setMaxCharCount(2);

        undeafen_input->setAnchorPoint(ccp(1.f,0.5f));

        undeafen_input->setScale(0.9f);

        undeafen_input->setID("undeafen-input");

        row3->addChildAtPosition(undeafen_input, Anchor::Right, ccp(-10.f,0.f));

        row3->updateLayout();

        // putting it together

        col1->addChild(row3);

        col1->addChild(row2);

        col1->addChild(row1);

        col1->updateLayout();

        col1->setPosition(180.f, 120.f);

        m_mainLayer->addChild(col1);

        // add version text

        CCLabelBMFont* version_text = CCLabelBMFont::create(

            Mod::get()->getVersion().toNonVString().c_str(),

            "bigFont.fnt"

        );

        version_text->setAnchorPoint(ccp(1.f,0.f));

        version_text->setScale(0.7f);

        version_text->setOpacity(100);

        version_text->setPosition(ccp(353,9));

        version_text->setID("version-text");

        m_mainLayer->addChild(version_text);

        return true;

    }

    void onClose(CCObject* obj) override {

        std::optional<int> _dp = geode::utils::numFromString<int>(

            ((TextInput*)(m_mainLayer->getChildByIDRecursive("deafen-input")))->getString()

        ).ok();

        if (_dp.has_value()) {

            current_level.deafen_percentage = _dp.value();

        }

        std::optional<int> _up = geode::utils::numFromString<int>(

            ((TextInput*)(m_mainLayer->getChildByIDRecursive("undeafen-input")))->getString()

        ).ok();

        if (_up.has_value()) {

            current_level.undeafen_percentage = _up.value();

        }

        Mod::get()->setSavedValue<LevelConfig>(std::to_string(level_id), current_level);

        geode::log::info("Data saved!");

        Popup::onClose(obj);

    }

    public:

    static ADPSettingsLayer* create() {

        ADPSettingsLayer* pp = new ADPSettingsLayer();

        if (pp->init()) {

            pp->autorelease();

            return pp;

        }

        delete pp;

        return nullptr;

    }

    void onADPLayerEnableToggle(CCObject* sendor) {

        current_level.enable = !current_level.enable;

        return;

    }

};


class $modify(ADPPlayLayer, PlayLayer) {

    struct Fields {

        bool m_flagIgnoreUpdates = false;

    };

    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {

        if (!PlayLayer::init(level, useReplay, dontCreateObjects)) return false;

        if (active) {

            active = false;

            press_keys(&settings.discord_keybind);

            geode::log::info("Disabled auto deafen");

        }

        level_id = m_level->m_levelID;

        geode::log::info("Loading config for level {}", level_id);

        current_level = Mod::get()->getSavedValue<LevelConfig>(

            std::to_string(level_id), 

            LevelConfig()

        );

        geode::log::info(

            "Found config: e -> {}, d% -> {}, u% -> {}",

            current_level.enable,

            current_level.deafen_percentage,

            current_level.undeafen_percentage

        );

        return true;

    }

    virtual void postUpdate(float dt) {

        PlayLayer::postUpdate(dt);

        if (!settings.enable || !current_level.enable) { return; }

        if (current_level.undeafen_percentage <= current_level.deafen_percentage) { return; }

        if (m_isPracticeMode) {

            if (!settings.practise) {

                return;

            }

        }

        if (m_isTestMode) {

            if (!settings.startpos) {

                return;

            }

        }

        if (m_playerDied) {

            if (active) {

                active = false;

                press_keys(&settings.discord_keybind);

                geode::log::info("Disabled auto deafen (player died)");

            }

            return;

        }

        if (m_fields->m_flagIgnoreUpdates) { return; }

        int current_percentage = PlayLayer::getCurrentPercentInt();

        if (active && (current_percentage < current_level.deafen_percentage)) {

            active = false;

            press_keys(&settings.discord_keybind);

            geode::log::info("Disabled auto deafen (before deafen percent)");

            return;

        }

        if (active && !settings.undeafen) { return; }

        if (!active && settings.undeafen && (current_percentage >= current_level.undeafen_percentage)) { return; }

        if (current_percentage >= current_level.deafen_percentage) {

            if (settings.undeafen && (current_percentage >= current_level.undeafen_percentage)) {

                active = false;

                press_keys(&settings.discord_keybind);

                geode::log::info("Disabled auto deafen (passed undeafen percent)");

                return;

            }

            active = true;

            press_keys(&settings.discord_keybind);

            geode::log::info("Enabled auto deafen (passed deafen percent)");

        }

    }

    void pauseGame(bool unfocused) {

        PlayLayer::pauseGame(unfocused);

        if (settings.enable && current_level.enable && settings.pause_toggle && active) {

            active = false;

            press_keys(&settings.discord_keybind);

            geode::log::info("Disabled auto deafen (paused)");

        }

    }

    void resume() {

        PlayLayer::resume();

        if (!settings.enable || !current_level.enable || !settings.pause_toggle || active) { return; }

        int current_percentage = PlayLayer::getCurrentPercentInt();

        if (current_percentage > current_level.deafen_percentage) {

            if (settings.undeafen && (current_percentage > current_level.undeafen_percentage)) {

                active = false;

                press_keys(&settings.discord_keybind);

                geode::log::info("Disabled auto deafen (past undeafen percentage)");

                return;

            }

            active = true;

            press_keys(&settings.discord_keybind);

            geode::log::info("Enabled auto deafen (unpaused)");

        }

    }

    void playEndAnimationToPos(CCPoint position) {

        PlayLayer::playEndAnimationToPos(position);

        if (!settings.enable || !current_level.enable || !active) { return; }

        active = false;

        press_keys(&settings.discord_keybind);

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

        ADPSettingsLayer* settings = ADPSettingsLayer::create();

        settings->show();

    }

};
