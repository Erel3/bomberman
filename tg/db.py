from sqlalchemy import Table, Column, Integer, Boolean, Float, String, MetaData
from sqlalchemy import create_engine
from sqlalchemy.ext.declarative import declarative_base
from sqlalchemy.orm import sessionmaker

engine = create_engine('sqlite:///sqlite.db', echo=False)
Session = sessionmaker(bind=engine)
Base = declarative_base()

class Leaderboard(Base):
  __tablename__ = 'leaderboard'
  id = Column(Integer, primary_key=True)
  place = Column(Integer)
  captain_id = Column(Integer)
  captain_name = Column(String)
  score = Column(Float)
  mu = Column(Float)
  sigma = Column(Float)
  last_game = Column(String)
  delta = Column(Float)

  def __init__(self, place, captain_id, captain_name, score, mu, sigma, last_game):
    self.place = place
    self.captain_id = captain_id
    self.captain_name = captain_name
    self.score = score
    self.mu = self.mu
    self.sigma = sigma
    self.last_game = last_game
    self.delta = 0


class Chats(Base):
  __tablename__ = 'chats'
  id = Column(Integer, primary_key=True)
  chat_id = Column(Integer)
  is_active = Column(Boolean)

  def __init__(self, chat_id):
    self.is_active = True
    self.chat_id = chat_id
  
Base.metadata.create_all(engine)

session = Session()
