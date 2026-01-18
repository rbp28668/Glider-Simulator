#!/usr/bin/env python

'''
Control input classes for flight simulation.
'''

import pyglet


class Controls:
    """Base class for control inputs."""
    
    def __init__(self):
        self.roll = 0.0
        self.pitch = 0.0
        self.rudder = 0.0
        self.spoiler = 0.0
        
    def update(self):
        """Update control values. Override in subclasses."""
        pass
    
    def get_controls(self):
        """Return current control values as tuple (pitch, roll, rudder, spoiler)."""
        return (self.pitch, self.roll, self.rudder, self.spoiler)


class JoystickControls(Controls):
    """Control inputs from a physical joystick."""
    
    def __init__(self):
        super().__init__()
        joysticks = pyglet.input.get_joysticks()
        if not joysticks:
            raise RuntimeError('No joystick device is connected')
        self.joystick = joysticks[0]
        self.joystick.open()
        
    def update(self):
        """Read current joystick inputs."""
        self.roll = self.joystick.x
        self.pitch = self.joystick.y
        self.spoiler = (self.joystick.z + 1) / 2  # normalize to [0,1]
        self.rudder = self.joystick.rz


class MouseControls(Controls):
    """Control inputs from mouse movements."""
    
    def __init__(self, window, sensitivity=0.003):
        super().__init__()
        self.window = window
        self.sensitivity = sensitivity
        self.mouse_x = window.width // 2
        self.mouse_y = window.height // 2
        self.center_x = window.width // 2
        self.center_y = window.height // 2
        
        # Keyboard state for rudder and spoiler
        self.keys = pyglet.window.key.KeyStateHandler()
        window.push_handlers(self.keys)
        
        # Mouse event handlers
        @window.event
        def on_mouse_motion(x, y, dx, dy):
            self.mouse_x = x
            self.mouse_y = y
            
    def update(self):
        """Calculate control inputs from mouse position."""
        # Calculate offset from center
        dx = self.mouse_x - self.center_x
        dy = self.mouse_y - self.center_y
        
        # Convert to control inputs (-1 to 1 range)
        self.roll = max(-1.0, min(1.0, dx * self.sensitivity))
        self.pitch = max(-1.0, min(1.0, -dy * self.sensitivity))  # Inverted Y
        
        # Rudder from keyboard (A/D or Left/Right arrows)
        rudder_input = 0.0
        if self.keys[pyglet.window.key.A] or self.keys[pyglet.window.key.LEFT]:
            rudder_input -= 1.0
        if self.keys[pyglet.window.key.D] or self.keys[pyglet.window.key.RIGHT]:
            rudder_input += 1.0
        self.rudder = rudder_input
        
        # Spoiler from keyboard (W/S or Up/Down arrows)
        if self.keys[pyglet.window.key.W] or self.keys[pyglet.window.key.UP]:
            self.spoiler = max(0.0, self.spoiler - 0.02)
        if self.keys[pyglet.window.key.S] or self.keys[pyglet.window.key.DOWN]:
            self.spoiler = min(1.0, self.spoiler + 0.02)

