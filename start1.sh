#!/usr/bin/bash
# g++ bot/bot_19.opt.cpp -o strategies/bot_19.opt -std=c++17 || exit -1
# g++ bot/bot_17.mika.cpp -o strategies/bot_17.mika -std=c++17 || exit -1

# g++ bot/bot_19.opt.cpp -o strategies/bot_19.opt -std=c++17 -DDEBUG_TIME || exit -1

rewind-client/main.py -p ../strategies/bot_19.opt 0 0 -p ../strategies/bot_17.mika 12 10 -f file
# rewind-client/main.py -p ../strategies/bot_19.opt 0 0 -p ../strategies/bot_17.mika 12 10 -f file -a 100 --proc 2
# rewind-client/main.py -p ../strategies/bot_19.opt 0 0 --replay ../games/19_12_2020_1608362543179.gamelog
