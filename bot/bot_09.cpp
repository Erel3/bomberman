#pragma comment(linker, "/stack:20000000")
#pragma GCC optimize("Ofast")
#pragma GCC target("sse,sse2,sse3,ssse3,sse4,popcnt,abm,mmx")

#include <stdio.h>
#include <bits/stdc++.h>
#include <x86intrin.h>
#include <ext/pb_ds/assoc_container.hpp>
using namespace __gnu_pbds;
template <typename T>
using ordered_set = tree<T, null_type, std::less<T>, rb_tree_tag, tree_order_statistics_node_update>;
template <typename T1, typename T2>
using ordered_map = tree<T1, T2, std::less<T1>, rb_tree_tag, tree_order_statistics_node_update>;

using namespace std;

#define pb push_back
#define ppb pop_back
#define mp make_pair
#define fs first
#define sc second
#define abs(a) ((a) < 0 ? -(a) : (a))
#define sqr(a) ((a) * (a))

typedef unsigned int uint;
typedef long long ll;
typedef unsigned long long ull;

int dx[] = {0, -1, 1, 0, 0}, dy[] = {-1, 0, 0, 1, 0};
double max_tick_time = 0;

const int BOMB_TIMER = 6;
const int W = 13, H = 11, K = W + H + 10;
const int MAX_BOMB = 12;

enum PlayerMove
{
  PLAYER_STAY,
  PLAYER_LEFT,
  PLAYER_RIGHT,
  PLAYER_DOWN,
  PLAYER_UP,
  PLAYER_BOMB
};

class Player
{
public:
  Player(int x, int y, int owner_id, int bombs, int range) : x(x), y(y), owner_id(owner_id), bombs(bombs), range(range)
  {
    this->max_bombs = bombs;
    if (range == 0)
      range = 2;
    this->range = range;
  }

  int x, y;
  int owner_id;
  int max_bombs, bombs, range;
  PlayerMove action = PLAYER_STAY;
};

class Bomb
{
public:
  Bomb() {}
  Bomb(int x, int y, int owner_id, int timer, int range) : x(x), y(y), owner_id(owner_id), timer(timer), range(range) {}

  int x, y;
  int owner_id;
  int timer, range;
};

class Monster
{
public:
  Monster(int x, int y, int owner_id) : x(x), y(y), owner_id(owner_id) {}

  int x, y;
  int owner_id;
};

enum FeatureType
{
  FEATURE_RANGE = 'r',
  FEATURE_AMOUNT = 'a'
};

class Feature
{
public:
  FeatureType type;
  int x, y;
  Feature() {}
  Feature(int x, int y, char c)
  {
    this->type = FeatureType(c);
    this->x = x;
    this->y = y;
  }
};

enum CellType
{
  CELL_EMPTY = '.',
  CELL_BOX = ';',
  CELL_BLOCK = '!'
};

class Cell
{
public:
  CellType type;

  Cell()
  {
    this->type = CELL_EMPTY;
  }

  Cell(char c)
  {
    this->type = CellType(c);
  }
  Cell(CellType cell)
  {
    this->type = cell;
  }
};

class Field
{
public:
  Field() {}

  Field(int width, int height)
  {
    this->width = width;
    this->height = height;
    this->cells = vector<vector<Cell>>(height, vector<Cell>(width));
  }

  int width, height;
  vector<vector<Cell>> cells;
};

class Game
{
private:
  Player *me = nullptr;
  Player *enemy = nullptr;
  Field field;
  vector<Player> players;
  vector<Bomb> bombs;
  vector<Monster> monsters;
  vector<Feature> features;
  int current_global_score = 0;

  void _log(string s)
  {
    s.append("\n");
    fprintf(stderr, s.c_str());
  }

public:
  int tick;
  int start_x, start_y;
  Game()
  {
    read_state(true);
    this->start_x = this->me->x;
    this->start_y = this->me->y;
    cerr << this->start_x << " " << this->start_y << endl;
  }

  void read_state(bool create = false)
  {
    int width, height, owner_id, tick;
    scanf("%d%d%d%d", &width, &height, &owner_id, &tick);
    if (width == -1 && height == -1)
    {
      fprintf(stderr, "max time: %.3lf ms\n", max_tick_time);
      exit(0);
    }

    if (create)
    {
      this->field = Field(width, height);
    }

    for (int i = 0; i < height; i++)
    {
      scanf("\n");
      for (int j = 0; j < width; j++)
      {
        char c;
        scanf("%c", &c);
        field.cells[i][j] = Cell(c);
      }
    }

    // clear
    players.clear();
    bombs.clear();
    monsters.clear();
    features.clear();

    int entities;
    scanf("%d", &entities);
    for (int i = 0; i < entities; i++)
    {
      char type;
      char feature_type;
      int owner_id, x, y, param1, param2;
      scanf("\n%c", &type);
      if (type == 'f')
        scanf("%*c%c", &feature_type);
      scanf("%d%d%d%d%d", &owner_id, &x, &y, &param1, &param2);
      if (type == 'p')
      {
        players.pb(Player(x, y, owner_id, param1, param2));
      }
      if (type == 'b')
      {
        if (param1 == 0)
          assert(0);
        bombs.pb(Bomb(x, y, owner_id, param1, param2));
      }
      if (type == 'm')
      {
        monsters.pb(Monster(x, y, owner_id));
      }
      if (type == 'f')
      {
        features.pb(Feature(x, y, feature_type));
      }
    }
    int features;
    cerr << "reading f" << endl;
    scanf("%d", &features);
    cerr << "reading f" << endl;
    for(int i = 0; i < features; i++) {
      scanf("%*d%*d");
    }
    this->me = nullptr;
    this->enemy = nullptr;
    for (Player &player : players)
    {
      if (player.owner_id == owner_id)
      {
        this->me = &player;
      }
      else
      {
        this->enemy = &player;
      }
      for (Bomb bomb : bombs)
      {
        if (player.owner_id == bomb.owner_id)
        {
          player.max_bombs++;
        }
      }
    }
    this->tick = tick;
  }

  int f(int dist, int own_score, int others_score)
  {
    return (own_score - others_score - 1) * 5 - dist + 100000;
  }

  void simulate_tick(int tick, bitset<W> (&accessibleness)[H], bitset<W> (&current_field)[H], bitset<W * H> (&destroy_boxes)[2], Bomb (&bombs)[MAX_BOMB], int &bombs_count)
  {
    bitset<W> next_accessibleness[H];
    bitset<W> destroy_field[H];

    for (int i = 0; i < H; i++)
    {
      next_accessibleness[i] = accessibleness[i];
      if (i + 1 < H)
        next_accessibleness[i] |= accessibleness[i + 1];
      if (i - 1 >= 0)
        next_accessibleness[i] |= accessibleness[i - 1];
      next_accessibleness[i] |= (accessibleness[i] << 1);
      next_accessibleness[i] |= (accessibleness[i] >> 1);
    }

    for (int i = 0; i < bombs_count; i++)
    {
      if (bombs[i].timer == tick || destroy_field[bombs[i].y][bombs[i].x])
      {
        destroy_field[bombs[i].y][bombs[i].x] = 1;
        for (int dir = 0; dir < 4; dir++)
        {
          for (int j = 1; j <= bombs[i].range; j++)
          {
            int to_x = bombs[i].x + dx[dir] * j, to_y = bombs[i].y + dy[dir] * j;
            if (to_x < 0 || to_x >= field.width ||
                to_y < 0 || to_y >= field.height || field.cells[to_y][to_x].type == CELL_BLOCK)
            {
              break;
            }
            destroy_field[to_y][to_x] = 1;
            if (field.cells[to_y][to_x].type == CELL_BOX && current_field[to_y][to_x] == 1)
            {
              destroy_boxes[bombs[i].owner_id == this->me->owner_id][to_y * W + to_x] = 1;
              break;
            }
          }
        }
        swap(bombs[i], bombs[bombs_count - 1]);
        bombs_count--;
        i = -1;
      }
    }

    for (int i = 0; i < bombs_count; i++)
    {
      int x = bombs[i].x;
      int y = bombs[i].y;
      next_accessibleness[y][x] = next_accessibleness[y][x] & accessibleness[y][x];
    }

    for (int i = 0; i < H; i++)
    {
      next_accessibleness[i] &= (next_accessibleness[i] ^ current_field[i]);
      next_accessibleness[i] &= (next_accessibleness[i] ^ destroy_field[i]);
      current_field[i] ^= (current_field[i] & destroy_field[i]);
    }

    for (int i = 0; i < H; i++)
      accessibleness[i] = next_accessibleness[i];
  }

  tuple<bool, int, int> check_safe_and_get_score(int tick, int own_x, int own_y, bitset<W> (&previous_field)[H], Bomb (&previous_bombs)[MAX_BOMB], int bombs_count, bool set_bomb)
  {
    Bomb bombs[MAX_BOMB];
    for (int i = 0; i < bombs_count; i++)
      bombs[i] = previous_bombs[i];
    bitset<W> current_field[H];
    for (int i = 0; i < H; i++)
      current_field[i] = previous_field[i];
    bitset<W> accessibleness[H];
    bitset<W * H> destroy_boxes[2];

    accessibleness[own_y][own_x] = 1;
    // check really tick + 6 or not
    if (set_bomb)
      bombs[bombs_count++] = Bomb(own_x, own_y, this->me->owner_id, tick + BOMB_TIMER, this->me->range);
    while (bombs_count > 0)
    {
      simulate_tick(tick, accessibleness, current_field, destroy_boxes, bombs, bombs_count);
      tick++;
    }
    bool is_safe = false;
    for (int i = 0; i < H; i++)
      is_safe |= (accessibleness[i].count() > 0);
    int own_score = destroy_boxes[1].count();
    int rival_score = destroy_boxes[0].count();
    return make_tuple(is_safe, own_score, rival_score);
  }

  tuple<int, int, int, int> get_action(vector<Bomb> vec_bombs, Field field, int own_x, int own_y, int next_tick_with_bomb, bool ignore_afk = false)
  {
    int max_f = -1;
    int go_x = own_x, go_y = own_y, go_tick = 0;

    int bombs_count = vec_bombs.size();
    Bomb bombs[MAX_BOMB];
    for (int i = 0; i < bombs_count; i++)
      bombs[i] = vec_bombs[i];

    bitset<W * H> destroy_boxes[2];
    bitset<W> accessibleness[H];

    accessibleness[own_y][own_x] = 1;

    bitset<W> current_field[H];

    bool is_calced[H][W];
    tuple<bool, int, int> calced_value[H][W];
    for (int i = 0; i < H; i++)
      for (int j = 0; j < W; j++)
        is_calced[i][j] = 0;

    for (int i = 0; i < H; i++)
    {
      for (int j = 0; j < W; j++)
      {
        if (field.cells[i][j].type == CELL_BLOCK || field.cells[i][j].type == CELL_BOX)
        {
          current_field[i][j] = 1;
        }
      }
    }
    // cerr << "STATE" << endl;
    // for (int i = 0; i < H; i++)
    //   cerr << current_field[i] << endl;
    // for (int i = 0; i < bombs_count; i++)
    //   cerr << bombs[i].x << " " << bombs[i].y << endl;
    // cerr << endl;
    // cerr << "START SIMULATING IN GET ACTION" << endl;
    bool blank;
    int own_afk_score, rival_next_score;
    tie(blank, own_afk_score, rival_next_score) = check_safe_and_get_score(1, own_x, own_y, current_field, bombs, bombs_count, false);
    for (int tick = 1; tick < K; tick++)
    {
      bitset<W> prev_accessibleness[H];
      for (int i = 0; i < H; i++)
        prev_accessibleness[i] = accessibleness[i];

      simulate_tick(tick, accessibleness, current_field, destroy_boxes, bombs, bombs_count);

      int own_score = destroy_boxes[1].count();
      int rival_score = destroy_boxes[0].count();
      // cerr << "START CALCING" << endl;
      for (int y = 0; y < H; y++)
      {
        for (int x = 0; x < W; x++)
        {
          if (prev_accessibleness[y][x] && accessibleness[y][x])
          {
            bool is_safe;
            int own_score_change, rival_score_change;

            if (bombs_count > 0)
            {
              tie(is_safe, own_score_change, rival_score_change) = check_safe_and_get_score(tick + 1, x, y, current_field, bombs, bombs_count, true);
            }
            else
            {
              if (!is_calced[y][x])
              {
                is_calced[y][x] = 1;
                calced_value[y][x] = check_safe_and_get_score(tick + 1, x, y, current_field, bombs, bombs_count, true);
              }
              tie(is_safe, own_score_change, rival_score_change) = calced_value[y][x];
            }

            if (is_safe)
            {
              int own_next_score = own_score + own_score_change;
              int rival_next_score = rival_score + rival_score_change;
              int cur_f = f(tick, own_next_score, rival_next_score);
              if ((ignore_afk || own_next_score - rival_next_score > own_afk_score - rival_next_score) &&
                  max_f < cur_f && tick >= next_tick_with_bomb + 1)
              {
                max_f = cur_f;
                go_x = x;
                go_y = y;
                go_tick = tick - 1;
              }
            }
          }
        }
      }
    }
    // cerr << "END GET ACTION" << endl;
    //maybe cerr max_f == -1?
    return make_tuple(go_tick, go_x, go_y, max_f);
  }

  PlayerMove get_move(int s_tick, int s_x, int s_y)
  {
    cerr << " DIR " << s_tick << " " << s_x << " " << s_y << endl;
    if (s_tick == 0)
      return PLAYER_BOMB;

    int own_x = this->me->x;
    int own_y = this->me->y;

    int features_count = this->features.size();
    Feature features[features_count];
    for (int i = 0; i < features_count; i++)
      features[i] = this->features[i];

    int bombs_count = this->bombs.size();
    Bomb bombs[MAX_BOMB];
    for (int i = 0; i < bombs_count; i++)
      bombs[i] = this->bombs[i];

    int dp[K][H][W], pr[K][H][W];
    for (int i = 0; i < K; i++)
      for (int j = 0; j < H; j++)
        for (int q = 0; q < W; q++)
          dp[i][j][q] = -1;
    pr[0][own_y][own_x] = -1;
    dp[0][own_y][own_x] = this->me->max_bombs * 1000;

    bitset<W * H> destroy_boxes[2];
    bitset<W> accessibleness[H];

    accessibleness[own_y][own_x] = 1;

    bitset<W> current_field[H];

    for (int i = 0; i < H; i++)
    {
      for (int j = 0; j < W; j++)
      {
        if (field.cells[i][j].type == CELL_BLOCK || field.cells[i][j].type == CELL_BOX)
        {
          current_field[i][j] = 1;
        }
      }
    }

    for (int tick = 1; tick < K; tick++)
    {
      bitset<W> prev_accessibleness[H];
      for (int i = 0; i < H; i++)
        prev_accessibleness[i] = accessibleness[i];

      {
        bitset<W> next_accessibleness[H];
        bitset<W> destroy_field[H];

        for (int i = 0; i < H; i++)
        {
          next_accessibleness[i] = accessibleness[i];
          if (i + 1 < H)
            next_accessibleness[i] |= accessibleness[i + 1];
          if (i - 1 >= 0)
            next_accessibleness[i] |= accessibleness[i - 1];
          next_accessibleness[i] |= (accessibleness[i] << 1);
          next_accessibleness[i] |= (accessibleness[i] >> 1);
        }

        for (int i = 0; i < bombs_count; i++)
        {
          if (bombs[i].timer == tick || destroy_field[bombs[i].y][bombs[i].x])
          {
            destroy_field[bombs[i].y][bombs[i].x] = 1;
            for (int dir = 0; dir < 4; dir++)
            {
              for (int j = 1; j <= bombs[i].range; j++)
              {
                int to_x = bombs[i].x + dx[dir] * j, to_y = bombs[i].y + dy[dir] * j;
                if (to_x < 0 || to_x >= field.width ||
                    to_y < 0 || to_y >= field.height || field.cells[to_y][to_x].type == CELL_BLOCK)
                {
                  break;
                }
                destroy_field[to_y][to_x] = 1;
                if (field.cells[to_y][to_x].type == CELL_BOX && current_field[to_y][to_x] == 1)
                {
                  destroy_boxes[bombs[i].owner_id == this->me->owner_id][to_y * W + to_x] = 1;
                  break;
                }
              }
            }
            swap(bombs[i], bombs[bombs_count - 1]);
            bombs_count--;
            i = -1;
          }
        }

        for (int i = 0; i < features_count; i++)
        {
          if (destroy_field[features[i].y][features[i].x])
          {
            swap(features[i], features[features_count - 1]);
            features_count--;
            i--;
          }
        }

        for (int i = 0; i < bombs_count; i++)
        {
          int x = bombs[i].x;
          int y = bombs[i].y;
          next_accessibleness[y][x] = next_accessibleness[y][x] & accessibleness[y][x];
        }

        for (int i = 0; i < H; i++)
        {
          next_accessibleness[i] &= (next_accessibleness[i] ^ current_field[i]);
          next_accessibleness[i] &= (next_accessibleness[i] ^ destroy_field[i]);
          current_field[i] ^= (current_field[i] & destroy_field[i]);
        }

        for (int i = 0; i < H; i++)
          accessibleness[i] = next_accessibleness[i];
      }
      for (int y = 0; y < H; y++)
      {
        for (int x = 0; x < W; x++)
        {
          if (prev_accessibleness[y][x])
          {
            for (int dir = 0; dir < 5; dir++)
            {
              int to_x = x + dx[dir], to_y = y + dy[dir];
              if (to_x < 0 || to_x >= field.width ||
                  to_y < 0 || to_y >= field.height ||
                  !accessibleness[to_y][to_x])
              {
                continue;
              }
              if (dir < 4)
              {
                // make it less dummy
                bool bad = 0;
                for (int i = 0; i < bombs_count; i++)
                {
                  if (bombs[i].x == to_x && bombs[i].y == to_y)
                    bad = 1;
                }
                if (bad)
                  continue;
              }
              int dp_val = dp[tick - 1][y][x];
              int pr_val = pr[tick - 1][y][x];
              if (dir < 4)
              {
                for (int i = 0; i < features_count; i++)
                {
                  if (to_x == features[i].x && to_y == features[i].y)
                  {
                    if (features[i].type == FEATURE_RANGE)
                    {
                      dp_val += 10;
                    }
                    if (features[i].type == FEATURE_AMOUNT)
                    {
                      if (this->me->max_bombs == 1)
                        // if (dp_val < 2000)
                        dp_val += 1000;
                      else
                        dp_val += 1;
                    }
                  }
                }
              }
              if (pr_val == -1)
                pr_val = dir;
              if (dp[tick][to_y][to_x] < dp_val)
                dp[tick][to_y][to_x] = dp_val, pr[tick][to_y][to_x] = pr_val;
            }
          }
        }
      }
    }

    int dir = pr[s_tick][s_y][s_x];
    cerr << "DIR " << dir << endl;
    switch (dir)
    {
    case 0:
      return PLAYER_UP;
    case 1:
      return PLAYER_LEFT;
    case 2:
      return PLAYER_RIGHT;
    case 3:
      return PLAYER_DOWN;
    case 4:
      return PLAYER_STAY;
    }
    cerr << "No move found" << endl;
    return PLAYER_STAY;
  }

  bool check_not_intersect(vector<Bomb> bombs, Field field, int x, int y)
  {
    if (x < 0 || x >= W || y < 0 || y >= H)
      return false;
    if (field.cells[y][x].type == CELL_BLOCK || field.cells[y][x].type == CELL_BOX)
    {
      return false;
    }
    for (Bomb bomb : bombs)
    {
      if (bomb.x == x && bomb.y == y)
        return false;
    }
    return true;
  }

  bool apply_move(vector<Bomb> &bombs, Field field, Player &player, PlayerMove move)
  {
    switch (move)
    {
    case PLAYER_LEFT:
      if (check_not_intersect(bombs, field, player.x - 1, player.y))
      {
        player.x--;
        return true;
      }
      else
      {
        return false;
      }
    case PLAYER_RIGHT:
      if (check_not_intersect(bombs, field, player.x + 1, player.y))
      {
        player.x++;
        return true;
      }
      else
      {
        return false;
      }
    case PLAYER_DOWN:
      if (check_not_intersect(bombs, field, player.x, player.y + 1))
      {
        player.y++;
        return true;
      }
      else
      {
        return false;
      }
    case PLAYER_UP:
      if (check_not_intersect(bombs, field, player.x, player.y - 1))
      {
        player.y--;
        return true;
      }
      else
      {
        return false;
      }
    case PLAYER_STAY:
      return true;
    case PLAYER_BOMB:
      if (player.bombs > 0)
      {
        bombs.pb(Bomb(player.x, player.y, player.owner_id, BOMB_TIMER, player.range));
        player.bombs--;
        return true;
      }
      else
      {
        return false;
      }
    }
    return false;
  }

  tuple<int, int, int, int> get_action_with_move(vector<Bomb> bombs, Field field, Player player, PlayerMove move)
  {
    if (!apply_move(bombs, field, player, move))
    {
      return make_tuple(0, player.x, player.y, -2);
    }
    bool our_bomb_destroyed = false;
    bool bomb_hits_our_position = false;
    next_tick(bombs, field, player, our_bomb_destroyed, bomb_hits_our_position);
    int next_tick_with_bomb = get_bomb_restore_ticks(bombs, field, player);
    if (bomb_hits_our_position)
      return make_tuple(0, player.x, player.y, -2);
    return get_action(bombs, field, player.x, player.y, next_tick_with_bomb);
  }

  // do not call this function inside main(prepare) function!!!
  // simulate one tick. updates state, returns score of tick
  int next_tick(vector<Bomb> &bombs, Field &field, Player &player, bool &our_bomb_destroyed, bool &bomb_hits_our_position)
  {
    // ask Batyr set or array(check bits)
    our_bomb_destroyed = false;
    bomb_hits_our_position = false;
    set<pair<int, int>> destroyed_me;
    set<pair<int, int>> destroyed_others;
    int score = 0;
    vector<Bomb> new_bombs;

    for (Bomb &bomb : bombs)
      bomb.timer--;
    bool recheck = true;
    while (recheck)
    {
      recheck = false;
      for (Bomb &bomb : bombs)
      {
        if (bomb.timer == 0 || destroyed_me.count(mp(bomb.x, bomb.y)) || destroyed_others.count(mp(bomb.x, bomb.y)))
        {
          recheck = true;
          for (int dir = 0; dir < 5; dir++)
          {
            for (int i = 0; i <= bomb.range; i++)
            {
              int nx = bomb.x + dx[dir] * i, ny = bomb.y + dy[dir] * i;
              if (nx < 0 || nx >= W || ny < 0 || ny >= H || field.cells[ny][nx].type == CELL_BLOCK)
                break;
              if (bomb.owner_id == player.owner_id)
              {
                our_bomb_destroyed = true;
                score += field.cells[ny][nx].type == CELL_BOX && destroyed_me.count(mp(nx, ny)) == 0;
                destroyed_me.insert(mp(nx, ny));
              }
              else
              {
                score -= field.cells[ny][nx].type == CELL_BOX && destroyed_others.count(mp(nx, ny)) == 0;
                destroyed_others.insert(mp(nx, ny));
              }
              if (field.cells[ny][nx].type == CELL_BOX)
                break;
            }
          }
          if (bomb.owner_id == player.owner_id)
            player.bombs++;
        }
        else
        {
          new_bombs.pb(bomb);
        }
      }
      bombs.swap(new_bombs);
      new_bombs.clear();
    }

    for (const pair<int, int> &p : destroyed_me)
      field.cells[p.sc][p.fs].type = CELL_EMPTY;
    for (const pair<int, int> &p : destroyed_others)
      field.cells[p.sc][p.fs].type = CELL_EMPTY;
    bomb_hits_our_position = destroyed_me.count(mp(player.x, player.y)) > 0 || destroyed_others.count(mp(player.x, player.y)) > 0;
    return score;
  }

  // do not call this function inside main(prepare) function!!!
  int next_tick(vector<Bomb> &bombs, Field &field, Player &player)
  {
    bool our_bomb_destroyed = false;
    bool bomb_hits_our_position = false;
    return next_tick(bombs, field, player, our_bomb_destroyed, bomb_hits_our_position);
  }

  // number of destroyed boxes with our bombs minus number of destroyed boxes with other bombs
  int get_score(vector<Bomb> bombs, Field field, Player player)
  {
    int score = 0;
    while (!bombs.empty())
    {
      score += next_tick(bombs, field, player);
    }
    return score;
  }

  int get_tick_score(vector<Bomb> bombs, Field field, Player player)
  {
    int score = 0;
    if (!bombs.empty())
    {
      score += next_tick(bombs, field, player);
    }
    return score;
  }

  int get_score_with_bomb(vector<Bomb> bombs, Field field, Player player)
  {
    bombs.pb(Bomb(player.x, player.y, player.owner_id, BOMB_TIMER, this->me->range));
    int score = 0;
    while (!bombs.empty())
    {
      score += next_tick(bombs, field, player);
    }
    return score;
  }

  // first tick when our bomb boom
  int get_bomb_restore_ticks(vector<Bomb> bombs, Field field, Player player)
  {
    if (player.bombs > 0)
      return 0;
    bool we_have_bomb_on_field = false;
    for (Bomb &bomb : bombs)
    {
      if (bomb.owner_id == player.owner_id)
      {
        we_have_bomb_on_field = true;
        break;
      }
    }
    if (!we_have_bomb_on_field)
    {
      assert(0);
      return 10000;
    }

    int tick = 0;
    while (!bombs.empty())
    {
      bool our_bomb_destroyed = false;
      bool bomb_hits_our_position = false;
      next_tick(bombs, field, player, our_bomb_destroyed, bomb_hits_our_position);
      tick++;
      if (our_bomb_destroyed)
      {
        return tick;
      }
    }
    // must be impossible
    assert(0);
    return 10001;
  }

  int ff(int dist, int own_score, int ene_score, bool is_own_safe, bool ene_safe_afk, bool is_ene_safe, bool better_then_afk)
  {
    if (ene_safe_afk && !is_ene_safe)
    {
      if (is_own_safe)
      {
        return 1000000 - dist;
      }
      if (current_global_score + (own_score - ene_score) >= 1)
      {
        return 900000 - dist * 30 + (own_score - ene_score);
      }
    }
    if (!is_own_safe)
    {
      return own_score - ene_score;
    }
    if (better_then_afk)
    {
      return (own_score - ene_score) * 5 - dist + 100000;
    }
    return -1;
  }

  void simulate_tick_akim(int tick, bitset<W> (&accessibleness_own)[H], bitset<W> (&accessibleness_ene)[H], bitset<W> (&current_field)[H], bitset<W * H> (&destroy_boxes)[2], Bomb (&bombs)[MAX_BOMB], int &bombs_count)
  {
    bitset<W> next_accessibleness_own[H];
    bitset<W> next_accessibleness_ene[H];
    bitset<W> destroy_field[H];

    for (int i = 0; i < H; i++)
    {
      next_accessibleness_own[i] = accessibleness_own[i];
      if (i + 1 < H)
        next_accessibleness_own[i] |= accessibleness_own[i + 1];
      if (i - 1 >= 0)
        next_accessibleness_own[i] |= accessibleness_own[i - 1];
      next_accessibleness_own[i] |= (accessibleness_own[i] << 1);
      next_accessibleness_own[i] |= (accessibleness_own[i] >> 1);

      next_accessibleness_ene[i] = accessibleness_ene[i];
      if (i + 1 < H)
        next_accessibleness_ene[i] |= accessibleness_ene[i + 1];
      if (i - 1 >= 0)
        next_accessibleness_ene[i] |= accessibleness_ene[i - 1];
      next_accessibleness_ene[i] |= (accessibleness_ene[i] << 1);
      next_accessibleness_ene[i] |= (accessibleness_ene[i] >> 1);
    }

    for (int i = 0; i < bombs_count; i++)
    {
      if (bombs[i].timer == tick || destroy_field[bombs[i].y][bombs[i].x])
      {
        destroy_field[bombs[i].y][bombs[i].x] = 1;
        for (int dir = 0; dir < 4; dir++)
        {
          for (int j = 1; j <= bombs[i].range; j++)
          {
            int to_x = bombs[i].x + dx[dir] * j, to_y = bombs[i].y + dy[dir] * j;
            if (to_x < 0 || to_x >= field.width ||
                to_y < 0 || to_y >= field.height || field.cells[to_y][to_x].type == CELL_BLOCK)
            {
              break;
            }
            destroy_field[to_y][to_x] = 1;
            if (field.cells[to_y][to_x].type == CELL_BOX && current_field[to_y][to_x] == 1)
            {
              destroy_boxes[bombs[i].owner_id == this->me->owner_id][to_y * W + to_x] = 1;
              break;
            }
          }
        }
        swap(bombs[i], bombs[bombs_count - 1]);
        bombs_count--;
        i = -1;
      }
    }

    for (int i = 0; i < bombs_count; i++)
    {
      int x = bombs[i].x;
      int y = bombs[i].y;
      next_accessibleness_own[y][x] = next_accessibleness_own[y][x] & accessibleness_own[y][x];
      next_accessibleness_ene[y][x] = next_accessibleness_ene[y][x] & accessibleness_ene[y][x];
    }

    for (int i = 0; i < H; i++)
    {
      next_accessibleness_own[i] &= (next_accessibleness_own[i] ^ current_field[i]);
      next_accessibleness_own[i] &= (next_accessibleness_own[i] ^ destroy_field[i]);

      next_accessibleness_ene[i] &= (next_accessibleness_ene[i] ^ current_field[i]);
      next_accessibleness_ene[i] &= (next_accessibleness_ene[i] ^ destroy_field[i]);

      current_field[i] ^= (current_field[i] & destroy_field[i]);
    }

    for (int i = 0; i < H; i++)
    {
      accessibleness_own[i] = next_accessibleness_own[i];
      accessibleness_ene[i] = next_accessibleness_ene[i];
    }
  }

  tuple<bool, bool, int, int> check_safe_and_get_score_akim(int tick, int own_x, int own_y, bitset<W> (&previous_accessibleness_ene)[H], bitset<W> (&previous_field)[H], Bomb (&previous_bombs)[MAX_BOMB], int bombs_count, bool set_bomb)
  {
    Bomb bombs[MAX_BOMB];
    for (int i = 0; i < bombs_count; i++)
      bombs[i] = previous_bombs[i];
    bitset<W> current_field[H];
    for (int i = 0; i < H; i++)
      current_field[i] = previous_field[i];
    bitset<W> accessibleness_own[H];

    bitset<W> accessibleness_ene[H];
    for (int i = 0; i < H; i++)
      accessibleness_ene[i] = previous_accessibleness_ene[i];

    bitset<W * H> destroy_boxes[2];

    accessibleness_own[own_y][own_x] = 1;
    // check really tick + 6 or not
    if (set_bomb)
      bombs[bombs_count++] = Bomb(own_x, own_y, this->me->owner_id, tick + BOMB_TIMER, this->me->range);
    while (bombs_count > 0)
    {
      simulate_tick_akim(tick, accessibleness_own, accessibleness_ene, current_field, destroy_boxes, bombs, bombs_count);
      tick++;
    }
    bool is_own_safe = false, is_ene_safe = false;
    for (int i = 0; i < H; i++)
    {
      is_own_safe |= (accessibleness_own[i].count() > 0);
      is_ene_safe |= (accessibleness_ene[i].count() > 0);
    }
    int own_score = destroy_boxes[1].count();
    int ene_score = destroy_boxes[0].count();
    return make_tuple(is_own_safe, is_ene_safe, own_score, ene_score);
  }

  tuple<int, int, int, int> get_action_akim(vector<Bomb> vec_bombs, Field field, int own_x, int own_y, int ene_x, int ene_y, int next_tick_with_bomb)
  {
    int max_f = -1;
    int go_x = own_x, go_y = own_y, go_tick = 0;

    int bombs_count = vec_bombs.size();
    Bomb bombs[MAX_BOMB];
    for (int i = 0; i < bombs_count; i++)
      bombs[i] = vec_bombs[i];

    bitset<W * H> destroy_boxes[2];
    bitset<W> accessibleness_own[H];
    bitset<W> accessibleness_ene[H];

    accessibleness_own[own_y][own_x] = 1;
    if (ene_x != -1 && ene_y != -1)
      accessibleness_ene[ene_y][ene_x] = 1;

    bitset<W> current_field[H];

    bool is_calced[H][W];
    tuple<bool, bool, int, int> calced_value[H][W];
    for (int i = 0; i < H; i++)
      for (int j = 0; j < W; j++)
        is_calced[i][j] = 0;

    for (int i = 0; i < H; i++)
    {
      for (int j = 0; j < W; j++)
      {
        if (field.cells[i][j].type == CELL_BLOCK || field.cells[i][j].type == CELL_BOX)
        {
          current_field[i][j] = 1;
        }
      }
    }
    // cerr << "STATE" << endl;
    // for (int i = 0; i < H; i++)
    //   cerr << current_field[i] << endl;
    // for (int i = 0; i < bombs_count; i++)
    //   cerr << bombs[i].x << " " << bombs[i].y << endl;
    // cerr << endl;
    // cerr << "START SIMULATING IN GET ACTION" << endl;
    bool own_safe_afk, ene_safe_afk;
    int own_afk_score, ene_afk_score;
    tie(own_safe_afk, ene_safe_afk, own_afk_score, ene_afk_score) = check_safe_and_get_score_akim(1, own_x, own_y, accessibleness_ene, current_field, bombs, bombs_count, false);
    for (int tick = 1; tick < K; tick++)
    {
      bitset<W> prev_accessibleness_own[H];
      bitset<W> prev_accessibleness_ene[H];
      for (int i = 0; i < H; i++)
      {
        prev_accessibleness_own[i] = accessibleness_own[i];
        prev_accessibleness_ene[i] = accessibleness_ene[i];
      }

      simulate_tick_akim(tick, accessibleness_own, accessibleness_ene, current_field, destroy_boxes, bombs, bombs_count);

      int own_score = destroy_boxes[1].count();
      int ene_score = destroy_boxes[0].count();
      // cerr << "START CALCING" << endl;
      if (tick >= next_tick_with_bomb + 1)
      {
        for (int y = 0; y < H; y++)
        {
          for (int x = 0; x < W; x++)
          {
            if (prev_accessibleness_own[y][x] && accessibleness_own[y][x])
            {
              bool is_own_safe, is_ene_safe;
              int own_score_change, ene_score_change;


            if (bombs_count > 0)
            {
              tie(is_own_safe, is_ene_safe, own_score_change, ene_score_change) = check_safe_and_get_score_akim(tick + 1, x, y, accessibleness_ene, current_field, bombs, bombs_count, true);
            }
            else
            {
              if (!is_calced[y][x])
              {
                is_calced[y][x] = 1;
                calced_value[y][x] = check_safe_and_get_score_akim(tick + 1, x, y, accessibleness_ene, current_field, bombs, bombs_count, true);
              }
              tie(is_own_safe, is_ene_safe, own_score_change, ene_score_change) = calced_value[y][x];
            }


              {
                int own_next_score = own_score + own_score_change;
                int ene_next_score = ene_score + ene_score_change;
                int cur_f = ff(tick - 1, own_next_score, ene_next_score, is_own_safe, ene_safe_afk, is_ene_safe, own_next_score - ene_next_score > own_afk_score - ene_afk_score);
                if (max_f < cur_f)
                {
                  max_f = cur_f;
                  go_x = x;
                  go_y = y;
                  go_tick = tick - 1;
                }
              }
            }
          }
        }
      }
    }
    // cerr << "END GET ACTION" << endl;
    //maybe cerr max_f == -1?
    return make_tuple(go_tick, go_x, go_y, max_f);
  }


  tuple<int, int, int, int> get_action_with_move_akim(vector<Bomb> bombs, Field field, Player player, PlayerMove move)
  {
    if (!apply_move(bombs, field, player, move))
    {
      return make_tuple(0, player.x, player.y, -2);
    }
    bool our_bomb_destroyed = false;
    bool bomb_hits_our_position = false;
    next_tick(bombs, field, player, our_bomb_destroyed, bomb_hits_our_position);
    int next_tick_with_bomb = get_bomb_restore_ticks(bombs, field, player);
    if (bomb_hits_our_position)
      return make_tuple(0, player.x, player.y, -2);
    return get_action_akim(bombs, field, player.x, player.y, -1, -1, next_tick_with_bomb);
  }

  void prepare()
  {
    {
      int next_tick_with_bomb = get_bomb_restore_ticks(this->bombs, this->field, *this->me);
      cerr << "next_bomb: " << next_tick_with_bomb << endl;
      int ene_x = -1, ene_y = -1;
      if (this->enemy != nullptr)
        ene_x = this->enemy->x, ene_y = this->enemy->y;
      auto [tick, go_x, go_y, max_f] = get_action_akim(this->bombs, this->field, this->me->x, this->me->y, ene_x, ene_y, next_tick_with_bomb);
      cerr << "GO TO: " << tick << " " << go_x << " " << go_y << " " << max_f << endl;

      if (next_tick_with_bomb == 0 && max_f < 200000)
      {
        int prev_score = get_score(this->bombs, this->field, *this->me);
        int new_score = get_score_with_bomb(bombs, this->field, *this->me);
        cerr << "old and new score: " << prev_score << " " << new_score << endl;
        if (prev_score < new_score)
        {
          auto [ntick, ngo_x, ngo_y, nmax_f] = get_action_with_move_akim(this->bombs, this->field, *this->me, PLAYER_BOMB);
          cerr << "max f after placing bomb: " << nmax_f << endl;
          if (nmax_f > max_f && ngo_x == go_x && ngo_y == go_y && ntick == tick)
          {
            cerr << "BOMB!" << endl;
            this->me->action = PLAYER_BOMB;
            return;
          }
        }
      }

      if (max_f >= 200000)
      {
        cerr << "------------------------------------------------------------------------------------DIE!"
             << " " << max_f << endl;
      }
      if (max_f < 500)
      {
        cerr << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~PANIC!"
             << " " << max_f << endl;
      }
      this->me->action = get_move(tick, go_x, go_y);
      current_global_score += get_tick_score(this->bombs, this->field, *this->me);
      cerr << "SCORE: " << current_global_score << endl;
      return;
    }

    int next_tick_with_bomb = get_bomb_restore_ticks(this->bombs, this->field, *this->me);
    cerr << "next_bomb: " << next_tick_with_bomb << endl;

    auto [tick, go_x, go_y, max_f] = get_action(this->bombs, this->field, this->me->x, this->me->y, next_tick_with_bomb);
    cerr << "GO TO: " << tick << " " << go_x << " " << go_y << " " << max_f << endl;
    if (next_tick_with_bomb == 0)
    {
      int prev_score = get_score(this->bombs, this->field, *this->me);
      int new_score = get_score_with_bomb(bombs, this->field, *this->me);
      cerr << "old and new score: " << prev_score << " " << new_score << endl;
      if (prev_score < new_score)
      {
        auto [ntick, ngo_x, ngo_y, nmax_f] = get_action_with_move(this->bombs, this->field, *this->me, PLAYER_BOMB);
        cerr << "max f after placing bomb: " << nmax_f << endl;
        if (nmax_f > max_f && ngo_x == go_x && ngo_y == go_y && ntick == tick)
        {
          cerr << "BOMB!" << endl;
          this->me->action = PLAYER_BOMB;
          return;
        }
      }
    }
    this->me->action = get_move(tick, go_x, go_y);
  }

  void apply()
  {
    switch (me->action)
    {
    case PLAYER_STAY:
      printf("stay\n");
      break;
    case PLAYER_LEFT:
      printf("left\n");
      break;
    case PLAYER_RIGHT:
      printf("right\n");
      break;
    case PLAYER_DOWN:
      printf("down\n");
      break;
    case PLAYER_UP:
      printf("up\n");
      break;
    case PLAYER_BOMB:
      printf("bomb\n");
      break;
    }
    fflush(stdout);
    me->action = PLAYER_STAY;
  }
};

int main()
{
  Game game;
  while (true)
  {
    double tick_starttime = 1000. * clock() / CLOCKS_PER_SEC;
    game.prepare();
    game.apply();
    double tick_endtime = 1000. * clock() / CLOCKS_PER_SEC;
    game.read_state(); // here we also wait for server response do we need to count it?
    fprintf(stderr, "tick %d time: %.3lf ms\n", game.tick, max_tick_time);
    max_tick_time = max(max_tick_time, tick_endtime - tick_starttime);
  }

  return 0;
}
