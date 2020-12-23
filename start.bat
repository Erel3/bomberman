g++ bot/bot_19.opt.cpp -o strategies/A.exe
g++ bot/bot_18.opt.cpp -o strategies/B.exe
python rewind-client/main.py -p ../strategies/A.exe 0 0 -p ../strategies/B.exe 12 10 -f file
