#!/usr/bin/bash
g++ bot/bot_05.cpp -o strategies/bot_05 || exit -1
rewind-client/main.py -p ../strategies/bot_05 0 0 -p ../strategies/bot02 12 10 -f default
