from math import radians
from aircraft_params import AircraftParameters
from panel import Panel, AirbrakePanel, AileronPanel
from contact_point import ContactPoint
from aerofoil import Aerofoil

class ASK21:

    """Class representing the ASK21 glider model with its physical and aerodynamic properties.
    Note coordinate system.  
    Origin: Datum at root leading edge on fuselage centerline
    X-axis: Forward through nose (positive forward)
    Y-axis: Right wing direction (starboard positive)
    Z-axis: Down through belly (perpendicular to XY plane, positive down)
    """

    def __init__(self):
        pass
        # Initialize ASK21-specific parameters here

        #Moments of inertia about principal axes through center of gravity (roll, pitch, yaw)
        self.Ixx = 1285.0  # kg·m²
        self.Iyy = 1824.0  # kg·m²
        self.Izz = 2663.0  # kg·m²
        self.Ixz = 100.0     # kg·m² Wild guess
        self.mass = 687.0  # kg

        self.cg = -0.30  # m from datum (negative is aft of datum), was 30 originally

        root = Aerofoil('FX-60-126.txt')
        tip = Aerofoil('FX-02-196.txt')

        incidenceDegrees = 0.6 # was 3.192472613
        # Panels:  area, mid-span position from centreline, quarter-chord position (relative to datum), mean chord, incidence angle
        # mid-span is distance from centerline to panel center
        # quarter-chord is distance from root leading edge (datum) to quarter-chord of panel
        # root and tip are the root and tip sections and interp determines interpolation between them.
        rootPanel =     Panel(         3.215313306, 1.545740741, -0.3368518519, 1.347407407, incidenceDegrees, root, tip, 0.0)
        airbrakePanel = AirbrakePanel( 1.648746982, 3.440925926, -0.2935648148, 1.174259259, incidenceDegrees, root, tip, 0.2)  
        outerPanel =    Panel(         1.112233745, 4.668703704, -0.2644444444, 1.057777778, incidenceDegrees, root, tip, 0.5)
        aileronPanel =  AileronPanel(  2.216578464, 6.585925926, -0.211712963, 0.7964814815, incidenceDegrees, root, tip, 0.7,
                                        max_up_deg=18.0, max_down_deg=12.0,  # differential aileron
                                        lift_effectiveness=0.6, moment_coeff=-0.4, profile_drag_coeff=0.01)  
        tipPanel =      Panel(         0.287909808, 8.238703704, -0.1629166667, 0.5509259259, incidenceDegrees, root, tip, 1.0)  
        self.wing = [
            rootPanel,
            airbrakePanel,
            outerPanel,
            aileronPanel,
            tipPanel
        ]  
        
        




        self.dihedral_angle = radians(4.0)  # degrees (under each tip)
        
        # Tailplane and fin aerofoils
        tail = Aerofoil('NACA0010.txt')

        # Tailplane area and moment
        self.tailplane_area = 1.796160768  # m²
        self.tailplane_incidence = radians(-2.5)  # tailplane incidence relative to fuselage (negative = download at trim)
        self.tailplane_quarter_chord = -5.210185185  # m from datum
        self.tailplane = tail

        # Fin area and moment
        self.fin_area = 1.412849246  # m²
        self.fin_incidence = 0.0  # degrees
        self.fin_quarter_chord = -5.082685185  # m from datum
        self.fin = tail

        self.wing_span = 17.0  # m
        self.mean_chord = 1.121 # m


        # Contact points for ground detection. Relative to datum. Fwd and down are +ve.
        # contact_type determines stiffness, damping, friction and max penetration:
        #   nose_wheel: 5cm max penetration
        #   main_wheel: 10cm max penetration
        #   tail_wheel: 5cm max penetration
        #   wingtip: 2cm max penetration
        noseWheel = ContactPoint(1.64962963, 0.0, 0.6737037037, contact_type='nose_wheel')
        mainWheel = ContactPoint(-0.6611111111, 0.0, 0.7303703704, contact_type='main_wheel')
        tailWheel = ContactPoint(-5.263703704, 0.0, 0.2203703704, contact_type='tail_wheel')
        leftTip = ContactPoint(-0.4092592593, -self.wing_span / 2, -0.4533333333, contact_type='wingtip')
        rightTip = ContactPoint(-0.4092592593, self.wing_span / 2, -0.4533333333, contact_type='wingtip')
        self.contact_points = [
            noseWheel,
            mainWheel,
            tailWheel,
            leftTip,
            rightTip
        ]


        self.aerotow_hook = ContactPoint(2.291851852, 0.0, 0.447037037)  # Approximate position of aerotow hook
        self.winch_hook = ContactPoint(0.1574074074, -0.05, 0.572962963)  # approximate position of winch hook, slightly left of centerline

    def get_params(self) -> AircraftParameters :
        params = AircraftParameters()
        params.AR = self.wing_span / self.mean_chord
        params.oswald = 0.95
        params.CG = self.cg
        params.dihedral_angle = self.dihedral_angle

        return params