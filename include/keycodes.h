# include <unordered_map>

# include <string>


// keycodes shamelessly stolen from rob (and by him from the win32 api), with some slight modifications

const std::unordered_map<std::string, int> WINDOWS_KEYCODES = {

    {"Backspace",8},
    {"Tab",9},
    {"Clear",12},
    {"Enter",13},
    {"Shift",16},
    {"Ctrl",17},
    {"Alt",18},
    {"Pause",19},
    {"CapsLock",20},
    {"Escape",27},
    {"Space",32},
    {"PageUp",33},
    {"PageDown",34},
    {"End",35},
    {"Home",36},
    {"Left",37},
    {"Up",38},
    {"Right",39},
    {"Down",40},
    {"Select",41},
    {"Print",42},
    {"Execute",43},
    {"PrintScreen",44},
    {"Insert",45},
    {"Delete",46},
    {"Help",47},
    {"0",48},
    {"1",49},
    {"2",50},
    {"3",51},
    {"4",52},
    {"5",53},
    {"6",54},
    {"7",55},
    {"8",56},
    {"9",57},
    {"A",65},
    {"B",66},
    {"C",67},
    {"D",68},
    {"E",69},
    {"F",70},
    {"G",71},
    {"H",72},
    {"I",73},
    {"J",74},
    {"K",75},
    {"L",76},
    {"M",77},
    {"N",78},
    {"O",79},
    {"P",80},
    {"Q",81},
    {"R",82},
    {"S",83},
    {"T",84},
    {"U",85},
    {"V",86},
    {"W",87},
    {"X",88},
    {"Y",89},
    {"Z",90},
    {"LeftWindowsKey",91},
    {"RightWindowsKey",92},
    {"ApplicationsKey",93},
    {"Sleep",95},
    {"NumPad0",96},
    {"NumPad1",97},
    {"NumPad2",98},
    {"NumPad3",99},
    {"NumPad4",100},
    {"NumPad5",101},
    {"NumPad6",102},
    {"NumPad7",103},
    {"NumPad8",104},
    {"NumPad9",105},
    {"Multiply",106},
    {"Add",107},
    {"Seperator",108},
    {"Subtract",109},
    {"Decimal",110},
    {"Divide",111},
    {"F1",112},
    {"F2",113},
    {"F3",114},
    {"F4",115},
    {"F5",116},
    {"F6",117},
    {"F7",118},
    {"F8",119},
    {"F9",120},
    {"F10",121},
    {"F11",122},
    {"F12",123},
    {"F13",124},
    {"F14",125},
    {"F15",126},
    {"F16",127},
    {"F17",128},
    {"F18",129},
    {"F19",130},
    {"F20",131},
    {"F21",132},
    {"F22",133},
    {"F23",134},
    {"F24",135},
    {"Numlock",144},
    {"ScrollLock",145},
    {"LeftShift",160},
    {"RightShift",161},
    {"LeftControl",162},
    {"RightControl",163},
    {"LeftMenu",164},
    {"RightMenu",165},
    {"BrowserBack",166},
    {"BrowserForward",167},
    {"BrowserRefresh",168},
    {"BrowserStop",169},
    {"BrowserSearch",170},
    {"BrowserFavorites",171},
    {"BrowserHome",172},
    {"VolumeMute",173},
    {"VolumeDown",174},
    {"VolumeUp",175},
    {"NextTrack",176},
    {"PreviousTrack",177},
    {"StopMedia",178},
    {"PlayPause",179},
    {"LaunchMail",180},
    {"SelectMedia",181},
    {"LaunchApp1",182},
    {"LaunchApp2",183},
    {";",186},                  // TODO: fix non US (ANSI) layouts
    {"=",184},                  // TODO: fix non US (ANSI) layouts
    {",",188},                  // TODO: fix non US (ANSI) layouts
    {"-",189},                  // TODO: fix non US (ANSI) layouts
    {".",190},                  // TODO: fix non US (ANSI) layouts
    {"/",191},                  // TODO: fix non US (ANSI) layouts
    {"`",192},                  // TODO: fix non US (ANSI) layouts
    {"[",219},                  // TODO: fix non US (ANSI) layouts
    {"\\",220},                 // TODO: fix non US (ANSI) layouts
    {"]",221},                  // TODO: fix non US (ANSI) layouts
    {"'",222},                  // TODO: fix non US (ANSI) layouts
    {"OEM8",223},               // TODO: fix non US (ANSI) layouts
    {"OEM102",226},             // TODO: fix non US (ANSI) layouts
    {"Process",229},
    {"Packet",231},
    {"Attn",246},
    {"CrSel",247},
    {"ExSel",248},
    {"EraseEOF",249},
    {"Play",250},
    {"Zoom",251},
    {"PA1",253},
    {"OEMClear",254}

};


// Linux keycodes, without unknown values set to KEY_AUTOPILOT_ENGAGE_TOGGLE

const std::unordered_map<std::string, int> LINUX_KEYCODES = {

    {"Backspace",14},
    {"Tab",15},
    {"Clear",355},
    {"Enter",28},
    {"Shift",42},               // Left Shift
    {"Ctrl",29},                // Left Control
    {"Alt",56},                 // Left Alt
    {"Pause",119},
    {"CapsLock",58},
    {"Escape",1},
    {"Space",57},
    {"PageUp",104},
    {"PageDown",109},
    {"End",107},
    {"Home",102},
    {"Left",105},
    {"Up",103},
    {"Right",106},
    {"Down",108},
    {"Select",353},
    {"Print",210},
    {"Execute",637},            // Unknown
    {"PrintScreen",210},        // Print
    {"Insert",110},
    {"Delete",111},
    {"Help",138},
    {"0",11},
    {"1",2},
    {"2",3},
    {"3",4},
    {"4",5},
    {"5",6},
    {"6",7},
    {"7",8},
    {"8",9},
    {"9",10},
    {"A",30},
    {"B",48},
    {"C",46},
    {"D",32},
    {"E",18},
    {"F",33},
    {"G",34},
    {"H",35},
    {"I",23},
    {"J",36},
    {"K",37},
    {"L",38},
    {"M",50},
    {"N",49},
    {"O",24},
    {"P",25},
    {"Q",16},
    {"R",19},
    {"S",31},
    {"T",20},
    {"U",22},
    {"V",47},
    {"W",17},
    {"X",45},
    {"Y",21},
    {"Z",44},
    {"LeftWindowsKey",125},     // LeftMeta
    {"RightWindowsKey",126},    // RightMeta
    {"ApplicationsKey",637},    // Unknown
    {"Sleep",142},
    {"NumPad0",512},
    {"NumPad1",513},
    {"NumPad2",514},
    {"NumPad3",515},
    {"NumPad4",516},
    {"NumPad5",517},
    {"NumPad6",518},
    {"NumPad7",519},
    {"NumPad8",520},
    {"NumPad9",521},
    {"Multiply",637},           // Unknown
    {"Add",637},                // Unknown
    {"Seperator",637},          // Unknown
    {"Subtract",637},           // Unknown
    {"Decimal",637},            // Unknown
    {"Divide",637},             // Unknown
    {"F1",59},
    {"F2",60},
    {"F3",61},
    {"F4",62},
    {"F5",63},
    {"F6",64},
    {"F7",65},
    {"F8",66},
    {"F9",67},
    {"F10",68},
    {"F11",87},
    {"F12",88},
    {"F13",183},
    {"F14",184},
    {"F15",185},
    {"F16",186},
    {"F17",187},
    {"F18",188},
    {"F19",189},
    {"F20",190},
    {"F21",191},
    {"F22",192},
    {"F23",193},
    {"F24",194},
    {"Numlock",69},
    {"ScrollLock",70},
    {"LeftShift",42},
    {"RightShift",54},
    {"LeftControl",29},
    {"RightControl",97},
    {"LeftMenu",139},           // Menu key
    {"RightMenu",139},          // Menu key
    {"BrowserBack",158},        // Same as non-browser?
    {"BrowserForward",159},     // Same as non-browser?
    {"BrowserRefresh",173},     // Same as non-browser?
    {"BrowserStop",128},        // Same as non-browser?
    {"BrowserSearch",217},      // Same as non-browser?
    {"BrowserFavorites",364},   // Same as non-browser?
    {"BrowserHome",102},        // Same as non-browser?
    {"VolumeMute",113},
    {"VolumeDown",114},
    {"VolumeUp",115},
    {"NextTrack",407},
    {"PreviousTrack",412},
    {"StopMedia",128},
    {"PlayPause",164},
    {"LaunchMail",155},
    {"SelectMedia",353},
    {"LaunchApp1",637},         // Unknown
    {"LaunchApp2",637},         // Unknown
    {";",39},
    {"=",13},
    {",",51}, 
    {"-",12},
    {".",52},                   // Unknown
    {"/",53},
    {"`",637},                  // Unknown
    {"[",26},
    {"\\",43},
    {"]",27},
    {"'",222},                  // Unknown
    {"OEM8",223},               // Unknown
    {"OEM102",226},             // Unknown
    {"Process",229},            // Unknown
    {"Packet",231},             // Unknown
    {"Attn",246},               // Unknown
    {"CrSel",247},              // Unknown
    {"ExSel",248},              // Unknown
    {"EraseEOF",249},           // Unknown
    {"Play",207},
    {"Zoom",372},
    {"PA1",253},                // Unknown
    {"OEMClear",254}            // Unknown

};