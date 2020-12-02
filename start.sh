#!/usr/bin/bash
g++ bot/bot.cpp -o strategies/bot || exit -1
rewind-client/main.py -p ../strategies/bot 12 10 -p ../strategies/bot 0 0 -f no-blocks
