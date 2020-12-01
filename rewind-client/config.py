from types import SimpleNamespace

config = SimpleNamespace()

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
config.bomb_count = 3
config.bomb_range = 2


# run variables

config.tick = 0
config.max_ticks = 400

# feature config
config.feature_percent = 30