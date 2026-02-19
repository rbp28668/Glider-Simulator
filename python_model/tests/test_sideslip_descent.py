"""Test script to verify descent rate in sideslip vs coordinated flight."""

from simulation import Simulation
from math import degrees, atan2, sqrt

def test_sideslip_descent():
    print("=" * 60)
    print("Test: Descent rate in sideslip vs coordinated flight")
    print("=" * 60)

    dt = 0.01

    # Test 1: Coordinated flight - let it stabilize first
    print("\n1. COORDINATED FLIGHT (no sideslip)")
    sim = Simulation()
    sim.state.set_velocity((25.0, 0.0, 1.0))  # Slower, slight descent
    sim.state.set_position((0.0, 0.0, -1000.0))

    # Stabilize for 10 seconds
    for _ in range(1000):
        sim.update(dt)

    # Measure over next 5 seconds
    z_start = sim.state.position()[2]
    for _ in range(500):
        sim.update(dt)

    u, v, w = sim.state.velocity()
    z_end = sim.state.position()[2]
    descent_coordinated = z_end - z_start  # positive = descended (z is down)
    sink_rate_coordinated = descent_coordinated / 5.0
    beta = degrees(atan2(v, u)) if u > 1 else 0
    tas = sqrt(u*u + v*v + w*w)

    print(f"  TAS = {tas:.1f} m/s, Forward speed = {u:.1f} m/s")
    print(f"  Sink rate = {sink_rate_coordinated:.2f} m/s ({sink_rate_coordinated*60:.0f} fpm)")
    print(f"  Sideslip = {beta:.1f}°")

    # Test 2: Full sideslip with rudder - develop slip then measure
    print("\n2. SIDESLIP (full rudder)")
    sim2 = Simulation()
    sim2.state.set_velocity((25.0, 0.0, 1.0))
    sim2.state.set_position((0.0, 0.0, -1000.0))

    # Stabilize first
    for _ in range(1000):
        sim2.update(dt)

    # Apply full rudder and let slip develop
    sim2.controls.rudder = 1.0
    for _ in range(300):  # 3 seconds to develop slip
        sim2.update(dt)

    # Measure over next 5 seconds
    z_start2 = sim2.state.position()[2]
    for i in range(500):
        sim2.update(dt)

    u2, v2, w2 = sim2.state.velocity()
    z_end2 = sim2.state.position()[2]
    descent_slip = z_end2 - z_start2
    sink_rate_slip = descent_slip / 5.0
    beta2 = degrees(atan2(v2, u2)) if abs(u2) > 1 else 0
    tas2 = sqrt(u2*u2 + v2*v2 + w2*w2)

    print(f"  TAS = {tas2:.1f} m/s, Forward speed = {u2:.1f} m/s")
    print(f"  Sink rate = {sink_rate_slip:.2f} m/s ({sink_rate_slip*60:.0f} fpm)")
    print(f"  Sideslip = {beta2:.1f}°")

    # Compare
    print("\n" + "=" * 60)
    print("COMPARISON:")
    print(f"  Coordinated: sink = {sink_rate_coordinated:.2f} m/s at {beta:.0f}° sideslip")
    print(f"  Slipping:    sink = {sink_rate_slip:.2f} m/s at {beta2:.0f}° sideslip")
    if abs(sink_rate_coordinated) > 0.1:
        ratio = sink_rate_slip / sink_rate_coordinated
        print(f"  Ratio: {ratio:.1f}x")
    print("\n  Expected: 2-3x more sink in full sideslip")

if __name__ == "__main__":
    test_sideslip_descent()
