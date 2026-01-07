from state_vector import StateVector


class StateLog :
    def __init__(self, file:str) -> None:
        self.file = file

        with open(file, mode="wt") as f:
            f.write('T, X,Y,Z, vx,vy,vz, qw,qx,qy,qz, wx,wy,wz, fx,fy,fz, mx,my,mz\n')
    

    def write(self, t: float, state: StateVector) :
        with open(self.file, mode="at") as f:
            pos = state.position()
            v = state.velocity()
            q = state.orientation()
            w = state.angular_velocity()
            fb = state.forces
            mb = state.moments
            f.write(f'{t},  {pos[0]},{pos[1]},{pos[2]},  {v[0]},{v[1]},{v[2]},  {q[0]},{q[1]},{q[2]},{q[3]},  {w[0]},{w[1]},{w[2]},  {fb[0]},{fb[1]},{fb[2]},  {mb[0]},{mb[1]},{mb[2]}\n')
