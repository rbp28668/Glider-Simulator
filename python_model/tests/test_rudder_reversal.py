"""Test script to simulate rudder reversal and diagnose divergence."""

from simulation import Simulation
from state_vector import StateVector
from math import degrees, atan2

def test_rudder_reversal():
    sim = Simulation()

    # Start in stable flight at 30 m/s
    sim.state.set_velocity((30.0, 0.0, 0.0))
    sim.state.set_position((0.0, 0.0, -500.0))  # 500m altitude

    dt = 0.01  # 10ms time step

    print("Phase 1: Stabilize (2 seconds)")
    for _ in range(200):
        sim.update(dt)

    print(f"After stabilize: u={sim.state.velocity()[0]:.1f} v={sim.state.velocity()[1]:.1f}")

    print("\nPhase 2: Full right rudder (2 seconds)")
    sim.controls.rudder = 1.0
    for i in range(200):
        sim.update(dt)
        if i % 50 == 0:
            p, q, r = sim.state.angular_velocity()
            u, v, w = sim.state.velocity()
            beta = degrees(atan2(v, u)) if u > 1 else 0
            print(f"  t={sim.total_time:.2f}s: p={p:.3f} r={r:.3f} beta={beta:.1f}°")

    print("\nPhase 3: Full LEFT rudder (reversal, 3 seconds)")
    sim.controls.rudder = -1.0
    for i in range(300):
        sim.update(dt)
        p, q, r = sim.state.angular_velocity()
        u, v, w = sim.state.velocity()
        beta = degrees(atan2(v, u)) if abs(u) > 1 else 0

        if i % 25 == 0:
            print(f"  t={sim.total_time:.2f}s: p={p:.3f} r={r:.3f} beta={beta:.1f}° u={u:.1f}")

        # Check for divergence
        if abs(p) > 10 or abs(r) > 10 or abs(v) > 100:
            print(f"\n*** DIVERGENCE DETECTED at t={sim.total_time:.2f}s ***")
            print(f"  p={p:.2f} rad/s ({degrees(p):.0f} deg/s)")
            print(f"  r={r:.2f} rad/s ({degrees(r):.0f} deg/s)")
            print(f"  v={v:.1f} m/s (sideslip velocity)")
            print(f"  beta={beta:.1f}°")
            return

    print("\nPhase 4: Neutral rudder (recovery, 2 seconds)")
    sim.controls.rudder = 0.0
    for i in range(200):
        sim.update(dt)
        if i % 50 == 0:
            p, q, r = sim.state.angular_velocity()
            u, v, w = sim.state.velocity()
            beta = degrees(atan2(v, u)) if u > 1 else 0
            print(f"  t={sim.total_time:.2f}s: p={p:.3f} r={r:.3f} beta={beta:.1f}°")

    print("\nTest completed without divergence!")

if __name__ == "__main__":
    test_rudder_reversal()
