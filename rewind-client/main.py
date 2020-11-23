#!/usr/bin/env python

from models import Field, Bomb, Monster, MonsterAction, Player, PlayerAction
from players import StrategyPlayer
from monsters import DummyMonster
from draw_helper import DrawHelper
from colors import Color
from config import config

import os

abspath = os.path.abspath(__file__)
dname = os.path.dirname(abspath)
os.chdir(dname)

def init():
  field = Field()
  helper = DrawHelper(field)
  players = [StrategyPlayer(1, 0, 0, Color.BLUE, "p1")]
  bombs = []
  monsters = [
    DummyMonster(12, 10), 
  DummyMonster(12, 10), 
  DummyMonster(12, 10), 
  DummyMonster(12, 10), 
  DummyMonster(12, 10), 
  DummyMonster(12, 10), 
  DummyMonster(12, 10), 
  DummyMonster(12, 10), 
  DummyMonster(12, 10), 
  DummyMonster(12, 10), 
  DummyMonster(12, 10), 
  DummyMonster(12, 10), 
  DummyMonster(12, 10), 
  DummyMonster(12, 10), 
  DummyMonster(12, 10)]
  entities = players + bombs + monsters
  helper.client.message(str(config.tick))
  helper.redraw(field, entities)
  return helper, field, players, bombs, monsters


def init_from_file(file="../maps/level05.txt"):
  field = Field()
  players = [StrategyPlayer(1, 0, 0, Color.BLUE, "p1")]
  bombs = []
  monsters = []
  with open(file, "r") as f:
    line = f.readline()
    w, h = map(int, line.split())
    field.width, field.height = w, h
    for id, line in enumerate(f.readlines()):
      for i in range(w):
        field.data[id][i] = line[i]
        if field.data[id][i] == 'm':
          field.data[id][i] = '.'
          monsters.append(DummyMonster(i, id))
  helper = DrawHelper(field)
  entities = players + bombs + monsters
  helper.client.message(str(config.tick))
  helper.redraw(field, entities)
  return helper, field, players, bombs, monsters


def next_tick_bombs(field, players, bombs, monsters):
  scan = True
  while scan:
    scan = False
    for bomb in bombs:
      bomb.tick(field, players, bombs, monsters)
    for bomb in bombs:
      if bomb.timer != 0 and field.destroy_data[bomb.y][bomb.x]:
        bomb.timer = 1
        bomb.tick(field, players, bombs, monsters)
        scan = True
  id = 0
  while id < len(bombs):
    if bombs[id].timer == 0:
      bombs.pop(id)
    else:
      id += 1


def next_tick_field(field, players, bombs, monsters):
  for i in range(field.height):
    for j in range(field.width):
      if field.destroy_data[i][j]:
        field.data[i][j] = '.'
    

def next_tick_entities(field, players, bombs, monsters):
  id = 0
  while id < len(players):
    if field.destroy_data[players[id].y][players[id].x]:
      players.pop(id)
    else:
      id += 1

  id = 0
  while id < len(monsters):
    if field.destroy_data[monsters[id].y][monsters[id].x]:
      monsters.pop(id)
    else:
      id += 1


def next_tick_monsters(field, players, bombs, monsters):
  for monster in monsters:
    monster.tick(field, players, bombs, monsters)
  for monster in monsters:
    monster.apply(field, players, bombs, monsters)


def next_tick_monsters_kill_players(field, players, bombs, monsters):
  id = 0
  while id < len(players):
    killed = False
    for monster in monsters:
      if monster.x == players[id].x and monster.y == players[id].y:
        killed = True
        break
    if killed:
      players.pop(id)
    else:
      id += 1      


def next_tick_players(field, players, bombs, monsters):
  for player in players:
    player.recalculate(field, players, bombs, monsters)
  for player in players:
    player.tick(field, players, bombs, monsters)


def next_apply_players(field, players, bombs, monsters):
  for player in players:
    player.apply(field, players, bombs, monsters)


if __name__ == "__main__":
  # helper, field, players, bombs, monsters = init_from_file()
  helper, field, players, bombs, monsters = init()

  for config.tick in range(1, config.max_ticks + 1):
    helper.client.message(str(config.tick))

    # bombs
    next_tick_bombs(field, players, bombs, monsters)
    next_tick_field(field, players, bombs, monsters)
    next_tick_entities(field, players, bombs, monsters)
    field.draw_destroy_data(helper.client)
    field.clean_destroy_data()
    if config.every_step_redraw:
      helper.current_step("bombs", field, players + bombs + monsters)

    # monsters
    next_tick_players(field, players, bombs, monsters)
    next_tick_monsters(field, players, bombs, monsters)
    next_tick_monsters_kill_players(field, players, bombs, monsters)
    next_apply_players(field, players, bombs, monsters)
    next_tick_monsters_kill_players(field, players, bombs, monsters)
    if config.every_step_redraw:
      helper.current_step("monsters", field, players + bombs + monsters)
    if config.every_step_redraw:
      helper.current_step("players", field, players + bombs + monsters)
    helper.redraw(field, players + bombs + monsters)
