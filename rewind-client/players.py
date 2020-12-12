import subprocess

from models.entity import Player, PlayerAction
from config import config

import os
import select

##### USED NAMED PIPES
# class StrategyPlayer(Player):

#   def __init__(self, owner_id, x, y, color, pipe_name):
#     Player.__init__(self, owner_id, x, y, color)
#     self.pipe_name = pipe_name
#     if os.path.exists("../pipes/{}_strategy".format(self.pipe_name)):
#       os.remove("../pipes/{}_strategy".format(self.pipe_name))
#     os.mkfifo("../pipes/{}_strategy".format(self.pipe_name))
#     if os.path.exists("../pipes/{}_client".format(self.pipe_name)):
#       os.remove("../pipes/{}_client".format(self.pipe_name))
#     os.mkfifo("../pipes/{}_client".format(self.pipe_name))
#     self.pin = None
#     self.pout = None


#   def _read_line(self):
#     try:
#       if self.pin == None:
#         self.pin = open("../pipes/{}_strategy".format(self.pipe_name), "r")
#       line = self.pin.readline()
#       return line
#     except:
#       print("Player1 not responded")
#       return ''

#   def _write_state(self, field, players, bombs, monsters, features):
#     try:
#       if self.pout == None:
#         self.pout = open("../pipes/{}_client".format(self.pipe_name), "w")
#       # game info
#       self.pout.write("{} {} {} {}\n".format(field.width, field.height, self.owner, config.tick))
#       # field
#       for i in range(field.height):
#         self.pout.write("{}\n".format("".join(field.data[i])))    
#       # entities cnt
#       self.pout.write("{}\n".format(len(players + bombs + monsters)))
#       for player in players:
#         self.pout.write("{} {} {} {} {} {}\n".format(player.type, player.owner, player.x, player.y, player.current_bomb_count, player.bomb_range))
#       for bomb in bombs:
#         self.pout.write("{} {} {} {} {} {}\n".format(bomb.type, bomb.owner, bomb.x, bomb.y, bomb.timer, bomb.range))
#       for monster in monsters:
#         self.pout.write("{} {} {} {} {} {}\n".format(monster.type, monster.owner, monster.x, monster.y, 0, 0))
#       self.pout.flush()
#     except:
#       pass

#   def tick(self, field, players, bombs, monsters, features):
#     self._write_state(field, players, bombs, monsters, features)
#     action = self._read_line().strip()
#     if action == '':
#       action = 'stay'
#     self.action = PlayerAction(action)
#     if self.action == PlayerAction.BOMB and self.current_bomb_count == 0:
#       self.action = PlayerAction.STAY

#   def __del__(self):
#     try:
#       self.pout.write("-1 -1 -1 -1")

#       self.pin.close()
#       if os.path.exists("../pipes/{}_strategy".format(self.pipe_name)):
#         os.remove("../pipes/{}_strategy".format(self.pipe_name))

#       self.pout.close()
#       if os.path.exists("../pipes/{}_client".format(self.pipe_name)):
#         os.remove("../pipes/{}_client".format(self.pipe_name))
#     except:
#       pass


##### USED SUBPROCESS PIPES
class StrategyPlayer(Player):

  def __init__(self, owner_id, x, y, color, file, hide_stderr=True):
    Player.__init__(self, owner_id, x, y, color, config.bomb_count, config.bomb_range)
    self.broken = False
    self.proc = subprocess.Popen(file, stdin=subprocess.PIPE, stdout=subprocess.PIPE, universal_newlines=True, stderr=subprocess.DEVNULL if hide_stderr else None)


  def copy_from(self, player):
    self.owner = player.owner
    self.x = player.x
    self.y = player.y
    self.color = player.color
    self.bomb_count = player.bomb_count
    self.current_bomb_count = player.current_bomb_count
    self.bomb_range = player.bomb_range
    self.teleport = player.teleport
    self.jump = player.jump

  def _read_line(self):
    try:
      line = self.proc.stdout.readline()
      return line
    except:
      self.broken = True
      if config.with_viewer:
        print("player {} not responded".format(self.owner))
      return ''

  def _write_state(self, field, players, bombs, monsters, features):
    try:
      # game info
      self.proc.stdin.write("{} {} {} {}\n".format(field.width, field.height, self.owner, config.tick))
      # field
      for i in range(field.height):
        self.proc.stdin.write("{}\n".format("".join(field.data[i])))    
      # entities cnt
      self.proc.stdin.write("{}\n".format(len(players + bombs + monsters + features)))
      for player in players:
        self.proc.stdin.write("{} {} {} {} {} {}\n".format(player.type, player.owner, player.x, player.y, player.current_bomb_count, player.bomb_range))
      for bomb in bombs:
        self.proc.stdin.write("{} {} {} {} {} {}\n".format(bomb.type, bomb.owner, bomb.x, bomb.y, bomb.timer, bomb.range))
      for monster in monsters:
        self.proc.stdin.write("{} {} {} {} {} {}\n".format(monster.type, monster.owner, monster.x, monster.y, 0, 0))
      for feature in features:
        self.proc.stdin.write("{} {} {} {} {} {}\n".format(feature.type, feature.owner, feature.x, feature.y, 0, 0))
      player_features = []
      for player in players:
        if player.teleport:
          player_features.append((player.owner, 1))
        if player.jump:
          player_features.append((player.owner, 0))
      self.proc.stdin.write("{}\n".format(len(player_features)))
      for feature in player_features:
        self.proc.stdin.write("{} {}\n".format(feature[0], feature[1]))
      self.proc.stdin.flush()
    except:
      self.broken = True
      if config.with_viewer:
        print("error writing state to player {}".format(self.owner))
      pass

  def tick(self, field, players, bombs, monsters, features):
    if not self.broken:
      self._write_state(field, players, bombs, monsters, features)
    action_elements = self._read_line().strip().split() if not self.broken else ['']
    action = action_elements[0]
    if len(action_elements) > 1:
      self.action_x, self.action_y = int(action_elements[1]), int(action_elements[2])
    if action == '':
      action = 'stay'
    self.action = PlayerAction(action)
    if self.action == PlayerAction.BOMB and self.current_bomb_count == 0:
      self.action = PlayerAction.STAY
    if self.action == PlayerAction.JUMP and self.jump == False:
      self.action = PlayerAction.STAY
    if self.action == PlayerAction.TELEPORT and self.teleport == False:
      self.action = PlayerAction.STAY

  def __del__(self):
    try:
      self.proc.stdin.write("-1 -1 -1 -1")
      self.proc.stdout.close()
      self.proc.stdin.close()
      self.proc.terminate
      del self.proc
    except:
      pass
