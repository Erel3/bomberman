from colors import Color
from client import RewindClient
from config import config

class DrawHelper():
  
  def __init__(self, field):
    self.client = RewindClient()

    field.draw(self.client)
    field.draw_permanent_blocks(self.client)

  def current_step(self, name, field, entities):
    self.client.message(name)
    self.redraw(field, entities)

  def redraw(self, field, entities):
    if (config.with_viewer):
      field.draw_breakable_blocks(self.client)
      for entity in entities:
        entity.draw(self.client)
      self.end_frame()

  def end_frame(self):
    self.client.end_frame()
