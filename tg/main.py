from db import Session, Leaderboard, Chats
from tabulate import tabulate
import requests

import telebot
import time
import traceback

import signal
import sys

token = ""
secret = ""

bot = telebot.TeleBot(token, threaded=False)


def notify_all(message):
  session = Session()
  chats = session.query(Chats).filter(Chats.is_active == True).all()
  for chat in chats:
    bot.send_message(chat.chat_id, message, parse_mode="Markdown")
  session.close()


def signal_handler(sig, frame):
  notify_all('bot stopped')
  print("\nYou pressed Ctrl+C! Exiting.")
  sys.exit(0)


@bot.message_handler(commands=['secret', 's'])
def check_secret(message):
  if len(message.text.split(' ', 1)) > 1:
    if(message.text.split(' ', 1)[1] == secret):
      session = Session()
      chat = session.query(Chats).filter(
          Chats.chat_id == message.chat.id).first()
      if chat == None:
        chat = Chats(message.chat.id)
        session.add(chat)
      else:
        chat.is_active = True
      session.commit()
      session.close()
      bot.delete_message(message.chat.id, message.message_id)
    else:
      bot.reply_to(message, "wrong secret")


@bot.message_handler(commands=['deactivate'])
def deactivate_chat(message):
  session = Session()
  chat = session.query(Chats).filter(Chats.chat_id == message.chat.id).first()
  if chat != None:
    chat.is_active = False
    session.add(chat)
    bot.reply_to(message, "deactivated")
  session.commit()
  session.close()


@bot.message_handler(commands=['activate'])
def activate_chat(message):
  session = Session()
  chat = session.query(Chats).filter(Chats.chat_id == message.chat.id).first()
  if chat != None:
    chat.is_active = True
    session.add(chat)
    bot.reply_to(message, "activated")
  session.commit()
  session.close()


@bot.message_handler(commands=['top'])
def leaderboard_top(message):
  session = Session()
  chat = session.query(Chats).filter(Chats.chat_id == message.chat.id).first()
  if chat == None:
    return
  leaderboard = session.query(Leaderboard).order_by(
      Leaderboard.place).all()[:10]
  msg_text = tabulate([[leader.place, leader.captain_name, leader.score] for leader in leaderboard], headers=['', 'PLAYER', 'SCORE'])
  session.commit()
  session.close()
  bot.reply_to(message, "`"+msg_text+"`", parse_mode="Markdown")


@bot.message_handler(commands=['all'])
def leaderboard_all(message):
  session = Session()
  chat = session.query(Chats).filter(Chats.chat_id == message.chat.id).first()
  if chat == None:
    return
  leaderboard = session.query(Leaderboard).order_by(Leaderboard.place).all()
  msg_text = tabulate([[leader.place, leader.captain_name, leader.score] for leader in leaderboard], headers=['', 'PLAYER', 'SCORE'])
  session.commit()
  session.close()
  bot.reply_to(message, "`"+msg_text+"`", parse_mode="Markdown")


def notify_update_leaderboard():
  session = Session()
  leaderboard = session.query(Leaderboard).order_by(
      Leaderboard.place).all()[:10]
  msg_text = tabulate([[leader.place, leader.captain_name, leader.score] for leader in leaderboard], headers=['', 'PLAYER', 'SCORE'])
  chats = session.query(Chats).filter(Chats.is_active == True)
  for chat in chats:
    bot.send_message(chat.chat_id, "`"+msg_text+"`", parse_mode="Markdown")
  session.commit()
  session.close()


def get_leaderboard():
  link = "https://cup.alem.school/api/leaderboard/2"
  data = requests.get(link).json()
  return data


def update_leaderboard(leaderboard_data):
  session = Session()
  top_updated = False
  for id, participant in enumerate(leaderboard_data, 1):
    place = id
    captain_id = [p for p in participant['players']
                  if p['is_captain']][0]['id']
    captain_name = [p for p in participant['players']
                    if p['is_captain']][0]['username']
    score = participant['score']
    mu = participant['mu']
    sigma = participant['sigma']
    last_game = "" if len(
        participant['last_games']) == 0 else participant['last_games'][0]["game_session_id"]

    leaderboard_participant = session.query(Leaderboard).filter(
        Leaderboard.captain_id == captain_id).first()
    if leaderboard_participant == None:
      leaderboard_participant = Leaderboard(
          place, captain_id, captain_name, score, mu, sigma, last_game)
    else:
      leaderboard_participant.place = place
      if leaderboard_participant.score != score:
        leaderboard_participant.delta = score - leaderboard_participant.score
        if place <= 10:
          top_updated = True
      leaderboard_participant.score = score
      leaderboard_participant.mu = mu
      leaderboard_participant.sigma = sigma
      leaderboard_participant.last_game = last_game

    session.add(leaderboard_participant)
  session.commit()
  session.close()
  return top_updated


def get_messages():
  updates = bot.get_updates(offset=(bot.last_update_id + 1), timeout=15)
  bot.process_new_updates(updates)


def send_all_error():
  notify_all('*some error occured. please check!*')


def start():
  signal.signal(signal.SIGINT, signal_handler)
  print("Press Ctrl+C to exit")
  cycle = 0

  notify_all('bot started')
  leaderboard_data = get_leaderboard()
  changed = update_leaderboard(leaderboard_data)
  latest_leaderboard_update_milis = int(round(time.time() * 1000))
  latest_leaderboard_update_delay_milis = 125 * 1000
  notify_update_leaderboard()
  while True:
    try:
      current_milis = int(round(time.time() * 1000))
      if latest_leaderboard_update_milis + latest_leaderboard_update_delay_milis < current_milis:
        print('update start')
        leaderboard_data = get_leaderboard()
        top_changed = update_leaderboard(leaderboard_data)
        latest_leaderboard_update_milis = current_milis
        if top_changed:
          notify_update_leaderboard()
        print('update end')
      print('messages start')
      get_messages()
      print('messages end')
      print('5 sec start')
      time.sleep(5)
      print('5 sec end')
    except SystemExit:
      raise
    except Exception:
      traceback.print_exc()
      send_all_error()
      pass
    except:
      pass
    cycle += 1


if __name__ == "__main__":
  start()
