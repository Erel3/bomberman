#!/usr/bin/bash
g++ bot/bot_19.opt.cpp -o strategies/bot_19.opt -std=c++17 || exit -1
# g++ bot/bot_17.mika.cpp -o strategies/bot_17.mika -std=c++17 || exit -1

# g++ bot/bot_19.opt.cpp -o strategies/bot_19.opt -std=c++17 -DDEBUG_TIME || exit -1

# rewind-client/main.py -p ../strategies/bot_19.opt 0 0 -p ../strategies/bot_17.mika 12 10 -f file
# rewind-client/main.py -p ../strategies/bot_19.opt 0 0 -p ../strategies/bot_17.mika 12 10 -f file -a 100 --proc 2
rewind-client/main.py -p ../strategies/bot_19.opt 0 0 --replay ../games/20_12_2020_1608410672125.gamelog

# ? why not kill twice!! ../games/19_12_2020_1608380446485.gamelog

# * god ../games/19_12_2020_1608380098699.gamelog
