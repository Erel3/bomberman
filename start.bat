g++ bot/bot_17.mika.cpp -o strategies/bot_17.exe
g++ bot/bot_12.mika.cpp -o strategies/bot_12.exe
python rewind-client/main.py -p ../strategies/bot_17.exe 12 10 -p ../strategies/bot_12.exe 0 0 --auto-test 10