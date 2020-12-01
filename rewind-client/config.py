from types import SimpleNamespace
from argparse import ArgumentParser
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
    self.namespace = config

  def parse(self):
    self.parse_args(namespace=self.namespace)

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


# run variables
config.tick = 0
config.max_ticks = 400
