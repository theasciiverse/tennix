
# Experimental implementation of a Python-based Tennix AI Bot

import random
import time
import threading
import math

(INPUT_AXIS_X, INPUT_AXIS_Y) = range(2)
(INPUT_KEY_HIT, INPUT_KEY_TOPSPIN, INPUT_KEY_SMASH) = range(3)

__name__ = 'John Python'

class DumbPythonBot(object):
    SCREEN_MID_Y = 480./2.
    DESIRED_POWER = { INPUT_KEY_HIT: 90, INPUT_KEY_TOPSPIN: 70, INPUT_KEY_SMASH: 70 }

    def __init__(self):
        self.t = None
        self.cancelled = False
        self.ball_x = 0.0
        self.ball_y = 0.0
        self.player_x = 0.0
        self.player_y = 0.0
        self.power = 0.0
        self.power_locked = False
        self.current_tactic = INPUT_KEY_HIT
        self.desired_power = self.DESIRED_POWER[self.current_tactic]

    def blubb(self):
        i = 0
        while not self.cancelled:
            time.sleep(2)
            self.current_tactic = random.randint(INPUT_KEY_HIT, INPUT_KEY_SMASH)
            self.desired_power = self.DESIRED_POWER[self.current_tactic]*(1.2-0.4*random.random())
            print 'determined new tactic:', self.current_tactic
            i += 1
        print 'thread cancelled'

    def loaded(self):
        print __name__, 'has been loaded'
        self.t = threading.Thread(target=self.blubb)
        self.t.start()

    def unloaded(self):
        print __name__, 'has been unloaded'
        self.cancelled = True
        if self.t is not None:
            self.t.join()

    def get_key(self, key_id):
        if math.sqrt((self.player_x - self.ball_x)**2 + (self.player_y - self.ball_y)**2) > 150:
            return False

        if self.power > self.desired_power and random.randint(0, 3)==0:
            self.power_locked = True
            return False
        if key_id == self.current_tactic and not self.power_locked:
            #if abs(self.player_x - self.ball_x) < 10 and self.power > self.desired_power:
            return abs(self.player_x - self.ball_x) > 5 and self.power < self.desired_power + 0.3*random.random()
        else:
            return False

    def get_axis(self, axis_id):
        desired_position = self.ball_y - 20. * (self.ball_y - self.SCREEN_MID_Y) / self.SCREEN_MID_Y
        if axis_id == INPUT_AXIS_Y and abs(desired_position - self.player_y) > 5:
            return max(-1.0, min(1.0, desired_position - self.player_y))

        return 0.0

    def ball_moved(self, x, y):
        self.ball_x = x
        self.ball_y = y

    def player_moved(self, x, y):
        self.player_x = x
        self.player_y = y
        
    def power_changed(self, power):
        if power < max(0.05, self.desired_power - 0.8):
            self.power_locked = False
        self.power = power
    

bot = DumbPythonBot()

loaded = bot.loaded
unloaded = bot.unloaded
get_key = bot.get_key
get_axis = bot.get_axis
set_ball_coords = bot.ball_moved
set_player_coords = bot.player_moved
set_player_power = bot.power_changed

