#!/usr/bin/env python
import sys
import random
import time


"""
    height width player_id
    -----
    width     - map width (int)
    height    - map height (int)
    player_id - player id (int)
    tick      - iteration number (int)


    Example:
    16 16 1
"""
while True:
    
    line = input()
    millis = int(round(time.time() * 1000))
    print(millis, file=sys.stderr)
    h = int(line.split()[1])
    print(h, file=sys.stderr)
    for i in range(h):
        # read map line
        line = input()
        print(line, file=sys.stderr)

    # n - number of actions takes during the tick
    n = int(input())
    print(n, file=sys.stderr)
    for i in range(n):
        # read actions
        line = input()
        print(line, file=sys.stderr)
    
    # use file=sys.stderr to print for debugging
    print("debug code", file=sys.stderr)

    # this will choose one of random actions
    actions = ["left", "up", "bomb", "right", "down"]
    random_index = random.randint(0, len(actions) - 1)

    # bot action
    print(actions[random_index])
    millis = int(round(time.time() * 1000))
    print(millis, file=sys.stderr)
