"""Test script to examine rotation rates during stall."""

from simulation import Simulation
from math import degrees, sqrt, atan2

def test_stall_rotation():
    print("=" * 70)
    print("Test: Rotation rates during stall and wing drop")
    print("=" * 70)

    dt = 0.01
    sim = Simulation()

    # Start in level flight
    sim.state.set_velocity((28.0, 0.0, 0.5))
    sim.state.set_position((0.0, 0.0, -500.0))

    # Stabilize
    print("\nStabilizing...")
    for _ in range(200):
        sim.update(dt)

    # Add small asymmetry to trigger wing drop (simulates real-world perturbations)
    p, q, r = sim.state.angular_velocity()
    sim.state.set_angular_velocity((0.02, q, 0.01))  # Small roll/yaw perturbation

    print("\nPulling into stall with full back stick (with small perturbation)...")
    print("-" * 70)
    print(f"{'Time':>5} {'Speed':>6} {'AoA':>6} {'Roll':>8} {'Yaw':>8} {'p':>9} {'r':>9}")
    print("-" * 70)

    sim.controls.pitch = 0.9  # Back stick

    max_p = 0
    max_r = 0

    for i in range(600):  # 6 seconds
        sim.update(dt)

        u, v, w = sim.state.velocity()
        speed = sqrt(u*u + v*v + w*w)
        aoa_deg = degrees(atan2(w, u)) if u > 0.1 else 0

        p, q, r = sim.state.angular_velocity()
        p_deg = degrees(p)
        r_deg = degrees(r)

        max_p = max(max_p, abs(p_deg))
        max_r = max(max_r, abs(r_deg))

        # Get roll angle from quaternion
        qw, qx, qy, qz = sim.state.orientation()
        roll_rad = atan2(2*(qw*qx + qy*qz), 1 - 2*(qx*qx + qy*qy))
        roll_deg = degrees(roll_rad)

        # Get yaw angle
        yaw_rad = atan2(2*(qw*qz + qx*qy), 1 - 2*(qy*qy + qz*qz))
        yaw_deg = degrees(yaw_rad)

        if i % 50 == 0:
            print(f"{sim.total_time:5.1f}s {speed:5.1f} {aoa_deg:+5.1f}° {roll_deg:+7.1f}° {yaw_deg:+7.1f}° {p_deg:+8.1f}°/s {r_deg:+8.1f}°/s")

    print("-" * 70)
    print(f"\nMaximum rotation rates:")
    print(f"  Roll rate (p): {max_p:.1f} deg/s")
    print(f"  Yaw rate (r):  {max_r:.1f} deg/s")

    if max_p > 100 or max_r > 100:
        print("\n  WARNING: Excessive rotation rates detected!")
        print("  Typical stall should have rates < 60 deg/s")


if __name__ == "__main__":
    test_stall_rotation()
