import requests
import time
import os
from datetime import datetime

user_id = 290
access_token = "Bearer eyJ0eXAiOiJKV1QiLCJhbGciOiJIUzI1NiJ9.eyJzdWIiOiJhYWFiYXQ3MUBnbWFpbC5jb20iLCJleHAiOjE2MDc4MjcxNjd9.rCR3k2-Jot2sQFOr_E4CwvqhypOK9e7N7cmaRxqI6e8"
games_count = 100
field_width = 13
field_height = 11

destroyed_boxes = 0

amount_features = 0
range_features = 0
jump_features = 0
teleport_features = 0

fields = set()

for i in range(games_count):
    game_number = i
    headers_dict = {
        "Cookie": 'access_token="{}"'.format(access_token)
    }

    url = "https://cup.alem.school/api/last_games/{}?skip={}&limit=10".format(user_id, game_number)
    game_info = requests.get(url, headers=headers_dict).json()
    game_session_id = game_info[0]['game_session_id']

    url = "https://cup.alem.school/api/logs_duel/{}".format(game_session_id)
    game_log = requests.get(url, headers=headers_dict).json()

    field = [['.'] * field_width for _ in range(field_height)]

    walls = game_log['logs']['initial_state']['walls']
    for wall in walls:
        field[wall['y']][wall['x']] = '!'

    boxes = game_log['logs']['initial_state']['boxes']
    for box in boxes:
        field[box['y']][box['x']] = ';'

    field_tuple = tuple(tuple(row) for row in field)
    fields.add(field_tuple)

    frames = game_log['logs']['frames']
    for frame in frames:
        for d in frame['d']:
            if d['n'] == '#' and field[d['y']][d['x']] == ';':
                destroyed_boxes += 1
                field[d['y']][d['x']] = '.'
        for t in frame['t']:
            if t['n'] == 'f_a':
                amount_features += 1
            if t['n'] == 'f_r':
                range_features += 1
            if t['n'] == 'f_j':
                jump_features += 1
            if t['n'] == 'f_t':
                teleport_features += 1
    print("processed {} from {}".format(i, games_count))
    print("collect {} different maps".format(len(fields)))
    print("destroyed boxes {}".format(destroyed_boxes))
    print("features probability {:10.4f} {:10.4f} {:10.4f} {:10.4f}".format(
        amount_features / destroyed_boxes,
        range_features / destroyed_boxes,
        jump_features / destroyed_boxes,
        teleport_features / destroyed_boxes,
        (amount_features + range_features + jump_features + teleport_features) / destroyed_boxes,
    ))
    time.sleep(5)


# folder_name = datetime.strftime(datetime.now(), "%Y.%m.%d-%H.%M.%S")
# path = '../maps/{}'.format(folder_name)
# # ne ebu 4to eto
# access_rights = 0o755
#
# try:
#     os.mkdir(path, access_rights)
# except Exception as e:
#     print ("Creation of the directory {} ailed: {}".format(path, e))
#
# for id, field in enumerate(fields):
#     file_path = path + '/' + str(id)
#     file = open(file_path, 'w')
#     for row in field:
#         for cell in row:
#             file.write(cell)
#         file.write(
#         '\n')
#     file.close()
