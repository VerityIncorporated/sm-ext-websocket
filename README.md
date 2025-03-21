# SourceMod WebSocket Extension

## What is this?
This is a [SourceMod](http://www.sourcemod.net/) extension that provides some methods for HTTP JSON and websocket communication

## Features
* Relies on [YYJSON](https://github.com/ibireme/yyjson) which A high performance JSON library written in ANSI C
* Relies on [IXWebSocket](https://github.com/machinezone/IXWebSocket) which is C++ library for WebSocket client and server development. It has minimal dependencies
* Support TEXT and JSON data
* Support client and server
* Support permessage-deflate
* Support SSL
* Support x64

## How to build this?
``` sh
sudo dpkg --add-architecture i386
sudo apt-get update
sudo apt-get install clang g++-multilib zlib1g-dev zlib1g-dev:i386 libssl-dev libssl-dev:i386
clone project
mkdir build && cd build
python ../configure.py --enable-optimize --symbol-files --sm-path=YOU_SOURCEMOD_PATH --targets=x86,x64
ambuild
```

## Native
* [json](https://github.com/ProjectSky/sm-ext-websocket/blob/main/scripting/include/websocket/yyjson.inc)
* [websocket](https://github.com/ProjectSky/sm-ext-websocket/blob/main/scripting/include/websocket/ws.inc)

# Binary files
* [GitHub Releases](https://github.com/ProjectSky/sm-ext-websocket/releases)

## TODO
- [x] WebSocket server support
- [x] Windows support
- [ ] HTTP support? (Because this is a WebSocket library I might not support it)

## NOTES
* HTTP functionality is not yet complete. Currently, only basic features are available
* Server will not process data during the hibernation. You can set sv_hibernate_when_empty to 0 to disable hibernation

## Example
* [Example Script](https://github.com/ProjectSky/sm-ext-websocket/tree/main/scripting)