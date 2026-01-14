# Winch Launch Model

This document describes the winch launch simulation model implemented in `winch.py`.

## Overview

The winch model simulates a ground-based winch that reels in a cable attached to the glider's winch hook, providing the force for a winch launch. This is a common method of launching gliders where a powerful motor-driven drum winds in a steel cable attached to the glider, pulling it into the air.

## Physics Model

### Cable Geometry

The winch is positioned at a fixed location in the earth frame (default: 1000m ahead on the X-axis). The cable runs from the winch drum to a hook on the glider's fuselage. The cable direction vector points from the hook towards the winch.

The cable force acts along the cable direction, pulling the hook towards the winch. This creates both:
- A direct force on the glider (accelerating it forward and upward)
- A pitching moment about the CG (due to the hook being offset from the CG)

### Tension Calculation

The winch uses a power-limited drum model:

1. **Drum Speed Target**: The winch drum attempts to reel in cable at a constant target speed (default: 30 m/s)

2. **Speed Error**: The difference between the target drum speed and the actual cable speed (velocity component of the hook along the cable direction)

3. **Power Limiting**: When the cable is moving slower than the target:
   - Tension is limited by available power: `T <= Power / v`
   - Also limited by maximum tension setting

4. **Slack Cable**: When the cable moves faster than the drum, minimal tension (100N) keeps the cable taut

### Hook Velocity

The velocity of the winch hook accounts for both:
- CG velocity (u, v, w)
- Rotational contribution: `V_hook = V_cg + omega x r_hook`

## Configuration Parameters

| Parameter | Default | Description |
|-----------|---------|-------------|
| `winch_position` | (1000, 0, 0) | Winch location in earth frame (m) |
| `max_tension` | 6000 N | Maximum cable tension from winch power |
| `weak_link` | 8000 N | Tension at which weak link breaks |
| `cable_length` | 1200 m | Total cable length on drum |
| `drum_speed` | 30 m/s | Target cable reel-in speed |
| `drum_power` | 150000 W | Winch motor power (~200 HP) |

## Safety Features

### Back-Release

If the cable direction has a negative X component in the glider's body frame (i.e., pulling backwards), the cable automatically releases. This simulates the back-release mechanism that prevents the cable from pulling the glider's nose down if it gets behind the aircraft.

**Trigger condition**: `cable_body[0] < 0`

### Weak Link

A weak link is included in the cable system. If the tension exceeds the weak link strength (default: 8000N), the link breaks and the cable releases. This protects the glider from excessive loads.

**Trigger condition**: `tension > weak_link`

### Cable Run Out

When the glider travels far enough that the cable distance exceeds the total cable length, the cable releases automatically (simulating the cable coming off the drum).

**Trigger condition**: `cable_out > cable_length`

## API Reference

### Constructor

```python
Winch(winch_position=(1000.0, 0.0, 0.0),
      max_tension=6000.0,
      weak_link=8000.0,
      cable_length=1200.0)
```

### Methods

#### `engage(initial_cable_out=None)`
Engage the winch cable. If `initial_cable_out` is not provided, the cable length is calculated from the glider's current position.

#### `release(reason="manual")`
Release the cable. The reason is stored for diagnostic purposes.

#### `calculate_forces(state, hook_position_body, dt)`
Calculate cable forces and moments for the current timestep.

**Parameters:**
- `state`: StateVector - Current aircraft state
- `hook_position_body`: tuple - Winch hook position in body frame (m)
- `dt`: float - Time step (s)

**Returns:**
- `forces_body`: tuple (Fx, Fy, Fz) - Cable force in body frame (N)
- `moments_body`: tuple (L, M, N) - Moment about CG in body frame (N.m)
- `info`: dict - Diagnostic information

#### `get_status()`
Returns a dictionary with current winch status:
- `engaged`: bool
- `tension`: float (N)
- `cable_out`: float (m)
- `release_reason`: str or None

## Diagnostic Information

The `calculate_forces` method returns an `info` dictionary containing:

| Key | Type | Description |
|-----|------|-------------|
| `engaged` | bool | Whether cable is engaged |
| `cable_out` | float | Current cable length from winch to hook (m) |
| `tension` | float | Current cable tension (N) |
| `cable_angle_deg` | float | Cable angle from horizontal (degrees) |
| `back_release` | bool | True if back-release triggered |
| `weak_link_break` | bool | True if weak link broke |
| `cable_run_out` | bool | True if cable ran out |

## Typical Launch Profile

1. **Ground Run**: Glider accelerates along the runway. Cable angle is shallow, tension provides mostly forward acceleration.

2. **Rotation**: As speed builds, the glider rotates and begins climbing. Cable angle increases.

3. **Full Climb**: Glider climbs steeply (typically 30-45 degrees). Cable tension provides both lift and forward force.

4. **Top of Launch**: As the glider approaches the winch, the cable angle becomes too steep. Pilot releases the cable (or back-release triggers).

5. **Release**: Cable releases, glider continues in free flight at 300-400m altitude.

## Keyboard Controls

In `joystick.py`, the following keyboard controls are available:

| Key | Action |
|-----|--------|
| **R** | Reset simulation to stationary on ground |
| **W** | Initiate winch launch (resets to ground first) |

The winch launch sets up the winch 1000m ahead with default tension parameters.

## Integration Notes

To integrate with the simulation:

1. Create a `Winch` instance with appropriate parameters
2. Define the hook position in the glider's body frame
3. Call `engage()` to start the launch
4. In each simulation step, call `calculate_forces()` and add the returned forces/moments to the aircraft equations of motion
5. The cable will auto-release when appropriate, or call `release()` for manual release
