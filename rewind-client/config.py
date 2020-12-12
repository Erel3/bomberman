from types import SimpleNamespace
from argparse import ArgumentParser
from models.field.field_type import FieldType
import sys

config = SimpleNamespace()


class ConfigParser(ArgumentParser):
  def __init__(self, config, prog, description):
    ArgumentParser.__init__(self, prog, description, add_help=False)
    self.add_argument('-w', '--width', action='store', type=int,
                      default=config.width, metavar=config.width, dest='width', help='width of field')
    self.add_argument('-h', '--height', action='store', type=int,
                      default=config.height, metavar=config.height, dest='height', help='height of field')
    self.add_argument('-bt', '--bomb-timer', action='store', type=int,
                      default=config.bomb_timer, metavar=config.bomb_timer, dest='bomb_timer', help='timer of bombs')
    self.add_argument('-bc', '--bomb-count', action='store', type=int,
                      default=config.bomb_count, metavar=config.bomb_count, dest='bomb_count', help='number of bombs that each player has at game start')
    self.add_argument('-br', '--bomb-range', action='store', type=int,
                      default=config.bomb_range, metavar=config.bomb_range, dest='bomb_range', help='range of bombs that each player has at game start')
    self.add_argument('-r', '--redraw', action='store_true',
                      default=config.every_step_redraw, dest='every_step_redraw', help='redraw field on every step inside one tick (bombs-clean-monsters-clean-players-clean)')
    self.add_argument('-t', '--ticks', action='store', type=int,
                      default=config.max_ticks, metavar=config.max_ticks, dest='max_ticks', help='number of ticks in game')
    self.add_argument('-p', '--player-strategy', action='append', nargs=3,
                      metavar=('strategy-file', 'x', 'y'), dest='players', help='player and it\'s position')
    self.add_argument('-f', '--field', action='store', metavar=list(map(lambda f: f.value, FieldType)),
                      default=FieldType.DEFAULT, type=FieldType, choices=list(FieldType), dest='field', help='field type generator')
    self.add_argument('-a', '--auto-test', action='store', type=int,
                      default=config.game_count, metavar=config.game_count, dest='game_count', help='auto test multiple games')
    self.add_argument('--proc', action='store', type=int,
                      default=config.proc, metavar=config.proc, dest='proc', help='number of processes for autotest')
    self.add_argument('--replay', action='store', type=str,
                      default=config.replay_file, metavar="../games/file.gamelog", dest='replay_file', help='replay game from game log')
    self.add_argument('-kp', '--keyboard-player', action='store_true',
                      default=config.with_keyboard_player, dest='with_keyboard_player', help='enable keyboard player')
    self.namespace = config

  def parse(self):
    self.parse_args(namespace=self.namespace)
    if len(self.namespace.replay_file) > 0:
      self.namespace.is_replay = True
      return
    if len(self.namespace.players) == 0:
      self.namespace.players.append(["../strategies/bot", '0', '0'])
    if self.namespace.game_count > 1:
      self.namespace.with_viewer = False

  def error(self, message):
    sys.stderr.write('error: %s\n' % message)
    self.print_help()
    sys.exit(2)


# drawer
config.every_step_redraw = False

# drawer config
config.block_size = 50
config.permanent_indent = 2
config.breakable_indent = 5
config.player_radius = 23
config.bomb_radius = 18
config.monster_radius = 21

# game config
config.width = 13
config.height = 11
config.bomb_timer = 6
config.bomb_count = 1
config.bomb_range = 2
config.players = []

# auto test
config.with_viewer = True
config.game_count = 1
config.proc = 4

# run variables
config.tick = 0
config.max_ticks = 400

# feature config
config.feature_percent = 28

# replay
config.is_replay = False
config.replay_file = ""

# with keyboard player
config.with_keyboard_player = False
