"""
Small script to automate adding a prebuild libzmq.dll file to the binaries folder of
okayscylla.autodeafenplus.geode in order to make local development less annoying, since
the Geode CLI doesn't support that and I'm too lazy to add it.
"""

import zipfile

MOD_PATH = "/home/annabelle/.local/share/Steam/steamapps/common/Geometry Dash/geode/mods/okayscylla.autodeafenplus.geode"
ZMQ_DLL_PATH = "/home/annabelle/other/programming/personal/geode/autodeafenplus/build/_deps/libzmq-build/bin/libzmq.dll"

with zipfile.ZipFile(MOD_PATH, "a") as zip:
    zip.mkdir("binaries")
    zip.mkdir("binaries/win")
    zip.write(ZMQ_DLL_PATH, "binaries/win/libzmq.dll")