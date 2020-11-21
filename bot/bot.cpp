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

class Player {
  public:
    int x, y;
    int owner_id;
    int max_bombs, bombs, range;
};

enum CellType {empty, box, block, bomb};

class Cell {
  public:
    CellType type;
};

class Field {
  public:
    vector<vector<Cell>> cells;
};

class Game {
  private:
    // Player *me;
    // Field field
    // vector<Player> players;
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
      for(int i = 0; i < height; i++) {
        scanf("\n");
        for(int j = 0; j < width; j++) {
          scanf("%c", &field[i][j]);
        }
      }

      int entities;
      scanf("%d", &entities);
      for(int i = 0; i < entities; i++) {
        char type;
        int owner_id, x, y, param1, param2;
        scanf("\n%c%d%d%d%d%d", &type, &owner_id, &x, &y, &param1, &param2);
      }
      this -> tick = tick;
    }

  
};

int main() {
  while(true) {

    Game game;
    int tick = game.tick;
    if(tick == 1) {
      printf("right\n");
    } else
    if(tick == 2) {
      printf("right\n");
    } else
    if(tick == 3) {
      printf("bomb\n");
    } else
    if(tick == 6) {
      printf("left\n");
    } else
    if(tick == 7) {
      printf("left\n");
    } else
    if(tick == 8) {
      printf("down\n");
    } else {
      printf("stay\n");
    }
    fflush(stdout);

    // int w, h, ow, tt;
    // scanf("%d %d %d %d", &w, &h, &ow, &tt);
    // for(int i = 0; i < h; i++) {
    //   scanf("\n");
    //   for(int j = 0; j < w; j++) {
    //     char c;
    //     scanf("%c", &c);
    //     cerr << c << " ";
    //   }
    //   cerr << endl;
    // }
    // cerr << "field fi" << endl;
    // int e;
    // scanf("%d", &e);
    // cerr << e << endl;
    // for(int i = 0; i < e; i++) {
    //   scanf("\n%*c %*d %*d %*d %*d %*d");
    // }
    // if(tick == 1) {
    //   printf("right\n");
    // } else
    // if(tick == 2) {
    //   printf("right\n");
    // } else
    // if(tick == 3) {
    //   printf("bomb\n");
    // } else
    // if(tick == 7) {
    //   printf("left\n");
    // } else
    // if(tick == 8) {
    //   printf("left\n");
    // } else
    // if(tick == 9) {
    //   printf("down\n");
    // } else {
    //   printf("stay\n");
    // }
    // fflush(stdout);
    // tick++;
  }

  return 0;
}
