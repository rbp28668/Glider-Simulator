"""Test script to verify stall behavior."""

from simulation import Simulation
from math import degrees, sqrt, atan2

def test_stall():
    print("=" * 70)
    print("Test: Stall behavior - progressive pull to stall")
    print("=" * 70)

    dt = 0.01
    sim = Simulation()

    # Start in level flight at moderate speed
    sim.state.set_velocity((30.0, 0.0, 0.5))
    sim.state.set_position((0.0, 0.0, -500.0))  # 500m altitude

    # Stabilize first
    print("\nStabilizing for 3 seconds...")
    for _ in range(300):
        sim.update(dt)

    u, v, w = sim.state.velocity()
    speed = sqrt(u*u + v*v + w*w)
    print(f"Initial: Speed={speed:.1f} m/s, Alt={-sim.state.position()[2]:.0f}m")

    print("\n" + "-" * 70)
    print(f"{'Time':>6} {'Speed':>7} {'AoA':>7} {'Pitch':>8} {'PitchRate':>10} {'Altitude':>8} {'Stick':>6}")
    print("-" * 70)

    # Progressive pull back on stick (positive = back/aft)
    stick_position = 0.0
    stall_detected = False
    stall_time = None
    max_aoa = 0

    for i in range(800):  # 8 seconds
        # Gradually pull back on stick (positive values = aft/back stick)
        if i < 200:
            stick_position = 0.3 * (i / 200)  # Ramp to +0.3 over 2s
        elif i < 400:
            stick_position = 0.3 + 0.3 * ((i - 200) / 200)  # Ramp to +0.6
        elif i < 600:
            stick_position = 0.6 + 0.3 * ((i - 400) / 200)  # Ramp to +0.9
        else:
            stick_position = 0.9  # Hold at +0.9

        sim.controls.pitch = stick_position
        sim.update(dt)

        # Calculate flight parameters
        u, v, w = sim.state.velocity()
        speed = sqrt(u*u + v*v + w*w)

        # Angle of attack (body frame)
        aoa_deg = degrees(atan2(w, u)) if u > 0.1 else 0
        max_aoa = max(max_aoa, aoa_deg)

        # Pitch angle from quaternion
        qw, qx, qy, qz = sim.state.orientation()
        pitch_rad = atan2(2*(qw*qy - qz*qx), 1 - 2*(qx*qx + qy*qy))
        pitch_deg = degrees(pitch_rad)

        # Pitch rate
        p, q, r = sim.state.angular_velocity()
        pitch_rate_deg = degrees(q)

        # Altitude
        alt = -sim.state.position()[2]

        # Detect stall (AoA > 15° or sudden pitch down)
        if not stall_detected and aoa_deg > 15 and pitch_rate_deg < -5:
            stall_detected = True
            stall_time = sim.total_time
            print(f"  *** STALL DETECTED at t={stall_time:.2f}s, AoA={aoa_deg:.1f}° ***")

        # Print every 0.5 seconds
        if i % 50 == 0:
            print(f"{sim.total_time:6.1f}s {speed:6.1f} {aoa_deg:+6.1f}° {pitch_deg:+7.1f}° {pitch_rate_deg:+9.1f}°/s {alt:7.0f}m {stick_position:+5.2f}")

    print("-" * 70)

    # Summary
    print("\nSUMMARY:")
    print(f"  Maximum AoA reached: {max_aoa:.1f}°")
    if stall_detected:
        print(f"  Stall detected at: t={stall_time:.2f}s")
        print("  PASS: Aircraft exhibits stall behavior")
    else:
        print("  Stall not clearly detected (may need more back stick)")

    u, v, w = sim.state.velocity()
    final_speed = sqrt(u*u + v*v + w*w)
    final_alt = -sim.state.position()[2]
    print(f"  Final speed: {final_speed:.1f} m/s")
    print(f"  Final altitude: {final_alt:.0f}m (lost {500 - final_alt:.0f}m)")


def test_stall_recovery():
    print("\n" + "=" * 70)
    print("Test: Stall recovery - push nose down")
    print("=" * 70)

    dt = 0.01
    sim = Simulation()

    # Start slow with high AoA (near stall)
    sim.state.set_velocity((18.0, 0.0, 6.0))  # Slow, high AoA
    sim.state.set_position((0.0, 0.0, -500.0))

    print("\nStarting near stall condition...")

    # Hold back stick briefly to enter stall (positive = back/aft)
    sim.controls.pitch = 0.8
    for _ in range(100):
        sim.update(dt)

    u, v, w = sim.state.velocity()
    aoa_deg = degrees(atan2(w, u)) if u > 0.1 else 0
    print(f"In stall: Speed={sqrt(u*u+v*v+w*w):.1f} m/s, AoA={aoa_deg:.1f}°")

    print("\nPushing nose down to recover...")
    print(f"{'Time':>6} {'Speed':>7} {'AoA':>7} {'PitchRate':>10} {'Stick':>6}")
    print("-" * 50)

    # Push nose down to recover (negative = forward stick)
    for i in range(300):
        if i < 50:
            sim.controls.pitch = -0.5  # Forward stick
        else:
            sim.controls.pitch = 0.0  # Neutral

        sim.update(dt)

        if i % 30 == 0:
            u, v, w = sim.state.velocity()
            speed = sqrt(u*u + v*v + w*w)
            aoa_deg = degrees(atan2(w, u)) if u > 0.1 else 0
            p, q, r = sim.state.angular_velocity()
            print(f"{sim.total_time:6.2f}s {speed:6.1f} {aoa_deg:+6.1f}° {degrees(q):+9.1f}°/s {sim.controls.pitch:+5.2f}")

    u, v, w = sim.state.velocity()
    final_speed = sqrt(u*u + v*v + w*w)
    aoa_deg = degrees(atan2(w, u)) if u > 0.1 else 0

    print("-" * 50)
    if final_speed > 25 and aoa_deg < 10:
        print(f"PASS: Recovered - Speed={final_speed:.1f} m/s, AoA={aoa_deg:.1f}°")
    else:
        print(f"Recovery incomplete - Speed={final_speed:.1f} m/s, AoA={aoa_deg:.1f}°")


if __name__ == "__main__":
    test_stall()
    test_stall_recovery()
