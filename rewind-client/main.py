#!/usr/bin/env python

from config import config, ConfigParser
import concurrent.futures
import game

import os

abspath = os.path.abspath(__file__)
dname = os.path.dirname(abspath)
os.chdir(dname)

def run():
  return game.run()
if __name__ == "__main__":
  parser = ConfigParser(config=config, prog="main.py", description="rewind client for bomberman")
  parser.parse()
  
  if not config.is_replay:
    win_count, win_avg_diff = 0, 0
    lose_count, lose_avg_diff = 0, 0
    draw_count = 0

    stat = {
      'me_alive, ene_alive': {'win' : 0, 'lose' : 0, 'draw' : 0},
      'me_alive, ene_dead': {'win' : 0, 'lose' : 0, 'draw' : 0},
      'me_dead, ene_alive': {'win' : 0, 'lose' : 0, 'draw' : 0},
      'me_dead, ene_dead': {'win' : 0, 'lose' : 0, 'draw' : 0}
    }

    executor = concurrent.futures.ThreadPoolExecutor(max_workers=config.proc)

    tasks = []
    for game_id in range(0, config.game_count):
      tasks.append(executor.submit(run))
    for game_id in range(0, config.game_count):    
      score, players, log_filename = tasks[game_id].result()

      win = score[0] > score[1]
      lose = score[0] < score[1]
      draw = score[0] == score[1]

      state = '111'
      if len(players) == 2:
        state = 'me_alive, ene_alive' 
      elif len(players) == 1:
        me_alive = False
        for player in players:
          if player.owner == 0:
            me_alive = True
        if me_alive:
          state = 'me_alive, ene_dead'
        else:
          state = 'me_dead, ene_alive'
      else:
        state = 'me_dead, ene_dead'
      
      if win:
        stat[state]['win'] += 1
        win_count += 1
        win_avg_diff += score[0] - score[1]
      elif lose:
        stat[state]['lose'] += 1
        lose_count += 1
        lose_avg_diff += score[1] - score[0]
      else:
        stat[state]['draw'] += 1
        draw_count += 1

      print("game {:03}: result: {} file: {}".format(game_id, str(list(score.items())), log_filename)) 
      print("win: {} lose: {} draw: {}".format(win_count, lose_count, draw_count))
    if win_count > 0:
      win_avg_diff /= win_count
    if lose_count > 0:
      lose_avg_diff /= lose_count
    win_percent = win_count * 100 / config.game_count
    lose_percent = lose_count * 100 / config.game_count
    draw_percent = draw_count * 100 / config.game_count
    print("total game count: {}\nwin: {} {:.2f} {:.2f}%\nlose: {} {:.2f} {:.2f}%\ndraw: {} {:.2f}%\n".format(config.game_count, win_count, win_avg_diff, win_percent, lose_count, lose_avg_diff, lose_percent, draw_count, draw_percent))
    #statistics with alive and dead
    for key in stat:
      print("state: {}\nwin: {} {:.2f}%\nlose: {} {:.2f}%\ndraw: {} {:.2f}%\n".format(key, stat[key]['win'], stat[key]['win'] * 100 / config.game_count, stat[key]['lose'], stat[key]['lose'] * 100 / config.game_count, stat[key]['draw'], stat[key]['draw'] * 100 / config.game_count))
  else:
    game.replay()
