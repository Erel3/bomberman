#pragma comment(linker, "/stack:20000000")
#pragma GCC optimize("Ofast")
#pragma GCC target("sse,sse2,sse3,ssse3,sse4,popcnt,abm,mmx")

#include <stdio.h>
#include <bits/stdc++.h>
#include <x86intrin.h>
#include <ext/pb_ds/assoc_container.hpp>
using namespace __gnu_pbds;
template <typename T> using ordered_set = tree<T, null_type, std::less<T>, rb_tree_tag, tree_order_statistics_node_update>;
template <typename T1, typename T2> using ordered_map = tree<T1, T2, std::less<T1>, rb_tree_tag, tree_order_statistics_node_update>;

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


enum PlayerMove {
  PLAYER_STAY, 
  PLAYER_LEFT, 
  PLAYER_RIGHT, 
  PLAYER_DOWN,
  PLAYER_UP,
  PLAYER_BOMB
};

class Player {
  public:

    Player(int x, int y, int owner_id, int bombs, int range) : x(x), y(y), owner_id(owner_id), bombs(bombs), range(range) {
      // TODO: fix it
      this -> range = 2;
    }

    int x, y;
    int owner_id;
    int max_bombs, bombs, range;
};

enum CellType {
  CELL_EMPTY = '.', 
  CELL_BOX = ';', 
  CELL_BLOCK = '!', 
  CELL_BOMB = 'O'
};

class Cell {
  public:
    CellType type;

    Cell() {
      this -> type = CELL_EMPTY;
    }

    Cell(char c) {
      this -> type = CellType(c);
    }
};

class Field {
  public:

    Field() {}

    Field(int width, int height) {
      this -> width = width;
      this -> height = height;
      this -> cells = vector<vector<Cell>>(height, vector<Cell>(width));
    }

    int width, height;
    vector<vector<Cell>> cells;
};

class Game {
  private:
    Player *me = nullptr;
    Field field;
    vector<Player> players;
    // vector<Bomb> bombs;
    // vector<Monster> monsters;

  public:

    int tick;
    Game() {
      read_state(true);
    }

    void read_state(bool create = false) {
      int width, height, owner_id, tick;
      scanf("%d%d%d%d", &width, &height, &owner_id, &tick);

      if(width == -1 && height == -1) {
        fprintf(stderr, "%s", "strategy exited normally");
        exit(0);
      }

      if (create) {
        this -> field = Field(width, height);
      }

      for(int i = 0; i < height; i++) {
        scanf("\n");
        for(int j = 0; j < width; j++) {
          char c;
          scanf("%c", &c);
          field.cells[i][j] = Cell(c);
        }
      }

      // clean
      players.clear();
      // bombs.clear();
      // monsters.clear();

      int entities;
      scanf("%d", &entities);
      for(int i = 0; i < entities; i++) {
        char type;
        int owner_id, x, y, param1, param2;
        scanf("\n%c%d%d%d%d%d", &type, &owner_id, &x, &y, &param1, &param2);
        if (type == 'p') {
          players.pb(Player(x, y, owner_id, param1, param2));
          if (players.back().owner_id == owner_id) {
            this -> me = &players.back();
          }
        }
      }
      this -> tick = tick;
    }

    void next() {
      if(this -> tick == 1) {
        printf("right\n");
      } else
      if(this -> tick == 2) {
        printf("right\n");
      } else
      if(this -> tick == 3) {
        printf("bomb\n");
      } else
      if(this -> tick == 6) {
        printf("left\n");
      } else
      if(this -> tick == 7) {
        printf("left\n");
      } else
      if(this -> tick == 8) {
        printf("down\n");
      } else {
        printf("stay\n");
      }
      fflush(stdout);
      read_state();
    }

  
};

int main() {
  Game game;
  while(true) {
    game.next();
  }

  return 0;
}
