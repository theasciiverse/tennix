# Pulls the match functions out of Thomas Perl's c/src/game.cc and game.h
# for harness.cc: the game state types and constants (gh.h), step,
# get_move_y, input_ai, game_setup_serve, score_game and game_get_winner
# (fns.inc), and input_human with its device reads replaced by the
# harness's scripted player (human.inc).
import sys, re
src, out = sys.argv[1], sys.argv[2]
h = open(f"{src}/game.h").read()
a = h.index("#define SETS_TO_WIN"); b = h.index("/* GameState handling")
open(f"{out}/gh.h", "w").write("\n".join(l for l in h[a:b].splitlines() if "InputDevice* input;" not in l) + "\n")
c = open(f"{src}/game.cc").read()
def fn(sig):
    i = c.index(sig); j = c.index("\n}\n", i) + 3; return c[i:j]
sigs = ["void step(GameState* s)", "float get_move_y(", "void input_ai(", "void game_setup_serve(", "int score_game(", "winner_t game_get_winner("]
open(f"{out}/fns.inc", "w").write("".join(fn(s) for s in sigs))
f = fn("void input_human(")
f = f[:f.index("    /* For mouse input")] + f[f.index("    move_y = PLAYER_MOVE_Y"):]
f = f.replace("input_device_get_axis(PLAYER(s, player).input, INPUT_AXIS_Y)", "pat_axis()")
for k, n in [("HIT", 0), ("TOPSPIN", 1), ("SMASH", 2)]:
    f = f.replace(f"input_device_get_key(PLAYER(s, player).input, INPUT_KEY_{k})", f"pat_key({n})")
open(f"{out}/human.inc", "w").write(f)
