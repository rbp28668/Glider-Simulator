"""Test script to compare drag forces with and without sideslip."""

from simulation import Simulation
from state_vector import StateVector
from math import degrees, radians, sin, cos, sqrt

def test_sideslip_drag():
    print("=" * 60)
    print("Test: Drag comparison - coordinated vs sideslip")
    print("=" * 60)

    sim = Simulation()

    # Set up straight and level at 30 m/s
    TAS = 30.0

    print(f"\nTesting at TAS = {TAS} m/s")
    print("-" * 60)

    for beta_deg in [0, 10, 20, 30]:
        beta = radians(beta_deg)

        # Set velocity with sideslip
        u = TAS * cos(beta)  # forward component
        v = TAS * sin(beta)  # sideways component

        sim.state = StateVector()
        sim.state.set_velocity((u, v, 0.5))  # slight descent
        sim.state.set_position((0.0, 0.0, -500.0))

        # Calculate forces
        forces, moments = sim.calculate_forces_moments(sim.state)

        # Total drag (X direction in body frame)
        drag = -forces[0]  # force in -X is drag

        # For comparison, calculate expected glide ratio
        # At coordinated flight, L/D = TAS / sink_rate
        # More drag = steeper descent

        print(f"  Beta = {beta_deg:2d}°: Drag = {drag:7.1f} N, "
              f"Side force = {forces[1]:7.1f} N")

    print("\n" + "=" * 60)
    print("At 30° sideslip, drag should be significantly higher")
    print("This causes the rapid descent used in slip-to-land technique")

if __name__ == "__main__":
    test_sideslip_drag()
