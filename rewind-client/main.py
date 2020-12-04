#!/usr/bin/env python

from config import config, ConfigParser
import game

import os

abspath = os.path.abspath(__file__)
dname = os.path.dirname(abspath)
os.chdir(dname)

if __name__ == "__main__":
  gameCount = config.game_count
  while(gameCount > 0):
    parser = ConfigParser(config=config, prog="main.py", description="rewind client for bomberman")
    parser.parse()
    game.run()
    gameCount -= 1
