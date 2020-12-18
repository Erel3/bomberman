#!/usr/bin/bash
# g++ bot/bot_18.mika.cpp -o strategies/bot_18.mika -std=c++17 || exit -1
# g++ bot/bot_12.mika.cpp -o strategies/bot_12.mika -std=c++17 || exit -1

g++ bot/bot_18.mika.cpp -o strategies/bot_18.mika -std=c++17 -DDEBUG_TIME || exit -1

rewind-client/main.py -p ../strategies/bot_18.mika 0 0 -p ../strategies/bot_12.mika 12 10 -f file
# rewind-client/main.py -p ../strategies/bot_18.mika 0 0 -p ../strategies/bot_17.mika 12 10 -f file -a 50 --proc 2
# rewind-client/main.py -p ../strategies/bot_18.mika 0 0 --replay ../games/18_12_2020_1608274856873.gamelog


# ? waited.. for what?? ../games/18_12_2020_1608250976775.gamelog
# ? https://cup.alem.school/main/game/replay/36e5c963-d46f-467a-8b20-9790accea79a
