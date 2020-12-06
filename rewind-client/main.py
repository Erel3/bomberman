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
  
  if not config.is_replay:
    win_count, win_avg_diff = 0, 0
    lose_count, lose_avg_diff = 0, 0
    draw_count = 0
    for game_id in range(0, config.game_count):    
      score, log_filename = game.run()
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
      print("game {:03}: result: {} file: {}".format(game_id, str(list(score.items())), log_filename)) 
    if win_count > 0:
      win_avg_diff /= win_count
    if lose_count > 0:
      lose_avg_diff /= lose_count
    win_percent = win_count * 100 / config.game_count
    lose_percent = lose_count * 100 / config.game_count
    draw_percent = draw_count * 100 / config.game_count
    print("total game count: {}\nwin: {} {:.2f} {:.2f}%\nlose: {} {:.2f} {:.2f}%\ndraw: {} {:.2f}%\n".format(config.game_count, win_count, win_avg_diff, win_percent, lose_count, lose_avg_diff, lose_percent, draw_count, draw_percent))
  else:
    game.replay()
