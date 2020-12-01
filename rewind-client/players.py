import subprocess

from models import Player
from models import PlayerAction
from config import config

import os

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
    Player.__init__(self, owner_id, x, y, color)
    self.proc = subprocess.Popen(file, stdin=subprocess.PIPE, stdout=subprocess.PIPE, universal_newlines=True, stderr=subprocess.DEVNULL if hide_stderr else None)


  def _read_line(self):
    try:
      line = self.proc.stdout.readline()
      return line
    except:
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
      self.proc.stdin.write("{}\n".format(len(players + bombs + monsters)))
      for player in players:
        self.proc.stdin.write("{} {} {} {} {} {}\n".format(player.type, player.owner, player.x, player.y, player.current_bomb_count, player.bomb_range))
      for bomb in bombs:
        self.proc.stdin.write("{} {} {} {} {} {}\n".format(bomb.type, bomb.owner, bomb.x, bomb.y, bomb.timer, bomb.range))
      for monster in monsters:
        self.proc.stdin.write("{} {} {} {} {} {}\n".format(monster.type, monster.owner, monster.x, monster.y, 0, 0))
      self.proc.stdin.flush()
    except:
      print("error writing state to player".format(self.owner))
      pass

  def tick(self, field, players, bombs, monsters, features):
    self._write_state(field, players, bombs, monsters, features)
    action = self._read_line().strip()
    if action == '':
      action = 'stay'
    self.action = PlayerAction(action)
    if self.action == PlayerAction.BOMB and self.current_bomb_count == 0:
      self.action = PlayerAction.STAY

  def __del__(self):
    try:
      self.proc.stdin.write("-1 -1 -1 -1")
      self.proc.terminate
    except:
      pass
