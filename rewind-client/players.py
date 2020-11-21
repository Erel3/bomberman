from models import Player
from models import PlayerAction
from config import config

import os

class StrategyPlayer(Player):

  def __init__(self, owner_id, x, y, color, pipe_name):
    Player.__init__(self, owner_id, x, y, color)
    self.pipe_name = pipe_name
    if os.path.exists("../pipes/{}_strategy".format(self.pipe_name)):
      os.remove("../pipes/{}_strategy".format(self.pipe_name))
    os.mkfifo("../pipes/{}_strategy".format(self.pipe_name))
    if os.path.exists("../pipes/{}_client".format(self.pipe_name)):
      os.remove("../pipes/{}_client".format(self.pipe_name))
    os.mkfifo("../pipes/{}_client".format(self.pipe_name))
    self.pin = None
    self.pout = None


  def _read_line(self):
    if self.pin == None:
      self.pin = open("../pipes/{}_strategy".format(self.pipe_name), "r")
    line = self.pin.readline()
    return line

  def _write_state(self, field, players, bombs, monsters):
    if self.pout == None:
      self.pout = open("../pipes/{}_client".format(self.pipe_name), "w")
    # game info
    self.pout.write("{} {} {} {}\n".format(field.width, field.height, self.owner, config.tick))
    # field
    for i in range(field.height):
      self.pout.write("{}\n".format("".join(field.data[i])))    
    # entities cnt
    self.pout.write("{}\n".format(len(players + bombs + monsters)))
    for player in players:
      self.pout.write("{} {} {} {} {} {}\n".format(player.type, player.owner, player.x, player.y, player.current_bomb_count, player.bomb_range))
    for bomb in bombs:
      self.pout.write("{} {} {} {} {} {}\n".format(bomb.type, bomb.owner, bomb.x, bomb.y, bomb.timer, bomb.range))
    for monster in monsters:
      self.pout.write("{} {} {} {} {} {}\n".format(monster.type, monster.owner, monster.x, monster.y, 0, 0))
    self.pout.flush()

  def tick(self, field, players, bombs, monsters):
    self._write_state(field, players, bombs, monsters)
    action = self._read_line().strip()
    self.action = PlayerAction(action)
    if self.action == PlayerAction.BOMB and self.current_bomb_count == 0:
      self.action = PlayerAction.STAY

  def __del__(self):
    self.pin.close()
    if os.path.exists("../pipes/{}_strategy".format(self.pipe_name)):
      os.remove("../pipes/{}_strategy".format(self.pipe_name))

    self.pout.close()
    if os.path.exists("../pipes/{}_client".format(self.pipe_name)):
      os.remove("../pipes/{}_client".format(self.pipe_name))
