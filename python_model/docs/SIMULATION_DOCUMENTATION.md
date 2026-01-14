# ASK-21 Glider Flight Simulation Documentation

## Overview

This document describes the flight dynamics simulation for an ASK-21 two-seat training glider. The simulation implements a full 12-degree-of-freedom (12-DOF) rigid body model with realistic aerodynamic modeling, including multi-panel wing representation, control surface effects, and environmental factors.

---

## Table of Contents

1. [Coordinate Systems](#coordinate-systems)
2. [State Vector](#state-vector)
3. [Physics Model](#physics-model)
4. [Aerodynamic Model](#aerodynamic-model)
5. [Aircraft Parameters](#aircraft-parameters)
6. [Numerical Integration](#numerical-integration)
7. [File Structure](#file-structure)

---

## Coordinate Systems

The simulation uses two primary coordinate frames with the North-East-Down (NED) convention. The earth is considered to be locally flat.

### Earth Frame (Inertial Reference)

| Axis | Direction | Notes |
|------|-----------|-------|
| X | North | Positive towards geographic north |
| Y | East | Positive towards geographic east |
| Z | Down | Positive towards Earth's center |

**Key implications:**
- Altitude = -Z (flying at 1000m means Z = -1000)
- Position is relative to an arbitrary origin point

### Body Frame (Aircraft-Fixed)

| Axis | Direction | Notes |
|------|-----------|-------|
| X | Forward | Along aircraft longitudinal axis |
| Y | Right | Towards starboard wing |
| Z | Down | Perpendicular to XY plane |

**Sign conventions:**
- Roll right (starboard wing down) = positive roll angle/rate (p), clockwise around +ve X axis
- Pitch up (nose up) = positive pitch angle/rate (q), clockwise around +ve Y axis
- Yaw right (nose right) = positive yaw angle/rate (r), clockwise around +ve Z axis

### Frame Transformations

Transformations between frames use unit quaternions to avoid gimbal lock:

```
v_earth = q * v_body * q^(-1)
v_body = q^(-1) * v_earth * q
```

Where `q` is the orientation quaternion and `*` denotes quaternion multiplication.

---

## State Vector

The 12-DOF state vector contains all information needed to describe the aircraft's instantaneous condition.

### State Components

| Index | Symbol | Description | Units | Frame |
|-------|--------|-------------|-------|-------|
| 0-2 | X, Y, Z | Position | m | Earth |
| 3-5 | u, v, w | Velocity | m/s | Body |
| 6-9 | qw, qx, qy, qz | Orientation quaternion | - | - |
| 10-12 | p, q, r | Angular velocity | rad/s | Body |

### Quaternion Convention

The orientation quaternion follows the scalar-first convention:

```
q = [qw, qx, qy, qz]
```

With the constraint: `qw^2 + qx^2 + qy^2 + qz^2 = 1`

The quaternion relates to Euler angles (ZYX order: yaw-pitch-roll) as:

```
Roll (phi)   = atan2(2(qw*qx + qy*qz), 1 - 2(qx^2 + qy^2))
Pitch (theta) = asin(2(qw*qy - qz*qx))
Yaw (psi)    = atan2(2(qw*qz + qx*qy), 1 - 2(qy^2 + qz^2))
```

### Derived Quantities

From the state vector, the following quantities are computed:

| Quantity | Formula | Description |
|----------|---------|-------------|
| V | sqrt(u^2 + v^2 + w^2) | Total airspeed |
| alpha | atan2(w, u) | Angle of attack |
| beta | asin(v / V) | Sideslip angle |

---

## Physics Model

### Linear Motion (Newton's Second Law in Body Frame)

The equations of motion account for rotation of the body frame:

```
u_dot = Fx/m + r*v - q*w + gx_body
v_dot = Fy/m + p*w - r*u + gy_body
w_dot = Fz/m + q*u - p*v + gz_body
```

Where:
- `Fx, Fy, Fz` = Total aerodynamic forces (N)
- `m` = Aircraft mass (kg)
- `(r*v - q*w, ...)` = Coriolis acceleration terms
- `g_body` = Gravity vector transformed to body frame

### Angular Motion (Euler's Equations)

The rotational dynamics include gyroscopic coupling effects:

```
I * omega_dot = M - omega x (I * omega)
```

Expanded for the inertia tensor with Ixz cross-coupling:

```
p_dot = [Izz*(L + gyro_L) + Ixz*(N + gyro_N)] / det(I)
q_dot = (M + gyro_M) / Iyy
r_dot = [Ixz*(L + gyro_L) + Ixx*(N + gyro_N)] / det(I)
```

Gyroscopic coupling terms:
```
gyro_L = (Izz - Iyy)*q*r - Ixz*p*q
gyro_M = (Ixx - Izz)*p*r + Ixz*(p^2 - r^2)
gyro_N = (Iyy - Ixx)*p*q + Ixz*q*r
det(I) = Ixx*Izz - Ixz^2
```

### Position Update

Position is integrated in the Earth frame:

```
[X_dot, Y_dot, Z_dot] = quaternion_rotate(q, [u, v, w])
```

### Orientation Update

The quaternion derivative is:

```
q_dot = 0.5 * q * [0, p, q, r]
```

Where the multiplication uses quaternion algebra.

---

## Aerodynamic Model

### Multi-Panel Wing Representation

The ASK-21 wing is divided into 5 panels per side to capture spanwise variations:

| Panel | Area (m^2) | Features |
|-------|------------|----------|
| Root | 3.215 | Largest panel, near fuselage |
| Airbrake | 1.649 | Contains spoiler/airbrake |
| Outer | 1.112 | Transition region |
| Aileron | 2.217 | Differential aileron control |
| Tip | 0.288 | Smallest, at wing tip |

### Panel Force Calculation

For each panel, the following process computes forces:

**1. Local Airflow Velocity**

Accounts for angular velocity effects on local wing section:

```
u_local = u - r * y_span
v_local = v
w_local = w + p * y_span
```

Where `y_span` is the spanwise distance from centerline.

**2. Local Angle of Attack**

```
alpha_local = atan2(w_local, u_local)
alpha_effective = alpha_local * cos(beta) + atan2(v,u) * sin(beta) + dihedral
```

**3. Aerodynamic Coefficients**

Coefficients are obtained via cubic spline interpolation of aerofoil polar data:

```
(Cl, Cd, Cm) = aerofoil.coefficients_at(alpha)
```

Coefficients are blended between root (FX-60-126) and tip (FX-02-196) aerofoils.

**4. Induced Drag**

Using Oswald efficiency factor:

```
Cdi = Cl^2 / (pi * AR * e)
Cd_total = Cd_profile + Cdi
```

Where AR = 15.16 and e = 0.95 for the ASK-21.

**5. Forces in Wind Axes**

```
q_dyn = 0.5 * rho * V^2  (dynamic pressure)
L = Cl * q_dyn * S_panel  (lift)
D = Cd * q_dyn * S_panel  (drag)
```

**6. Transformation to Body Axes**

```
Fx = -D*cos(alpha) - L*sin(alpha)
Fz = D*sin(alpha) - L*cos(alpha)
```

**7. Moment Calculation**

```
Rolling moment = Fz * y_span * sign(wing_side)
Pitching moment = M_aero + Fz * (x_ac - x_cg)
Yawing moment = -Fx * y_span * sign(wing_side)
```

### Tailplane and Fin

**Tailplane:**
- Area: 1.796 m^2
- Incidence: -2.5 degrees (provides nose-up trim moment)
- Elevator: +/- 5 degrees deflection
- Accounts for pitch-rate-induced alpha change:
  ```
  delta_alpha = q * x_tail / V
  ```

**Fin/Rudder:**
- Area: 1.413 m^2
- Rudder: +/- 15 degrees deflection
- Yaw damping (Cnr effect) opposes yaw rate

### Control Surface Effects

| Control | Deflection Range | Effect |
|---------|------------------|--------|
| Elevator | +/- 5 deg | Pitch control |
| Ailerons | +18/-12 deg (differential) | Roll control |
| Rudder | +/- 15 deg | Yaw control |
| Airbrakes | 0-100% | Lift reduction (80%), drag increase |

**Aileron Differential:**
The ailerons use differential deflection to reduce adverse yaw:
- Down-going aileron: 12 degrees maximum
- Up-going aileron: 18 degrees maximum

### Fuselage

Simplified as equivalent flat plate drag:
```
Cd*S_fuselage = 0.025 m^2
D_fuselage = 0.025 * q_dyn
```

---

## Aircraft Parameters

### Mass and Inertia (ASK-21)

| Parameter | Value | Units |
|-----------|-------|-------|
| Mass | 687 | kg |
| Ixx (roll) | 1285 | kg*m^2 |
| Iyy (pitch) | 1824 | kg*m^2 |
| Izz (yaw) | 2663 | kg*m^2 |
| Ixz (coupling) | 100 | kg*m^2 |

### Geometry

| Parameter | Value | Units |
|-----------|-------|-------|
| Wing span | 17.0 | m |
| Mean chord | 1.121 | m |
| Wing area | 17.95 | m^2 |
| Aspect ratio | 15.16 | - |
| Oswald efficiency | 0.95 | - |
| CG position | -0.30 | m (aft of datum) |

### Aerofoil Sections

| Location | Aerofoil | Characteristics |
|----------|----------|-----------------|
| Wing root | FX-60-126 | High lift, laminar flow |
| Wing tip | FX-02-196 | Lower lift, good stall characteristics |
| Tail/Fin | NACA0010 | Symmetric, predictable |

---

## Numerical Integration

### Runge-Kutta 4th Order (Primary Method)

The simulation uses RK4 for numerical stability:

```
k1 = f(t, y)
k2 = f(t + dt/2, y + dt/2 * k1)
k3 = f(t + dt/2, y + dt/2 * k2)
k4 = f(t + dt, y + dt * k3)

y_next = y + (dt/6) * (k1 + 2*k2 + 2*k3 + k4)
```

**Key features:**
- Quaternion normalization after each stage
- Default timestep: 0.01 seconds (100 Hz)
- Fourth-order accuracy (error ~ O(dt^5))

### Stability Measures

| Protection | Limit | Purpose |
|------------|-------|---------|
| Force clamping | 100,000 N | Prevent overflow |
| Moment clamping | 500,000 N*m | Prevent overflow |
| Velocity clamping | 500 m/s | Physical limits |
| Angular rate clamping | 20 rad/s | Physical limits |
| Low airspeed smoothing | < 1 m/s | Prevent division by zero |
| NaN/Inf checking | - | Fallback to safe values |

---

## File Structure

| File | Purpose |
|------|---------|
| `simulation.py` | Main simulation loop and integration |
| `state_vector.py` | 12-DOF state representation |
| `model.py` | Aerodynamic force/moment calculations |
| `ask21.py` | ASK-21 aircraft properties |
| `panel.py` | Wing panel aerodynamics |
| `aerofoil.py` | Coefficient lookup via spline |
| `quaternion.py` | Quaternion operations |
| `v3d.py` | Vector utilities |
| `world.py` | Environmental conditions |
| `control_inputs.py` | Control interface |
| `aircraft_params.py` | AR, Oswald efficiency |
| `spline.py` | Cubic spline interpolation |

---

## Control Input Interface

Normalized control inputs in range [-1, +1] or [0, 1]:

```python
controls.pitch   # [-1, +1] forward/aft stick (elevator)
controls.roll    # [-1, +1] left/right stick (ailerons)
controls.rudder  # [-1, +1] left/right pedals
controls.spoiler # [0, 1] airbrake retracted to extended
```

---

## Wind Effects

Wind is defined in the Earth frame and transformed to body frame:

```
wind_body = quaternion_rotate(q^-1, wind_earth)
u_air = u - wind_body_x
v_air = v - wind_body_y
w_air = w - wind_body_z
```

All aerodynamic calculations use airspeed (relative to air mass), not groundspeed.

---

## References

- Etkin, B. & Reid, L.D. "Dynamics of Flight: Stability and Control"
- Stevens, B.L. & Lewis, F.L. "Aircraft Control and Simulation"
- ASK-21 Flight Manual and Type Certificate Data
- UIUC Airfoil Data Site (FX-60-126, FX-02-196, NACA0010 polars)
