
#pragma once
// Interface for aircraft parameters
class AircraftParameters{

    public:
    virtual float CG() const = 0;                    // Center of gravity from datum in meters
    virtual float AR() const = 0;                     // Aspect ratio
    virtual float Oswald() const = 0;                 // Oswald efficiency factor
    virtual float DihedralAngle() const = 0;         // Dihedral angle in radians
};