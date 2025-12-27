class ControlInputs:
    def __init__(self):
        self.stick_longitudinal = 0.0  # [-1, +1] fwd/aft
        self.stick_lateral = 0.0       # [-1, +1] left/right
        self.rudder_pedal = 0.0        # [-1, +1] left/right
        self.spoiler_lever = 0.0       # [0, 1] retracted/extended
        
    def set_controls(self, longitudinal, lateral, rudder, spoiler):
        self.stick_longitudinal = max(-1.0, min(1.0, longitudinal))
        self.stick_lateral = max(-1.0, min(1.0, lateral))
        self.rudder_pedal = max(-1.0, min(1.0, rudder))
        self.spoiler_lever = max(0.0, min(1.0, spoiler))

        