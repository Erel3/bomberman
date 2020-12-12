#!/usr/bin/bash
g++ bot/bot_12.mika.cpp -o strategies/bot_12.mika -std=c++17 || exit -1
rewind-client/main.py -p ../strategies/bot_12.mika 0 0 -p ../strategies/bot_09 12 10 -f file
# rewind-client/main.py -p ../strategies/bot_12.mika 0 0 -p ../strategies/bot_09 12 10 -f file -a 100
# rewind-client/main.py -p ../strategies/bot_12.mika 0 0 -p ../strategies/bot_09 12 10 --replay ../games/12_12_2020_1607776232465.gamelo
