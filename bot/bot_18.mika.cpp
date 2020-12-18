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
  PLAYER_BOMB,
  PLAYER_JUMP,
  PLAYER_TELEPORT
};

class Player
{
public:
  Player() {}
  Player(int x, int y, int owner_id, int bombs, int range) : x(x), y(y), owner_id(owner_id), bombs(bombs), range(range)
  {
    this->max_bombs = bombs;
    if (range == 0)
      range = 2;
    this->range = range;
    this->teleport = false;
    this->jump = false;
  }
  Player(const Player &p)
  {
    this->x = p.x;
    this->y = p.y;
    this->owner_id = p.owner_id;
    this->max_bombs = p.max_bombs;
    this->bombs = p.bombs;
    this->range = p.range;
    this->teleport = p.teleport;
    this->jump = p.jump;
    this->action = p.action;
    this->action_x = p.action_x;
    this->action_y = p.action_y;
  }
  Player(const Player *p)
  {
    if (p == nullptr)
    {
      this->dead = true;
      return;
    }
    this->x = p->x;
    this->y = p->y;
    this->owner_id = p->owner_id;
    this->max_bombs = p->max_bombs;
    this->bombs = p->bombs;
    this->range = p->range;
    this->teleport = p->teleport;
    this->jump = p->jump;
    this->action = p->action;
    this->action_x = p->action_x;
    this->action_y = p->action_y;
  }

  int x, y;
  int owner_id;
  int max_bombs, bombs, range;
  bool teleport, jump;
  bool dead = false;

  PlayerMove action = PLAYER_STAY;
  int action_x, action_y;
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

enum FeatureType
{
  FEATURE_RANGE = 'r',
  FEATURE_AMOUNT = 'a',
  FEATURE_TELEPORT = 't',
  FEATURE_JUMP = 'j'
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

class Target
{
public:
  Target() {}

  int x, y;
  int tick;
  int type = -1; // 0 - bomb; 1 - feature; -1 - undefined
};

int fd(int dist)
{
  return 0;
  if (dist < -5)
  {
    return -1;
  }
  if (dist > 20)
  {
    return 100;
  }
  if (dist > 15)
  {
    return 4;
  }
  if (dist > 10)
  {
    return 2;
  }
  if (dist > 5)
  {
    return 1;
  }
  return 0;
}

class StateQuality
{
public:
  StateQuality() {}

  StateQuality(int me_score, int enemy_score) : me_score(me_score), enemy_score(enemy_score) {}

  int dist = 0;
  int me_score = 0;
  int enemy_score = 0;
  int me_will_score = 0;
  int enemy_will_score = 0;
  int me_default_score = 0;
  int enemy_default_score = 0;
  int me_dist_penalty = 0;
  int me_will_dist_penalty = 0;
  int enemy_dist = 0;
  bool me_alive = true;
  bool enemy_alive = true;
  bool me_will_alive = true;
  bool enemy_will_alive = true;

  // int ff()
  // {
  //   if (enemy_alive && !enemy_will_alive)
  //   {
  //     if (me_will_alive)
  //     {
  //       return 1000000 - dist;
  //     }
  //     if (me_score + me_will_score - enemy_score - enemy_will_score >= 1)
  //     {
  //       return 900000 - dist * 30 + (me_score + me_will_score - enemy_score - enemy_will_score);
  //     }
  //   }
  //   if (!me_will_alive)
  //   {
  //     return me_score + me_will_score - enemy_score - enemy_will_score;
  //   }
  //   if (me_score + me_will_score > me_default_score || enemy_score + enemy_will_score < enemy_default_score)
  //     return (me_score + me_will_score - enemy_score - enemy_will_score) * 5 - dist + 100000;
  //   return -1;
  // }

  bool operator<(StateQuality &rhs) // a < b: if a better then b true else false; if b better then a false else true
  {
    // return this->ff() > rhs.ff();

    if (enemy_alive)
    {
      if (!enemy_will_alive && !rhs.enemy_will_alive)
      {
        if (me_will_alive && rhs.me_will_alive)
        {
          return dist < rhs.dist;
        }
        else if (me_will_alive)
        {
          return true;
        }
        else if (rhs.me_will_alive)
        {
          return false;
        }
        // ! do not do suicide because it's hard to compute which bomb will kill you; also unbroken bombs won't add any point
      }
      else if (!enemy_will_alive && me_will_alive)
      {
        return true;
      }
      else if (!rhs.enemy_will_alive && rhs.me_will_alive)
      {
        return false;
      }
    }
    if (!me_will_alive && !rhs.me_will_alive)
    {
      return me_score + me_will_score - enemy_score - enemy_will_score > rhs.me_score + rhs.me_will_score - rhs.enemy_score - rhs.enemy_will_score;
    }
    else if (!me_will_alive)
    {
      return false;
    }
    else if (!rhs.me_will_alive)
    {
      return true;
    }

    int pen = 0;
    if (!(me_score + me_will_score > me_default_score || enemy_score + enemy_will_score < enemy_default_score))
      pen = 10000;
    int rhspen = 0;
    if (!(rhs.me_score + rhs.me_will_score > rhs.me_default_score || rhs.enemy_score + rhs.enemy_will_score < rhs.enemy_default_score))
      rhspen = 10000;

    int cost1 = (me_score + me_will_score - enemy_score - enemy_will_score) * 4 - (dist + me_dist_penalty + me_will_dist_penalty) - pen;
    int cost2 = (rhs.me_score + rhs.me_will_score - rhs.enemy_score - rhs.enemy_will_score) * 4 - (rhs.dist + rhs.me_dist_penalty + rhs.me_will_dist_penalty) - rhspen;
    if (cost1 == cost2)
    {
      if ((me_score + me_will_score - enemy_score - enemy_will_score) == (rhs.me_score + rhs.me_will_score - rhs.enemy_score - rhs.enemy_will_score))
      {
        if (me_will_score - enemy_will_score != rhs.me_will_score - rhs.enemy_will_score)
          return me_will_score - enemy_will_score > rhs.me_will_score - rhs.enemy_will_score;
      }
      if (dist + me_dist_penalty + me_will_dist_penalty != rhs.dist + rhs.me_dist_penalty + rhs.me_will_dist_penalty)
        return dist + me_dist_penalty + me_will_dist_penalty < rhs.dist + rhs.me_dist_penalty + rhs.me_will_dist_penalty;
      else
        return dist < rhs.dist;
    }
    return cost1 > cost2;
  }
};

class TupleComp
{
public:
  bool operator()(const tuple<vector<Bomb>, vector<bitset<W>>, Player, Player, vector<bitset<W>>, vector<Feature>, StateQuality, Target> a, const tuple<vector<Bomb>, vector<bitset<W>>, Player, Player, vector<bitset<W>>, vector<Feature>, StateQuality, Target> b)
  {
    StateQuality a_state = get<6>(a);
    StateQuality b_state = get<6>(b);
    return a_state < b_state;
  }
};

class Game
{
private:
  Player *me = nullptr;
  Player *enemy = nullptr;
  Field field;
  vector<Player> players;
  vector<Bomb> bombs;
  vector<Feature> features;
  vector<bitset<W>> bit_field = vector<bitset<W>>(H, bitset<W>());
  bitset<W> bit_full, bit_empty;
  int me_global_score = 0;
  int enemy_global_score = 0;

  void _log(string s)
  {
    s.append("\n");
    fprintf(stderr, s.c_str());
  }

public:
  int tick;
  Game()
  {
    read_state(true);
    for (int i = 0; i < W; i++)
    {
      bit_full[i] = 1;
    }
    for (int i = 0; i < W; i++)
    {
      bit_empty[i] = 0;
    }
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
      if (type == 'f')
      {
        features.pb(Feature(x, y, feature_type));
      }
    }
    int features;
    scanf("%d", &features);
    for (int i = 0; i < features; i++)
    {
      int owner_id, fid;
      scanf("%d%d", &owner_id, &fid);
      for (Player &player : players)
      {
        if (owner_id == player.owner_id)
        {
          if (fid == 0)
          {
            player.jump = true;
          }
          else
          {
            player.teleport = true;
          }
        }
      }
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

    for (int i = 0; i < H; i++)
    {
      for (int j = 0; j < W; j++)
      {
        if (this->field.cells[i][j].type == CELL_BLOCK || this->field.cells[i][j].type == CELL_BOX)
        {
          this->bit_field[i][j] = 1;
        }
        else
        {
          this->bit_field[i][j] = 0;
        }
      }
    }

    this->tick = tick;
  }

  tuple<int, int, int> simulate_moves_and_bombs(Player &me, Player &enemy, vector<bitset<W>> &me_possible_pos, vector<bitset<W>> &enemy_possible_pos, vector<bitset<W>> &bit_field, vector<Bomb> &bombs, vector<Feature> &features, PlayerMove action = PLAYER_STAY, bool can_change_action = true)
  {
    int me_dist_penalty = 0;

    vector<bitset<W>> me_possible_pos_next = me_possible_pos;
    if (action == PLAYER_BOMB)
    {
      bombs.pb(Bomb(me.x, me.y, me.owner_id, BOMB_TIMER, me.range));
      me.bombs--;
      for (int i = 0; i < H; i++)
        me_possible_pos_next[i] = bit_empty;
      me_possible_pos_next[me.y][me.x] = 1;
    }
    else if (action == PLAYER_JUMP)
    {
      me_dist_penalty = 6;
      me.jump = false;
    }
    else if (action == PLAYER_TELEPORT)
    {
      me.teleport = false;
      for (int i = 0; i < H; i++)
        me_possible_pos_next[i] = bit_full;
      me_dist_penalty = 8;
      me.teleport = false;
    }
    else if (action == PLAYER_STAY)
    {
      for (int i = 0; i < H; i++)
      {
        if (i + 1 < H)
          me_possible_pos_next[i] |= me_possible_pos[i + 1];
        if (i - 1 >= 0)
          me_possible_pos_next[i] |= me_possible_pos[i - 1];
        me_possible_pos_next[i] |= (me_possible_pos[i] << 1);
        me_possible_pos_next[i] |= (me_possible_pos[i] >> 1);
      }
    }
    vector<bitset<W>> enemy_possible_pos_next = enemy_possible_pos;
    for (int i = 0; i < H; i++)
    {
      if (i + 1 < H)
        enemy_possible_pos_next[i] |= enemy_possible_pos[i + 1];
      if (i - 1 >= 0)
        enemy_possible_pos_next[i] |= enemy_possible_pos[i - 1];
      enemy_possible_pos_next[i] |= (enemy_possible_pos[i] << 1);
      enemy_possible_pos_next[i] |= (enemy_possible_pos[i] >> 1);
    }
    for (int i = 0; i < bombs.size(); i++)
    {
      int x = bombs[i].x;
      int y = bombs[i].y;
      me_possible_pos_next[y][x] = me_possible_pos_next[y][x] & me_possible_pos[y][x];
      enemy_possible_pos_next[y][x] = enemy_possible_pos_next[y][x] & enemy_possible_pos[y][x];
    }
    for (int i = 0; i < H; i++)
    {
      me_possible_pos_next[i] &= (me_possible_pos_next[i] ^ bit_field[i]);
      enemy_possible_pos_next[i] &= (enemy_possible_pos_next[i] ^ bit_field[i]);
    }

    for (Bomb &bomb : bombs)
      bomb.timer--;

    vector<bitset<W>> destroy_field = vector<bitset<W>>(H, bitset<W>());
    bitset<W * H> destroy_boxes[2];

    for (int i = 0; i < bombs.size(); i++)
    {
      if (bombs[i].timer == 0 || destroy_field[bombs[i].y][bombs[i].x])
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
            if (field.cells[to_y][to_x].type == CELL_BOX && bit_field[to_y][to_x] == 1)
            {
              destroy_boxes[bombs[i].owner_id == me.owner_id][to_y * W + to_x] = 1;
              break;
            }
          }
        }

        if (bombs[i].owner_id == me.owner_id)
          me.bombs++;
        if (bombs[i].owner_id == enemy.owner_id)
          enemy.bombs++;

        swap(bombs[i], bombs.back());
        bombs.ppb();
        i = -1;
      }
    }

    for (int i = 0; i < features.size(); i++)
    {
      if (destroy_field[features[i].y][features[i].x])
      {
        swap(features[i], features.back());
        features.ppb();
        i--;
      }
    }

    bool me_alive = false;
    for (int i = 0; i < H; i++)
    {
      me_possible_pos_next[i] &= (me_possible_pos_next[i] ^ destroy_field[i]);
      me_alive |= me_possible_pos_next[i].count();
      enemy_possible_pos_next[i] &= (enemy_possible_pos_next[i] ^ destroy_field[i]);
      bit_field[i] ^= (bit_field[i] & destroy_field[i]);
    }

    if (can_change_action && action == PLAYER_STAY && !me_alive && me.jump)
    {
      me_dist_penalty = 6;
      me_possible_pos_next = me_possible_pos;
      me.jump = false;
    }
    else if (can_change_action && action == PLAYER_STAY && !me_alive && me.teleport)
    {
      me_dist_penalty = 10;
      for (int i = 0; i < H; i++)
        me_possible_pos[i] = bit_full ^ destroy_field[i];
      me.teleport = false;
    }
    else if (action == PLAYER_JUMP)
    {
    }
    else
    {
      me_possible_pos = me_possible_pos_next;
    }
    enemy_possible_pos = enemy_possible_pos_next;

    return make_tuple(destroy_boxes[1].count(), destroy_boxes[0].count(), me_dist_penalty);
  }

  tuple<bool, bool, int, int, int> check_safe_and_get_score(Player me, Player enemy, vector<bitset<W>> enemy_possible_pos, vector<bitset<W>> bit_field, vector<Bomb> bombs, vector<Feature> features)
  {
    vector<bitset<W>> me_possible_pos = vector<bitset<W>>(H, bitset<W>());
    me_possible_pos[me.y][me.x] = 1;

    int me_score = 0, enemy_score = 0;
    bool first = true;
    bool me_alive = false;
    bool enemy_alive = false;
    int me_dist_penalty = 0;
    bool can_escape = false;
    while (!bombs.empty())
    {
      int me_score_change = 0, enemy_score_change = 0, me_dist_penalty_add;
      tie(me_score_change, enemy_score_change, me_dist_penalty_add) = simulate_moves_and_bombs(me, enemy, me_possible_pos, enemy_possible_pos, bit_field, bombs, features);
      me_score += me_score_change;
      enemy_score += enemy_score_change;
      me_dist_penalty += me_dist_penalty_add;
    }

    for (int i = 0; i < H; i++)
      me_alive |= (me_possible_pos[i].count() > 0);

    for (int i = 0; i < H; i++)
      enemy_alive |= (enemy_possible_pos[i].count() > 0);
    enemy_alive |= enemy.teleport || enemy.jump;
    return make_tuple(me_alive, enemy_alive, me_score, enemy_score, me_dist_penalty);
  }

  vector<tuple<vector<Bomb>, vector<bitset<W>>, Player, Player, vector<bitset<W>>, vector<Feature>, StateQuality, Target>> get_action(vector<tuple<vector<Bomb>, vector<bitset<W>>, Player, Player, vector<bitset<W>>, vector<Feature>, StateQuality, Target>> states, int top_bombs, int top_features)
  {
    priority_queue<tuple<vector<Bomb>, vector<bitset<W>>, Player, Player, vector<bitset<W>>, vector<Feature>, StateQuality, Target>,
                   vector<tuple<vector<Bomb>, vector<bitset<W>>, Player, Player, vector<bitset<W>>, vector<Feature>, StateQuality, Target>>,
                   TupleComp>
        result_bombs, result_features;
    for (int state_id = 0; state_id < states.size(); state_id++)
    {
      bool is_calced[H][W];
      tuple<bool, bool, int, int, int> calced_value[H][W];
      for (int i = 0; i < H; i++)
        for (int j = 0; j < W; j++)
          is_calced[i][j] = 0;

      bool is_calced_f[H][W];
      tuple<bool, bool, int, int, int> calced_value_f[H][W];
      for (int i = 0; i < H; i++)
        for (int j = 0; j < W; j++)
          is_calced_f[i][j] = 0;
      // cerr << "starting bombs" << endl;
      { // * just place bomb at x y
        vector<Bomb> bombs;
        vector<bitset<W>> bit_field;
        Player me;
        Player enemy;
        vector<bitset<W>> enemy_possible_pos;
        vector<Feature> features;
        StateQuality state_quality;
        Target target;
        tie(bombs, bit_field, me, enemy, enemy_possible_pos, features, state_quality, target) = states[state_id];

        if (target.type != -1)
        {
          result_bombs.push(states[state_id]);
          if (result_bombs.size() > top_bombs)
            result_bombs.pop();
        }

        if (!state_quality.me_alive)
          continue;

        int me_score = state_quality.me_score, enemy_score = state_quality.enemy_score;

        vector<bitset<W>> me_possible_pos = vector<bitset<W>>(H, bitset<W>());
        me_possible_pos[me.y][me.x] = 1;
        int me_dist_penalty = 0;
        bool me_alive, enemy_alive;
        int me_default_score, enemy_default_score;
        int me_add_dist_penalty;
        tie(me_alive, enemy_alive, me_default_score, enemy_default_score, me_add_dist_penalty) = check_safe_and_get_score(me, enemy, enemy_possible_pos, bit_field, bombs, features);
        me_default_score += state_quality.me_score, enemy_default_score += state_quality.enemy_score;
        for (int tick = 0; tick < K; tick++)
        {
          if (me.bombs > 0)
            for (int y = 0; y < H; y++)
            {
              for (int x = 0; x < W; x++)
              {
                if (!me_possible_pos[y][x])
                  continue;
                Player me_bomb = me;
                Player enemy_bomb = enemy;
                vector<bitset<W>> bit_field_bomb = bit_field;
                vector<bitset<W>> me_possible_pos_bomb = me_possible_pos;
                vector<bitset<W>> enemy_possible_pos_bomb = enemy_possible_pos;
                vector<Bomb> bombs_bomb = bombs;
                vector<Feature> features_bomb = features;

                me_bomb.x = x;
                me_bomb.y = y;

                bool me_cur_alive = false, enemy_cur_alive = false;
                int me_cur_dist_penalty;
                int me_cur_score, enemy_cur_score;
                tie(me_cur_score, enemy_cur_score, me_cur_dist_penalty) = simulate_moves_and_bombs(me_bomb, enemy_bomb, me_possible_pos_bomb, enemy_possible_pos_bomb, bit_field_bomb, bombs_bomb, features_bomb, PLAYER_BOMB);
                for (int i = 0; i < H; i++)
                {
                  me_cur_alive |= me_possible_pos_bomb[i].count();
                  enemy_cur_alive |= enemy_possible_pos_bomb[i].count();
                }

                bool me_will_alive, enemy_will_alive;
                int me_will_dist_penalty;
                int me_will_score, enemy_will_score;
                if (bombs_bomb.size() > 1)
                {
                  tie(me_will_alive, enemy_will_alive, me_will_score, enemy_will_score, me_will_dist_penalty) = check_safe_and_get_score(me_bomb, enemy_bomb, enemy_possible_pos_bomb, bit_field_bomb, bombs_bomb, features_bomb);
                }
                else
                {
                  if (!is_calced[y][x])
                  {
                    is_calced[y][x] = 1;
                    calced_value[y][x] = check_safe_and_get_score(me_bomb, enemy_bomb, enemy_possible_pos_bomb, bit_field_bomb, bombs_bomb, features_bomb);
                  }
                  tie(me_will_alive, enemy_will_alive, me_will_score, enemy_will_score, me_will_dist_penalty) = calced_value[y][x];
                }

                {
                  StateQuality cur_state_quality;
                  cur_state_quality.dist = state_quality.dist + tick + 1;
                  cur_state_quality.me_score = me_score + me_cur_score;
                  cur_state_quality.enemy_score = enemy_score + enemy_cur_score;
                  cur_state_quality.me_will_score = me_will_score;
                  cur_state_quality.enemy_will_score = enemy_will_score;
                  cur_state_quality.me_default_score = me_default_score;
                  cur_state_quality.enemy_default_score = enemy_default_score;
                  cur_state_quality.me_dist_penalty = state_quality.me_dist_penalty + me_dist_penalty + me_cur_dist_penalty;
                  cur_state_quality.me_will_dist_penalty = me_will_dist_penalty;
                  cur_state_quality.enemy_dist = 100; // TODO
                  cur_state_quality.me_alive = me_alive && me_cur_alive;
                  cur_state_quality.enemy_alive = enemy_alive && enemy_cur_alive;
                  cur_state_quality.me_will_alive = me_will_alive && me_cur_alive;
                  cur_state_quality.enemy_will_alive = enemy_will_alive && enemy_cur_alive;

                  Target cur_target = target;
                  if (target.type == -1)
                  {
                    cur_target.x = x;
                    cur_target.y = y;
                    cur_target.tick = tick;
                    cur_target.type = 0;
                  }

                  bool ok = result_bombs.size() < top_bombs;
                  if (!ok)
                  {
                    StateQuality sq;
                    tie(ignore, ignore, ignore, ignore, ignore, ignore, sq, ignore) = result_bombs.top();
                    ok = cur_state_quality < sq;
                  }
                  if (ok)
                  {
                    result_bombs.push(
                        make_tuple(
                            bombs_bomb, bit_field_bomb, me_bomb, enemy_bomb, enemy_possible_pos_bomb, features_bomb, cur_state_quality, cur_target));
                    if (result_bombs.size() > top_bombs)
                      result_bombs.pop();
                  }
                }
              }
            }

          int me_dist_penalty_add;
          int me_score_change, enemy_score_change;
          tie(me_score_change, enemy_score_change, me_dist_penalty_add) = simulate_moves_and_bombs(me, enemy, me_possible_pos, enemy_possible_pos, bit_field, bombs, features, PLAYER_STAY, tick > 0);
          me_score += me_score_change;
          enemy_score += enemy_score_change;
          me_dist_penalty += me_dist_penalty_add;
        }
      }
      //////////////////////////////////////////////////////////////////////////
      // cerr << "starting tp" << endl;
      { // * tp and place bomb at x y
        vector<Bomb> bombs;
        vector<bitset<W>> bit_field;
        Player me;
        Player enemy;
        vector<bitset<W>> enemy_possible_pos;
        vector<Feature> features;
        StateQuality state_quality;
        Target target;
        tie(bombs, bit_field, me, enemy, enemy_possible_pos, features, state_quality, target) = states[state_id];

        int me_score = state_quality.me_score, enemy_score = state_quality.enemy_score;

        vector<bitset<W>> me_possible_pos = vector<bitset<W>>(H, bitset<W>());
        me_possible_pos[me.y][me.x] = 1;
        int me_dist_penalty = 0;
        bool me_alive, enemy_alive;
        int me_default_score, enemy_default_score;
        int me_add_dist_penalty;
        tie(me_alive, enemy_alive, me_default_score, enemy_default_score, me_add_dist_penalty) = check_safe_and_get_score(me, enemy, enemy_possible_pos, bit_field, bombs, features);
        me_default_score += state_quality.me_score, enemy_default_score += state_quality.enemy_score;
        if (me.teleport)
          for (int tick = 0; tick < K; tick++)
          {
            if (me.bombs > 0 && tick > 0)
              for (int y = 0; y < H; y++)
              {
                for (int x = 0; x < W; x++)
                {
                  if (!me_possible_pos[y][x])
                    continue;
                  Player me_bomb = me;
                  Player enemy_bomb = enemy;
                  vector<bitset<W>> bit_field_bomb = bit_field;
                  vector<bitset<W>> me_possible_pos_bomb = me_possible_pos;
                  vector<bitset<W>> enemy_possible_pos_bomb = enemy_possible_pos;
                  vector<Bomb> bombs_bomb = bombs;
                  vector<Feature> features_bomb = features;

                  me_bomb.x = x;
                  me_bomb.y = y;

                  bool me_cur_alive = false, enemy_cur_alive = false;
                  int me_cur_dist_penalty;
                  int me_cur_score, enemy_cur_score;
                  tie(me_cur_score, enemy_cur_score, me_cur_dist_penalty) = simulate_moves_and_bombs(me_bomb, enemy_bomb, me_possible_pos_bomb, enemy_possible_pos_bomb, bit_field_bomb, bombs_bomb, features_bomb, PLAYER_BOMB);
                  for (int i = 0; i < H; i++)
                  {
                    me_cur_alive |= me_possible_pos_bomb[i].count();
                    enemy_cur_alive |= enemy_possible_pos_bomb[i].count();
                  }

                  bool me_will_alive, enemy_will_alive;
                  int me_will_dist_penalty;
                  int me_will_score, enemy_will_score;
                  if (bombs_bomb.size() > 1)
                  {
                    tie(me_will_alive, enemy_will_alive, me_will_score, enemy_will_score, me_will_dist_penalty) = check_safe_and_get_score(me_bomb, enemy_bomb, enemy_possible_pos_bomb, bit_field_bomb, bombs_bomb, features_bomb);
                  }
                  else
                  {
                    if (!is_calced[y][x])
                    {
                      is_calced[y][x] = 1;
                      calced_value[y][x] = check_safe_and_get_score(me_bomb, enemy_bomb, enemy_possible_pos_bomb, bit_field_bomb, bombs_bomb, features_bomb);
                    }
                    tie(me_will_alive, enemy_will_alive, me_will_score, enemy_will_score, me_will_dist_penalty) = calced_value[y][x];
                  }

                  {
                    StateQuality cur_state_quality;
                    cur_state_quality.dist = state_quality.dist + tick + 1;
                    cur_state_quality.me_score = me_score + me_cur_score;
                    cur_state_quality.enemy_score = enemy_score + enemy_cur_score;
                    cur_state_quality.me_will_score = me_will_score;
                    cur_state_quality.enemy_will_score = enemy_will_score;
                    cur_state_quality.me_default_score = me_default_score;
                    cur_state_quality.enemy_default_score = enemy_default_score;
                    cur_state_quality.me_dist_penalty = state_quality.me_dist_penalty + me_dist_penalty + me_cur_dist_penalty;
                    cur_state_quality.me_will_dist_penalty = me_will_dist_penalty;
                    cur_state_quality.enemy_dist = 100; // TODO
                    cur_state_quality.me_alive = me_alive && me_cur_alive;
                    cur_state_quality.enemy_alive = enemy_alive && enemy_cur_alive;
                    cur_state_quality.me_will_alive = me_will_alive && me_cur_alive;
                    cur_state_quality.enemy_will_alive = enemy_will_alive && enemy_cur_alive;

                    Target cur_target = target;
                    if (target.type == -1)
                    {
                      cur_target.x = x;
                      cur_target.y = y;
                      cur_target.tick = tick;
                      cur_target.type = 1; // TODO: find tp target
                    }

                    bool ok = result_bombs.size() < top_bombs;
                    if (!ok)
                    {
                      StateQuality sq;
                      tie(ignore, ignore, ignore, ignore, ignore, ignore, sq, ignore) = result_bombs.top();
                      ok = cur_state_quality < sq;
                    }
                    if (ok)
                    {
                      result_bombs.push(
                          make_tuple(
                              bombs_bomb, bit_field_bomb, me_bomb, enemy_bomb, enemy_possible_pos_bomb, features_bomb, cur_state_quality, cur_target));
                      if (result_bombs.size() > top_bombs)
                        result_bombs.pop();
                    }
                  }
                }
              }

            int me_dist_penalty_add;
            int me_score_change, enemy_score_change;
            tie(me_score_change, enemy_score_change, me_dist_penalty_add) = simulate_moves_and_bombs(me, enemy, me_possible_pos, enemy_possible_pos, bit_field, bombs, features, tick == 0 ? PLAYER_TELEPORT : PLAYER_STAY, tick > 0);
            me_score += me_score_change;
            enemy_score += enemy_score_change;
            me_dist_penalty += me_dist_penalty_add;
          }
      }
      //////////////////////////////////////////////////////////////////////////
      // cerr << "starting jump" << endl;
      { // * jump
        vector<Bomb> bombs;
        vector<bitset<W>> bit_field;
        Player me;
        Player enemy;
        vector<bitset<W>> enemy_possible_pos;
        vector<Feature> features;
        StateQuality state_quality;
        Target target;
        tie(bombs, bit_field, me, enemy, enemy_possible_pos, features, state_quality, target) = states[state_id];

        int me_score = state_quality.me_score, enemy_score = state_quality.enemy_score;

        vector<bitset<W>> me_possible_pos = vector<bitset<W>>(H, bitset<W>());
        me_possible_pos[me.y][me.x] = 1;
        int me_dist_penalty = 0;
        bool me_alive, enemy_alive;
        int me_default_score, enemy_default_score;
        int me_add_dist_penalty;
        tie(me_alive, enemy_alive, me_default_score, enemy_default_score, me_add_dist_penalty) = check_safe_and_get_score(me, enemy, enemy_possible_pos, bit_field, bombs, features);
        me_default_score += state_quality.me_score, enemy_default_score += state_quality.enemy_score;
        if (me.jump)
        {

          bool me_cur_alive = false, enemy_cur_alive = false;
          int me_cur_dist_penalty;
          int me_cur_score, enemy_cur_score;
          tie(me_cur_score, enemy_cur_score, me_cur_dist_penalty) = simulate_moves_and_bombs(me, enemy, me_possible_pos, enemy_possible_pos, bit_field, bombs, features, PLAYER_JUMP, false);
          for (int i = 0; i < H; i++)
          {
            me_cur_alive |= me_possible_pos[i].count();
            enemy_cur_alive |= enemy_possible_pos[i].count();
          }

          bool me_will_alive, enemy_will_alive;
          int me_will_dist_penalty;
          int me_will_score, enemy_will_score;
          tie(me_will_alive, enemy_will_alive, me_will_score, enemy_will_score, me_will_dist_penalty) = check_safe_and_get_score(me, enemy, enemy_possible_pos, bit_field, bombs, features);

          {
            StateQuality cur_state_quality;
            cur_state_quality.dist = state_quality.dist + tick + 1;
            cur_state_quality.me_score = me_score + me_cur_score;
            cur_state_quality.enemy_score = enemy_score + enemy_cur_score;
            cur_state_quality.me_will_score = me_will_score;
            cur_state_quality.enemy_will_score = enemy_will_score;
            cur_state_quality.me_default_score = me_default_score;
            cur_state_quality.enemy_default_score = enemy_default_score;
            cur_state_quality.me_dist_penalty = state_quality.me_dist_penalty + me_dist_penalty + me_cur_dist_penalty;
            cur_state_quality.me_will_dist_penalty = me_will_dist_penalty;
            cur_state_quality.enemy_dist = 100; // TODO
            cur_state_quality.me_alive = me_alive && me_cur_alive;
            cur_state_quality.enemy_alive = enemy_alive && enemy_cur_alive;
            cur_state_quality.me_will_alive = me_will_alive && me_cur_alive;
            cur_state_quality.enemy_will_alive = enemy_will_alive && enemy_cur_alive;

            Target cur_target = target;
            if (target.type == -1)
            {
              cur_target.x = me.x;
              cur_target.y = me.y;
              cur_target.tick = tick;
              cur_target.type = 3;
            }

            bool ok = result_bombs.size() < top_bombs;
            if (!ok)
            {
              StateQuality sq;
              tie(ignore, ignore, ignore, ignore, ignore, ignore, sq, ignore) = result_bombs.top();
              ok = cur_state_quality < sq;
            }
            if (ok)
            {
              result_bombs.push(
                  make_tuple(
                      bombs, bit_field, me, enemy, enemy_possible_pos, features, cur_state_quality, cur_target));
              if (result_bombs.size() > top_bombs)
                result_bombs.pop();
            }
          }
        }
      }
      //////////////////////////////////////////////////////////////////////////
      // cerr << "starting features" << endl;
      if (top_features > 0)
      { // * get features
        vector<Bomb> bombs;
        vector<bitset<W>> bit_field;
        Player me;
        Player enemy;
        vector<bitset<W>> enemy_possible_pos;
        vector<Feature> features;
        StateQuality state_quality;
        Target target;
        tie(bombs, bit_field, me, enemy, enemy_possible_pos, features, state_quality, target) = states[state_id];

        int me_score = state_quality.me_score, enemy_score = state_quality.enemy_score;

        vector<bitset<W>> me_possible_pos = vector<bitset<W>>(H, bitset<W>());
        me_possible_pos[me.y][me.x] = 1;
        int me_dist_penalty = 0;
        bool me_alive, enemy_alive;
        int me_default_score, enemy_default_score;
        int me_add_dist_penalty;
        tie(me_alive, enemy_alive, me_default_score, enemy_default_score, me_add_dist_penalty) = check_safe_and_get_score(me, enemy, enemy_possible_pos, bit_field, bombs, features);
        me_default_score += state_quality.me_score, enemy_default_score += state_quality.enemy_score;
        for (int tick = 0; tick < K; tick++)
        {
          for (int id = 0; id < features.size(); id++)
          {
            Feature &feature = features[id];
            int x = feature.x;
            int y = feature.y;
            if (!me_possible_pos[y][x])
              continue;

            Player me_bomb = me;
            Player enemy_bomb = enemy;
            vector<bitset<W>> bit_field_bomb = bit_field;
            vector<bitset<W>> me_possible_pos_bomb = me_possible_pos;
            vector<bitset<W>> enemy_possible_pos_bomb = enemy_possible_pos;
            vector<Bomb> bombs_bomb = bombs;
            vector<Feature> features_bomb = features;

            me_bomb.x = x;
            me_bomb.y = y;

            int me_took_dist_penalty = 0;
            if (feature.type == FEATURE_AMOUNT)
            {
              if (me_bomb.max_bombs == 1)
              {
                me_took_dist_penalty = -3;
              }
              else if (me_bomb.max_bombs == 2)
              {
                me_took_dist_penalty = -1;
              }
              else
              {
                me_took_dist_penalty = 0;
              }
              me_bomb.bombs++;
              me_bomb.max_bombs++;
            }
            else if (feature.type == FEATURE_RANGE)
            {
              if (me_bomb.range < 5)
              {
                me_took_dist_penalty = -4;
              }
              else
              {
                me_took_dist_penalty = -1;
              }
              me_bomb.range++;
            }
            else if (feature.type == FEATURE_JUMP)
            {
              if (me_bomb.jump)
              {
                me_took_dist_penalty = 0;
              }
              else
              {
                me_took_dist_penalty = -5;
              }
              me_bomb.jump = true;
            }
            else if (feature.type == FEATURE_TELEPORT)
            {
              if (me_bomb.teleport)
              {
                me_took_dist_penalty = -1;
              }
              else
              {
                me_took_dist_penalty = -10;
              }
              me_bomb.teleport = true;
            }
            swap(features_bomb[id], features_bomb.back());
            features_bomb.ppb();

            bool me_will_alive, enemy_will_alive;
            int me_will_dist_penalty;
            int me_will_score, enemy_will_score;
            bool took_calced = false;
            if (bombs_bomb.size() > 0)
            {
              tie(me_will_alive, enemy_will_alive, me_will_score, enemy_will_score, me_will_dist_penalty) = check_safe_and_get_score(me_bomb, enemy_bomb, enemy_possible_pos_bomb, bit_field_bomb, bombs_bomb, features_bomb);
            }
            else
            {
              if (!is_calced_f[y][x])
              {
                is_calced_f[y][x] = 1;
                calced_value_f[y][x] = check_safe_and_get_score(me_bomb, enemy_bomb, enemy_possible_pos_bomb, bit_field_bomb, bombs_bomb, features_bomb);
              }
              else
              {
                took_calced = true;
              }
              tie(me_will_alive, enemy_will_alive, me_will_score, enemy_will_score, me_will_dist_penalty) = calced_value[y][x];
            }

            if (!took_calced)
            {
              StateQuality cur_state_quality;
              cur_state_quality.dist = state_quality.dist + tick;
              cur_state_quality.me_score = me_score;
              cur_state_quality.enemy_score = enemy_score;
              cur_state_quality.me_will_score = me_will_score;
              cur_state_quality.enemy_will_score = enemy_will_score;
              cur_state_quality.me_default_score = me_default_score;
              cur_state_quality.enemy_default_score = enemy_default_score;
              cur_state_quality.me_dist_penalty = state_quality.me_dist_penalty + me_dist_penalty + me_took_dist_penalty;
              cur_state_quality.me_will_dist_penalty = me_will_dist_penalty;
              cur_state_quality.enemy_dist = 100; // TODO
              cur_state_quality.me_alive = me_alive;
              cur_state_quality.enemy_alive = enemy_alive;
              cur_state_quality.me_will_alive = me_will_alive;
              cur_state_quality.enemy_will_alive = enemy_will_alive;

              Target cur_target = target;
              if (target.type == -1)
              {
                cur_target.x = x;
                cur_target.y = y;
                cur_target.tick = tick;
                cur_target.type = 2; // TODO: find tp target
              }
              bool ok = result_features.size() < top_features;
              if (!ok)
              {
                StateQuality sq;
                tie(ignore, ignore, ignore, ignore, ignore, ignore, sq, ignore) = result_features.top();
                ok = cur_state_quality < sq;
              }
              if (ok)
              {
                result_features.push(
                    make_tuple(
                        bombs_bomb, bit_field_bomb, me_bomb, enemy_bomb, enemy_possible_pos_bomb, features_bomb, cur_state_quality, cur_target));
                if (result_features.size() > top_bombs)
                  result_features.pop();
              }
            }
          }

          int me_dist_penalty_add;
          int me_score_change, enemy_score_change;
          tie(me_score_change, enemy_score_change, me_dist_penalty_add) = simulate_moves_and_bombs(me, enemy, me_possible_pos, enemy_possible_pos, bit_field, bombs, features, PLAYER_STAY, tick > 0);
          me_score += me_score_change;
          enemy_score += enemy_score_change;
          me_dist_penalty += me_dist_penalty_add;
        }
      }
    }
    vector<tuple<vector<Bomb>, vector<bitset<W>>, Player, Player, vector<bitset<W>>, vector<Feature>, StateQuality, Target>> result;
    while (!result_bombs.empty())
    {
      result.pb(result_bombs.top());
      result_bombs.pop();
    }
    while (!result_features.empty())
    {
      result.pb(result_features.top());
      result_features.pop();
    }
    cerr << "finished calc " << result.size() << endl;
    sort(result.begin(), result.end(), TupleComp());
    for (int i = 0; i < result.size(); i++)
    {
      Target target;
      StateQuality sq;
      tie(ignore, ignore, ignore, ignore, ignore, ignore, sq, target) = result[i];
      cerr << target.x << " " << target.y << " " << target.tick << " " << sq.me_alive << " " << sq.me_dist_penalty << " " << target.type << endl;
    }
    return result;
  }

  tuple<int, int, int> get_tick_score(Player me, Player enemy, vector<bitset<W>> bit_field, vector<Bomb> bombs, vector<Feature> features)
  {
    vector<bitset<W>> me_possible_pos = vector<bitset<W>>(H, bitset<W>());
    vector<bitset<W>> enemy_possible_pos = vector<bitset<W>>(H, bitset<W>());
    return simulate_moves_and_bombs(me, enemy, me_possible_pos, enemy_possible_pos, bit_field, bombs, features);
  }

  PlayerMove get_move(Target target)
  {
    int s_x = target.x, s_y = target.y, s_tick = target.tick;
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

      bool jump = false;
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

        bool is_alive = false;
        for (int i = 0; i < H; i++)
          is_alive |= next_accessibleness[i].count();

        if (!is_alive)
        {
          jump = true;
        }
        else
          for (int i = 0; i < H; i++)
            accessibleness[i] = next_accessibleness[i];
      }
      for (int y = 0; y < H; y++)
      {
        for (int x = 0; x < W; x++)
        {
          if (prev_accessibleness[y][x])
          {
            for (int dir = jump ? 4 : 0; dir < 5; dir++)
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
                // TODO: make it less dummy
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
                      // if (this->me->max_bombs == 1)
                      if (dp_val < 2000)
                        dp_val += 1000;
                      else
                        dp_val += 1;
                    }
                    if (features[i].type == FEATURE_JUMP && !this->me->jump)
                    {
                      dp_val += 10;
                    }
                    if (features[i].type == FEATURE_TELEPORT && !this->me->teleport)
                    {
                      dp_val += 100;
                    }
                  }
                }
              }
              if (pr_val == -1)
                pr_val = jump ? 5 : dir;
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
    case 5:
      return PLAYER_JUMP;
    }
    cerr << "No move found" << endl;
    return PLAYER_STAY;
  }

  void prepare()
  {
    cerr << "tick " << this->tick << endl;
    if (me == nullptr)
      exit(1);

    //           bombs         field               me       enemy     enemy_pos          features         state         target(first)
    vector<tuple<vector<Bomb>, vector<bitset<W>>, Player, Player, vector<bitset<W>>, vector<Feature>, StateQuality, Target>> initial_states, first_states, second_states, final_states;

    vector<bitset<W>> enemy_possible_pos = vector<bitset<W>>(H, bitset<W>());

    Player me = Player(this->me);
    Player enemy = Player(this->enemy);

    if (!enemy.dead)
      enemy_possible_pos[enemy.y][enemy.x] = 1;

    initial_states.pb(make_tuple(bombs, bit_field, me, enemy, enemy_possible_pos, features, StateQuality(me_global_score, enemy_global_score), Target()));

    first_states = get_action(initial_states, 3, 2);
    second_states = get_action(first_states, 3, 2);
    final_states = get_action(second_states, 1, 0);
    Target target;
    StateQuality sq;
    tie(ignore, ignore, ignore, ignore, ignore, ignore, sq, target) = final_states[0];

    cerr << target.x << " " << target.y << " " << target.tick << " " << target.type << " " << sq.me_score << " " << sq.me_dist_penalty << " " << sq.me_alive << endl;

    if (target.type == 0 || target.type == 2)
    {
      this->me->action = get_move(target);
      if (this->me->action == PLAYER_JUMP)
      {
        cerr << "----------------------------- JUMP!" << endl;
      }
      if (target.type == 2)
      {
        cerr << "----------------------------- FEATURE!" << endl;
      }
    }
    else if (target.type == 1)
    {
      this->me->action = PLAYER_TELEPORT;
      this->me->action_x = target.x;
      this->me->action_y = target.y;
      cerr << "------------------------------ TP " << target.x << " " << target.y << endl;
    }
    else if (target.type == 3)
    {
      this->me->action = PLAYER_JUMP;
      cerr << "----------------------------- JUMP!!" << endl;
    }
    int me_global_score_change, enemy_global_score_change;
    tie(me_global_score_change, enemy_global_score_change, ignore) = get_tick_score(me, enemy, bit_field, bombs, features);
    me_global_score += me_global_score_change;
    enemy_global_score += enemy_global_score_change;
    cerr << "SCORE: " << me_global_score - enemy_global_score << endl;
  }

  void apply()
  {
    switch (this->me->action)
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
    case PLAYER_JUMP:
      printf("jump\n");
      break;
    case PLAYER_TELEPORT:
      printf("tp %d %d\n", this->me->action_x, this->me->action_y);
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
    max_tick_time = max(max_tick_time, tick_endtime - tick_starttime);
    fprintf(stderr, "tick %d time: %.3lf ms\n", game.tick, max_tick_time);
  }

  return 0;
}
