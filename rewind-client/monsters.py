from models.entity import Monster, MonsterAction
import random
from config import config

import os

class DummyMonster(Monster):


  def tick(self, field, players, bombs, monsters):
    self.action = random.choice(list(MonsterAction)[1:])
    for player in players:
      if self.x + 1 == player.x and self.y == player.y:
        self.action = MonsterAction.RIGHT
      if self.x - 1 == player.x and self.y == player.y:
        self.action = MonsterAction.LEFT
      if self.x == player.x and self.y == player.y + 1:
        self.action = MonsterAction.DOWN
      if self.x == player.x and self.y == player.y - 1:
        self.action = MonsterAction.UP
