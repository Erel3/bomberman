#!/usr/bin/bash
g++ bot/bot_20.opt.cpp -o strategies/bot_20.opt.cpp -std=c++17 || exit -1
g++ bot/bot_12.mika.cpp -o strategies/bot_12.mika -std=c++17 || exit -1

# g++ bot/bot_20.opt.cpp -o strategies/bot_20.opt -std=c++17 -DDEBUG_TIME || exit -1

rewind-client/main.py -p ../strategies/bot_20.opt 0 0 -p ../strategies/bot_12.mika 12 10 -f file
# rewind-client/main.py -p ../strategies/bot_20.opt 0 0 -p ../strategies/bot_12.mika 12 10 -f file -a 300 --proc 4
# rewind-client/main.py -p ../strategies/bot_20.opt 0 0 --replay ../games/19_12_2020_1608380098699.gamelog
