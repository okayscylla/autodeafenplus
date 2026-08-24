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

# include <winbase.h>

# include <synchapi.h>

# include <processthreadsapi.h>


# include <cstddef>

# include <cstring>

# include <cctype>

# include <sstream>

# include <string>

# include <vector>

# include <algorithm>


# include <zmq.h>

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

        discord_keybind([]() -> std::vector<int> {

            std::stringstream _s(

                Mod::get()->getSettingValue<std::vector<geode::Keybind>>("discord_keybind")[0].toString()

            );

            std::string _i;

            std::vector<int> codes;

            while(std::getline(_s, _i, '+')) {

                try {

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


void *b_context = zmq_ctx_new();

void *b_socket = zmq_socket(b_context, ZMQ_PUSH);


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

        void *_s = zmq_socket(b_context, ZMQ_PUSH);

        zmq_connect(_s, "tcp://localhost:6767");

        matjson::Value _shutdown_req;

        _shutdown_req["type"] = "shutdown";

        _shutdown_req["keys"] = std::vector<int>(); // for clarity

        zmq_send(

            _s,

            _strdup(_shutdown_req.dump(matjson::NO_INDENTATION).c_str()),

            strlen(_shutdown_req.dump(matjson::NO_INDENTATION).c_str()),

            ZMQ_DONTWAIT

        );

        zmq_close(_s);

        // startup new bridge

        geode::log::info("Found bridge path: {}", Mod::get()->getResourcesDir().append("bridge").string().c_str());

        STARTUPINFOA _si;

        PROCESS_INFORMATION _pi;

        ZeroMemory(&_si, sizeof(_si));

        _si.cb = sizeof(_si);

        ZeroMemory(&_pi, sizeof(_pi));

        geode::log::info("Attempting to start new input bridge");

        int success = CreateProcessA(

            NULL,

            const_cast<char*>(Mod::get()->getResourcesDir().append("bridge").string().c_str()),

            NULL, NULL,

            false, BELOW_NORMAL_PRIORITY_CLASS,

            NULL, NULL, &_si, &_pi

        );

        if (success) {

            geode::log::info("Found input bridge PID: {}", _pi.dwProcessId);

            DWORD status = WaitForSingleObject(_pi.hProcess, 0);

            if (status == WAIT_OBJECT_0) {

                DWORD _exit_code;

                GetExitCodeProcess(_pi.hProcess, &_exit_code);

                geode::log::error("Input bridge crashed on startup with exit code {}", _exit_code);

            } else {

                geode::log::info("Input bridge started successful");

            }

        } else {

            geode::log::error("Failed to start new input bridge");

        }

        // ShellExecuteA(NULL, NULL, Mod::get()->getResourcesDir().append("bridge").string().c_str(), NULL, NULL, 0);

        // reconnect and get ready for input

        zmq_connect(b_socket, "tcp://localhost:6767");

    } else {

        geode::log::info("Detected Windows environment");

        user_platform = 0;

    }

}


$on_game(Exiting) {

    matjson::Value _shutdown_req;

    _shutdown_req["type"] = "shutdown";

    _shutdown_req["keys"] = std::vector<int>(); // for clarity

    zmq_send(

        b_socket,

        _strdup(_shutdown_req.dump(matjson::NO_INDENTATION).c_str()),

        strlen(_shutdown_req.dump(matjson::NO_INDENTATION).c_str()),

        ZMQ_DONTWAIT

    );

    zmq_close(b_socket);

    zmq_ctx_destroy(b_context);

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


const void press_keys(const std::vector<int>* keycodes) { // TODO: profile this function

    if (keycodes->size() == 0) { return; }

    if (user_platform == 1) { // linux, on main thread as zmq_send is non blocking

        matjson::Value _input_req;

        _input_req["type"] = "input";

        _input_req["keys"] = *keycodes; // for clarity

        zmq_send( // FIXME: remove extra memory allocation here by reusing _input_req

            b_socket,

            _strdup(_input_req.dump(matjson::NO_INDENTATION).c_str()),

            strlen(_input_req.dump(matjson::NO_INDENTATION).c_str()),

            ZMQ_DONTWAIT

        );

    } else if (user_platform == 0) { // windows, on main thread as while SendInput is blocking, has very minimal overhead

        INPUT keycombo[keycodes->size() * 2];

        ZeroMemory(keycombo, sizeof(keycombo));

        for (int i = 0; i < keycodes->size() * 2; i ++) {

            if (i < keycodes->size()) {

                keycombo[i].type = INPUT_KEYBOARD;

                keycombo[i].ki.wVk = keycodes->at(i);

            } else {

                keycombo[i].type = INPUT_KEYBOARD;

                keycombo[i].ki.wVk = keycodes->at((keycodes->size() * 2) - (i + 1));

                keycombo[i].ki.dwFlags = KEYEVENTF_KEYUP;

            }

        }

        SendInput(keycodes->size() * 2, keycombo, sizeof(INPUT));

    } else { // something has gone terribly wrong

        geode::log::info("Invalid platform: {}", user_platform);

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

        if (!settings.undeafen) {

            row3->setOpacity(67);

            undeafen_input->setCallbackEnabled(false);

            undeafen_input->setEnabled(false);

        }

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

        if (active && settings.undeafen && (current_percentage < current_level.undeafen_percentage)) { return; }

        if (current_percentage >= current_level.deafen_percentage) {

            if (settings.undeafen && (current_percentage >= current_level.undeafen_percentage)) {

                active = false;

                press_keys(&settings.discord_keybind);

                geode::log::info("Disabled auto deafen (passed undeafen percent)");

                return;

            }

            active = true;

            press_keys(&settings.discord_keybind);

            geode::log::info("Enabled auto deafen (past deafen percent)");

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

        if (!settings.enable || !current_level.enable || !settings.pause_toggle || active || (!active && settings.undeafen)) { return; }

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
