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
  Field field;
  vector<Player> players;
  vector<Bomb> bombs;
  vector<Monster> monsters;
  vector<Feature> features;

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
        if (players.back().owner_id == owner_id)
        {
          this->me = &players.back();
        }
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

    for (Player &player : players)
    {
      if (player.owner_id == owner_id)
      {
        this->me = &player;
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

  tuple<bool, int, int> check_safe_and_get_score(int tick, int own_x, int own_y, bitset<W> (&previous_field)[H], Bomb (&previous_bombs)[MAX_BOMB], int bombs_count)
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
    // check realy tick + 6 or not
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

  tuple<int, int, int, int> get_action(vector<Bomb> vec_bombs, Field field, int next_tick_with_bomb)
  {
    int max_f = -1;
    int go_x, go_y, go_tick;

    int own_x = this->me->x;
    int own_y = this->me->y;

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
              tie(is_safe, own_score_change, rival_score_change) = check_safe_and_get_score(tick + 1, x, y, current_field, bombs, bombs_count);
            }
            else
            {
              if (!is_calced[y][x])
              {
                is_calced[y][x] = 1;
                calced_value[y][x] = check_safe_and_get_score(tick + 1, x, y, current_field, bombs, bombs_count);
              }
              tie(is_safe, own_score_change, rival_score_change) = calced_value[y][x];
            }

            if (is_safe)
            {
              int own_next_score = own_score + own_score_change;
              int rival_next_score = rival_score + rival_score_change;
              int cur_f = f(tick, own_next_score, rival_next_score);
              if (max_f < cur_f && tick >= next_tick_with_bomb)
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
                  if (features[i].type == FEATURE_RANGE)
                  {
                    dp_val += 10;
                  }
                  if (features[i].type == FEATURE_AMOUNT)
                  {
                    if (dp_val < 2000)
                      dp_val += 1000;
                    else
                      dp_val += 1;
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

  tuple<int, int, int, int> get_action_with_bomb(vector<Bomb> bombs, Field field)
  {
    bool our_bomb_destroyed = false;
    bool bomb_hits_our_position = false;
    // cerr << "A" << bombs.size() << endl;
    next_tick(bombs, field, our_bomb_destroyed, bomb_hits_our_position);
    // cerr << "B" << bombs.size() << endl;
    int next_tick_with_bomb = this->me->bombs - 1 + our_bomb_destroyed > 0 ? 0 : get_bomb_restore_ticks(bombs, field);
    if (bomb_hits_our_position)
      return make_tuple(0, 0, 0, -2); // TODO: fix it
    return get_action(bombs, field, next_tick_with_bomb);
  }

  // simulate one tick. updates state, returns score of tick
  int next_tick(vector<Bomb> &bombs, Field &field, bool &our_bomb_destroyed, bool &bomb_hits_our_position)
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
              if (bomb.owner_id == this->me->owner_id)
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
    bomb_hits_our_position = destroyed_me.count(mp(this->me->x, this->me->y)) > 0 || destroyed_others.count(mp(this->me->x, this->me->y)) > 0;
    return score;
  }

  int next_tick(vector<Bomb> &bombs, Field &field)
  {
    bool our_bomb_destroyed = false;
    bool bomb_hits_our_position = false;
    return next_tick(bombs, field, our_bomb_destroyed, bomb_hits_our_position);
  }

  // number of destroyed boxes with our bombs minus number of destroyed boxes with other bombs
  int get_score(vector<Bomb> bombs, Field field)
  {
    int score = 0;
    while (!bombs.empty())
    {
      score += next_tick(bombs, field);
    }
    return score;
  }

  // first tick when our bomb boom
  int get_bomb_restore_ticks(vector<Bomb> bombs, Field field)
  {

    bool we_have_bomb_on_field = false;
    for (Bomb &bomb : bombs)
    {
      if (bomb.owner_id == this->me->owner_id)
      {
        we_have_bomb_on_field = true;
        break;
      }
    }
    if (!we_have_bomb_on_field)
      return 10000;

    int tick = 0;
    while (!bombs.empty())
    {
      bool our_bomb_destroyed = false;
      bool bomb_hits_our_position = false;
      next_tick(bombs, field, our_bomb_destroyed, bomb_hits_our_position);
      tick++;
      if (our_bomb_destroyed)
      {
        return tick;
      }
    }
    // must be impossible
    return 10001;
  }

  void prepare()
  {
    int next_tick_with_bomb = this->me->bombs > 0 ? 0 : get_bomb_restore_ticks(this->bombs, this->field);
    cerr << "next_bomb" << next_tick_with_bomb << endl;
    auto [tick, go_x, go_y, max_f] = get_action(this->bombs, this->field, next_tick_with_bomb);
    cerr << "GO TO " << tick << " " << go_x << " " << go_y << " " << max_f << endl;
    if (this->me->bombs > 0)
    {
      int prev_score = get_score(this->bombs, this->field);
      vector<Bomb> bombs = this->bombs;
      bombs.pb(Bomb(this->me->x, this->me->y, this->me->owner_id, BOMB_TIMER, this->me->range));
      int new_score = get_score(bombs, this->field);
      cerr << "here " << prev_score << " " << new_score << endl;
      if (prev_score < new_score)
      {
        auto [ntick, ngo_x, ngo_y, nmax_f] = get_action_with_bomb(bombs, this->field);
        cerr << "new max f " << nmax_f << endl;
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
