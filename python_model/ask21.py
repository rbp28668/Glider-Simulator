
from panel import Panel
from contact_point import ContactPoint

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

        self.Ixx = 1285.0  # kg·m²
        self.Iyy = 1824.0  # kg·m²
        self.Izz = 2663.0  # kg·m²

        self.Mass = 687.0  # kg

        # TODO load aerofoil data from files and add to panels

        # Panels:  area, mid-span, quarter-chord position, incidence angle
        # mid-span is distance from centerline to panel center
        # quarter-chord is distance from root leading edge (datum) to quarter-chord of panel
        rootPanel =     Panel( 3.215313306, 1.545740741, 0.3368518519, 0.0)
        airbrakePanel = Panel( 1.648746982, 3.440925926, 0.2935648148, 0.0)  # Placeholder for airbrake panel
        outerPanel =    Panel( 1.112233745, 4.668703704, 0.2644444444, 0.0)
        aileronPanel =  Panel( 2.216578464, 6.585925926, 0.211712963, 0.0)  # Placeholder for aileron panel
        tipPanel =      Panel( 0.287909808, 8.238703704, 0.1629166667, 0.0)  # Placeholder for tip panel
        self.wing = [
            rootPanel,
            airbrakePanel,
            outerPanel,
            aileronPanel,
            tipPanel
        ]  
        
        self.dihedral_angle = 4.0  # degrees (under each tip)
        


        # TODO: Fill in tail and fin panels with accurate data
        self.tailplane_area = 2.96  # m²
        self.tailplane_span = 4.8   # m
        self.tailplane_incidence = 0.0  # degrees
        self.tailplane_quarter_chord = 6.5  # m from datum

        self.fin_area = 1.44  # m²
        self.fin_height = 2.4  # m
        self.fin_incidence = 0.0  # degrees
        self.fin_quarter_chord = 6.0  # m from datum

        self.wing_span = 17.0  # m


        # Contact points for ground detection. Fwd and down are +ve.
        noseWheel = ContactPoint(1.64962963, 0.0, 0.6737037037)  
        mainWheelLeft = ContactPoint(-0.6611111111, 0.0, 0.7303703704) 
        tailWheel = ContactPoint(-5.263703704, 0.0, 0.2203703704)  
        leftTip = ContactPoint(-0.4092592593, -self.wing_span / 2, -0.4533333333)
        rightTip = ContactPoint(-0.4092592593, self.wing_span / 2, -0.4533333333)
        self.contact_points = [
            noseWheel,
            mainWheelLeft,
            tailWheel,
            leftTip,
            rightTip
        ]


        self.aerotow_hook = ContactPoint(2.291851852, 0.0, 0.447037037)  # Approximate position of aerotow hook
        self.winch_hook = ContactPoint(0.1574074074, -0.05, 0.572962963)  # approximate position of winch hook, slightly left of centerline
