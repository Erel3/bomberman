#!/usr/bin/bash
g++ bot/bot_07.cpp -o strategies/bot_07 || exit -1
rewind-client/main.py -p ../strategies/bot_07 0 0 -p ../strategies/bot02 12 10 -f default
# rewind-client/main.py -p ../strategies/bot_07 0 0 -p ../strategies/bot02 12 10 -f default -a 1000
# rewind-client/main.py -p ../strategies/bot_07 0 0 -p ../strategies/bot02 12 10 -f default --replay ../games/08_12_2020_1607364762517.gamelog
