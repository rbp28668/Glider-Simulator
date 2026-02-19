"""Test script to verify elevator effectiveness."""

from simulation import Simulation
from math import degrees, sqrt

def test_elevator():
    print("=" * 60)
    print("Test: Elevator effectiveness")
    print("=" * 60)

    dt = 0.01

    # Test 1: Pitch response to elevator input
    print("\n1. PITCH RESPONSE TO FULL BACK STICK")
    sim = Simulation()
    sim.state.set_velocity((30.0, 0.0, 0.0))  # 30 m/s forward
    sim.state.set_position((0.0, 0.0, -500.0))  # 500m altitude

    # Stabilize first
    for _ in range(200):
        sim.update(dt)

    p, q, r = sim.state.angular_velocity()
    print(f"  After stabilize: pitch rate = {degrees(q):.2f} deg/s")

    # Apply full back stick (positive = back/aft)
    sim.controls.pitch = 1.0  # Full back stick

    print(f"\n  Time   Pitch Rate   Notes")
    print("  " + "-" * 40)
    for i in range(100):
        sim.update(dt)
        if i % 20 == 0:
            p, q, r = sim.state.angular_velocity()
            print(f"  {i*dt:.2f}s   {degrees(q):+7.2f} deg/s")

    p, q, r = sim.state.angular_velocity()
    print(f"\n  Final pitch rate: {degrees(q):.2f} deg/s")
    if q > 0.05:  # At least 3 deg/s nose up
        print("  PASS: Elevator produces significant pitch-up")
    else:
        print("  FAIL: Elevator not effective enough")

    # Test 2: Stall approach
    print("\n" + "=" * 60)
    print("2. STALL APPROACH - progressively slowing with back stick")
    print("=" * 60)

    sim2 = Simulation()
    sim2.state.set_velocity((25.0, 0.0, 0.5))  # 25 m/s, slight descent
    sim2.state.set_position((0.0, 0.0, -500.0))

    # Let it stabilize
    for _ in range(200):
        sim2.update(dt)

    print(f"\n  Time    Speed    Pitch Rate   AoA approx")
    print("  " + "-" * 50)

    # Progressively pull back
    for phase in range(3):
        # Increase back stick (positive = back/aft)
        sim2.controls.pitch = 0.3 * (phase + 1)  # +0.3, +0.6, +0.9

        for i in range(200):  # 2 seconds each phase
            sim2.update(dt)
            if i % 50 == 0:
                u, v, w = sim2.state.velocity()
                speed = sqrt(u*u + v*v + w*w)
                p, q, r = sim2.state.angular_velocity()
                # Rough AoA estimate
                aoa_deg = degrees(w / max(u, 1.0)) if u > 1 else 0
                print(f"  {sim2.total_time:5.1f}s  {speed:5.1f} m/s  {degrees(q):+7.2f} deg/s  {aoa_deg:+6.1f} deg")

    u, v, w = sim2.state.velocity()
    final_speed = sqrt(u*u + v*v + w*w)
    print(f"\n  Final speed: {final_speed:.1f} m/s")
    print("  Expected: Speed should decrease as nose rises, approaching stall")

if __name__ == "__main__":
    test_elevator()
