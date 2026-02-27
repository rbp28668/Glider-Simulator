
#pragma once

#include <cmath>
#include <algorithm>

#include "model.h"
#include "local_math.h"


// Calculate aerodynamic forces and moments
// Args:
//     state: Current state vector
//     aircraft: The aircraft model
//     control_inputs: Current control surface deflections
//     world: The simulation world
//     relative_velocity: The aircraft velocity relative to the air-mass (in body axes, wind corrected)
// Returns:
//     forces_body: [Fx, Fy, Fz] (N)
//     moments_body: [L, M, N] (N·m)
void Model::calculate_aerodynamics(const StateVector<float>& state, const ASK21& aircraft, const ControlInputs& control_inputs, const World& world, const V3d<float>& relative_velocity,
	V3d<float>& forces, V3d<float>& moments) {


	V3d<float> forces_body;
	V3d<float> moments_body;

	//Wings
	for (auto iter = aircraft.wing.begin(); iter != aircraft.wing.end(); ++iter) {
		Panel* panel = *iter;
		// Right hand side            
		panel->process(state, relative_velocity, (AircraftParameters&)aircraft, (World&)world, (ControlInputs&)control_inputs, 1.0, forces_body, moments_body);
		// Left hand side
		panel->process(state, relative_velocity, (AircraftParameters&)aircraft, (World&)world, (ControlInputs&)control_inputs, -1.0, forces_body, moments_body);
	}

	//Tailplane
	tailplane_forces(state, aircraft, control_inputs, world, relative_velocity, forces_body, moments_body);

	//Fin
	fin_forces(state, aircraft, control_inputs, world, relative_velocity, forces_body, moments_body);

	//Fuselage
	fuselage_forces(state, aircraft, control_inputs, world, relative_velocity, forces_body, moments_body);



	// TODO - Cm_beta : pitch down with sideslip

	// High-rate damping to prevent unrealistic spin-up during stall
	// This always applies, regardless of airspeed, to ensure stability
	// when normal aero damping breaks down at high AoA
	const float HIGH_RATE_THRESHOLD = 0.5f;  // rad/s (~30 deg/s)
	const float HIGH_RATE_DAMP = 8000.0f;    // N.m.s/rad

	auto roll_rate = state.angular_velocity()[0];
	auto yaw_rate = state.angular_velocity()[2];

	if (std::abs(roll_rate) > HIGH_RATE_THRESHOLD) {
		float excess_rate = roll_rate - std::copysign(HIGH_RATE_THRESHOLD, roll_rate);
		moments_body[0] -= HIGH_RATE_DAMP * excess_rate;
	}

	if (std::abs(yaw_rate) > HIGH_RATE_THRESHOLD) {
		float excess_rate = yaw_rate - std::copysign(HIGH_RATE_THRESHOLD, yaw_rate);
		moments_body[2] -= HIGH_RATE_DAMP * excess_rate;
	}

	// Sanitize and clamp final forces/moments to prevent numerical overflow
	forces[0] = clamp(safe_value(forces_body[0]), -MAX_FORCE, MAX_FORCE);
	forces[1] = clamp(safe_value(forces_body[1]), -MAX_FORCE, MAX_FORCE);
	forces[2] = clamp(safe_value(forces_body[2]), -MAX_FORCE, MAX_FORCE);
	moments[0] = clamp(safe_value(moments_body[0]), -MAX_MOMENT, MAX_MOMENT);
	moments[1] = clamp(safe_value(moments_body[1]), -MAX_MOMENT, MAX_MOMENT);
	moments[2] = clamp(safe_value(moments_body[2]), -MAX_MOMENT, MAX_MOMENT);

	return;
}

// Calculate tailplane aerodynamic forces
// Args:
//     state: Current state vector
//     aircraft: The aircraft model
//     relative_velocity: The aircraft velocity relative to the air-mass (in body axes, wind corrected)
//     world: The simulation world
// Returns:
//     nothing - updates forces and moments passed by reference
void Model::tailplane_forces(const StateVector<float>& state, const ASK21& aircraft, const ControlInputs& control_inputs, const World& world, const V3d<float>& relative_velocity,
	V3d<float>& forces, V3d<float>& moments) {

	//Allow for pitch rate to change airflow at tail.  Pitching up then tail going down (+ve direction)
	auto dist = aircraft.tailplane_quarter_chord - aircraft.cg;  // distance of tailplane A/C from c of g  (-ve )
	auto pitch_rate = state.angular_velocity()[1];               // +ve pitch rate -> nose up so tail down (+ve z dirn)
	auto vz_pitch = pitch_rate * -dist;

	auto tailplane_velocity = V3d<float>(
		relative_velocity[0],  // u - velocity forward
		relative_velocity[1],  // v - velocity to right
		relative_velocity[2] + vz_pitch); // add in extra vertical velocity do to pitch rate


	auto tas = tailplane_velocity.TotalAirspeed();

	// Low airspeed protection
	if (tas < MIN_AIRSPEED)
		return; // without making changes to forces/moments

	auto aoa_raf = tailplane_velocity.AngleOfAttack();
	auto aoa = aoa_raf + aircraft.tailplane_incidence;

	// Elevator effect: positive elevator input (forward stick) reduces tailplane AoA
	// Forward stick → elevator trailing edge DOWN → less lift at tail → nose down
	auto elevator_deflection = control_inputs.elevator * aircraft.elevator_max_deflection;
	aoa -= elevator_deflection * 0.6f;  // Elevator effectiveness ~0.6 (plain flap factor)

	auto coeffs = aircraft.tailplane.coefficients_at(aoa);
	auto Cl = coeffs.Cl;
	auto Cd = coeffs.Cd;
	auto Cm = coeffs.Cm;

	auto q = 0.5f * world.AirDensity() * tas * tas;
	auto tp_L = Cl * q * aircraft.tailplane_area;
	auto tp_D = Cd * q * aircraft.tailplane_area;

	// Pitching moment from tailplane airfoil Cm
	// M = Cm * q * S * c (positive Cm = nose up)
	auto M = Cm * q * aircraft.tailplane_area * aircraft.tailplane_chord;

	// Transform from wind axes to body axes (rotation by angle of attack about Y)
	auto D = -(tp_D * cos(aoa_raf) - tp_L * sin(aoa_raf));  // drag backwards
	auto L = -(tp_D * sin(aoa_raf) + tp_L * cos(aoa_raf));  // lift up is -ve Z in body axes

	//       tp_L, tp_D, tp_M = self.tailplane_forces(state, aircraft, relative_velocity, control_inputs, world) 
	forces[0] += D;     // drag in body X
	forces[2] += L;     // lift in body z

	// Moments(about c.g.).Note: dist is negative when tail is behind CG.
	// pitchMoment = M + L * dist where positive L(downwards) and positive dist
	// produce a positive pitchMoment(nose - up) as expected for tail downforce.
	auto pitchMoment = M + L * -dist;
	moments[1] += pitchMoment;  // pitch moment due to lift at tailplane quarter chord (- sign as dist is -ve as behind c.g.)
}

void Model::fin_forces(const StateVector<float>& state, const ASK21& aircraft, const ControlInputs& control_inputs, const World& world, const V3d<float>& relative_velocity,
	V3d<float>& forces, V3d<float>& moments) {
	// Distance from CG to fin (positive = aft of CG)
	auto fin_arm = aircraft.cg - aircraft.fin_quarter_chord;  // positive value (~4.78m)
	assert(fin_arm > 0);

	// Yaw rate effect on fin airflow
	// When yawing right (r > 0), fin at x<0 moves left, experiencing "headwind" from left
	// This reduces the v-component of airflow at fin, creating restoring moment
	auto yaw_rate = state.angular_velocity()[2];
	auto v_y_induced = yaw_rate * fin_arm;
	auto fin_airflow = V3d<float>(relative_velocity[0],
		relative_velocity[1] - v_y_induced,
		relative_velocity[2]);

	auto fin_tas = fin_airflow.TotalAirspeed();

	// Low airspeed protection
	if (fin_tas < MIN_AIRSPEED)
		return; // without making changes to forces/moments

	//Beta at the fin
	// If local_airflow[1] is positive(air from right), beta is positive.
	auto beta_fin_raf = atan2(fin_airflow[1], fin_airflow[0]);

	// Rudder: Right rudder(+1) should pull the tail LEFT(+Fy) to yaw nose RIGHT.
	// This means right rudder must create 'negative lift' in aero terms.
	auto beta_fin = beta_fin_raf + control_inputs.rudder * radians(15.0f);

	auto coeffs = aircraft.fin.coefficients_at(beta_fin);
	auto Cl = coeffs.Cl;
	auto Cd = coeffs.Cd;

	auto q = 0.5f * world.AirDensity() * fin_tas * fin_tas;
	auto L = Cl * q * aircraft.fin_area;
	auto D = Cd * q * aircraft.fin_area;

	// Transform from wind axes to body axes
	auto fx = -(D * cos(beta_fin_raf) - L * sin(beta_fin_raf));  // drag backwards
	auto fy = -(D * sin(beta_fin_raf) + L * cos(beta_fin_raf));  // side force (fin "lift")

	forces[0] += fx;     // drag in body X
	forces[1] += fy;     // side force already in body frame (negative = left)

	// Moments (about c.g.)
	// Yaw moment = position_x × Fy = dist × L
	auto dist = aircraft.fin_quarter_chord - aircraft.cg;
	assert(dist < 0);

	moments[2] += fy * dist;  // yaw moment from side force at fin


}

// Calculate forces and moments for the fuselage using a slender - body
// aerodynamic approximation.

void Model::fuselage_forces(const StateVector<float>& state, const ASK21& aircraft, const ControlInputs& control_inputs, const World& world, const V3d<float>& relative_velocity,
	V3d<float>& forces, V3d<float>& moments) {


	auto u = relative_velocity[0];
	auto v = relative_velocity[1];
	auto w = relative_velocity[2];

	auto V_sq = u * u + v * v + w * w;
	auto V = sqrt(V_sq);

	// Avoid division by zero at rest
	if (V < 0.1) return;

	// 1. Environment and Dynamic Pressure
	auto rho = world.AirDensity(); //#(state.position()[2])
	auto q_inf = 0.5f * rho * V_sq;

	// 2. Local Flow Angles
	// alpha(pitch) and beta(sideslip)
	auto alpha = (u != 0) ? atan2(w, u) : 0.0f;
	auto beta = asin(clamp(v / V, -1.0f, 1.0f));

	// 3. Aerodynamic Coefficients for ASK21 Fuselage
	// These are typical values for a high - performance tandem glider
	auto    C_d0 = aircraft.C_d0;       // Baseline parasite drag
	auto C_y_beta = aircraft.C_y_beta;   // Side force coefficient per radian
	auto C_z_alpha = aircraft.C_z_alpha;  // Vertical force coefficient(negligible lift)
	auto C_m_alpha = aircraft.C_m_alpha;   // Pitching instability(destabilizing)
	auto C_n_beta = aircraft.C_n_beta;   // Yawing instability(Munk moment)

	// 4. Force Calculation(Body Frame)
	// Drag is always opposite to the velocity vector
	auto drag = q_inf * aircraft.S * (C_d0 + 0.1f * alpha * alpha); // Simplified polar

	// Convert drag to body components and add transverse forces
	auto fx = -drag * (u / V);
	auto fy = q_inf * aircraft.S * C_y_beta * beta;
	auto fz = q_inf * aircraft.S * (C_z_alpha * alpha) - (drag * (w / V));

	forces[0] += fx;
	forces[1] += fy;
	forces[2] += fz;

	// 5. Moment Calculation(Body Frame)
	// Moments are referenced to the Mean Aerodynamic Chord(c_bar) and Wingspan(b)
	auto l_roll = 0.0f; // Fuselage roll contribution is usually negligible
	auto c_bar = aircraft.mean_chord;
	auto b = aircraft.wing_span;
	auto m_pitch = q_inf * aircraft.S * c_bar * C_m_alpha * alpha;
	auto n_yaw = q_inf * aircraft.S * b * C_n_beta * beta;

	// Note - yaw moment is an approximation and is only valid for small beta.
	// Munk moment : Mm = -1 / 2 . (Azz − Axx).U * *2.sin(2∂)
	// This starts at zero for zero betaand returns to zero at 90 degrees
	// Note also that vortex shedding at higher values of beta is likely to be stablising
	// as well as creating drag.So, at the moment :
	m_pitch = 0;
	n_yaw = 0;

	moments[0] += l_roll;
	moments[1] += m_pitch;
	moments[2] += n_yaw;

	return;

}



