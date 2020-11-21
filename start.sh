#!/usr/bin/bash
g++ bot/bot.cpp -o strategies/bot01 || exit -1

rewind-client/main.py & 

sleep 3 && 
strategies/bot01 < pipes/p1_client > pipes/p1_strategy
