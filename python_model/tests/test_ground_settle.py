"""Test script to verify ground settling behavior."""


from simulation import Simulation
from math import sqrt

def test_ground_settle():
    print("=" * 60)
    print("Test: Ground settling - aircraft should become still quickly")
    print("=" * 60)

    sim = Simulation()

    # Start on ground with slight velocity
    # Main wheel is at z=0.73 below datum, so datum needs to be at z=-0.73 for wheel to touch ground
    sim.state.set_velocity((0.5, 0.1, 0.0))  # Small forward and lateral velocity
    sim.state.set_position((0.0, 0.0, -0.73))  # Datum above ground so wheels touch
    sim.state.set_angular_velocity((0.01, 0.01, 0.01))  # Small rotation

    dt = 0.01

    print("\nInitial state:")
    u, v, w = sim.state.velocity()
    p, q, r = sim.state.angular_velocity()
    print(f"  Velocity: ({u:.4f}, {v:.4f}, {w:.4f}) m/s")
    print(f"  Angular:  ({p:.4f}, {q:.4f}, {r:.4f}) rad/s")

    print("\nSettling (1 second):")
    for i in range(100):
        # Check contact points before update
        forces, moments, contacts = sim.ground_contact.calculate_ground_forces(
            sim.state, sim.aircraft.contact_points, sim.world, sim.aircraft.cg
        )
        n_contacts = sum(1 for c in contacts if c.in_contact)

        sim.update(dt)
        if i % 20 == 19:
            u, v, w = sim.state.velocity()
            p, q, r = sim.state.angular_velocity()
            vel_mag = sqrt(u*u + v*v + w*w)
            ang_mag = sqrt(p*p + q*q + r*r)
            print(f"  t={sim.total_time:.2f}s: vel={vel_mag:.6f} m/s, ang={ang_mag:.6f} rad/s, contacts={n_contacts}")

    print("\nFinal state (should be nearly zero):")
    u, v, w = sim.state.velocity()
    p, q, r = sim.state.angular_velocity()
    vel_mag = sqrt(u*u + v*v + w*w)
    ang_mag = sqrt(p*p + q*q + r*r)
    print(f"  Velocity: ({u:.6f}, {v:.6f}, {w:.6f}) m/s  |mag|={vel_mag:.6f}")
    print(f"  Angular:  ({p:.6f}, {q:.6f}, {r:.6f}) rad/s  |mag|={ang_mag:.6f}")

    if vel_mag < 0.001 and ang_mag < 0.001:
        print("\nPASS: Aircraft settled correctly!")
    else:
        print("\nFAIL: Aircraft still moving - settling not working")

if __name__ == "__main__":
    test_ground_settle()
