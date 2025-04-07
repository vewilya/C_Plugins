#!/bin/zsh

mkdir -p build

cc -o build/C_PluginChain  main.c plugin_handler.c gain.c filter.c -I/opt/homebrew/include -L/opt/homebrew/lib -lportaudio

./build/C_PluginChain