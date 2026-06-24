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

## Linux Specific Info

In order to get **AutoDeafen+** running properly in Linux, you will need a few things.

* the python packages evdev and pyzmq installed on your system (most likely from your distribution's packages manager)
* if using Wayland, a compositor that supports some form of global hotkeys (ie: KDE Plasma, GNOME, Hyprland)

After fufilling these requirements (configuring global hotkeys to passthrough Discord on Wayland), **AutoDeafen+** *should* work.

Alternatively, a single file binary bundled with a Python runtime and the required packages can also be used (can be enabled in mod settings), however this will cause a slight increase in memory usage (about a ~2mb increase on my computer). This however removes the need for evdev and pyzmq to be installed as dependencies.

## Acknowledgements

Thank you to **ShmittWaffles**, **LegendoLight**, and **diamondarmorsteve** for helping test this mod and being awesome friends, as well as **lynxdeer** for making the original AutoDeafen mod that inspired this one.

The cppzmq (MIT License), pyzmq (BSD-3-Clause License), and python-evdev (BSD-3-Clause License) libraries were used in this project. All credit for any included code goes to the respective authors. Their licenses can be found in the licenses folder of the linked repository.

No AI of any sort was used in any way in the planning, creation, or any other aspect of making this mod.
Stay silly! :3