from models.field.field import Field, FieldConstructor
from models.field.field_type import FieldType
from models.entity import Bomb, Monster, MonsterAction, Player, PlayerAction, FeatureAdd, FeatureRange
from players import StrategyPlayer
from monsters import DummyMonster
from draw_helper import DrawHelper
from colors import Color, player_colors
from config import config

import random
import os


def init():
  score = {}
  field = Field(FieldConstructor(config.field))
  players = [StrategyPlayer(id, int(player[1]), int(player[2]), player_colors[id],
                            player[0], False if id == 0 and config.with_viewer else True) for id, player in enumerate(config.players)]
  for player in players:
    score[player.owner] = 0
    for i in range(0, 3):
      if (player.x + i < field.width):
        field.data[player.y][player.x + i] = '.'
      if player.x - i >= 0:
        field.data[player.y][player.x - i] = '.'
      if player.y + i < field.height and i < 2:
        field.data[player.y + i][player.x] = '.'
      if player.y - i >= 0 and i < 2:
        field.data[player.y - i][player.x] = '.'
  helper = DrawHelper(field)
  bombs = []
  monsters = []
  features = []
  entities = players + bombs + monsters + features
  helper.client.message(str(config.tick))
  helper.redraw(field, entities)
  return helper, field, players, bombs, monsters, features, score


def next_tick_bombs(field, players, bombs, monsters, features):
  for bomb in bombs:
    bomb.tick(field, players, bombs, monsters, features)
  scan = True
  while scan:
    scan = False
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


def count_box_of_player(field, players, bombs, monsters, features, score):
  for player in players:
    for i in range(config.width):
      for j in range(config.height):
        if player.owner in field.box_of_player[j][i]:
          score[player.owner] += 1
          player.score += 1


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


def next_apply_features(field, players, bombs, monsters, features):
  id = 0
  while id < len(features):
    used = False
    for player in players:
      if player.x == features[id].x and player.y == features[id].y:
        used = True
        if features[id].type == "f_a":
          player.bomb_count += 1
        else:
          player.bomb_range += 1
    if used:
      features.pop(id)
    else:
      id += 1


def run():
  # helper, field, players, bombs, monsters = init_from_file()
  helper, field, players, bombs, monsters, features, score = init()

  for config.tick in range(1, config.max_ticks + 1):
    helper.client.message(str(config.tick))

    # bombs
    next_tick_bombs(field, players, bombs, monsters, features)
    count_box_of_player(field, players, bombs, monsters, features, score)
    next_tick_field(field, players, bombs, monsters, features)
    next_tick_entities(field, players, bombs, monsters, features)
    field.draw_destroy_data(helper.client)
    field.clean()
    if config.every_step_redraw:
      helper.current_step("bombs", field, players +
                          bombs + monsters + features)

    # monsters
    next_tick_players(field, players, bombs, monsters, features)
    next_tick_monsters(field, players, bombs, monsters, features)
    next_tick_monsters_kill_players(field, players, bombs, monsters, features)
    next_apply_players(field, players, bombs, monsters, features)
    next_tick_monsters_kill_players(field, players, bombs, monsters, features)
    next_apply_features(field, players, bombs, monsters, features)
    if config.every_step_redraw:
      helper.current_step("monsters", field, players +
                          bombs + monsters + features)
    if config.every_step_redraw:
      helper.current_step("players", field, players +
                          bombs + monsters + features)
    helper.client.message(str(list(score.items())))
    helper.redraw(field, players + bombs + monsters + features)
  return score


# if __name__ == "__main__":
#   # helper, field, players, bombs, monsters = init_from_file()
#   helper, field, players, bombs, monsters, features = init()

#   for config.tick in range(1, config.max_ticks + 1):
#     helper.client.message(str(config.tick))

#     # bombs
#     next_tick_bombs(field, players, bombs, monsters, features)
#     count_box_of_player(field, players, bombs, monsters, features)
#     next_tick_field(field, players, bombs, monsters, features)
#     next_tick_entities(field, players, bombs, monsters, features)
#     field.draw_destroy_data(helper.client)
#     field.clean()
#     if config.every_step_redraw:
#       helper.current_step("bombs", field, players + bombs + monsters + features)

#     # monsters
#     next_tick_players(field, players, bombs, monsters, features)
#     next_tick_monsters(field, players, bombs, monsters, features)
#     next_tick_monsters_kill_players(field, players, bombs, monsters, features)
#     next_apply_players(field, players, bombs, monsters, features)
#     next_tick_monsters_kill_players(field, players, bombs, monsters, features)
#     next_apply_features(field, players, bombs, monsters, features)
#     if config.every_step_redraw:
#       helper.current_step("monsters", field, players + bombs + monsters + features)
#     if config.every_step_redraw:
#       helper.current_step("players", field, players + bombs + monsters + features)
#     helper.redraw(field, players + bombs + monsters + features)
