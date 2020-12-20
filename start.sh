#!/usr/bin/bash
# g++ bot/bot_20.opt.cpp -o strategies/bot_20.opt -std=c++17 || exit -1
# g++ bot/bot_19.opt.cpp -o strategies/bot_19.opt -std=c++17 || exit -1

# g++ bot/bot_20.opt.cpp -o strategies/bot_20.opt -std=c++17 -DDEBUG_TIME || exit -1

# rewind-client/main.py -p ../strategies/bot_20.opt 0 0 -p ../strategies/bot_19.opt 12 10 -f file
rewind-client/main.py -p ../strategies/bot_20.opt 0 0 -p ../strategies/bot_19.opt 12 10 -f file -a 300 --proc 4
# rewind-client/main.py -p ../strategies/bot_20.opt 0 0 --replay ../games/20_12_2020_1608410672125.gamelog

# ? fix it ../games/20_12_2020_1608412144109.gamelog
# ? wtf ../games/20_12_2020_1608414255838.gamelog
# ? tp to danger ../games/20_12_2020_1608412785341.gamelog

# * god ../games/19_12_2020_1608380098699.gamelog
