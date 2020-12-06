import _socket
import json
from colors import Color
from config import config
from itertools import chain


class RewindClient():

  def __init__(self, host=None, port=None):
    if (config.with_viewer):
      self._socket = _socket.socket()
      self._socket.setsockopt(_socket.IPPROTO_TCP, _socket.TCP_NODELAY, True)
      if host is None:
        host = "127.0.0.1"
        port = 9111
      self._socket.connect((host, port))
    else:
      self._socket = None

  @staticmethod
  def _to_geojson(points):
    return list(chain.from_iterable(points))

  def _send(self, obj):
    if self._socket:
      self._socket.sendall(json.dumps(obj).encode('utf-8'))

  def line(self, x1, y1, x2, y2, color):
    self._send({
      'type': 'polyline',
      'points': [x1, y1, x2, y2],
      'color': color
    })

  def polyline(self, points, color):
    self._send({
      'type': 'polyline',
      'points': RewindClient._to_geojson(points),
      'color': color
    })

  def circle(self, x, y, radius, color, fill=False):
    self._send({
      'type': 'circle',
      'p': [x, y],
      'r': radius,
      'color': color,
      'fill': fill
    })

  def rectangle(self, x1, y1, x2, y2, color, fill=False):
    self._send({
      'type': 'rectangle',
      'tl': [x1, y1],
      'br': [x2, y2],
      'color': color,
      'fill': fill
    })

  def triangle(self, p1, p2, p3, color, fill=False):
    self._send({
      'type': 'triangle',
      'points': RewindClient._to_geojson([p1, p2, p3]),
      'color': color,
      'fill': fill
    })

  def circle_popup(self, x, y, radius, message):
    self._send({
      'type': 'popup',
      'p': [x, y],
      'r': radius,
      'text': message
    })

  def rect_popup(self, tl, br, message):
    self._send({
      'type': 'popup',
      'tl': RewindClient._to_geojson([tl]),
      'br': RewindClient._to_geojson([br]),
      'text': message
    })

  def message(self, msg):
    self._send({
      'type': 'message',
      'message': msg
    })

  def set_options(self, layer=None, permanent=None):
    data={'type': 'options'}
    if layer is not None:
      data['layer']=layer
    if permanent is not None:
      data['permanent']=permanent
    self._send(data)

  def end_frame(self):
    self._send({'type': 'end'})
