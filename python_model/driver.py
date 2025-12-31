
def main():
    
    from simulation import Simulation

    sim = Simulation()
    while True:
        state = sim.update()
        if sim.total_time >= 10.0:
            break
        print(f"Time: {sim.total_time:.2f} s, Position: {state.position()}, Velocity: {state.velocity()}")
    # print(f"Angle of Attack: {args.angle_of_attack} degrees")
    # print(f"Lift Coefficient (Cl): {cl:.6f}")
    # print(f"Drag Coefficient (Cd): {cd:.6f}")
    # print(f"Moment Coefficient (Cm): {cm:.6f}")


if __name__ == "__main__":
    main()