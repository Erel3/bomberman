#!/usr/bin/bash
g++ bot/bot_17.mika.cpp -o strategies/bot_17.mika -std=c++17 || exit -1
# rewind-client/main.py -p ../strategies/bot_17.mika 0 0 -p ../strategies/bot_12.mika 12 10 -f file
rewind-client/main.py -p ../strategies/bot_17.mika 0 0 -p ../strategies/bot_12.mika 12 10 -f file -a 1000
# rewind-client/main.py -p ../strategies/bot_17.mika 0 0 -p ../strategies/bot_12.mika 12 10 --replay ../games/17_12_2020_1608159730029.gamelog


# ? why not took tp ../games/17_12_2020_1608190973963.gamelog ../games/17_12_2020_1608159730029.gamelog
# ? https://cup.alem.school/main/game/replay/36e5c963-d46f-467a-8b20-9790accea79a
