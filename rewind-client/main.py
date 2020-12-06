#!/usr/bin/env python

from config import config, ConfigParser
import game

import os

abspath = os.path.abspath(__file__)
dname = os.path.dirname(abspath)
os.chdir(dname)

if __name__ == "__main__":
  parser = ConfigParser(config=config, prog="main.py", description="rewind client for bomberman")
  parser.parse()
  
  win_count, win_avg_diff = 0, 0
  lose_count, lose_avg_diff = 0, 0
  draw_count = 0
  for game_id in range(0, config.game_count):
    score = game.run()
    win = score[0] > score[1]
    lose = score[0] < score[1]
    draw = score[0] == score[1]
    if win:
      win_count += 1
      win_avg_diff += score[0] - score[1]
    elif lose:
      lose_count += 1
      lose_avg_diff += score[1] - score[0]
    else:
      draw_count += 1
    print("game id: {}\nresult: {}".format(game_id, str(list(score.items())))) 
  if win_count > 0:
    win_avg_diff /= win_count
  if lose_count > 0:
    lose_avg_diff /= lose_count
  print("win: {} {:.2f}\nlose: {} {:.2f}\ndraw: {}\n".format(win_count, win_avg_diff, lose_count, lose_avg_diff, draw_count))
