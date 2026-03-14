#pragma once


#include "TestBase.h"

class TestFinEffect : public TestBase
{


	// Case 1 - Straight and level flight
	void test_straight_and_level() {
		state.set_velocity(25.0f, 0.0f, 0.0f);  // 25 m / s level flight
		state.set_position(0.0f, 0.0f, -500.0f);

		V3d<NumberT> fin_forces;
		V3d<NumberT> fin_moments;
		model.fin_forces(state, aircraft, controls, world, state.velocity(), fin_forces, fin_moments);


		std::cout << "S&L, Fin Force: " << fin_forces << "," << "Fin moments : " << fin_moments << std::endl;

		assertLess(fin_forces[0], 0.0f, "Fin force X component should be negative due to drag");
		assertEqual(fin_forces[1], 0.0f, "Fin force Y component should not be zero");
		assertEqual(fin_forces[2], 0.0f, "Fin force Z component should be zero");

		assertEqual(fin_moments[0], 0.0f, "Fin moment roll should be zero");
		assertEqual(fin_moments[1], 0.0f, "Fin moment pitch should be zero");
		assertEqual(fin_moments[2], 0.0f, "Fin moment yaw should be zero");
	}

	// Case 2 - aircraft with right velocity component in body frame(nose yawed left)
	void test_yawed_left() {
		state.set_velocity(25.0f, 1.0f, 0.0f);  // 25 m / s level flight
		V3d<NumberT> fin_forces;
		V3d<NumberT> fin_moments;
		model.fin_forces(state, aircraft, controls, world, state.velocity(), fin_forces, fin_moments);

		std::cout << "Yawed left, Fin Force: " << fin_forces << "," << "Fin moments : " << fin_moments << std::endl;

		assertLess(fin_forces[1], 0.0f, "Fin force Y component should be negative"); // Should produce a left force to counter right velocity
		assertEqual(fin_forces[2], 0.0f, "Fin force Z component should be zero");

		assertGreater(fin_moments[2], 0.0f, "Fin moment yaw should be positive "); // should yaw into oncoming airflow
	}

	// Case 3 - aircraft with left velocity component in body frame(nose yawed right)
	void test_yawed_right() {
		state.set_velocity(25.0f, -1.0f, 0.0f);  // 25 m / s level flight
		V3d<NumberT> fin_forces;
		V3d<NumberT> fin_moments;
		model.fin_forces(state, aircraft, controls, world, state.velocity(), fin_forces, fin_moments);
		std::cout << "Yawed right, Fin Force: " << fin_forces << "," << "Fin moments : " << fin_moments << std::endl;

		assertGreater(fin_forces[1], 0.0f, "Fin force Y component should be positive"); // Should produce a right force to counter left velocity
		assertEqual(fin_forces[2], 0.0f, "Fin force Z component should be zero");

		assertLess(fin_moments[2], 0.0f, "Fin moment yaw should be negative "); // should yaw into oncoming airflow
	}


	// Case 4 - aircraft with with right yaw rate
	// If nose to the right, tail will be going left, so fin should produce a right force to counteract
	void test_yawing_right() {
		state.set_velocity(25.0f, 0.0f, 0.0f);  // 25 m / s level flight
		state.set_angular_velocity(0.0f, 0.0f, 0.1f);  // Yaw rate to right
		V3d<NumberT> fin_forces;
		V3d<NumberT> fin_moments;

		model.fin_forces(state, aircraft, controls, world, state.velocity(), fin_forces, fin_moments);
		std::cout << "Yawing right, Fin Force: " << fin_forces << "," << "Fin moments : " << fin_moments << std::endl;

		assertGreater(fin_forces[1], 0.0f, "Fin force Y component should be positive"); // Should produce a right force to counter right yaw rate
		assertEqual(fin_forces[2], 0.0f, "Fin force Z component should be zero");

		assertLess(fin_moments[2], 0.0f, "Fin moment yaw should be negative "); // should oppose yaw rate

	}

	// Case 5 - aircraft with with left yaw rate
	// If nose yawing to the left, tail will be going right, so fin should produce a left force to counteract
	void test_yawing_left() {
		state.set_velocity(25.0f, 0.0f, 0.0f);  // 25 m / s level flight
		state.set_angular_velocity(0.0f, 0.0f, -0.1f);  // Yaw rate to left
		V3d<NumberT> fin_forces;
		V3d<NumberT> fin_moments;
		model.fin_forces(state, aircraft, controls, world, state.velocity(), fin_forces, fin_moments);
		std::cout << "Yawing left, Fin Force: " << fin_forces << "," << "Fin moments : " << fin_moments << std::endl;

		assertLess(fin_forces[1], 0.0f, "Fin force Y component should be negative"); // Should produce a left force to counter left yaw rate
		assertEqual(fin_forces[2], 0.0f, "Fin force Z component should be zero");

		assertGreater(fin_moments[2], 0.0f, "Fin moment yaw should be negative "); // should oppose yaw rate
	}
	// Case 6 - test increasing yaw - as per //2 but more so
	void test_increasing_yaw_angle_left() {

		state.set_angular_velocity(0.0f, 0.0f, 0.0f);  // No yaw rate

		// Just increasing yaw, constant Vx - i.e. 0 to 45 degrees
		for (NumberT vy = 1; vy <= 25; vy += 1) {
			state.set_velocity(25.0f, vy, 0.0f);  // 25 m / s level flight
			V3d<NumberT> fin_forces;
			V3d<NumberT> fin_moments;
			model.fin_forces(state, aircraft, controls, world, state.velocity(), fin_forces, fin_moments);
			std::cout << "Yawed left: Vy=" << vy << ", Fin Force : " << fin_forces << ", " << "Fin moments : " << fin_moments << std::endl;

			assertLess(fin_forces[1], 0.0f, "Fin force Y component should be negative"); // Should produce a left force to counter left yaw rate
			assertEqual(fin_forces[2], 0.0f, "Fin force Z component should be zero");

			assertGreater(fin_moments[2], 0.0f, "Fin moment yaw should be negative "); // should oppose yaw rate
		}


		// increasing side velocity, decreasing forward 0..90 degrees
		for (NumberT vy = 1; vy <= 25; vy += 1) {
			state.set_velocity(25.0f - vy, vy, 0.0f);  // 25 m / s level flight

			V3d<NumberT> fin_forces;
			V3d<NumberT> fin_moments;
			model.fin_forces(state, aircraft, controls, world, state.velocity(), fin_forces, fin_moments);
			std::cout << "Yawed left: Vy=" << vy << ", Fin Force : " << fin_forces << ", " << "Fin moments : " << fin_moments << std::endl;

			assertLess(fin_forces[1], 0.0f, "Fin force Y component should be negative"); // Should produce a left force to counter left yaw rate
			assertEqual(fin_forces[2], 0.0f, "Fin force Z component should be zero");

			assertGreater(fin_moments[2], 0.0f, "Fin moment yaw should be negative "); // should oppose yaw rate
		}
	}

	// Case 3 - aircraft with left velocity component in body frame(nose yawed right)
	void test_increasing_yaw_angle_right() {

		state.set_angular_velocity(0.0f, 0.0f, 0.0f);  // No yaw rate

		for (NumberT vy = 1; vy <= 25; vy += 1) {
			state.set_velocity(25.0f, -vy, 0.0f);  // 25 m / s level flight
			V3d<NumberT> fin_forces;
			V3d<NumberT> fin_moments;
			model.fin_forces(state, aircraft, controls, world, state.velocity(), fin_forces, fin_moments);
			std::cout << "Yawed right: Vy=" << vy << ", Fin Force : " << fin_forces << ", " << "Fin moments : " << fin_moments << std::endl;

			assertGreater(fin_forces[1], 0.0f, "Fin force Y component should be positive"); // Should produce a right force to counter left velocity
			assertEqual(fin_forces[2], 0.0f, "Fin force Z component should be zero");

			assertLess(fin_moments[2], 0.0f, "Fin moment yaw should be negative "); // should yaw into oncoming airflow
		}
		for (NumberT vy = 1; vy <= 25; vy += 1) {
			state.set_velocity(25.0f - vy, -vy, 0.0f);  // 25 m / s level flight
			V3d<NumberT> fin_forces;
			V3d<NumberT> fin_moments;
			model.fin_forces(state, aircraft, controls, world, state.velocity(), fin_forces, fin_moments);
			std::cout << "Yawed right: Vy=" << vy << ", Fin Force : " << fin_forces << ", " << "Fin moments : " << fin_moments << std::endl;

			assertGreater(fin_forces[1], 0.0f, "Fin force Y component should be positive"); // Should produce a right force to counter left velocity
			assertEqual(fin_forces[2], 0.0f, "Fin force Z component should be zero");

			assertLess(fin_moments[2], 0.0f, "Fin moment yaw should be negative "); // should yaw into oncoming airflow
		}
	}
	public:

	void test_all() {
		test_straight_and_level();
		test_yawed_left();
		test_yawed_right();
		test_yawing_right();
		test_yawing_left();
		test_increasing_yaw_angle_left();
		test_increasing_yaw_angle_right();
	}
};
