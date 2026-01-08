#!/usr/bin/env python

'''
'''

__docformat__ = 'restructuredtext'
__version__ = '$Id: $'

from math import degrees, pi, radians
from condor_instruments import CondorInstruments
import pyglet
from pyglet.gl import *
from quaternion import euler_to_quaternion
from simulation import Simulation
from state_log import StateLog

joysticks = pyglet.input.get_joysticks()
assert joysticks, 'No joystick device is connected'
joystick = joysticks[0]
joystick.open()

window = pyglet.window.Window()

# Create grid of labels
idx = 0
labels = []
h = window.height
for iy in range(0,5) :
    for ix in range(0,3) :
        xpos = 20 + ix * 400
        ypos = h - (20 + iy * 70)
        label = pyglet.text.Label('Hello, world',
                                font_name='Times New Roman',
                                font_size=36,
                                x=xpos, y=ypos,
                                anchor_x='left', anchor_y='top')
        labels.append(label)


sim = Simulation()
sim.state.set_position((0.0, 0.0, -1000.0))  # Start at 1000m altitude
sim.state.set_velocity((30.0, 0.0, 0.0))      # Initial forward speed 30 m/s
sim.state.set_orientation(euler_to_quaternion(0, radians(1.5),0))  # Pointing north, slight pitch up
sim.state.set_angular_velocity((0.0, 0.0, 0.0))  # No initial rotation

instruments = CondorInstruments("localhost",55278)

log = StateLog('state.txt')

count:int = 0

@window.event
def on_draw():
    # Axes
    window.clear()
    for label in labels : 
        label.draw()
    
def update(dt):

    # Read joystick inputs
    roll = joystick.x
    pitch = joystick.y
    spoiler = (joystick.z +1) /2  # normalize to [0,1]
    rudder = joystick.rz # rudder
    #roll = rudder = 0 # TODO remove!!
    sim.controls.set_controls(pitch, roll, rudder, spoiler)

    # Update simulation
    state = sim.update(dt)

    log.write(sim.total_time, state)

    #label.text = f'roll: {roll:.2f}, pitch: {pitch:.2f} rudder: {rudder:.2f} spoiler: {spoiler:.2f}'
    pos = state.position()
    v = state.velocity()
    att = state.Attitude()

    roll = att[0]
    pitch = att[1]
    yaw = att[2]
    if(roll > pi): roll -= 2*pi
    if(pitch > pi): pitch -= 2*pi
    if(yaw > pi): yaw -= 2*pi

    pitch = degrees(pitch)
    roll = degrees(roll)
    yaw = degrees(yaw)

    labels[0].text = f'Roll: {roll:4.2f}'
    labels[1].text = f'Pitch: {pitch:4.2f}'
    labels[2].text = f'Yaw: {yaw:4.2f}'

    labels[3].text = f'Tas: {state.TotalAirspeed():.1f} m/s'
    labels[4].text = f'Alt: {state.Altitude():.1f} m'
    labels[5].text = f'Heading: {state.Heading():.1f}°'
    #label2.text = f'pos: x={pos[0]:.1f} y={pos[1]:.1f} z={pos[2]:.1f}'

    labels[6].text = f'vx={v[0]:.1f}'
    labels[7].text = f'vy={v[1]:.1f}'
    labels[8].text = f'vz={v[2]:.1f}'

    labels[9].text = f'fx={state.forces[0]:.1f}'
    labels[10].text = f'fy={state.forces[1]:.1f}'
    labels[11].text = f'fz={state.forces[2]:.1f}'

    labels[12].text = f'M roll={state.moments[0]:.1f}'
    labels[13].text = f'M pitch={state.moments[1]:.1f}'
    labels[14].text = f'M yaw={state.moments[2]:.1f}'

    
    global count
    count += 1
    if(count == 2) :
        count = 0
        instruments.set(sim.total_time, sim.state)
        g = state.forces[2] / sim.aircraft.mass / 9.81
        instruments.gforce = -g
        instruments.send()


pyglet.clock.schedule_interval(update, sim.time_step)
pyglet.app.run()
