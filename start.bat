g++ bot/bot.cpp -o strategies/bot.exe
python rewind-client/main.py -p ../strategies/bot 12 10 -p ../strategies/bot 0 0 -f default
