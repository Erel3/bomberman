#!/usr/bin/bash
g++ bot/bot_05_get_move.cpp -o strategies/bot_05_get_move || exit -1
# rewind-client/main.py -p ../strategies/bot_05 0 0 -p ../strategies/bot02 12 10 -f default
# rewind-client/main.py -p ../strategies/bot_05_get_move 0 0 -p ../strategies/bot02 12 10 -f default -a 1000
rewind-client/main.py -p ../strategies/bot_05 0 0 -p ../strategies/bot02 12 10 -f default --replay ../games/07_12_2020_1607354887286.gamelog
