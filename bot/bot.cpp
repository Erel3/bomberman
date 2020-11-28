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
    // TODO: fix it
    this->max_bombs = bombs;
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
  Game()
  {
    read_state(true);
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
      scanf("\n%c%d%d%d%d%d", &type, &owner_id, &x, &y, &param1, &param2);
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
    this->tick = tick;
  }

  vector<vector<vector<int>>> shortest_paths;
  vector<vector<vector<int>>> action_id;
  int layers_to_check = 9;
  int last_layer = layers_to_check - 1;

  void shortest_path()
  {

    vector<vector<vector<int>>> destroy_map = vector<vector<vector<int>>>(layers_to_check, vector<vector<int>>(this->field.height, vector<int>(this->field.width, 0)));
    vector<vector<vector<int>>> current_map = vector<vector<vector<int>>>(layers_to_check, vector<vector<int>>(this->field.height, vector<int>(this->field.width, 0)));
    vector<Bomb> bombs = this->bombs;

    // setting up map at current time
    for (int i = 0; i < this->field.height; i++)
    {
      for (int j = 0; j < this->field.width; j++)
      {
        if (this->field.cells[i][j].type == CELL_BLOCK || this->field.cells[i][j].type == CELL_BOX)
        {
          current_map[0][i][j] = 1;
        }
      }
    }
    for (Bomb bomb : bombs)
      current_map[0][bomb.y][bomb.x] = 1;

    { // calculate field map, dangerous cells for next points of time
      set<int> alive_bombs;
      for (int i = 0; i < bombs.size(); i++)
        alive_bombs.insert(i);
      for (int layer = 1; layer < layers_to_check; layer++)
      {

        // copy current_map
        for (int i = 0; i < this->field.height; i++)
        {
          for (int j = 0; j < this->field.width; j++)
          {
            current_map[layer][i][j] = current_map[layer - 1][i][j];
          }
        }

        // boom
        vector<int> to_destroy;
        for (int id : alive_bombs)
        {
          Bomb &bomb = bombs[id];
          if (bomb.timer == layer)
          {
            to_destroy.pb(id);
            destroy_map[layer][bomb.y][bomb.x] = 1;
            current_map[layer][bomb.y][bomb.x] = 0;
            for (int dir = 0; dir < 4; dir++)
            {
              for (int i = 1; i <= bomb.range; i++)
              {
                int to_x = bomb.x + dx[dir] * i, to_y = bomb.y + dy[dir] * i;
                if (to_x < 0 || to_x >= this->field.width ||
                    to_y < 0 || to_y >= this->field.height || this->field.cells[to_y][to_x].type == CELL_BLOCK)
                {
                  break;
                }
                destroy_map[layer][to_y][to_x] = 1;
                current_map[layer][to_y][to_x] = 0;
                if (this->field.cells[to_y][to_x].type == CELL_BOX && current_map[layer - 1][to_y][to_x] == 1)
                {
                  break;
                }
              }
            }
          }
        }
        for (int id : to_destroy)
        {
          alive_bombs.erase(id);
        }
        to_destroy.clear();
        // check if bomb destroyed another bomb
        bool check = true;
        while (check)
        {
          check = false;
          for (int id : alive_bombs)
          {
            Bomb &bomb = bombs[id];
            if (destroy_map[layer][bomb.y][bomb.x] == 1)
            {
              check = true;
              to_destroy.pb(id);
              destroy_map[layer][bomb.y][bomb.x] = 1;
              current_map[layer][bomb.y][bomb.x] = 0;
              for (int dir = 0; dir < 4; dir++)
              {
                for (int i = 1; i <= bomb.range; i++)
                {
                  int to_x = bomb.x + dx[dir] * i, to_y = bomb.y + dy[dir] * i;
                  if (to_x < 0 || to_x >= this->field.width ||
                      to_y < 0 || to_y >= this->field.height || this->field.cells[to_y][to_x].type == CELL_BLOCK)
                  {
                    break;
                  }
                  destroy_map[layer][to_y][to_x] = 1;
                  current_map[layer][to_y][to_x] = 0;
                  if (this->field.cells[to_y][to_x].type == CELL_BOX && current_map[layer - 1][to_y][to_x] == 1)
                  {
                    break;
                  }
                }
              }
            }
          }
          for (int id : to_destroy)
          {
            alive_bombs.erase(id);
          }
          to_destroy.clear();
        }
      }
    }

    // for(int k = 0; k < 8; k++) {
    //   cerr << k << " - \n";
    //   for(int i = 0; i < 11; i++) {
    //     for(int j = 0; j < 13; j++) {
    //       cerr << destroy_map[k][i][j];
    //     }
    //     cerr << endl;
    //   }
    //   cerr << endl;
    // }

    // now we have destroy_map, current_map and we can find shortest path without dying. BUT!! maybe we can access some point at earliest time but die later
    shortest_paths = vector<vector<vector<int>>>(layers_to_check, vector<vector<int>>(this->field.height, vector<int>(this->field.width, -1)));
    action_id = vector<vector<vector<int>>>(layers_to_check, vector<vector<int>>(this->field.height, vector<int>(this->field.width, -1))); // to get previous action

    vector<pair<pair<int, pair<int, int>>, int>> q;
    shortest_paths[0][this->me->y][this->me->x] = 0;
    q.pb(mp(mp(0, mp(this->me->y, this->me->x)), 0));
    for (int i = 0; i < q.size(); i++)
    {
      int layer = q[i].fs.fs;
      int y = q[i].fs.sc.fs;
      int x = q[i].fs.sc.sc;
      int len = q[i].sc;
      for (int j = 0; j < 5; j++)
      {
        int nl = min(last_layer, layer + 1), nx = x + dx[j], ny = y + dy[j];
        if (nx >= 0 && nx < this->field.width && ny >= 0 && ny < this->field.height)
          if (destroy_map[nl][ny][nx] == 0 && (current_map[nl][ny][nx] == 0 || j == 4) && shortest_paths[nl][ny][nx] == -1)
          {
            shortest_paths[nl][ny][nx] = len + 1;
            if (nl == layer)
            {
              action_id[nl][ny][nx] = 10 + j;
            }
            else
            {
              action_id[nl][ny][nx] = j;
            }
            q.pb(mp(mp(nl, mp(ny, nx)), len + 1));
          }
      }
    }

    // for(int i = 0; i < layers_to_check; i++) {
    //   fprintf(stderr, "%d %d\n", i, q.size());
    //   for(int j = 0; j < this -> field.height; j++) {
    //     for(int k = 0; k < this -> field.width; k++) {
    //       if (shortest_paths[i][j][k] != -1) {
    //         fprintf(stderr, "%d", action_id[i][j][k]);
    //       } else {
    //         fprintf(stderr, ".");
    //       }
    //     }
    //     fprintf(stderr, "\n");
    //   }
    //   fprintf(stderr, "\n");
    // }

    vector<vector<vector<bool>>> is_safe = vector<vector<vector<bool>>>(layers_to_check, vector<vector<bool>>(this->field.height, vector<bool>(this->field.width, false)));
    for (int i = 0; i < this->field.height; i++)
    {
      for (int j = 0; j < this->field.width; j++)
      {
        is_safe[last_layer][i][j] = shortest_paths[last_layer][i][j] != -1;
      }
    }
    for (int layer = last_layer - 1; layer >= 0; layer--)
    {
      for (int i = 0; i < this->field.height; i++)
      {
        for (int j = 0; j < this->field.width; j++)
        {
          if (shortest_paths[layer][i][j] != -1)
          {
            bool ok = 0;
            for (int dir = 0; dir < 5; dir++)
            {
              int nx = j + dx[dir], ny = i + dy[dir];
              if (nx >= 0 && nx < this->field.width && ny >= 0 && ny < this->field.height)
                ok |= is_safe[layer + 1][ny][nx] && (!current_map[layer + 1][ny][nx] || dir == 4);
            }
            is_safe[layer][i][j] = ok;
          }
        }
      }
    }

    for (int layer = last_layer - 1; layer >= 0; layer--)
    {
      for (int i = 0; i < this->field.height; i++)
      {
        for (int j = 0; j < this->field.width; j++)
        {
          if (!is_safe[layer][i][j])
          {
            shortest_paths[layer][i][j] = -1;
          }
        }
      }
    }
  }

  vector<vector<vector<int>>> shortest_paths_with_bomb;
  void shortest_path_with_bomb()
  {

    vector<vector<vector<int>>> destroy_map = vector<vector<vector<int>>>(layers_to_check, vector<vector<int>>(this->field.height, vector<int>(this->field.width, 0)));
    vector<vector<vector<int>>> current_map = vector<vector<vector<int>>>(layers_to_check, vector<vector<int>>(this->field.height, vector<int>(this->field.width, 0)));
    vector<Bomb> bombs = this->bombs;
    bombs.pb(Bomb(this->me->x, this->me->y, this->me->owner_id, bomb_timer, this->me->range));
    // setting up map at current time
    for (int i = 0; i < this->field.height; i++)
    {
      for (int j = 0; j < this->field.width; j++)
      {
        if (this->field.cells[i][j].type == CELL_BLOCK || this->field.cells[i][j].type == CELL_BOX)
        {
          current_map[0][i][j] = 1;
        }
      }
    }
    for (Bomb bomb : bombs)
      current_map[0][bomb.y][bomb.x] = 1;

    { // calculate field map, dangerous cells for next points of time
      set<int> alive_bombs;
      for (int i = 0; i < bombs.size(); i++)
        alive_bombs.insert(i);
      for (int layer = 1; layer < layers_to_check; layer++)
      {

        // copy current_map
        for (int i = 0; i < this->field.height; i++)
        {
          for (int j = 0; j < this->field.width; j++)
          {
            current_map[layer][i][j] = current_map[layer - 1][i][j];
          }
        }

        // boom
        vector<int> to_destroy;
        for (int id : alive_bombs)
        {
          Bomb &bomb = bombs[id];
          if (bomb.timer == layer)
          {
            to_destroy.pb(id);
            destroy_map[layer][bomb.y][bomb.x] = 1;
            current_map[layer][bomb.y][bomb.x] = 0;
            for (int dir = 0; dir < 4; dir++)
            {
              for (int i = 1; i <= bomb.range; i++)
              {
                int to_x = bomb.x + dx[dir] * i, to_y = bomb.y + dy[dir] * i;
                if (to_x < 0 || to_x >= this->field.width ||
                    to_y < 0 || to_y >= this->field.height || this->field.cells[to_y][to_x].type == CELL_BLOCK)
                {
                  break;
                }
                destroy_map[layer][to_y][to_x] = 1;
                current_map[layer][to_y][to_x] = 0;
                if (this->field.cells[to_y][to_x].type == CELL_BOX && current_map[layer - 1][to_y][to_x] == 1)
                {
                  break;
                }
              }
            }
          }
        }
        for (int id : to_destroy)
        {
          alive_bombs.erase(id);
        }
        to_destroy.clear();
        // check if bomb destroyed another bomb
        bool check = true;
        while (check)
        {
          check = false;
          for (int id : alive_bombs)
          {
            Bomb &bomb = bombs[id];
            if (destroy_map[layer][bomb.y][bomb.x] == 1)
            {
              check = true;
              to_destroy.pb(id);
              destroy_map[layer][bomb.y][bomb.x] = 1;
              current_map[layer][bomb.y][bomb.x] = 0;
              for (int dir = 0; dir < 4; dir++)
              {
                for (int i = 1; i <= bomb.range; i++)
                {
                  int to_x = bomb.x + dx[dir] * i, to_y = bomb.y + dy[dir] * i;
                  if (to_x < 0 || to_x >= this->field.width ||
                      to_y < 0 || to_y >= this->field.height || this->field.cells[to_y][to_x].type == CELL_BLOCK)
                  {
                    break;
                  }
                  destroy_map[layer][to_y][to_x] = 1;
                  current_map[layer][to_y][to_x] = 0;
                  if (this->field.cells[to_y][to_x].type == CELL_BOX && current_map[layer - 1][to_y][to_x] == 1)
                  {
                    break;
                  }
                }
              }
            }
          }
          for (int id : to_destroy)
          {
            alive_bombs.erase(id);
          }
          to_destroy.clear();
        }
      }
    }

    // for(int k = 0; k < 8; k++) {
    //   cerr << k << " - \n";
    //   for(int i = 0; i < 11; i++) {
    //     for(int j = 0; j < 13; j++) {
    //       cerr << destroy_map[k][i][j];
    //     }
    //     cerr << endl;
    //   }
    //   cerr << endl;
    // }

    // now we have destroy_map, current_map and we can find shortest path without dying. BUT!! maybe we can access some point at earliest time but die later
    shortest_paths_with_bomb = vector<vector<vector<int>>>(layers_to_check, vector<vector<int>>(this->field.height, vector<int>(this->field.width, -1)));

    vector<pair<pair<int, pair<int, int>>, int>> q;
    shortest_paths_with_bomb[0][this->me->y][this->me->x] = 0;
    q.pb(mp(mp(0, mp(this->me->y, this->me->x)), 0));
    for (int i = 0; i < q.size(); i++)
    {
      int layer = q[i].fs.fs;
      int y = q[i].fs.sc.fs;
      int x = q[i].fs.sc.sc;
      int len = q[i].sc;
      for (int j = (layer == 0 ? 4 : 0); j < 5; j++)
      {
        int nl = min(last_layer, layer + 1), nx = x + dx[j], ny = y + dy[j];
        if (nx >= 0 && nx < this->field.width && ny >= 0 && ny < this->field.height)
          if (destroy_map[nl][ny][nx] == 0 && (current_map[nl][ny][nx] == 0 || j == 4) && shortest_paths_with_bomb[nl][ny][nx] == -1)
          {
            shortest_paths_with_bomb[nl][ny][nx] = len + 1;
            q.pb(mp(mp(nl, mp(ny, nx)), len + 1));
          }
      }
    }

    // for(int i = 0; i < layers_to_check; i++) {
    //   fprintf(stderr, "%d %d\n", i, q.size());
    //   for(int j = 0; j < this -> field.height; j++) {
    //     for(int k = 0; k < this -> field.width; k++) {
    //       if (shortest_paths_with_bomb[i][j][k] != -1) {
    //         fprintf(stderr, "%d", action_id[i][j][k]);
    //       } else {
    //         fprintf(stderr, ".");
    //       }
    //     }
    //     fprintf(stderr, "\n");
    //   }
    //   fprintf(stderr, "\n");
    // }

    vector<vector<vector<bool>>> is_safe = vector<vector<vector<bool>>>(layers_to_check, vector<vector<bool>>(this->field.height, vector<bool>(this->field.width, false)));
    for (int i = 0; i < this->field.height; i++)
    {
      for (int j = 0; j < this->field.width; j++)
      {
        is_safe[last_layer][i][j] = shortest_paths_with_bomb[last_layer][i][j] != -1;
      }
    }
    for (int layer = last_layer - 1; layer >= 0; layer--)
    {
      for (int i = 0; i < this->field.height; i++)
      {
        for (int j = 0; j < this->field.width; j++)
        {
          if (shortest_paths_with_bomb[layer][i][j] != -1)
          {
            bool ok = 0;
            for (int dir = 0; dir < 5; dir++)
            {
              int nx = j + dx[dir], ny = i + dy[dir];
              if (nx >= 0 && nx < this->field.width && ny >= 0 && ny < this->field.height)
                ok |= is_safe[layer + 1][ny][nx] && (!current_map[layer + 1][ny][nx] || dir == 4);
            }
            is_safe[layer][i][j] = ok;
          }
        }
      }
    }

    for (int layer = last_layer - 1; layer >= 0; layer--)
    {
      for (int i = 0; i < this->field.height; i++)
      {
        for (int j = 0; j < this->field.width; j++)
        {
          if (!is_safe[layer][i][j])
          {
            shortest_paths_with_bomb[layer][i][j] = -1;
          }
        }
      }
    }
  }

  vector<vector<int>> shortest_paths_dummy;
  vector<vector<int>> action_id_dummy;
  void shortest_path_dummy()
  {
    shortest_paths_dummy = vector<vector<int>>(this->field.height, vector<int>(this->field.width, -1));
    action_id_dummy = vector<vector<int>>(this->field.height, vector<int>(this->field.width, -1));
    priority_queue<pair<pair<int, int>, pair<int, int>>> q;
    q.push(mp(mp(0, -1), mp(this->me->y, this->me->x)));
    while (!q.empty())
    {
      pair<pair<int, int>, pair<int, int>> e = q.top();
      q.pop();
      int y = e.sc.fs;
      int x = e.sc.sc;
      int len = -e.fs.fs;
      int act = e.fs.sc;
      if (shortest_paths_dummy[y][x] != -1)
        continue;
      shortest_paths_dummy[y][x] = len;
      action_id_dummy[y][x] = act;
      for (int j = 0; j < 5; j++)
      {
        int nx = x + dx[j], ny = y + dy[j];
        if (nx >= 0 && nx < this->field.width && ny >= 0 && ny < this->field.height)
          if (this->field.cells[ny][nx].type != CELL_BLOCK)
          {
            q.push(mp(mp(-(len + (field.cells[ny][nx].type == CELL_BOX ? 9 : 1)), j), mp(ny, nx)));
          }
      }
    }
  }

  pair<int, int> get_box_by_target(int x, int y)
  {
    pair<int, int> box = mp(-1, -1);
    while (shortest_paths_dummy[y][x] > 0)
    {
      if (this->field.cells[y][x].type == CELL_BOX)
        box = mp(y, x);
      int dir = action_id_dummy[y][x];
      int nx = x - dx[dir], ny = y - dy[dir];
      x = nx, y = ny;
    }
    return box;
  }

  PlayerMove get_action_by_target(int x, int y)
  {
    for (int i = 1; i < layers_to_check; i++)
    {
      if (shortest_paths[i][y][x] != -1)
      {
        while (i > 1)
        {
          int dir = action_id[i][y][x];
          int nx = x - dx[dir % 10], ny = y - dy[dir % 10];
          x = nx, y = ny, i -= dir < 10 ? 1 : 0;
        }
        int dir = action_id[i][y][x] % 10;
        cerr << "layer: " << i << endl;
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
        assert(0);
      }
    }
    assert(0);
  }

  void destroy_cell(int ix, int iy)
  {
    int gox, goy, dd = 10000;

    if (this->field.cells[iy][ix].type == CELL_BOX)
      for (int dir = 0; dir < 4; dir++)
      {
        for (int i = 1; i <= this->me->range; i++)
        {
          int to_x = ix + dx[dir] * i, to_y = iy + dy[dir] * i;
          if (to_x < 0 || to_x >= this->field.width ||
              to_y < 0 || to_y >= this->field.height || this->field.cells[to_y][to_x].type == CELL_BLOCK || this->field.cells[to_y][to_x].type == CELL_BOX)
          {
            break;
          }

          for (int l = 0; l < layers_to_check; l++)
          {
            if (shortest_paths[l][to_y][to_x] != -1 && dd > shortest_paths[l][to_y][to_x])
            {
              dd = shortest_paths[l][to_y][to_x];
              gox = to_x;
              goy = to_y;
            }
          }
        }
      }
    // cerr << gox << " " << goy << endl;
    if (dd == 10000)
      return;
    if (gox == this->me->x && goy == this->me->y)
    {
      cerr << "t3" << endl;
      if (this->me->bombs)
      {
        this->me->action = PLAYER_BOMB;
      }
      else
      {
        cerr << "shit" << endl;
        gox = 0, goy = 0;
        this->me->action = get_action_by_target(gox, goy);
      }
    }
    else
    {
      cerr << "t4 " << (this->me->x) << " " << (this->me->y) << " - " << gox << " " << goy << endl;
      this->me->action = get_action_by_target(gox, goy);
    }
  }

  int get_way_length_to_target(int x, int y)
  {
    for (int i = 1; i < layers_to_check; i++)
    {
      if (shortest_paths[i][y][x] != -1)
      {
        return shortest_paths[i][y][x];
      }
    }
  }

  void prepare()
  {

    shortest_path();
    shortest_path_with_bomb();

    // ///monstercatcher
    // // open path
    // if(this -> field.cells[2][0].type == CELL_BOX) {
    //   destroy_cell(0, 2);
    //   return;
    // } else if(this -> field.cells[3][0].type == CELL_BOX) {
    //   destroy_cell(0, 3);
    //   return;
    // } else if(this -> field.cells[4][0].type == CELL_BOX) {
    //   destroy_cell(0, 4);
    //   return;
    // }
    // // zasada
    // if(this -> monsters.size() == 3) {
    //   int gox = 0, goy = 2;
    //   if (this -> me -> bombs && this -> me -> x == gox && this -> me -> y == goy) {
    //     for (Monster monster: this -> monsters) {
    //       if (get_way_length_to_target(monster.x, monster.y) <= 7) {
    //         this -> me -> action = PLAYER_BOMB;
    //         return;
    //       }
    //     }
    //     this -> me -> action = PLAYER_STAY;
    //   } else {
    //     for(int i = 1; i < layers_to_check; i++) {
    //       if (shortest_paths[i][goy][gox] != -1) {
    //         this -> me -> action = get_action_by_target(i, gox, goy);
    //         break;
    //       }
    //     }
    //   }
    // } else {
    //   // if(this -> field.cells[2][6].type == CELL_BOX) {
    //   //   destroy_cell(6, 2);
    //   //   return;
    //   // }
    //   int gox = 4, goy = 4;
    //   if (this -> me -> bombs && this -> me -> x == gox && this -> me -> y == goy) {
    //     for (Monster monster: this -> monsters) {
    //       if (get_way_length_to_target(monster.x, monster.y) <= 7) {
    //         this -> me -> action = PLAYER_BOMB;
    //         return;
    //       }
    //     }
    //     this -> me -> action = PLAYER_STAY;
    //   } else {
    //     for(int i = 1; i < layers_to_check; i++) {
    //       if (shortest_paths[i][goy][gox] != -1) {
    //         this -> me -> action = get_action_by_target(i, gox, goy);
    //         break;
    //       }
    //     }
    //   }
    // }

    // box/monster destroy
    int gox = -1, goy, dd = 10000;
    for (Monster monster : this->monsters)
    {
      for (int jj = 0; jj < 5; jj++)
      {
        int ix = monster.x + dx[jj], iy = monster.y + dy[jj];
        for (int dir = 0; dir < 4; dir++)
        {
          for (int i = 1; i <= this->me->range; i++)
          {
            int to_x = ix + dx[dir] * i, to_y = iy + dy[dir] * i;
            if (to_x < 0 || to_x >= this->field.width ||
                to_y < 0 || to_y >= this->field.height || this->field.cells[to_y][to_x].type == CELL_BLOCK || this->field.cells[to_y][to_x].type == CELL_BOX)
            {
              break;
            }

            for (int l = 0; l < layers_to_check; l++)
            {
              if (shortest_paths[l][to_y][to_x] != -1 && dd > shortest_paths[l][to_y][to_x])
              {
                dd = shortest_paths[l][to_y][to_x];
                gox = to_x;
                goy = to_y;
              }
            }
          }
        }
      }
    }
    if (gox == -1)
      for (int iy = 0; iy < this->field.height; iy++)
      {
        for (int ix = 0; ix < this->field.width; ix++)
        {
          if (this->field.cells[iy][ix].type == CELL_BOX && shortest_paths[last_layer][iy][ix] == -1)
            for (int dir = 0; dir < 4; dir++)
            {
              for (int i = 1; i <= this->me->range; i++)
              {
                int to_x = ix + dx[dir] * i, to_y = iy + dy[dir] * i;
                if (to_x < 0 || to_x >= this->field.width ||
                    to_y < 0 || to_y >= this->field.height || this->field.cells[to_y][to_x].type == CELL_BLOCK || this->field.cells[to_y][to_x].type == CELL_BOX)
                {
                  break;
                }

                for (int l = 0; l < layers_to_check; l++)
                {
                  if (shortest_paths[l][to_y][to_x] != -1 && dd > shortest_paths[l][to_y][to_x])
                  {
                    dd = shortest_paths[l][to_y][to_x];
                    gox = to_x;
                    goy = to_y;
                  }
                }
              }
            }
        }
      }

    if (gox == this->me->x && goy == this->me->y)
    {
      cerr << "t3" << endl;
      if (this->me->bombs && shortest_paths_with_bomb[last_layer][goy][gox] != -1)
      {
        this->me->action = PLAYER_BOMB;
      }
      else
      {
        cerr << "shit" << endl;
        gox = 0, goy = 0;
        this->me->action = get_action_by_target(gox, goy);
      }
    }
    else if (gox != -1)
    {
      cerr << "t4 " << (this->me->x) << " " << (this->me->y) << " - " << gox << " " << goy << endl;
      this->me->action = get_action_by_target(gox, goy);
    }
    else
    {
      this->me->action = get_action_by_target(0, 0);
    }

    ///gototarget
    // shortest_path_dummy();
    // int targetx = 12, targety = 10;
    // pair<int, int> box = get_box_by_target(targetx, targety);
    // cerr << box.sc << " " << box.fs << " " << shortest_paths_dummy[targety][targetx] << endl;
    // if (box.fs == -1) {
    //   cerr << "t1" << endl;
    //   for(int i = 1; i < layers_to_check; i++) {
    //     if (shortest_paths[i][targety][targetx] != -1) {
    //       this -> me -> action = get_action_by_target(i, targetx, targety);
    //       break;
    //     }
    //   }
    // } else {
    //   if(shortest_paths[last_layer][box.fs][box.sc] != -1) {
    //     cerr << "t2 " << box.sc << " " << box.fs << endl;
    //     for(int i = 1; i < layers_to_check; i++) {
    //       if (shortest_paths[i][box.fs][box.sc] != -1) {
    //         this -> me -> action = get_action_by_target(i, box.sc, box.fs);
    //         break;
    //       }
    //     }
    //   } else {
    //     // destroy_box(box.sc, box.fs);
    //     int gox, goy, dd = 10000;
    //     for (int dir = 0; dir < 4; dir++) {
    //       for (int i = 1; i <= this -> me -> range; i++) {
    //         int to_x = box.sc + dx[dir] * i, to_y = box.fs + dy[dir] * i;
    //         if (to_x < 0 || to_x >= this -> field.width ||
    //           to_y < 0 || to_y >= this -> field.height || this -> field.cells[to_y][to_x].type == CELL_BLOCK || this -> field.cells[to_y][to_x].type == CELL_BOX) {
    //             break;
    //           }

    //           for (int l = 0; l < layers_to_check; l++) {
    //             if (shortest_paths[l][to_y][to_x] != -1 && dd > shortest_paths[l][to_y][to_x]) {
    //               dd = shortest_paths[l][to_y][to_x];
    //               gox = to_x;
    //               goy = to_y;
    //             }
    //           }

    //       }
    //     }

    //     if (gox == this -> me -> x && goy == this -> me -> y) {
    //       cerr << "t3" << endl;
    //       if (this -> me -> bombs) {
    //         this -> me -> action = PLAYER_BOMB;
    //       } else {
    //         cerr << "shit" << endl;
    //         gox = 0, goy = 0;
    //         for(int i = 1; i < layers_to_check; i++) {
    //           if (shortest_paths[i][goy][gox] != -1) {
    //             this -> me -> action = get_action_by_target(i, gox, goy);
    //             break;
    //           }
    //         }
    //       }
    //     } else {
    //       cerr << "t4 " << (this -> me -> x) << " " << (this -> me -> y) << " - " << gox << " " << goy << " - " << box.sc << " " << box.fs << endl;
    //       for(int i = 1; i < layers_to_check; i++) {
    //         if (shortest_paths[i][goy][gox] != -1) {
    //           this -> me -> action = get_action_by_target(i, gox, goy);
    //           break;
    //         }
    //       }
    //     }
    //   }
    // }
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
