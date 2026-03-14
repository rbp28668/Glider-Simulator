#pragma once

#include <cmath>
#include <algorithm>

#include "panel.h"
#include "local_math.h"


Panel::Panel(float area, float mid_span, float quater_chord, float mean_chord, float incidenceDegrees, const Aerofoil& rootFoil, const Aerofoil& tipFoil, float interp)
	: area(area), mid_span(mid_span), quater_chord(quater_chord), incidence(radians(incidenceDegrees)), rootFoil(rootFoil), tipFoil(tipFoil), interp(interp), mean_chord(mean_chord)
{
}

//     Process the panel to calculate forces and moments.
//     Args:
//         state: Current state vector
//         relative_velocity: The aircraft velocity relative to the air around it (in body axes, wind corrected)
//         aircraft: Aircraft parameters including CG position
//         world: The simulation world
//         controls: Current control surface deflections
//         sign: +1 for right wing, -1 for left wing
//         forces - updated by adding in forces_body: [Fx, Fy, Fz] (N)
//         moments - updated by adding in moments_body: [L, M, N] (N.m)
void Panel::process(const StateVector<NumberT>& state, const V3d<NumberT>& relative_velocity, AircraftParameters& aircraft,
	World& world, ControlInputs& controls, NumberT sign, V3d<NumberT>& forces, V3d<NumberT>& moments)
{
	// Get local airflow at panel due to angular velocity
	auto local_velocity = get_local_velocity(state, relative_velocity, sign);

	auto local_tas = local_velocity.TotalAirspeed();

	// Protection against very low airspeed (stall/spin conditions)
	NumberT airspeed_factor = 1.0f;
	if (local_tas < MIN_AIRSPEED)
	{
		// Scale forces smoothly to zero as airspeed drops
		airspeed_factor = local_tas / MIN_AIRSPEED;
		local_tas = MIN_AIRSPEED; // Prevent division issues
	}

	auto local_alpha = local_velocity.AngleOfAttack();
	auto beta = local_velocity.SideslipAngle();

	auto aoa = local_alpha + incidence; // add geometric incidence angle
	assert(!isnan(aoa));


	// Hook: allow subclasses to modify AoA (e.g., aileron deflection)
	aoa = modify_aoa(aoa, controls, sign);

	// Dihedral effect: when slipping right (beta > 0), right wing sees increased AoA,
	// left wing sees decreased AoA. This creates restoring roll moment (Cl_beta).
	// The sign parameter differentiates right (+1) from left (-1) wing.
	// Simple linear model: delta_aoa = dihedral * beta * sign
	// Limited to prevent runaway at extreme sideslip
	const NumberT MAX_DIHEDRAL_BETA = 0.35f;  // ~20 degrees
	auto beta_limited = clamp(beta, -MAX_DIHEDRAL_BETA, MAX_DIHEDRAL_BETA);
	aoa += aircraft.DihedralAngle() * beta_limited * sign;

	assert(!isnan(aoa));
	Aerofoil::Coefficients coeffs = coefficients_at(aoa);

	// Hook: allow subclasses to modify coefficients (e.g., spoiler lift reduction)
	modify_coefficients(coeffs, controls);

	auto Cl = coeffs.Cl;
	auto Cd = coeffs.Cd;
	auto Cm = coeffs.Cm;
	// Lift dependent drag
	auto Cdi = (Cl * Cl) / (NumberT(PI) * aircraft.AR() * aircraft.Oswald());
	Cd += Cdi;

	auto q = 0.5f * world.AirDensity() * local_tas * local_tas; // dynamic pressure
	auto L = Cl * q * area;
	auto D = Cd * q * area;
	auto M = Cm * q * area * mean_chord;

	// Hook: allow subclasses to add extra drag (e.g., deployed airbrakes)
	D += additional_drag(q, controls);

	// Transform from wind axes to body axes (rotation by angle of attack about Y)
	// Wind axes: -X is drag direction, -Z is lift direction
	// Body axes: X forward, Z down
	auto Fx = -(D * cos(local_alpha) - L * sin(local_alpha)); // drag backwards in S&L flight
	auto Fz = -(D * sin(local_alpha) + L * cos(local_alpha));  // lift is -ve Z in body axes

	// Apply low-airspeed scaling
	Fx *= airspeed_factor;
	Fz *= airspeed_factor;
	M *= airspeed_factor;

	// Clamp forces to prevent numerical instability
	Fx = clamp(safe_value(Fx), -MAX_PANEL_FORCE, MAX_PANEL_FORCE);
	Fz = clamp(safe_value(Fz), -MAX_PANEL_FORCE, MAX_PANEL_FORCE);
	M = clamp(safe_value(M), -MAX_PANEL_FORCE * 10, MAX_PANEL_FORCE * 10);

	auto forces_body = V3d<NumberT>(Fx, 0.0, Fz); // drag in body X, side force 0, lift in body Z

	auto dist = quater_chord - aircraft.CG(); // calculate moments from c.g. not datum

	// Moments (about c.g.)
	// Roll moment due to lift at panel mid-span
	auto moments_body = V3d<NumberT>(
		Fz * mid_span * sign, // roll moment
		M - Fz * dist,        // pitch moment about c.g.
		-Fx * mid_span * sign // yaw moment due to drag
		);

	forces += forces_body;
	moments += moments_body;
}

// --- Hook methods for subclasses to override ---

// Hook: modify angle of attack based on control inputs. Override in subclasses.
NumberT Panel::modify_aoa(NumberT aoa, ControlInputs& controls, NumberT sign)
{
	return aoa;
}

// Hook: modify aerodynamic coefficients. Override in subclasses.
void Panel::modify_coefficients(Aerofoil::Coefficients& coeffs, ControlInputs& controls)
{
}

// Hook: add additional drag based on control inputs. Override in subclasses.
NumberT Panel::additional_drag(NumberT q, ControlInputs& controls)
{
	return 0.0;
}

// --- Utility methods ---


// Calculate local velocity at panel due to angular velocity.
// Retreating wing has reduced local airflow, advancing wing has increased local airflow.
// Downgoing wing has increased local airflow, upgoing wing has reduced local airflow.
// Args:
//     state: Current state vector
//     relative_velocity: Relative airframe velocity vector [u, v, w] in body frame relative to air-mass
//     sign: +1 for right wing, -1 for left wing
// Returns:
//     local_airflow: Local airflow vector [u, v, w] at panel in body frame.
V3d<NumberT> Panel::get_local_velocity(const StateVector<NumberT>& state, const V3d<NumberT>& relative_velocity, NumberT sign) const
{
	auto p = state.angular_velocity()[0];
	auto r = state.angular_velocity()[2];

	// Change in z velocity. If rolling right, panel going down and Z increasing
	auto dz = p * mid_span * sign;
	// Change in x velocity. If yawing right, right panel retreating and X decreasing
	auto dx = -r * mid_span * sign;

	auto local_airflow = V3d<NumberT>(relative_velocity[0] + dx, relative_velocity[1], relative_velocity[2] + dz);
	return local_airflow;
}

// Get lift, drag, moment coefficients at given angle of attack.
// Args:
//     aoa: Angle of attack in radians
// Returns:
//     (Cl, Cd, Cm) - lift, drag, moment coefficients
Aerofoil::Coefficients Panel::coefficients_at(NumberT aoa) const
{

	auto rootFraction = 1.0f - interp;
	auto tipFraction = interp;

	auto root = rootFoil.coefficients_at(aoa);
	auto tip = tipFoil.coefficients_at(aoa);

	auto Cl = rootFraction * root.Cl + tipFraction * tip.Cl;
	auto Cd = rootFraction * root.Cd + tipFraction * tip.Cd;
	auto Cm = rootFraction * root.Cm + tipFraction * tip.Cm;
	return Aerofoil::Coefficients(Cl, Cd, Cm);
}


//===================================================================================================
// AileronPanel


// Args:
//     lift_effectiveness: Fraction of deflection that acts as AoA change for lift.
//                         Thin airfoil theory gives ~0.5-0.7 for typical aileron chord ratios.
//     moment_coeff: Change in Cm per radian of deflection (negative = nose down for
//                     trailing-edge-down deflection). Typical range -0.3 to -0.5.
//     profile_drag_coeff: Drag coefficient per radian² of deflection.
AileronPanel::AileronPanel(float area, float mid_span, float quater_chord, float mean_chord, float incidenceDegrees,
	Aerofoil& rootFoil, Aerofoil& tipFoil, float interp,
	float max_up_deg, float max_down_deg,
	float lift_effectiveness, float moment_coeff,
	float profile_drag_coeff)
	: Panel(area, mid_span, quater_chord, mean_chord, incidenceDegrees, rootFoil, tipFoil, interp), max_up(radians(max_up_deg)) // max deflection for up-going aileron
	,
	max_down(radians(max_down_deg)) // max deflection for down-going aileron
	,
	lift_effectiveness(lift_effectiveness) // how much deflection changes effective AoA
	,
	moment_coeff(moment_coeff) // ΔCm per radian of deflection
	,
	profile_drag_coeff(profile_drag_coeff) // drag coefficient per radian² of deflection
{
}

// Aileron deflection changes effective angle of attack (camber effect on lift).
// Differential: up-going aileron can deflect more than down-going.
// - controls.roll * sign > 0: aileron goes up (reduces AoA/lift)
// - controls.roll * sign < 0: aileron goes down (increases AoA/lift)
// The lift_effectiveness factor accounts for the fact that a plain flap
// is less effective at changing lift than a pure AoA change.
NumberT AileronPanel::modify_aoa(NumberT aoa, ControlInputs& controls, NumberT sign)
{
	auto command = controls.aileron * sign;
	auto deflection = (command >= 0) ?
		// Up-going aileron (reduces lift on this wing)
		command * max_up
		:
		// Down-going aileron (increases lift on this wing)
		command * max_down;

	_last_deflection = deflection;
	// Apply lift effectiveness - deflection is less effective than pure AoA change
	return aoa - deflection * lift_effectiveness;
}

// Modify pitching moment due to aileron camber change.
// Trailing-edge-down deflection (positive δ) creates nose-down moment (negative ΔCm).
// This is because the aft camber increase shifts the center of pressure rearward.
void AileronPanel::modify_coefficients(Aerofoil::Coefficients& coeffs, ControlInputs& controls)
{
	// Moment change due to camber: ΔCm = moment_coeff * δ
	// Note: _last_deflection is positive for up-aileron (reduces camber)
	// So we negate it: down deflection should give negative ΔCm
	auto delta_Cm = moment_coeff * (-_last_deflection);
	coeffs.Cm += delta_Cm;
}

NumberT AileronPanel::additional_drag(NumberT q, ControlInputs& controls)
{
	// Deflected aileron adds profile drag proportional to deflection².
	//  Drag increment: Cd = k * δ²
	auto Cd_aileron = profile_drag_coeff * _last_deflection * _last_deflection;
	return Cd_aileron * q * area;
}

//===================================================================================================
// Panel with airbrake/spoiler control surface.
// AirbrakePanel

AirbrakePanel::AirbrakePanel(float area, float mid_span, float quater_chord, float mean_chord, float incidenceDegrees,
	Aerofoil& rootFoil, Aerofoil& tipFoil, float interp)
	: Panel(area, mid_span, quater_chord, mean_chord, incidenceDegrees, rootFoil, tipFoil, interp)
{
}

void AirbrakePanel::modify_coefficients(Aerofoil::Coefficients& coeffs, ControlInputs& controls)
{
	// Airbrake deployment reduces lift coefficient.
	// Crude model: reduce lift proportionally to spoiler deployment
	auto lift_reduction = 0.8f * controls.spoiler;
	coeffs.Cl *= (1.0f - lift_reduction);
}

NumberT AirbrakePanel::additional_drag(NumberT q, ControlInputs& controls)
{
	// Deployed airbrake adds significant drag.
	auto Cd_spoiler = 1.8f;          // flat plate drag coefficient
	auto spoiler_area = 0.3f * area; // airbrake is ~1/3 of panel area
	return Cd_spoiler * q * spoiler_area * controls.spoiler;
}
