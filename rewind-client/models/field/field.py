from config import config
from colors import Color
from enum import Enum
from .field_type import FieldType
import random


def _no_blocks_gen(self, width, height):
  return [
      ['.' if random.randint(0, 1) == 0 else ';' for i in range(width)]
      for j in range(height)
  ]


def _full_gen(self, width, height):
  return [
      [';' for i in range(width)]
      for j in range(height)
  ]


def _snake_gen(self, width, height):
  return [
      ['!' if (j & 1) and ((i > 0 and j % 4 == 1) or (i < width - 1 and j % 4 == 3))
       else ('.' if random.randint(0, 1) == 0 else ';') for i in range(width)]
      for j in range(height)
  ]


def _default_gen(self, width, height):
  return [
      ['!' if (i & 1) and (j & 1) else ('.' if random.randint(
          0, 1) == 0 else ';') for i in range(width)]
      for j in range(height)
  ]


class FieldConstructor():

  def __init__(self, ftype):
    if (ftype == FieldType.SNAKE):
      self.construct = _snake_gen
    elif (ftype == FieldType.NO_BLOCKS):
      self.construct = _no_blocks_gen
    elif (ftype == FieldType.FULL):
      self.construct = _full_gen
    else:
      self.construct = _default_gen

  def create_field(self, width, height):
    return self.construct(self, width, height)


class Field():

  def __init__(self, constructor):
    self.width = config.width
    self.height = config.height
    self.data = constructor.create_field(self.width, self.height)
    self.destroy_data = [
        [False for i in range(self.width)]
        for j in range(self.height)
    ]
    self.box_of_player = [
      [{} for i in range(self.width)]
        for j in range(self.height)
    ]

  def draw(self, client, layer=1, permanent=True):
    client.set_options(layer=layer, permanent=permanent)
    block_size = config.block_size
    client.rectangle(-1000, -1000, 2000, 2000, Color.GRAY.value, True)
    client.rectangle(0, 0, self.width * block_size,
                     self.height * block_size, Color.DARK_GREEN.value, True)
    for i in range(self.width + 1):
      client.line(i * block_size, 0, i * block_size,
                  self.height * block_size, Color.BLACK.value)
    for i in range(self.height + 1):
      client.line(0, i * block_size, self.width * block_size,
                  i * block_size, Color.BLACK.value)

  def draw_permanent_blocks(self, client, layer=2, permanent=True):
    client.set_options(layer=layer, permanent=permanent)
    block_size = config.block_size
    permanent_indent = config.permanent_indent
    for i in range(self.width):
      for j in range(self.height):
        if self.data[j][i] == '!':
          client.rectangle(
              i * block_size + permanent_indent, j * block_size + permanent_indent,
              (i + 1) * block_size - permanent_indent, (j + 1) *
              block_size - permanent_indent,
              Color.DARK_GRAY.value, True
          )

  def draw_breakable_blocks(self, client, layer=3, permanent=False):
    client.set_options(layer=layer, permanent=permanent)
    block_size = config.block_size
    breakable_indent = config.breakable_indent
    for i in range(self.width):
      for j in range(self.height):
        if self.data[j][i] == ';':
          client.rectangle(
              i * block_size + breakable_indent, j * block_size + breakable_indent,
              (i + 1) * block_size - breakable_indent, (j + 1) *
              block_size - breakable_indent,
              Color.BLACK.value
          )
          client.rectangle(
              i * block_size + breakable_indent, j * block_size + breakable_indent,
              (i + 1) * block_size - breakable_indent, (j + 1) *
              block_size - breakable_indent,
              Color.BROWN.value, True
          )
          client.line(
              i * block_size +
              breakable_indent, (j + 1) * block_size - breakable_indent,
              (i + 1) * block_size - breakable_indent, j *
              block_size + breakable_indent,
              Color.BLACK.value
          )
          client.line(
              i * block_size + breakable_indent, j * block_size + breakable_indent,
              (i + 1) * block_size - breakable_indent, (j + 1) *
              block_size - breakable_indent,
              Color.BLACK.value
          )

  def draw_destroy_data(self, client, layer=5, permanent=False):
    client.set_options(layer=layer, permanent=permanent)
    block_size = config.block_size
    permanent_indent = config.permanent_indent
    for i in range(self.width):
      for j in range(self.height):
        if self.destroy_data[j][i]:
          client.rectangle(
              i * block_size + permanent_indent, j * block_size + permanent_indent,
              (i + 1) * block_size - permanent_indent, (j + 1) *
              block_size - permanent_indent,
              Color.TRANSPARENT_RED.value, True
          )

  def clean(self):
    for i in range(self.height):
      for j in range(self.width):
        self.destroy_data[i][j] = False
    self.box_of_player = [
      [{} for i in range(self.width)]
        for j in range(self.height)
    ]
