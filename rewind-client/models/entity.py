from config import config
from colors import Color
from enum import Enum
import random


class Entity():
  
  def __init__(self):
    self.type = None
    self.owner = None
    self.x = 0
    self.y = 0
  
  def tick(self, field, players, bombs, monsters, features):
    pass


class PlayerAction(Enum):
  BOMB = 'bomb'
  STAY = 'stay'
  UP = 'up'
  LEFT = 'left'
  RIGHT = 'right'
  DOWN = 'down'


class Player(Entity):

  def __init__(self, owner_id, x, y, color):
    Entity.__init__(self)
    self.type = 'p'
    self.owner = owner_id
    self.x = x
    self.y = y
    self.bomb_count = config.bomb_count
    self.current_bomb_count = config.bomb_count
    self.bomb_range = config.bomb_range
    self.action = None
    self.color = color

    self.score = 0

  def recalculate(self, field, players, bombs, monsters, features):
    self.current_bomb_count = self.bomb_count
    for bomb in bombs:
      if bomb.owner == self.owner:
        self.current_bomb_count -= 1

  def tick(self, field, players, bombs, monsters, features):
    self.action = random.choice(list(PlayerAction)[1:])

  def apply(self, field, players, bombs, monsters, features):
    nx = self.x
    ny = self.y
    if self.action == PlayerAction.STAY:
      return
    if self.action == PlayerAction.BOMB:
      bombs.append(Bomb(self))
      self.current_bomb_count -= 1
      return
    if self.action == PlayerAction.LEFT:
      if self.x > 0 and field.data[self.y][self.x - 1] == '.':
        nx -= 1
    if self.action == PlayerAction.RIGHT:
      if self.x < field.width - 1 and field.data[self.y][self.x + 1] == '.':
        nx += 1
    if self.action == PlayerAction.UP:
      if self.y > 0 and field.data[self.y - 1][self.x] == '.':
        ny -= 1
    if self.action == PlayerAction.DOWN:
      if self.y < field.height - 1 and field.data[self.y + 1][self.x] == '.':
        ny += 1
    for bomb in bombs:
      if nx == bomb.x and ny == bomb.y:
        return
    self.x = nx
    self.y = ny

  def draw(self, client, layer = 4, permanent = False):
    client.set_options(layer=layer, permanent=permanent)
    block_size = config.block_size
    player_radius = config.player_radius
    client.circle(block_size * self.x + block_size // 2, block_size * self.y + block_size // 2, player_radius, self.color.value, True)
    client.circle_popup(block_size * self.x + block_size // 2, block_size * self.y + block_size // 2, player_radius, "mb:{}, b:{}, r:{}".format(self.bomb_count, self.current_bomb_count, self.bomb_range))
    client.circle_popup(block_size * self.x + block_size // 2, block_size * self.y + block_size // 2, player_radius, "s:{}".format(self.score))


class Bomb(Entity):

  def __init__(self, player):
    Entity.__init__(self)
    self.type = 'b'
    self.owner = player.owner
    self.x = player.x
    self.y = player.y
    self.timer = config.bomb_timer # bomb timer
    self.range = player.bomb_range # bomb radius

  def tick(self, field, players, bombs, monsters, features):
    if self.timer == 0:
      return
    self.timer -= 1
    if self.timer > 0:
      return
    field.destroy_data[self.y][self.x] = True
    for i in range(self.range):
        if self.x + 1 + i >= field.width or field.data[self.y][self.x + 1 + i] == '!':
          break
        field.destroy_data[self.y][self.x + 1 + i] = True
        if field.data[self.y][self.x + 1 + i] == ';':
          if self.owner not in field.box_of_player[self.y][self.x + 1 + i]:
            field.box_of_player[self.y][self.x + 1 + i][self.owner] = True
          break
    for i in range(self.range):
        if self.x - 1 - i < 0 or field.data[self.y][self.x - 1 - i] == '!':
          break
        field.destroy_data[self.y][self.x - 1 - i] = True
        if field.data[self.y][self.x - 1 - i] == ';':
          if self.owner not in field.box_of_player[self.y][self.x - 1 - i]:
            field.box_of_player[self.y][self.x - 1 - i][self.owner] = True
          break
    for i in range(self.range):
        if self.y + 1 + i >= field.height or field.data[self.y + 1 + i][self.x] == '!':
          break
        field.destroy_data[self.y + 1 + i][self.x] = True
        if field.data[self.y + 1 + i][self.x] == ';':
          if self.owner not in field.box_of_player[self.y + 1 + i][self.x]:
            field.box_of_player[self.y + 1 + i][self.x][self.owner] = True
          break
    for i in range(self.range):
        if self.y - 1 - i < 0 or field.data[self.y - 1 - i][self.x] == '!':
          break
        field.destroy_data[self.y - 1 - i][self.x] = True
        if field.data[self.y - 1 - i][self.x] == ';':
          if self.owner not in field.box_of_player[self.y - 1 - i][self.x]:
            field.box_of_player[self.y - 1 - i][self.x][self.owner] = True          
          break

  def draw(self, client, layer = 5, permanent = False):
    client.set_options(layer=layer, permanent=permanent)
    block_size = config.block_size
    bomb_radius = config.bomb_radius
    client.circle(block_size * self.x + block_size // 2, block_size * self.y + block_size // 2, bomb_radius, Color.BLACK.value, True)
    client.circle_popup(block_size * self.x + block_size // 2, block_size * self.y + block_size // 2, bomb_radius, "t: {}, r: {}".format(self.timer, self.range))


class MonsterAction(Enum):
  STAY = 'stay'
  UP = 'up'
  LEFT = 'left'
  RIGHT = 'right'
  DOWN = 'down'


class Monster(Entity):

  def __init__(self, x, y):
    Entity.__init__(self)
    self.type = 'm'
    self.owner = -1
    self.x = x
    self.y = y
    self.action = None

  def tick(self, field, players, bombs, monsters, features):
    self.action = random.choice(list(MonsterAction)[1:])

  def apply(self, field, players, bombs, monsters, features):
    nx = self.x
    ny = self.y
    if self.action == MonsterAction.STAY:
      return
    if self.action == MonsterAction.LEFT:
      if self.x > 0 and field.data[self.y][self.x - 1] == '.':
        nx -= 1
    if self.action == MonsterAction.RIGHT:
      if self.x < field.width - 1 and field.data[self.y][self.x + 1] == '.':
        nx += 1
    if self.action == MonsterAction.UP:
      if self.y > 0 and field.data[self.y - 1][self.x] == '.':
        ny -= 1
    if self.action == MonsterAction.DOWN:
      if self.y < field.height - 1 and field.data[self.y + 1][self.x] == '.':
        ny += 1
    for bomb in bombs:
      if nx == bomb.x and ny == bomb.y:
        return
    self.x = nx
    self.y = ny

  def draw(self, client, layer = 4, permanent = False):
    client.set_options(layer=layer, permanent=permanent)
    block_size = config.block_size
    monster_radius = config.monster_radius
    client.circle(block_size * self.x + block_size // 2, block_size * self.y + block_size // 2, monster_radius, Color.PURPLE.value, True)

  
class FeatureAdd(Entity):
  def __init__(self, x, y):
    Entity.__init__(self)
    self.type = "f_a"
    self.owner = -1
    self.x = x
    self.y = y

  def draw(self, client, layer = 5, permanent = False):
    client.set_options(layer=layer, permanent=permanent)
    block_size = config.block_size
    client.circle(block_size * self.x + block_size // 2, block_size * self.y + block_size // 2, block_size // 4, Color.DARK_BLUE.value, True)
    
class FeatureRange(Entity):
  def __init__(self, x, y):
    Entity.__init__(self)
    self.type = "f_r"
    self.owner = -1
    self.x = x
    self.y = y

  def draw(self, client, layer = 5, permanent = False):
    client.set_options(layer=layer, permanent=permanent)
    block_size = config.block_size
    client.circle(block_size * self.x + block_size // 2, block_size * self.y + block_size // 2, block_size // 4, Color.DARK_RED.value, True)
    
