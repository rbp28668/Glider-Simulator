# Winch Launch Simulation - Physics and Implementation

Technical reference for the winch launch model in P3DWinch.

## 1. Overview

The simulation models a Skylaunch-type ground-based winch launching an ASK-21 glider. The winch drivetrain is:

```
Engine --> Torque Converter --> 3-Speed Auto Transmission --> Final Drive --> Drum --> Cable --> Glider
```

The physical principle is that the **glider controls cable speed** (it moves at whatever speed the forces dictate) and the **winch reacts to the load**.  For a given cable speed and throttle position, the drivetrain model determines how much tension the cable carries.

### Real-World Equipment Modelled

| Component | Model | Key Specification |
|-----------|-------|-------------------|
| Engine | GM 7.4L (454ci) Big Block V8, LPG | 597 Nm @ 3400 RPM, 250 kW @ 4600 RPM |
| Torque Converter | TH400-style fluid coupling | 2.1:1 stall torque ratio |
| Transmission | Turbo-Hydramatic 400 | 3-speed auto: 2.48 / 1.48 / 1.00 |
| Drum | Skylaunch-type | 0.35 m inner radius, 2000 m cable capacity |
| Cable | Steel | 4.6 mm diameter |
| Weak link | Tost-type | Default 10,000 N (black link, ASK-21) |

### Source Files

| File | Role |
|------|------|
| `winch.h` | Winch class: constants, state, method declarations |
| `winch.cpp` | Drivetrain model, engine dynamics, cable force computation |
| `Simulation.cpp` | Integration loop (RK4), force accumulation |
| `simulation.h` | Simulation class definition |
| `LaunchController.cpp` | Launch sequence state machine and throttle management |
| `LaunchController.h` | Launch stage definitions and timing constants |
| `ask21.h` | ASK-21 aircraft parameters (mass, inertia, hook position) |
| `model.cpp` | Aerodynamic force model |
| `state_vector.h` | 13-element aircraft state (position, velocity, quaternion, angular rates) |

---

## 2. Drivetrain Physics

### 2.1 Engine Torque Model

The engine is modelled by a full-throttle torque curve interpolated from a cubic spline through 10 data points:

| RPM  | 700 | 1000 | 1500 | 2000 | 2500 | 3000 | 3400 | 4000 | 4600 | 5000 |
|------|-----|------|------|------|------|------|------|------|------|------|
| Nm   | 300 |  380 |  455 |  515 |  560 |  588 |  597 |  570 |  519 |  470 |

Partial throttle is handled by a butterfly-valve characteristic:

```
eff = throttle ^ 1.5
idle_torque = WOT_torque(700 RPM) * 0.25 = 75 Nm
T_engine = idle_torque + (WOT_torque(RPM) - idle_torque) * eff
```

At closed throttle the engine still produces 25% of its idle-RPM torque (75 Nm), representing the torque needed to sustain idle against accessories and friction.  The exponent of 1.5 models the non-linear airflow characteristic of a butterfly throttle body.

### 2.2 Torque Converter

The torque converter is the critical coupling between the engine and the transmission.  It is a fluid device characterised by three quantities that vary with the **speed ratio** SR:

```
SR = N_turbine / N_pump    (0 = stall, 1 = lockup)
```

| SR   | 0.00 | 0.10 | 0.20 | 0.30 | 0.40 | 0.50 | 0.60 | 0.70 | 0.80 | 0.87 | 0.93 | 0.97 | 1.00 |
|------|------|------|------|------|------|------|------|------|------|------|------|------|------|
| TR   | 2.10 | 2.05 | 1.97 | 1.86 | 1.73 | 1.58 | 1.41 | 1.24 | 1.10 | 1.00 | 1.00 | 1.00 | 1.00 |
| K    | 86.8 | 88.0 | 91.0 | 95.0 | 100  | 107  | 117  | 132  | 155  | 195  | 240  | 310  | 420  |

**TR (Torque Ratio):** The turbine output torque is `T_turbine = T_pump * TR`.  At stall (SR=0), the converter multiplies torque by 2.1x.  Above the coupling point (SR >= 0.87) the converter acts as a 1:1 fluid coupling.

**K (K-factor):** Relates pump RPM to pump torque absorption: `T_pump = (N_pump / K)^2`.  At stall, K is low (86.8) so the pump absorbs significant torque; near lockup, K is high (420) so very little torque is absorbed.

The key physics:

- **At stall** (glider stationary): turbine RPM = 0, SR = 0, maximum torque multiplication (2.1x), maximum pump load on engine.
- **At cruise** (glider at speed): SR approaches 0.87-0.97, TR approaches 1.0, converter acts as near-rigid coupling.
- **The converter naturally limits launch forces** because at low cable speed the pump absorbs torque proportional to RPM^2, preventing the engine from instantly reaching full power.

### 2.3 Transmission

The TH400 3-speed automatic:

| Gear | Ratio | Typical Use |
|------|-------|-------------|
| 1st  | 2.48  | Launch start, low cable speed |
| 2nd  | 1.48  | Mid-launch |
| 3rd  | 1.00  | High cable speed cruise |

Efficiency: 95% (models hydraulic pump losses, bearing friction).

Gear selection is automatic: the highest gear in which engine RPM stays above 1400 RPM is selected.  Gear 0 represents neutral (no torque path to the drum).

### 2.4 Final Drive

A simple reduction gear:

- Ratio: 4.0:1
- Efficiency: 97%

### 2.5 Drum

The drum has an inner radius of 0.35 m and holds up to 2000 m of 4.6 mm steel cable wound in spiral layers.

**Variable effective radius:**  As cable pays out, the radius decreases.  Each layer holds `43 * 2 * pi * r_layer` metres of cable.  The code computes the exact fractional layer to determine the current effective radius:

```
r_eff = r_inner + (layer + fraction) * cable_diameter
```

This matters because a smaller drum radius means higher mechanical advantage (more tension for the same drum torque) but lower cable speed for the same drum RPM.

**Drum RPM** from cable speed:

```
drum_RPM = cable_speed / r_eff * 30 / pi
```

### 2.6 Complete Torque Path

For a given engine RPM, throttle position, cable speed, and gear:

```
1.  turbine_RPM = drum_RPM * final_drive_ratio * gear_ratio
2.  SR = turbine_RPM / engine_RPM
3.  T_pump = (engine_RPM / K(SR))^2
4.  T_turbine = T_pump * TR(SR)
5.  T_trans_out = T_turbine * gear_ratio * 0.95
6.  T_drum = T_trans_out * final_drive_ratio * 0.97
7.  cable_tension = T_drum / r_eff
```

---

## 3. Engine Dynamics

### 3.1 Dynamic Engine RPM Model

Engine RPM is a **persistent state variable** that evolves over time according to Newton's second law for rotation.  Each simulation step:

```
net_torque = T_engine(RPM, throttle) - T_pump(RPM, SR)
alpha = net_torque / I_engine                              (rad/s^2)
RPM_dot = alpha / RPM_TO_RADS                              (RPM/s)
engine_RPM += RPM_dot * dt
```

Where `I_engine = 1.5 kg.m^2` represents the combined rotational inertia of the flywheel and torque converter pump impeller.

This means:
- When engine torque exceeds pump load, the engine accelerates.
- When pump load exceeds engine torque, the engine decelerates.
- The engine naturally finds its equilibrium RPM over time rather than jumping to it instantly.

### 3.2 Why Dynamic RPM Matters

Without inertia, the previous quasi-static model (Brent's method equilibrium solver) found the instantaneous equilibrium RPM each timestep.  At low cable speed with even modest throttle, the solver found a high equilibrium RPM.  Combined with the 2.1x stall torque multiplication and ~26:1 overall drivetrain ratio, this produced tension spikes exceeding the weak link limit.

With inertia, the engine must physically spin up from idle (700 RPM) against the flywheel and TC pump mass.  At 1.5 kg.m^2 with a net torque of 10 Nm, acceleration is only ~64 RPM/s -- the tension builds progressively over several seconds.

### 3.3 Idle Governor

A proportional controller prevents the engine from stalling:

```
if (engine_RPM < IDLE_RPM):
    net_torque += (IDLE_RPM - engine_RPM) * 0.5    (Nm per RPM of error)
```

Combined with a hard floor clamp at 700 RPM.

### 3.4 Neutral Gear

When throttle is zero, the transmission is placed in neutral (gear 0).  No torque path exists from the engine to the drum, so cable tension is zero.  The engine decays toward idle at 500 RPM/s.  When throttle opens, gear 1 engages and the engine begins loading the TC.

### 3.5 Rev Limiter

Engine RPM is hard-clamped to 5000 RPM (MAX_RPM), simulating fuel cut-off.  In practice, the TC pump torque (proportional to RPM^2) prevents the engine from reaching this limit under load.

---

## 4. Cable Force Application

### 4.1 Geometry

The cable runs from the winch drum position (in earth frame NED) to the winch hook on the glider.  The hook is located at body position `(0.157, -0.05, 0.573)` m -- slightly forward of the CG, slightly left of centreline, and below the fuselage.

```
cable_vector = winch_position - hook_earth_position
cable_unit = normalise(cable_vector)
cable_distance = |cable_vector|
```

### 4.2 Cable Speed

The velocity of the hook point (not just the CG) is computed including the rotational contribution:

```
V_hook = V_cg + omega x r_hook
```

The component along the cable direction gives the cable speed:

```
v_cable = dot(V_hook_earth, cable_unit)
```

Positive `v_cable` means the hook is moving toward the winch (cable winding in).  Negative means moving away (cable paying out / slack).

### 4.3 Tension to Force

The cable tension acts along the cable unit vector from the hook toward the winch:

```
F_earth = tension * cable_unit
F_body = rotate_to_body_frame(F_earth)
```

The moment about the CG is the cross product of the hook arm and the force:

```
M_body = r_hook x F_body
```

This naturally creates:
- A pitching moment (nose-up tendency as the cable pulls forward and slightly down)
- A small rolling moment (hook is offset 50 mm left of centreline)
- A yawing moment if the cable has a lateral component

### 4.4 Cable Slack

When `v_cable < 0`, the glider is moving away from the winch and the cable is slack.  A token tension of 100 N is applied (representing cable weight/catenary) to maintain numerical stability and prevent the cable from "passing through" the glider.

### 4.5 Cable Angle

The angle of the cable below the horizontal is tracked:

```
cable_angle = atan2(cable_z, horizontal_distance)
```

This is used by the launch controller to reduce throttle when the cable angle becomes steep near the top of the launch.

---

## 5. Automatic Release Conditions

The winch automatically releases the cable under four conditions:

| Condition | Trigger | Reason String |
|-----------|---------|---------------|
| Weak link | `tension > weak_link` | `"weak_link"` |
| Back-release | Cable pulling backward in body frame (`cable_body.x < 0`) | `"back_release"` |
| Cable run out | `cable_distance > cable_length` | `"cable_run_out"` |
| Cable too short | `cable_distance < 100 m` | `"cable_run_out"` |

The back-release is a safety mechanism: if the glider overflies the winch or the cable geometry reverses, the cable releases automatically rather than pulling the tail up.

---

## 6. Launch Controller

The launch controller (`LaunchController`) manages the sequencing and throttle profile of a winch launch through a state machine:

```
SETTLE --> CABLE_ON --> WINGS_LEVEL --> UP_SLACK --> GROUND_RUN --> LAUNCHING
  4s          3s           2s            5s          ramp to        maintain
                                                    target         target
```

### 6.1 Stage Details

**SETTLE (4 s):** No action.  Allows the flight sim to stabilise after initiating the launch.

**CABLE_ON (3 s):** Plays "cable on and secure, black link" audio callout.  No winch action.

**WINGS_LEVEL (2 s):** Smoothly levels the wings from whatever bank angle the glider started at.  Simulates a wing-runner.

**UP_SLACK (5 s):** The winch cable is engaged (`engage_winch()`).  Throttle is set to zero.  With the dynamic engine model, the transmission is in neutral during this phase -- the cable carries no tension and the glider does not move.  Wings are held level.  Plays "take up slack" audio.

**GROUND_RUN (variable):** Throttle ramps linearly from 0 to `targetThrottle` (default 0.8) over 5 seconds:

```
throttle = targetThrottle * elapsed_time / THROTTLE_RAMP_TIME
```

Wings are held level until the glider reaches `WING_RELEASE_SPEED` (2.7 m/s -- a fast jog, representing the wing-runner letting go).

Transitions to LAUNCHING when throttle reaches target AND the glider is off the ground.

**LAUNCHING:** Maintains `targetThrottle`.  When cable angle exceeds 80 degrees (near-vertical cable at top of launch), throttle is cut to 0.2 to reduce the nose-over tendency.  Optionally applies power fade and wing drop effects.

### 6.2 Throttle Control

The pilot can adjust throttle during the launch:
- `adjustPower(amount)` -- incremental change (+/-)
- `setPower(amount)` -- absolute setting (0.0 to 1.0)
- `startPowerFade(time, duration)` -- gradual power reduction to zero

---

## 7. Integration Architecture

### 7.1 Timestep and Rate

The simulation runs at a **fixed 500 Hz** internal rate (dt = 0.002 s), independent of the host flight simulator's frame rate.  The integration loop in `StateInput::tickModel()` runs as many 2 ms steps as needed to catch up with the simulator clock:

```
while (lastSimTime < data.time):
    simulation.update(0.002, controls, world)
    lastSimTime += 0.002
```

### 7.2 RK4 Integration

Each simulation step uses 4th-order Runge-Kutta integration of the 13-element aircraft state vector:

```
State = [X, Y, Z,  u, v, w,  qw, qx, qy, qz,  p, q, r]
         position    velocity   orientation       angular rates
         (earth)     (body)     (quaternion)      (body)
```

The RK4 method computes four derivative evaluations (k1 through k4), each requiring a full force/moment calculation.  This means `Winch::calculate_forces()` is called **4 times per timestep** (2000 evaluations per second).

### 7.3 Engine Update Timing

The engine RPM is advanced **once per timestep**, before the RK4 step:

```cpp
// In Simulation::update():
winch.update(dt);          // Advance engine RPM (once)
state = rk4_step(state, dt);  // 4 force evaluations using current engine_rpm
```

All four RK4 evaluations see the same engine_rpm, ensuring consistency.  The `calculate_forces()` method reads `engine_rpm` to compute tension but does not modify it.  Cable speed from each evaluation is cached for use by the next `update()` call.

### 7.4 Force Accumulation

Total forces and moments are the sum of three sources:

```
F_total = F_aero + F_ground + F_winch
M_total = M_aero + M_ground + M_winch + roll_bias
```

The roll bias is an external input used by the launch controller to simulate wing drops.

### 7.5 Linear Acceleration

Forces are converted to accelerations in the body frame including Coriolis terms:

```
u_dot = (Fx_total / mass) + r*v - q*w
v_dot = (Fy_total / mass) + p*w - r*u
w_dot = (Fz_total / mass) + q*u - p*v
```

Gravity is rotated into the body frame and added to the total force before division by mass.

---

## 8. Aircraft Parameters (ASK-21)

| Parameter | Value | Notes |
|-----------|-------|-------|
| Mass | 687 kg | Empty + pilot, adjustable via spin kit |
| Ixx (roll) | 1285 kg.m^2 | |
| Iyy (pitch) | 1824 kg.m^2 | Adjustable via spin kit |
| Izz (yaw) | 2663 kg.m^2 | Adjustable via spin kit |
| Ixz (coupling) | 100 kg.m^2 | Estimated |
| Wing span | 17.0 m | |
| Wing area | 17.95 m^2 | |
| CG position | -0.30 m from datum | Adjustable via spin kit |
| Winch hook | (0.157, -0.050, 0.573) m | Body frame, slightly left of CL |
| Aerotow hook | (2.292, 0.000, 0.447) m | Body frame, nose position |

---

## 9. Tuning Parameters

The following constants in `winch.h` control the launch dynamics and can be adjusted:

| Constant | Value | Effect |
|----------|-------|--------|
| `ENGINE_INERTIA` | 1.5 kg.m^2 | Combined flywheel + TC pump inertia.  Increase for gentler initial acceleration, decrease for more responsive engine. Range: 1.0 - 2.5. |
| `IDLE_GOVERNOR_GAIN` | 0.5 Nm/RPM | Stiffness of idle speed control.  Higher = more resistant to stalling under load. |
| `IDLE_RPM` | 700 | Engine idle speed.  Affects minimum pump torque absorption. |
| `IDLE_TORQUE_FRAC` | 0.25 | Fraction of WOT torque at closed throttle.  Lower = less creep in gear at idle. |
| `THROTTLE_EXPONENT` | 1.5 | Butterfly valve non-linearity.  Higher = more progressive low-end response. |
| `FINAL_DRIVE_RATIO` | 4.0 | Overall gearing.  Higher = more cable tension, lower cable speed. |

In `LaunchController.h`:

| Constant | Value | Effect |
|----------|-------|--------|
| `UP_SLACK_TIME` | 5.0 s | Duration of take-up-slack phase (throttle = 0) |
| `THROTTLE_RAMP_TIME` | 5.0 s | Time to ramp throttle from 0 to target |
| `targetThrottle` | 0.8 | Default peak throttle position |
| `WING_RELEASE_SPEED` | 2.7 m/s | Speed at which wing-runner releases |

---

## 10. Worked Example: Launch Start

To illustrate how the numbers flow at the critical moment -- the start of the ground run when throttle begins to open:

**Initial conditions:** Engine at idle (700 RPM), glider stationary, gear 1 engaged.

**Throttle at 0.1 (1 second into ground run):**

1. Engine torque: `eff = 0.1^1.5 = 0.032`, `T_engine = 75 + (300 - 75) * 0.032 = 82 Nm`
2. TC pump at stall (SR=0): `T_pump = (700 / 86.8)^2 = 65 Nm`
3. Net torque: `82 - 65 = 17 Nm`
4. RPM acceleration: `17 / (1.5 * 0.1047) = 108 RPM/s`
5. After 1 second: engine at approximately 808 RPM

Cable tension at 808 RPM, SR=0 (glider just starting to move):
- `T_pump = (808 / 86.8)^2 = 86.7 Nm`
- `T_turbine = 86.7 * 2.1 = 182 Nm`
- Through drivetrain: `182 * 2.48 * 0.95 * 4.0 * 0.97 = 1664 Nm` at drum
- With drum radius ~0.38 m: `tension = 1664 / 0.38 = 4380 N`
- Glider acceleration: `4380 / 687 = 6.4 m/s^2` (0.65g)

This is a firm but realistic initial pull.  Without the inertia model, the solver would have found the equilibrium RPM of ~1100 RPM instantly, producing over 8000 N.

**Throttle at 0.8 (5 seconds, steady state):**

By this point the engine has had time to reach its loaded equilibrium.  Cable speed is substantial, SR is well above zero, and the TC torque multiplication has reduced.  Typical steady-state tension is 3000-5000 N depending on cable speed and angle, well within the 10,000 N weak link.
