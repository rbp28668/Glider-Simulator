#!/usr/bin/env python

'''
'''

__docformat__ = 'restructuredtext'
__version__ = '$Id: $'

from math import degrees, pi, radians
import math
from condor_instruments import CondorInstruments
import pyglet
from pyglet.gl import *
from pyglet.window import key
from quaternion import euler_to_quaternion
from simulation import Simulation
from state_log import StateLog
from artificial_horizon import (
    ArtificialHorizon, AOAIndicator, AirspeedIndicator,
    VerticalSpeedIndicator, SideslipIndicator, ControlPositionIndicator
)
from controls import JoystickControls, MouseControls

window = pyglet.window.Window(width=1200, height=800)

# Try to use joystick, fall back to mouse if not available
try:
    controls = JoystickControls()
    print("Using joystick controls")
except RuntimeError:
    controls = MouseControls(window)
    print("No joystick found, using mouse controls")

# Create instruments (positioned in top half, 20% larger)
horizon = ArtificialHorizon(x=700, y=530, radius=144)
aoa_indicator = AOAIndicator(x=520, y=530, width=36, height=180)
airspeed_indicator = AirspeedIndicator(x=420, y=640, radius=72)
vsi = VerticalSpeedIndicator(x=980, y=640, radius=72)
sideslip = SideslipIndicator(x=700, y=350, width=144, height=36)
control_indicator = ControlPositionIndicator(x=1100, y=530, size=120)

# Instrument labels
instrument_labels = [
    pyglet.text.Label('ATTITUDE', font_name='Arial', font_size=14,
                      x=700, y=680, anchor_x='center', anchor_y='bottom'),
    pyglet.text.Label('AOA', font_name='Arial', font_size=14,
                      x=520, y=680, anchor_x='center', anchor_y='bottom'),
    pyglet.text.Label('AIRSPEED', font_name='Arial', font_size=14,
                      x=420, y=720, anchor_x='center', anchor_y='bottom'),
    pyglet.text.Label('VSI', font_name='Arial', font_size=14,
                      x=980, y=720, anchor_x='center', anchor_y='bottom'),
    pyglet.text.Label('SLIP', font_name='Arial', font_size=14,
                      x=700, y=380, anchor_x='center', anchor_y='bottom'),
    pyglet.text.Label('CONTROLS', font_name='Arial', font_size=14,
                      x=1100, y=600, anchor_x='center', anchor_y='bottom'),
    pyglet.text.Label('R', font_name='Arial', font_size=10,
                      x=1100, y=435, anchor_x='center', anchor_y='top'),
    pyglet.text.Label('S', font_name='Arial', font_size=10,
                      x=1190, y=530, anchor_x='center', anchor_y='center'),
]

# Numeric value labels below ASI and VSI
asi_value_label = pyglet.text.Label('0 m/s', font_name='Arial', font_size=16,
                                     x=420, y=555, anchor_x='center', anchor_y='top',
                                     color=(255, 255, 255, 255))
vsi_value_label = pyglet.text.Label('0 m/s', font_name='Arial', font_size=16,
                                     x=980, y=555, anchor_x='center', anchor_y='top',
                                     color=(255, 255, 255, 255))

# Create grid of telemetry labels (positioned in bottom half, reduced size)
idx = 0
labels = []
for iy in range(0, 5):
    for ix in range(0, 3):
        xpos = 20 + ix * 400
        ypos = 300 - iy * 50  # Start from y=300, smaller spacing
        label = pyglet.text.Label('Hello, world',
                                  font_name='Arial',
                                  font_size=24,
                                  x=xpos, y=ypos,
                                  anchor_x='left', anchor_y='top')
        labels.append(label)


sim = Simulation()
sim.state.set_position((0.0, 0.0, -1000.0))  # Start at 1000m altitude
sim.state.set_velocity((30.0, 0.0, 0.0))      # Initial forward speed 30 m/s
sim.state.set_orientation(euler_to_quaternion(0, radians(1.5),0))  # Pointing north, slight pitch up
sim.state.set_angular_velocity((0.0, 0.0, 0.0))  # No initial rotation

instruments = CondorInstruments("localhost",55278)


def reset_to_ground():
    """Reset simulation to stationary on ground."""
    sim.reset()
    # Position at equilibrium - determined from test harness
    # Main wheel penetration ~0.095m, CG height ~0.643m
    sim.state.set_position((0.0, 0.0, -0.643))
    sim.state.set_velocity((0.0, 0.0, 0.0))
    
    # Equilibrium pitch from test: ~0.63 degrees
    sim.state.set_orientation(euler_to_quaternion(0, radians(0.63), 0))
    sim.state.set_angular_velocity((0.0, 0.0, 0.0))
    # Ensure winch is disengaged
    sim.winch.release("reset")
    print("Reset to ground")


def start_winch_launch():
    """Reset to ground and initiate winch launch."""
    reset_to_ground()
    # Setup winch 1000m ahead
    sim.setup_winch_launch(winch_distance=1000.0, max_tension=6000.0, weak_link=8000.0)
    sim.engage_winch()
    print("Winch launch initiated")

def restart_in_air():
    """Reset to 1000m altitude and flying at 30 m/s."""
    sim.reset()
    sim.state.set_position((0.0, 0.0, -1000.0))  # 1000m altitude
    sim.state.set_velocity((30.0, 0.0, 0.0))      # Initial forward speed 30 m/s
    sim.state.set_orientation(euler_to_quaternion(0, radians(1.5),0))  # Pointing north, slight pitch up
    sim.state.set_angular_velocity((0.0, 0.0, 0.0))  # No initial rotation
    print("Restarted in air at 1000m")


@window.event
def on_key_press(symbol, modifiers):
    if symbol == key.R:
        reset_to_ground()
    elif symbol == key.W:
        start_winch_launch()
    elif symbol == key.F:
        restart_in_air()
        
log = StateLog('state.txt')

count:int = 0

@window.event
def on_draw():
    window.clear()

    # Draw instruments
    horizon.draw()
    aoa_indicator.draw()
    airspeed_indicator.draw()
    vsi.draw()
    sideslip.draw()
    control_indicator.draw()

    # Draw instrument labels
    for label in instrument_labels:
        label.draw()

    # Draw numeric values
    asi_value_label.draw()
    vsi_value_label.draw()

    # Draw telemetry labels
    for label in labels:
        label.draw()
    
def update(dt):
    # Clamp dt to prevent instability from large timesteps (e.g., on first frame or lag)
    dt = min(dt, 0.05)  # Max 50ms per step

    # Read joystick inputs
    controls.update()
    ctrl_pitch, ctrl_roll, ctrl_rudder, ctrl_spoiler = controls.get_controls()
    sim.controls.set_controls(ctrl_pitch, ctrl_roll, ctrl_rudder, ctrl_spoiler)

    # Update control position indicator
    control_indicator.update(ctrl_roll, ctrl_pitch, ctrl_rudder, ctrl_spoiler)

    # Update simulation
    state = sim.update(dt)

    log.write(sim.total_time, state)

    #label.text = f'roll: {roll:.2f}, pitch: {pitch:.2f} rudder: {rudder:.2f} spoiler: {spoiler:.2f}'
    pos = state.position()
    v = state.velocity()
    att = state.Attitude()

    alpha = math.degrees(math.atan2(v[2],v[0]))

    roll = att[0]
    pitch = att[1]
    hdg = att[2]

    if(roll > pi): roll -= 2*pi
    if(pitch > pi): pitch -= 2*pi
    
    pitch = degrees(pitch)
    roll = degrees(roll)
    yaw = degrees(-state.SideslipAngle()) # yawing right is positive sideslip, but negative yaw angle convention, so invert sign
    hdg = hdg * 180/pi
    if hdg < 0:
        hdg += 360



    labels[0].text = f'Roll: {roll:4.2f}'
    labels[1].text = f'Pitch: {pitch:4.2f}'
    labels[2].text = f'Yaw: {yaw:4.2f}'

    labels[3].text = f'Tas: {state.TotalAirspeed():.1f} m/s'
    labels[4].text = f'Alt: {state.Altitude():.1f} m'
    #labels[5].text = f'Heading: {hdg:.1f}°'
    labels[5].text = f'Alpha: {alpha:.2f}°'
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

    # Calculate sideslip angle (beta)
    beta = math.degrees(math.atan2(v[1], v[0])) if v[0] != 0 else 0

    # Update instruments
    horizon.update(pitch, roll)
    aoa_indicator.update(alpha)
    airspeed = state.TotalAirspeed()
    vertical_speed = state.VerticalSpeed()
    airspeed_indicator.update(airspeed)
    vsi.update(vertical_speed)
    sideslip.update(beta)

    # Update numeric value labels
    asi_value_label.text = f'{airspeed:.1f} m/s'
    vsi_value_label.text = f'{vertical_speed:+.1f} m/s'
    
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
