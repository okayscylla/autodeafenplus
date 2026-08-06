# AutoDeafen+

**AutoDeafen+** is a small mod designed to fill a gap in existing auto deafen mods, that being of Linux compatibility and overall configurability.

## Features

* Cross platform support (**Linux** and **Windows**)
* Powerful **per-level** auto deafen configuration
* Lightweight and performant

## Roadmap

* **MacOS** support (I do not own a Mac but if someone does and wants to help out the project by testing feel free to contact me)
* **C++** Linux input bridge rewrite (currently written in **Python**, new bridge would remove need for extra Linux dependencies)
* (maybe) Discord **IPC** / **Social SDK** Integration

If you have any issues feel free to reach out to me on Discord (username: okayscylla).

## Known Bugs

* When using a non **US (ANSI) keyboard** layout on Windows, some autodeafen keybinds (including the default deafen keybind in Discord of **Control+Shift+/**) may not work. While a fix for this is in the works, this issue can currently be mitigated by setting a new deafen keybind in Discord to a combination of **(Ctrl | Shift | Alt)** and **(any number or letter)**. From here, **AutoDeafen+** *should* work as long as the ingame mod config has been updated to reflect the new key combo.

## Linux Specific Info

Getting **Autodeafen+** to run on Linux may need some manual configuration. If the mod is not working properly on Linux, try the following things:

* ensure the user running Geometry Dash is part of the **input** group (run **"sudo usermod -a -G input $USER"** in your terminal application)
* **(REQUIRED FOR WAYLAND SESSIONS)** setup global hotkeys in your compositor for your Discord deafen keybind

## Acknowledgements

Thank you to **ShmittWaffles**, **LegendoLight**, and **diamondarmorsteve** for helping test this mod and being awesome friends, as well as **lynxdeer** for making the original AutoDeafen mod that inspired this one.

The libzmq (Mozilla Public License 2.0), pyzmq (BSD-3-Clause License), and python-evdev (BSD-3-Clause License) libraries were used in this project. All credit for any included code goes to the respective authors. Their licenses can be found in the licenses folder of the linked repository.

No AI of any sort was used in any way in the planning, creation, or any other aspect of making this mod.
Stay silly! :3