#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
typedef int image_id;
#define WIDTH 640
#define HEIGHT 480
#define tnx_assert(x) do { if (!(x)) { fprintf(stderr, "assert %s\n", #x); exit(1);} } while(0)
#include "gh.h"
GameState *gamestate_new();
void step(GameState*); float get_move_y(GameState*, unsigned char); void input_ai(GameState*, int);
void game_setup_serve(GameState*); int score_game(GameState*); winner_t game_get_winner(const GameState*);
static long T;
static GameState* G;
static float pat_axis() { float target = G->ball.y - G->ball.z + (float)((T/300)%7 - 3) * 7.0f; float d = target - G->players[0].y; return d > 3.0f ? 1.0f : (d < -3.0f ? -1.0f : 0.0f); }
static bool pat_key(int k) { int d = (T/2000)%3; if (G->ball.inhibit_gravity) return (T%60) < 40 && k == d; return fabsf(G->ball.x - G->players[0].x) > 40.0f + (float)((T/500)%4)*10.0f && k == d; }
#include "fns.inc"
#include "human.inc"
int main(int argc, char** argv) {
    long n = argc > 1 ? atol(argv[1]) : 200000;
    static Location loc = {"x","x","x",0,6000,"x",0,1,0,0,true,false,0,0};
    GameState gs; memset(&gs, 0, sizeof gs); G = &gs;
    gs.location = &loc; gs.ball.last_hit_by = -1;
    gs.players[0].x = GAME_X_MIN-RACKET_X_MID*2; gs.players[0].y = GAME_Y_MID; gs.players[0].use_power = true; gs.players[0].type = PLAYER_TYPE_AI; gs.players[0].accelerate = PLAYER_ACCEL_DEFAULT;
    gs.players[1] = gs.players[0]; gs.players[1].x = GAME_X_MAX+RACKET_X_MID*2;
    gs.serving_player = 1; gs.status_message = STATUSMSG_WELCOME;
    game_setup_serve(&gs);
    int hits = 0; int last = -1;
    for (long t = 0; t < n; t++) {
        step(&gs);
        if (gs.ball.last_hit_by != last && gs.ball.last_hit_by != -1) hits++;
        last = gs.ball.last_hit_by;
        T = t; if (argc > 3) gs.players[0].type = PLAYER_TYPE_HUMAN;
        if (gs.winner == WINNER_NONE) for (int p = 1; p <= 2; p++) { if (gs.players[p-1].type == PLAYER_TYPE_HUMAN) input_human(&gs, p); else input_ai(&gs, p);
            if (gs.players[p-1].y < PLAYER_Y_MIN) gs.players[p-1].y = PLAYER_Y_MIN; else if (gs.players[p-1].y > PLAYER_Y_MAX) gs.players[p-1].y = PLAYER_Y_MAX; }
        if (argc > 2 && t % atol(argv[2]) == 0) printf("%ld ball %.3f %.3f %.3f p1 %.3f %.1f p2 %.3f %.1f games %d %d sets %d:%d set %d\n", t, gs.ball.x, gs.ball.y, gs.ball.z, gs.players[0].y, gs.players[0].power, gs.players[1].y, gs.players[1].power, gs.players[0].game, gs.players[1].game, gs.players[0].sets[gs.current_set<5?gs.current_set:4], gs.players[1].sets[gs.current_set<5?gs.current_set:4], gs.current_set);
    }
    printf("hits %d winner %d set %d games %d %d sets %d:%d\n", hits, gs.winner, gs.current_set, gs.players[0].game, gs.players[1].game, gs.players[0].sets[0], gs.players[1].sets[0]);
}
