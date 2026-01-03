#!/usr/bin/env python

'''
'''

__docformat__ = 'restructuredtext'
__version__ = '$Id: $'

from math import radians
import pyglet
from pyglet.gl import *
from quaternion import euler_to_quaternion
from simulation import Simulation

joysticks = pyglet.input.get_joysticks()
assert joysticks, 'No joystick device is connected'
joystick = joysticks[0]
joystick.open()

window = pyglet.window.Window()

label1 = pyglet.text.Label('Hello, world',
                          font_name='Times New Roman',
                          font_size=36,
                          x=window.width//2, y=window.height//4,
                          anchor_x='center', anchor_y='center')

label2 = pyglet.text.Label('Hello, world 2',
                          font_name='Times New Roman',
                          font_size=36,
                          x=window.width//2, y=window.height//2,
                          anchor_x='center', anchor_y='center')

label3 = pyglet.text.Label('Hello, world 3',
                          font_name='Times New Roman',
                          font_size=36,
                          x=window.width//2, y=window.height//4*3,
                          anchor_x='center', anchor_y='center')

sim = Simulation()
sim.state.set_position((0.0, 0.0, -1000.0))  # Start at 1000m altitude
sim.state.set_velocity((30.0, 0.0, 0.0))      # Initial forward speed 30 m/s
sim.state.set_orientation(euler_to_quaternion(0, radians(1.5),0))  # Pointing north, slight pitch up
sim.state.set_angular_velocity((0.0, 0.0, 0.0))  # No initial rotation

@window.event
def on_draw():
    # Axes
    window.clear()
    label1.draw()
    label2.draw()
    label3.draw()
    
def update(dt):

    # Read joystick inputs
    roll = joystick.x
    pitch = joystick.y
    spoiler = (joystick.z +1) /2  # normalize to [0,1]
    rudder = joystick.rz # rudder
    sim.controls.set_controls(pitch, roll, rudder, spoiler)

    # Update simulation
    state = sim.update()

    #label.text = f'roll: {roll:.2f}, pitch: {pitch:.2f} rudder: {rudder:.2f} spoiler: {spoiler:.2f}'
    pos = state.position()
    v = state.velocity()
    att = state.Attitude()

    label1.text = f'Roll: {att[0]:4.2f}, Pitch: {att[1]:4.2f}'
    label2.text = f'pos: x={pos[0]:.1f} y={pos[1]:.1f} z={pos[2]:.1f}'
    label3.text = f'Tas: {state.TotalAirspeed():.1f} m/s, Alt: {state.Altitude():.1f} m, Heading: {state.Heading():.1f}°'

pyglet.clock.schedule_interval(update, sim.time_step)
pyglet.app.run()
