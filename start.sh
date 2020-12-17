#!/usr/bin/bash
g++ bot/bot_12.mika.cpp -o strategies/bot_12.mika -std=c++17 || exit -1
rewind-client/main.py -p ../strategies/bot_12.mika 0 0 -p ../strategies/bot_09 12 10 -f file
# rewind-client/main.py -p ../strategies/bot_12.mika 0 0 -p ../strategies/bot_09 12 10 -f file -a 100
# rewind-client/main.py -p ../strategies/bot_12.mika 0 0 -p ../strategies/bot_09 12 10 --replay ../games/17_12_2020_1608162155472.gamelog


# ? why not took tp ../games/17_12_2020_1608190973963.gamelog
