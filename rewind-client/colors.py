from enum import Enum
class Color(Enum):
  BLACK = 0x000000
  WHITE = 0xffffff
  GRAY = 0x5f5f5f
  DARK_GRAY = 0x404040
  BROWN = 0xa8800a
  PURPLE = 0x800080
  RED = 0xff0000
  TRANSPARENT_RED = 0x7fff0000
  GREEN = 0x00ff00
  BLUE = 0x0000ff
  DARK_RED = 0x770000
  DARK_GREEN = 0x007700
  DARK_BLUE = 0x000077
  TRANSPARENT = 0x7f000000
  INVISIBLE = 0x01000000


player_colors = [
  Color.BLUE,
  Color.RED,
  Color.GREEN,
  Color.DARK_BLUE,
  Color.DARK_RED,
  Color.DARK_GREEN
]
