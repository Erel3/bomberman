#!/usr/bin/bash
g++ bot/bot.cpp -o strategies/bot || exit -1
rewind-client/main.py -p ../strategies/bot 0 0 -p ../strategies/bot02 12 10 -f default
