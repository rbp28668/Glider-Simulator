


from math import degrees, radians
import math

from quaternion import euler_to_quaternion, quaternion_rotate_vector, quaternion_rotate_vector_inverse, quaternion_to_euler

from simulation import Simulation
from state_vector import StateVector



pitch = radians(3) # 3 degrees nose up
q = euler_to_quaternion(0, pitch, 0)
g = (0,0,9.81)  # in world coordinates
g_body = quaternion_rotate_vector_inverse(q,g) # Convert to body coordinates

print(f'{g_body[0]},{g_body[1]},{g_body[2]}')
# -0.513415730743279,0.0,9.79655573594237  : slight backwards (X), mainly down (Z)


qw, qx, qy, qz = q
phi, theta, psi = quaternion_to_euler(qw, qx, qy, qz) # roll pitch yaw
print(f'Roll: {phi}, Pitch:{theta},Yaw:{psi},  Degrees: {degrees(phi)},{degrees(theta)},{degrees(psi)}')

pitch = radians(90) # Straight up
q = euler_to_quaternion(0, pitch, 0)
g = (0,0,9.81)
g_body = quaternion_rotate_vector_inverse(q,g)

print(f'{g_body[0]},{g_body[1]},{g_body[2]}')
# -9.810000000000002,0.0,0.0 - all towards the tail.

sim = Simulation()
sim.state.set_position((0.0, 0.0, -1000.0))  # Start at 1000m altitude
sim.state.set_velocity((30.0, 0.0, 0.0))      # Initial forward speed 30 m/s
sim.state.set_orientation(euler_to_quaternion(0, radians(1.5),0))  # Pointing north, slight pitch up
sim.state.set_angular_velocity((1.0, 0.0, 0.0))  # Rolling right

dt = 0.01  # simulate 100Hz update

new_state = sim.update(dt)

av = new_state.angular_velocity()
print(f'Angular Velocity after 1 run: Roll: {av[0]}, Pitch:{av[1]},Yaw:{av[2]}')

for i in range(0,100) :
    new_state = sim.update(dt)
av = new_state.angular_velocity()
print(f'Angular Velocity after 100 run: Roll: {av[0]}, Pitch:{av[1]},Yaw:{av[2]}')



Vx = 30
for Vz in [ x / 10 for x in range(-50,50)] :
    sim.state.set_position((0.0, 0.0, -1000.0))  # Start at 1000m altitude
    sim.state.set_velocity((Vx, 0.0, Vz))      # Initial forward speed 30 m/s
    sim.state.set_orientation(euler_to_quaternion(0, radians(1.5),0))  # Pointing north, slight pitch up
    sim.state.set_angular_velocity((0.0, 0.0, 0.0))  # Rolling right

    sim.update(0.01)
    alpha = math.degrees(math.atan2(Vz,Vx))

    print(f'{Vx:.1f},{Vz:.1f},{alpha:.2f},{sim.state.forces[0]:.1f},{sim.state.forces[2]:.1f}')

