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

const int bomb_timer = 6;
const int W = 13, H = 11, K = W + H + 10;

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

    int entities;
    scanf("%d", &entities);
    for (int i = 0; i < entities; i++)
    {
      char type;
      int owner_id, x, y, param1, param2;
      scanf("\n%c", &type);
      if (type == 'f')
        scanf("%*c%*c");
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
    }

    for (Player &player : players)
    {
      if (player.owner_id == owner_id)
      {
        this->me = &player;
      }
      for (Bomb bomb: bombs) {
        if (player.owner_id == bomb.owner_id) {
          player.max_bombs++;
        }
      }
    }
    this->tick = tick;
  }

  
  int f(int dist, int box_cnt, int bomb_cnt){
    return box_cnt * 5 - dist;
  }
  
  void calc_accessibleness(vector<Bomb> bombs, Field field, bool (&result)[][][]){

  }

  pair<PlayerMove, int> get_action(vector<Bomb> bombs, Field field, bool have_bomb){
    simulate()
    int cost[K][W][H];
    calc_accessibleness(bombs, fiels, cost);

    for(int layer = first_bomb; layer < const; layer++)
      for
        for
          if(cost != -1)
            cost
    return 
  }
  
  pair<PlayerMove, int> get_action_with_bomb(vector<Bomb> bombs, Field field){
    //simulate first tick  
    //...
    get_action(last_bombs, new_field)
  }  

  vector< simulate(vector<Bomb> &bombs, Field &field){

  }

  bool check_bomb_will_destroy_box(vector<Bomb> bombs, Field field){
    bombs.pb();
    simulate()
  }
  
  /*
  simulate one tick
  first tick when our bomb boom
  number of destroyed boxes with our bomb
  */

  void prepare()
  {
    pair<PlayerMove, int> base_action = get_action(bombs, field, this->me->bombs > 0);
    this -> me -> action = base_action.fs;
    if(this->me->bombs > 0)
      int prev_cnt = get_score(bombs, field);
      vec bombs = this -> boms
      cnt = 
      if prev_cnt < cnt
        y = get_action_with_bomb
        if y.s <= x.s
          set_action y.f
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
