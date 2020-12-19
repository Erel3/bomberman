g++ bot/bot_18.opt.cpp -o strategies/bot_18.opt.exe
g++ bot/bot_17.mika.cpp -o strategies/bot_17.exe
python rewind-client/main.py -p ../strategies/bot_18.opt.exe 0 0 -p ../strategies/bot_17.exe 12 10