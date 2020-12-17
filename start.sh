#!/usr/bin/bash
g++ bot/bot_17.mika.cpp -o strategies/bot_17.mika -std=c++17 || exit -1
# g++ bot/bot_12.mika.cpp -o strategies/bot_12.mika -std=c++17 || exit -1

# rewind-client/main.py -p ../strategies/bot_17.mika 0 0 -p ../strategies/bot_12.mika 12 10 -f file
rewind-client/main.py -p ../strategies/bot_17.mika 0 0 -p ../strategies/bot_12.mika 12 10 -f file -a 50
# rewind-client/main.py -p ../strategies/bot_17.mika 0 0 -p ../strategies/bot_12.mika 12 10 --replay ../games/17_12_2020_1608221458547.gamelog


# ? https://cup.alem.school/main/game/replay/36e5c963-d46f-467a-8b20-9790accea79a
