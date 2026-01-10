import pyglet

class ControlInputs:
    def __init__(self):
        self.pitch = 0.0  # [-1, +1] fwd/aft
        self.roll = 0.0       # [-1, +1] left/right
        self.rudder = 0.0        # [-1, +1] left/right
        self.spoiler = 0.0       # [0, 1] retracted/extended
        
    def set_controls(self, longitudinal, lateral, rudder, spoiler):
        self.pitch = max(-1.0, min(1.0, longitudinal))
        self.roll = max(-1.0, min(1.0, lateral))
        self.rudder = max(-1.0, min(1.0, rudder))
        self.spoiler = max(0.0, min(1.0, spoiler))

       

 