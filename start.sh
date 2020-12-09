#!/usr/bin/bash
g++ bot/bot_09.cpp -o strategies/bot_09 -std=c++17 || exit -1
rewind-client/main.py -p ../strategies/bot_09 0 0 -p ../strategies/bot_02 12 10 -f default
# rewind-client/main.py -p ../strategies/bot_09 0 0 -p ../strategies/bot_02 12 10 -f default -a 1000
# rewind-client/main.py -p ../strategies/bot_09 0 0 -p ../strategies/bot_02 12 10 -f default --replay ../games/09_12_2020_1607505018494.gamelog
