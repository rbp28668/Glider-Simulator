from aerofoil import Aerofoil
from math import radians

class Panel:
    """
    Panel is part of a wing.  It may be extended to include control surfaces such as ailerons or airbrakes.
    """
    def __init__(self, span: float, area: float, quater_chord: float, incidenceDegrees: float, rootFoil : Aerofoil , tipFoil : Aerofoil, interp : float = 0.0):
        self.mid_span = span
        self.area = area
        self.quater_chord = quater_chord
        self.incidence = radians(incidenceDegrees)
        self.rootFoil = rootFoil
        self.tipFoil = tipFoil
        self.interp = interp  # interpolation factor between root and tip aerofoils (0.0 = root, 1.0 = tip)


class AileronPanel(Panel):
    """
    AileronPanel is a Panel with an aileron control surface.
    """
    pass

class AirbrakePanel(Panel):
    """
    AirbrakePanel is a Panel with an airbrake control surface.
    """
    pass