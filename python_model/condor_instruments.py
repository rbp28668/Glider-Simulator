import socket

from quaternion import quaternion_rotate_vector
from state_vector import StateVector 


class CondorInstruments :

    def __init__(self, addr, port) :
        self.addr = addr
        self.port = port
        self.sock = socket.socket(socket.AF_INET, # Internet
                     socket.SOCK_DGRAM) # UDP

        
        self.time = 0.0 # in game display time decimal hours 
        self.airspeed = 0.0
        self.altitude = 0.0 # altimeter reading m or ft 
        self.vario = 0.0 # pneumatic vario reading m/s 
        self.evario = 0.0 # electronic variometer reading m/s 
        self.nettovario = 0.0 # netto variometer value m/s 
        self.integrator = 0.0 # integrator value m/s 
        self.compass = 0.0 # compass reading degree 
        self.slipball = 0.0 # slip ball deflection angle rad 
        self.turnrate = 0.0 # turn indicator reading rad/s 
        self.yawstringangle = 0.0 # yawstring angle rad 
        self.yaw = 0.0 # yaw rad 
        self.pitch = 0.0 # pitch rad 
        self.bank = 0.0 # bank rad 
        self.quaternionx = 0.0 # quaternion x / 
        self.quaterniony = 0.0 # quaternion y / 
        self.quaternionz = 0.0 # quaternion z /
        self.vx = 0.0 # speed vector x m/s 
        self.vy = 0.0 # speed vector y m/s 
        self.vz = 0.0 # speed vector z m/s 
        self.rollrate = 0.0 # roll rate (local system x) rad/s 
        self.pitchrate = 0.0 # pitch rate (local system y) rad/s 
        self.yawrate = 0.0 # yaw rate (local system z) rad/s 
        self.gforce = 0.0 # g force factor /
        


    def set(self, total_time: float, state: StateVector):

        velocity = state.velocity()
    
        self.time = total_time
        self.airspeed = velocity[0] # x component only
        self.altitude = state.Altitude()
        self.compass = state.Heading()

        att = state.Attitude()
        self.yaw = att[2]
        self.pitch = att[1]
        self.bank = att[0]

        orientation = state.orientation()
        self.quaternionx = orientation[1]
        self.quaterniony = orientation[2]
        self.quaternionz = orientation[3]

        velocity = quaternion_rotate_vector(orientation, velocity)
        self.vx = velocity[0]
        self.vy = velocity[1]
        self.vz = velocity[2]

        av = state.angular_velocity()
        self.rollrate = av[0]
        self.pitchrate = av[1]
        self.yawrate = av[2]

    def _send(self, msg : str):
        self.sock.sendto(msg.encode(), (self.addr, self.port))
    
    def send(self):
        msg = '\n'.join([
        f'time={self.time}', # in game display time decimal hours 
        f'airspeed={self.airspeed}',
        f'altitude={self.altitude * 3.28084}', # altimeter reading ft 
        f'vario={self.vario}', # pneumatic vario reading m/s 
        f'evario={self.evario}', # electronic variometer reading m/s 
        f'nettovario={self.nettovario}', # netto variometer value m/s 
        f'integrator={self.integrator}', # integrator value m/s 
        f'compass={self.compass}', # compass reading degree 
        f'slipball={self.slipball}', # slip ball deflection angle rad 
        f'turnrate={self.turnrate}', # turn indicator reading rad/s 
        f'yawstringangle={self.yawstringangle}', # yawstring angle rad 
        f'yaw={self.yaw}', # yaw rad 
        f'pitch={-self.pitch}', # pitch rad (note condor expects -ve for pitch up)
        f'bank={self.bank}', # bank rad 
        f'quaternionx={self.quaternionx}', # quaternion x / 
        f'quaterniony={self.quaterniony}', # quaternion y / 
        f'quaternionz={self.quaternionz}', # quaternion z /
        f'vx={self.vx}', # speed vector x m/s 
        f'vy={self.vy}', # speed vector x m/s 
        f'vz={self.vz}', # speed vector x m/s 
        f'rollrate={self.rollrate}', # roll rate (local system x) rad/s 
        f'pitchrate={self.pitchrate}', # pitch rate (local system y) rad/s 
        f'yawrate={self.yawrate}', # yaw rate (local system z) rad/s 
        f'g={self.gforce}', # g force factor /
        ])
        self._send(msg)