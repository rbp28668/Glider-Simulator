"""Test script to check for adverse yaw during roll input."""

from simulation import Simulation
from math import degrees

def test_adverse_yaw():
    sim = Simulation()

    # Start in stable flight at 30 m/s
    sim.state.set_velocity((30.0, 0.0, 0.0))
    sim.state.set_position((0.0, 0.0, -500.0))  # 500m altitude

    dt = 0.01  # 10ms time step

    print("Phase 1: Stabilize (2 seconds)")
    for _ in range(200):
        sim.update(dt)

    p, q, r = sim.state.angular_velocity()
    print(f"After stabilize: p={p:.4f} r={r:.4f}")

    print("\nPhase 2: Apply RIGHT aileron (roll right)")
    print("Adverse yaw should show NEGATIVE r (nose left) initially")
    print()
    print(f"{'Time':>6} {'Aileron':>8} {'p (roll)':>10} {'r (yaw)':>10} {'Adverse?':>10}")
    print("-" * 50)

    sim.controls.roll = 1.0  # Full right aileron

    for i in range(100):  # 1 second
        sim.update(dt)
        if i % 10 == 0:
            p, q, r = sim.state.angular_velocity()
            # Adverse yaw: rolling right (p > 0) but yawing left (r < 0)
            adverse = "YES" if p > 0.01 and r < -0.001 else "no"
            print(f"{sim.total_time:6.2f} {sim.controls.roll:8.1f} {degrees(p):10.2f}°/s {degrees(r):10.2f}°/s {adverse:>10}")

    print("\nPhase 3: Neutral, then LEFT aileron")
    sim.controls.roll = 0.0
    for _ in range(50):
        sim.update(dt)

    print(f"\n{'Time':>6} {'Aileron':>8} {'p (roll)':>10} {'r (yaw)':>10} {'Adverse?':>10}")
    print("-" * 50)

    sim.controls.roll = -1.0  # Full left aileron

    for i in range(100):  # 1 second
        sim.update(dt)
        if i % 10 == 0:
            p, q, r = sim.state.angular_velocity()
            # Adverse yaw: rolling left (p < 0) but yawing right (r > 0)
            adverse = "YES" if p < -0.01 and r > 0.001 else "no"
            print(f"{sim.total_time:6.2f} {sim.controls.roll:8.1f} {degrees(p):10.2f}°/s {degrees(r):10.2f}°/s {adverse:>10}")

    print("\nExpected: Adverse yaw should appear in first ~0.5s of roll input")
    print("(nose yaws opposite to roll direction due to aileron drag differential)")

if __name__ == "__main__":
    test_adverse_yaw()
