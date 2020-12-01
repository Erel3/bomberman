from models import Field, FieldType, FieldConstructor, Bomb, Monster, MonsterAction, Player, PlayerAction, FeatureAdd, FeatureRange
from players import StrategyPlayer
from monsters import DummyMonster
from draw_helper import DrawHelper
from colors import Color
from config import config

import random
import os


def init():
  field = Field(FieldConstructor(FieldType.DEFAULT))
  helper = DrawHelper(field)
  players = [StrategyPlayer(1, 0, 0, Color.BLUE, "p1")]
  bombs = []
  monsters = []
  features = []
  entities = players + bombs + monsters + features
  helper.client.message(str(config.tick))
  helper.redraw(field, entities)
  return helper, field, players, bombs, monsters, features


def next_tick_bombs(field, players, bombs, monsters, features):
  scan = True
  while scan:
    scan = False
    for bomb in bombs:
      bomb.tick(field, players, bombs, monsters, features)
    for bomb in bombs:
      if bomb.timer != 0 and field.destroy_data[bomb.y][bomb.x]:
        bomb.timer = 1
        bomb.tick(field, players, bombs, monsters, features)
        scan = True
  id = 0
  while id < len(bombs):
    if bombs[id].timer == 0:
      bombs.pop(id)
    else:
      id += 1


def next_tick_field(field, players, bombs, monsters, features):
  id = 0
  while id < len(features):
    if field.destroy_data[features[id].y][features[id].x]:
      features.pop(id)
    else:
      id += 1

  for i in range(field.height):
    for j in range(field.width):
      if field.destroy_data[i][j]:
        if field.data[i][j] == ';':
          rand = random.randint(0, 100)
          if rand <= config.feature_percent:
            if random.randint(0, 1):
              features.append(FeatureAdd(j, i))
            else:
              features.append(FeatureRange(j, i))
        field.data[i][j] = '.'


def next_tick_entities(field, players, bombs, monsters, features):
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


def next_tick_monsters(field, players, bombs, monsters, features):
  for monster in monsters:
    monster.tick(field, players, bombs, monsters, features)
  for monster in monsters:
    monster.apply(field, players, bombs, monsters, features)


def next_tick_monsters_kill_players(field, players, bombs, monsters, features):
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


def next_tick_players(field, players, bombs, monsters, features):
  for player in players:
    player.recalculate(field, players, bombs, monsters, features)
  for player in players:
    player.tick(field, players, bombs, monsters, features)


def next_apply_players(field, players, bombs, monsters, features):
  for player in players:
    player.apply(field, players, bombs, monsters, features)


def run():
  # helper, field, players, bombs, monsters = init_from_file()
  helper, field, players, bombs, monsters, features = init()

  for config.tick in range(1, config.max_ticks + 1):
    helper.client.message(str(config.tick))

    # bombs
    next_tick_bombs(field, players, bombs, monsters, features)
    next_tick_field(field, players, bombs, monsters, features)
    next_tick_entities(field, players, bombs, monsters, features)
    field.draw_destroy_data(helper.client)
    field.clean_destroy_data()
    if config.every_step_redraw:
      helper.current_step("bombs", field, players + bombs + monsters + features)

    # monsters
    next_tick_players(field, players, bombs, monsters, features)
    next_tick_monsters(field, players, bombs, monsters, features)
    next_tick_monsters_kill_players(field, players, bombs, monsters, features)
    next_apply_players(field, players, bombs, monsters, features)
    next_tick_monsters_kill_players(field, players, bombs, monsters, features)
    if config.every_step_redraw:
      helper.current_step("monsters", field, players + bombs + monsters + features)
    if config.every_step_redraw:
      helper.current_step("players", field, players + bombs + monsters + features)
    helper.redraw(field, players + bombs + monsters + features)


if __name__ == "__main__":
  # helper, field, players, bombs, monsters = init_from_file()
  helper, field, players, bombs, monsters, features = init()

  for config.tick in range(1, config.max_ticks + 1):
    helper.client.message(str(config.tick))

    # bombs
    next_tick_bombs(field, players, bombs, monsters, features)
    next_tick_field(field, players, bombs, monsters, features)
    next_tick_entities(field, players, bombs, monsters, features)
    field.draw_destroy_data(helper.client)
    field.clean_destroy_data()
    if config.every_step_redraw:
      helper.current_step("bombs", field, players + bombs + monsters + features)

    # monsters
    next_tick_players(field, players, bombs, monsters, features)
    next_tick_monsters(field, players, bombs, monsters, features)
    next_tick_monsters_kill_players(field, players, bombs, monsters, features)
    next_apply_players(field, players, bombs, monsters, features)
    next_tick_monsters_kill_players(field, players, bombs, monsters, features)
    if config.every_step_redraw:
      helper.current_step("monsters", field, players + bombs + monsters + features)
    if config.every_step_redraw:
      helper.current_step("players", field, players + bombs + monsters + features)
    helper.redraw(field, players + bombs + monsters + features)
