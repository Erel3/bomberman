import requests
import time
from datetime import datetime
import os
access_token = "Bearer eyJ0eXAiOiJKV1QiLCJhbGciOiJIUzI1NiJ9.eyJzdWIiOiJhYWFiYXQ3MUBnbWFpbC5jb20iLCJleHAiOjE2MDg1ODg1MTZ9.4plAu-0N84A8gZziCehsXNFwnT_EnjpY8PVg1sdHG64"
headers_dict = {
    "Cookie": 'access_token="{}"'.format(access_token)
}
    
def f(user_id):
    skip = 0
    iter_jump = 50
    limit = datetime.strptime("2020-12-20T18:00:00", '%Y-%m-%dT%H:%M:%S')
    fields = set()

    stat = dict()
    summary = {
        'win': 0,
        'lose': 0,
        'draw': 0, 
    }

    while skip != -1:
        url = "https://cup.alem.school/api/last_games/{}?skip={}&limit={}".format(user_id, skip, iter_jump)
        skip = skip + iter_jump
        games = requests.get(url, headers=headers_dict).json()
        for game in games:
            created_at = game['created_at'] 
            created_at = datetime.strptime(created_at, '%Y-%m-%dT%H:%M:%S')
            if created_at < limit:
                skip = -1
                continue
            enemy = game['captain_username']
            status = game['status']
            if not stat.get(enemy):
                stat.update(
                    {
                        enemy: {
                            'win': 0,
                            'lose': 0,
                            'draw': 0,        
                        },
                    }
                )
            stat[enemy][status] = stat[enemy][status] + 1
            summary[status] = summary[status] + 1
        time.sleep(1)
    
    for enemy, stat_enemy in stat.items():
        print(enemy + ":", stat_enemy, )
    print(summary)


url = "https://cup.alem.school/api/leaderboard/2"
teams = requests.get(url, headers=headers_dict).json()

for i in range(5):
    team = teams[i]
    player = next(filter(lambda player: player['is_captain'], team['players']))
    username = player['username']
    user_id = player['id']
    print("#" * 20)
    print("Username: {}".format(username))
    f(user_id)
    
