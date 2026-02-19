#!/usr/bin/env python
"""
Test harness for ground contact and winch launch behavior.
"""

from math import degrees, radians
from simulation import Simulation
from quaternion import euler_to_quaternion

def run_ground_test():
    """Test ground contact - glider should settle without bouncing."""
    sim = Simulation()

    # Start at equilibrium position
    sim.state.set_position((0.0, 0.0, -0.643))
    sim.state.set_velocity((0.0, 0.0, 0.0))
    sim.state.set_orientation(euler_to_quaternion(0, radians(0.63), 0))
    sim.state.set_angular_velocity((0.0, 0.0, 0.0))

    # Controls centered, spoilers in
    sim.controls.set_controls(0.0, 0.0, 0.0, 0.0)

    # Ensure winch is not engaged
    sim.winch.release("test")

    print("Ground Contact Test")
    print("=" * 80)
    print(f"Aircraft mass: {sim.aircraft.mass} kg, Weight: {sim.aircraft.mass * 9.81:.1f} N")
    print()

    print(f"{'Time':>6} {'Alt':>8} {'Vz':>8} {'Pitch':>8} | {'Main F':>8} {'Nose F':>8} | {'Main Pen':>8}")
    print("-" * 80)

    # Run simulation
    dt = sim.time_step
    for step in range(300):  # 3 seconds
        state = sim.update(dt)

        # Get ground contact info
        _, _, contact_results = sim.ground_contact.calculate_ground_forces(
            state, sim.aircraft.contact_points, sim.world, sim.aircraft.cg
        )

        # Extract contact info
        main_force = nose_force = main_pen = 0
        for i, cp in enumerate(sim.aircraft.contact_points):
            result = contact_results[i]
            if cp.contact_type == 'main_wheel':
                main_force = result.normal_force
                main_pen = result.penetration
            elif cp.contact_type == 'nose_wheel':
                nose_force = result.normal_force

        pos = state.position()
        vel = state.velocity()
        att = state.Attitude()

        altitude = -pos[2]
        vz = vel[2]
        pitch_deg = degrees(att[1])

        if step % 30 == 0:
            print(f"{sim.total_time:6.2f} {altitude:8.3f} {vz:8.3f} {pitch_deg:8.2f} | {main_force:8.1f} {nose_force:8.1f} | {main_pen:8.4f}")

        if abs(pitch_deg) > 45:
            print(f"\n*** FLIP DETECTED at t={sim.total_time:.2f}s ***")
            return False
        if altitude > 2.0:
            print(f"\n*** EXCESSIVE BOUNCE at t={sim.total_time:.2f}s ***")
            return False

    print("\nGround test PASSED - glider settled normally")
    return True


def run_winch_test():
    """Test winch launch - glider should accelerate and climb."""
    sim = Simulation()

    # Start at equilibrium position
    sim.state.set_position((0.0, 0.0, -0.643))
    sim.state.set_velocity((0.0, 0.0, 0.0))
    sim.state.set_orientation(euler_to_quaternion(0, radians(0.63), 0))
    sim.state.set_angular_velocity((0.0, 0.0, 0.0))

    # Controls centered, spoilers in
    sim.controls.set_controls(0.0, 0.0, 0.0, 0.0)

    # Setup and engage winch
    sim.setup_winch_launch(winch_distance=1000.0, max_tension=6000.0, weak_link=8000.0)
    sim.engage_winch()

    print("\nWinch Launch Test")
    print("=" * 80)
    print(f"Winch position: {sim.winch.winch_position}")
    print(f"Max tension: {sim.winch.max_tension} N")
    print()

    print(f"{'Time':>6} {'Alt':>8} {'Speed':>8} {'Pitch':>8} {'Vz':>8} | {'Tension':>8} {'Cable':>8} | {'OnGnd':>6}")
    print("-" * 90)

    dt = sim.time_step
    max_alt = 0
    for step in range(3000):  # 30 seconds
        state = sim.update(dt)

        pos = state.position()
        vel = state.velocity()
        att = state.Attitude()

        altitude = -pos[2]
        max_alt = max(max_alt, altitude)
        speed = (vel[0]**2 + vel[1]**2 + vel[2]**2)**0.5
        pitch_deg = degrees(att[1])
        vz = vel[2]

        # Check ground contact
        _, _, contact_results = sim.ground_contact.calculate_ground_forces(
            state, sim.aircraft.contact_points, sim.world, sim.aircraft.cg
        )
        on_ground = any(r.in_contact for r in contact_results)

        winch_info = sim.winch_info
        tension = winch_info.get('tension', 0)
        cable_out = winch_info.get('cable_out', 0)

        if step % 100 == 0:
            print(f"{sim.total_time:6.2f} {altitude:8.1f} {speed:8.1f} {pitch_deg:8.1f} {vz:8.2f} | {tension:8.0f} {cable_out:8.0f} | {str(on_ground):>6}")

        # Check for problems
        if abs(pitch_deg) > 80:
            print(f"\n*** EXCESSIVE PITCH at t={sim.total_time:.2f}s, pitch={pitch_deg:.1f} ***")
            return False

        # Check for successful launch (reached 100m altitude)
        if altitude > 100 and not on_ground:
            print(f"\n*** LAUNCH SUCCESS at t={sim.total_time:.2f}s, alt={altitude:.1f}m ***")
            return True

        # Check if winch released
        if not sim.winch.engaged and sim.total_time > 1.0:
            print(f"\n*** WINCH RELEASED: {sim.winch.release_reason} at t={sim.total_time:.2f}s ***")
            print(f"    Max altitude reached: {max_alt:.1f}m")
            return max_alt > 50  # Success if got reasonably high

    print(f"\nWinch test ended - max altitude: {max_alt:.1f}m")
    return max_alt > 100


if __name__ == "__main__":
    ground_ok = run_ground_test()
    print()
    winch_ok = run_winch_test()
    print()
    print("=" * 80)
    print(f"Ground test: {'PASS' if ground_ok else 'FAIL'}")
    print(f"Winch test:  {'PASS' if winch_ok else 'FAIL'}")
