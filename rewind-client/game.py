from models.field.field import Field, FieldConstructor
from models.field.field_type import FieldType
from models.entity import Bomb, Monster, MonsterAction, Player, PlayerAction, FeatureAdd, FeatureRange, FeatureTeleport, FeatureJump
from players import StrategyPlayer
from monsters import DummyMonster
from draw_helper import DrawHelper
from colors import Color, player_colors
from config import config
from datetime import datetime
import time

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


def replay_update(replay):
  config.width, config.height, config.tick = map(int, replay.readline().split())
  if config.tick == -1:
    return None, None, None, None, None, True

  field = Field(FieldConstructor(config.field))
  players = []
  bombs = []
  monsters = []
  features = []

  for i in range(config.height):
    field.data[i] = list(replay.readline().strip())
  entities_count = int(replay.readline())
  for i in range(entities_count):
    entity_type, owner, x, y, param1, param2 = replay.readline().split()
    owner, x, y, param1, param2 = int(owner), int(x), int(y), int(param1), int(param2)
    if entity_type == 'p':
      players.append(Player(owner, x, y, player_colors[owner], param1, param2))
    elif entity_type == 'm':
      monsters.append(Monster(x, y))
    elif entity_type == 'b':
      bombs.append(Bomb(owner, x, y, param1, param2))
    elif entity_type == "f_a":
      features.append(FeatureAdd(x, y))
    elif entity_type == "f_r":
      features.append(FeatureRange(x, y))
  features_count = int(replay.readline())
  for i in range(features_count):
    owner, param = map(int, replay.readline().split())
    for player in players:
      if player.owner == owner:
        if param == 0:
          player.teleport = True
        else:
          player.jump = True

  for bomb in bombs:
    for player in players:
      if bomb.owner == player.owner:
        player.bomb_count += 1
  
  return field, players, bombs, monsters, features, False

def replay_init():
  replay = open(config.replay_file, "r")
  score = {}
  field, players, bombs, monsters, features, is_finish = replay_update(replay)
  helper = DrawHelper(field)
  entities = players + bombs + monsters + features
  helper.client.message(str(config.tick))
  helper.redraw(field, entities)

  for player in players:
    score[player.owner] = 0  

  return replay, helper, field, players, bombs, monsters, features, score

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
            type_feature_rand = random.randint(0, 99)
            if type_feature_rand < 35:
              features.append(FeatureAdd(j, i))
            elif type_feature_rand < 70:
              features.append(FeatureRange(j, i))
            elif type_feature_rand < 85:
              features.append(FeatureTeleport(j, i))
            elif type_feature_rand < 100:
              features.append(FeatureJump(j, i))
        field.data[i][j] = '.'


def next_tick_entities(field, players, bombs, monsters, features):
  id = 0
  while id < len(players):
    if field.destroy_data[players[id].y][players[id].x] and players[id].action != PlayerAction.JUMP:
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

def next_tick_players_for_replay(field, players, bombs, monsters, features, strategy_players):
  for player in players:
    player.recalculate(field, players, bombs, monsters, features)
  for sp in strategy_players:
    for p in players:
      if sp.owner == p.owner:
        sp.copy_from(p)
  for player in strategy_players:
    player.tick(field, players, bombs, monsters, features)


def next_apply_players(field, players, bombs, monsters, features):
  for player in players:
    if player.action == PlayerAction.BOMB:
      player.apply(field, players, bombs, monsters, features)
  for player in players:
    if player.action != PlayerAction.BOMB:
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
        elif features[id].type == "f_r":
          player.bomb_range += 1
        elif features[id].type == "f_t":
          player.teleport = True
        elif features[id].type == "f_j":
          player.jump = True
    if used:
      features.pop(id)
    else:
      id += 1

def finish(helper, field, players, bombs, monsters, features, score):
  del helper
  del field
  for player in players:
    del player
  del players
  del bombs
  del monsters
  del features
  del score

def write_logs(log_output, field, players, bombs, monsters, features):
  # game info
  log_output.write("{} {} {}\n".format(field.width, field.height, config.tick))
  # field
  for i in range(field.height):
    log_output.write("{}\n".format("".join(field.data[i])))    
  # entities cnt
  log_output.write("{}\n".format(len(players + bombs + monsters + features)))
  for player in players:
    log_output.write("{} {} {} {} {} {}\n".format(player.type, player.owner, player.x, player.y, player.current_bomb_count, player.bomb_range))
  for bomb in bombs:
    log_output.write("{} {} {} {} {} {}\n".format(bomb.type, bomb.owner, bomb.x, bomb.y, bomb.timer, bomb.range))
  for monster in monsters:
    log_output.write("{} {} {} {} {} {}\n".format(monster.type, monster.owner, monster.x, monster.y, 0, 0))
  for feature in features:
    log_output.write("{} {} {} {} {} {}\n".format(feature.type, feature.owner, feature.x, feature.y, 0, 0))

  player_features = []
  for player in players:
    if player.teleport:
      player_features.append((player.owner, 1))
    if player.jump:
      player_features.append((player.owner, 0))
  log_output.write("{}\n".format(len(player_features)))
  for feature in player_features:
    log_output.write("{} {}\n".format(feature[0], feature[1]))

def finish_log(log_output):
  log_output.write("{} {} {}\n".format(-1,-1, -1))
  log_output.close()

def is_finished_game(field, players):
  has_box = False
  for i in range(field.height):
    for j in range(field.width):
      if field.data[i][j] == ';':
        has_box = True
  if has_box == False and len(players) <= 1:
    return True
  if len(players) == 0:
    return True
  return False

def run():
  # open file to log
  now = datetime.now()
  dt_string = now.strftime("%d_%m_%Y_") + str(int(round(time.time() * 1000)))
  log_filename = "../games/{}.gamelog".format(dt_string)
  log_output = open(log_filename, "w")

  # helper, field, players, bombs, monsters = init_from_file()
  helper, field, players, bombs, monsters, features, score = init()

  for config.tick in range(1, config.max_ticks + 1):
    helper.client.message(str(config.tick))

    # write logs
    write_logs(log_output, field, players, bombs, monsters, features)

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
    if is_finished_game(field, players):
      break

  finish(helper, field, players, bombs, monsters, features, score)
  
  finish_log(log_output)
  return score, log_filename

def replay():
  # helper, field, players, bombs, monsters = init_from_file()
  replay, helper, field, players, bombs, monsters, features, score = replay_init()

  strategy_players = [StrategyPlayer(id, int(player[1]), int(player[2]), player_colors[id],
                            player[0], False if id == 0 and config.with_viewer else True) for id, player in enumerate(config.players)]

  config.tick = 0
  while True:
    config.tick += 1    
    helper.client.message(str(config.tick))

    # bombs
    next_tick_bombs(field, players, bombs, monsters, features)
    count_box_of_player(field, players, bombs, monsters, features, score)
    next_tick_field(field, players, bombs, monsters, features)
    next_tick_entities(field, players, bombs, monsters, features)
    field.draw_destroy_data(helper.client)
    field.clean()
    
    next_tick_players_for_replay(field, players, bombs, monsters, features, strategy_players)
    helper.client.message(str(list(score.items())))
    helper.redraw(field, players + bombs + monsters + features)
    
    field, players, bombs, monsters, features, is_finish = replay_update(replay)

    if is_finish:
      break

  replay.close()
  

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
