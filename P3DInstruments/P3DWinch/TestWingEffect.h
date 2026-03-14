#pragma once
#include "TestBase.h"
class TestWingEffect :
	public TestBase
{


	// Case 1 - Straight and level flight
	void test_straight_and_level() {
		state.set_velocity(25.0, 0.0, 0.0);  // 25 m / s level flight
		state.set_angular_velocity(0.0, 0.0, 0.0); // Zero yaw rate

		V3d<NumberT> forces;
		V3d<NumberT> moments;
		model.wing_forces(state, aircraft, controls, world, state.velocity(), forces, moments);
		std::cout << "S&L, Wing Force: " << forces << ", Wing moments : " << moments << std::endl;

		assertLess(forces[0], 0.0, "Wing force X component should be negative due to drag");
		assertEqual(forces[1], 0.0, "Wing force Y component should be zero"); // no sideforce
		assertLess(forces[2], 0.0, "Wing force Z component should be negative (up)");

		assertEqual(moments[0], 0.0, "Wing moment roll should be zero");
		assertLess(moments[1], 0.0, "Wing moment pitch should be negative"); // pitch nose down
		assertEqual(moments[2], 0.0, "Wing moment yaw should be zero");
	}


	// Case 2 - aircraft with right velocity component in body frame(nose yawed left)
	void test_yawed_left() {
		state.set_velocity(25.0, 1.0, 0.0);  // 25 m / s level flight
		state.set_angular_velocity(0.0, 0.0, 0.0); // Zero yaw rate

		V3d<NumberT> forces;
		V3d<NumberT> moments;
		model.wing_forces(state, aircraft, controls, world, state.velocity(), forces, moments);
		std::cout << "Yawed left, Wing Force: " << forces << ", Wing moments : " << moments << std::endl;

		assertLess(forces[0], 0.0, "Wing force X component should be negative"); // drag
		assertEqual(forces[1], 0.0, "Wing force Y component should be zero");
		assertLess(forces[2], 0.0, "Wing force Z component should be negative"); // lift - ve is up

		assertLess(moments[0], 0.0, "Wing moment roll should be negative");  // Slip to the right should roll left
		assertLess(moments[1], 0.0, "Wing moment pitch should be negative"); // pitch nose down
		assertGreater(moments[2], 0.0, "Wing moment yaw should be positive "); // should yaw into oncoming airflow

	}
	// Case 3 - aircraft with left velocity component in body frame(nose yawed right)
	void test_yawed_right() {
		state.set_velocity(25.0, -1.0, 0.0);  // 25 m / s level flight
		state.set_angular_velocity(0.0, 0.0, 0.0); // Zero yaw rate

		V3d<NumberT> forces;
		V3d<NumberT> moments;
		model.wing_forces(state, aircraft, controls, world, state.velocity(), forces, moments);
		std::cout << "Yawed right, Wing Force: " << forces << ", Wing moments : " << moments << std::endl;

		assertLess(forces[0], 0.0, "Wing force X component should be negative"); // drag
		assertEqual(forces[1], 0.0, "Wing force Y component should be zero");    // no sideforce(as a first approximation anyway)
		assertLess(forces[2], 0.0, "Wing force Z component should be negative"); // lift - ve is up

		assertGreater(moments[0], 0.0, "Wing moment roll should be negative");  // Slip to the left should roll right
		assertLess(moments[1], 0.0, "Wing moment pitch should be negative"); // pitch nose down
		assertLess(moments[2], 0.0, "Wing moment yaw should be positive "); // should yaw into oncoming airflow
	}

	// Case 4 - aircraft with with right yaw rate
	// If nose to the right, tail will be going left, so Wing should produce a right force to counteract
	void test_yawing_right() {
		state.set_velocity(25.0, 0.0, 0.0);  // 25 m / s level flight
		state.set_angular_velocity(0.0, 0.0, 0.1f);  // Yaw rate to right

		V3d<NumberT> forces;
		V3d<NumberT> moments;
		model.wing_forces(state, aircraft, controls, world, state.velocity(), forces, moments);
		std::cout << "Yawing right, Wing Force: " << forces << ", Wing moments : " << moments << std::endl;

		assertLess(forces[0], 0.0, "Wing force Y component should be negative"); // Drag is - ve
		assertEqual(forces[1], 0.0, "Wing force Y component should be zero"); // No sideways component of wing
		assertLess(forces[2], 0.0, "Wing force Z component should be negative (up)"); // lift - ve is up

		assertLess(moments[2], 0.0, "Wing moment yaw should be negative "); // differential drag should oppose yaw rate

	}
	// Case 5 - aircraft with with left yaw rate
	// If nose yawing to the left, tail will be going right, so Wing should produce a left force to counteract
	void test_yawing_left() {
		state.set_velocity(25.0f, 0.0, 0.0);  // 25 m / s level flight
		state.set_angular_velocity(0.0, 0.0, -0.1f);  // Yaw rate to left

		V3d<NumberT> forces;
		V3d<NumberT> moments;
		model.wing_forces(state, aircraft, controls, world, state.velocity(), forces, moments);
		std::cout << "Yawing left, Wing Force: " << forces << ", Wing moments : " << moments << std::endl;

		assertLess(forces[0], 0.0, "Wing force Y component should be negative"); // Drag is - ve
		assertEqual(forces[1], 0.0, "Wing force Y component should be zero"); // No sideways component of wing
		assertLess(forces[2], 0.0, "Wing force Z component should be negative (up)"); // lift - ve is up

		assertGreater(moments[2], 0.0, "Wing moment yaw should be positive "); // should oppose yaw rate


	}


	// Case 6, as per //2 - aircraft with right velocity component in body frame(nose yawed left); but increasing angles
	void test_increasing_yaw_angle_left() {

		state.set_angular_velocity(0.0, 0.0, 0.0); // Zero yaw rate
		for (int vy = 1; vy < 25; ++vy) {
			state.set_velocity(25.0, (NumberT)vy, 0.0);  // 25 m / s level flight, increasing vy
			V3d<NumberT> forces;
			V3d<NumberT> moments;
			model.wing_forces(state, aircraft, controls, world, state.velocity(), forces, moments);
			//print(f"Yawed left Vy:{vy} Wing Force: {forces}, Wing moments: {moments}")

			assertLess(forces[0], 0.0, "Wing force X component should be negative"); // drag
			assertEqual(forces[1], 0.0, "Wing force Y component should be zero");
			assertLess(forces[2], 0.0, "Wing force Z component should be negative"); // lift - ve is up

			assertLess(moments[0], 0.0, "Wing moment roll should be negative");  // Slip to the right should roll left
			assertLess(moments[1], 0.0, "Wing moment pitch should be negative"); // pitch nose down
			assertGreater(moments[2], 0.0, "Wing moment yaw should be positive "); // should yaw into oncoming airflow(lift dependent drag)
		}
	}
	// Case 7, as per //3 - aircraft with left velocity component in body frame(nose yawed right); but increasing angles
	void test_increasing_yaw_angle_right() {
		state.set_angular_velocity(0.0, 0.0, 0.0); // Zero yaw rate

		for (int vy = 1; vy < 25; ++vy) {
			state.set_velocity(25.0f, (NumberT)-vy, 0.0);  // 25 m / s level flight, increasing vy

			V3d<NumberT> forces;
			V3d<NumberT> moments;
			model.wing_forces(state, aircraft, controls, world, state.velocity(), forces, moments);
			//print(f"Yawed right, Vy={-vy}, Wing Force: {forces}, Wing moments: {moments}")

			assertLess(forces[0], 0.0, "Wing force X component should be negative"); // drag
			assertEqual(forces[1], 0.0, "Wing force Y component should be zero");    // no sideforce(as a first approximation anyway)
			assertLess(forces[2], 0.0, "Wing force Z component should be negative"); // lift - ve is up

			assertGreater(moments[0], 0.0, "Wing moment roll should be negative");  // Slip to the left should roll right
			assertLess(moments[1], 0.0, "Wing moment pitch should be negative"); // pitch nose down
			assertLess(moments[2], 0.0, "Wing moment yaw should be positive "); // should yaw into oncoming airflow
		}
	}
	// Case 8 - Test rolling right
	void test_rolling_right() {
		state.set_velocity(25.0f, 0.0, 0.0);  // 25 m / s level flight
		state.set_angular_velocity(0.1f, 0.0, 0.0); //  + ve roll rate

		V3d<NumberT> forces;
		V3d<NumberT> moments;
		model.wing_forces(state, aircraft, controls, world, state.velocity(), forces, moments);
		std::cout << "Rolling right, Wing Force: " << forces << ", Wing moments : " << moments << std::endl;

		assertLess(forces[0], 0.0, "Wing force X component should be negative due to drag");
		assertEqual(forces[1], 0.0, "Wing force Y component should be zero"); // no sideforce
		assertLess(forces[2], 0.0, "Wing force Z component should be negative (up)");

		assertLess(moments[0], 0.0, "Wing moment roll should be negative"); // in opposition to + ve roll
		assertLess(moments[1], 0.0, "Wing moment pitch should be negative"); // pitch nose down
		assertGreater(moments[2], 0.0, "Wing moment yaw should be positive"); // small yaw due to differential left dependent drag
	}
	// Case 9 - Test rolling left
	void test_rolling_left() {
		state.set_velocity(25.0f, 0.0, 0.0);  // 25 m / s level flight
		state.set_angular_velocity(-0.1f, 0.0, 0.0); //  - ve roll rate

		V3d<NumberT> forces;
		V3d<NumberT> moments;
		model.wing_forces(state, aircraft, controls, world, state.velocity(), forces, moments);
		std::cout << "Rolling left, Wing Force: " << forces << ", Wing moments : " << moments << std::endl;

		assertLess(forces[0], 0.0, "Wing force X component should be negative due to drag");
		assertEqual(forces[1], 0.0, "Wing force Y component should be zero"); // no sideforce
		assertLess(forces[2], 0.0, "Wing force Z component should be negative (up)");

		assertGreater(moments[0], 0.0, "Wing moment roll should be positive"); // in opposition to + ve roll
		assertLess(moments[1], 0.0, "Wing moment pitch should be negative"); // pitch nose down
		assertLess(moments[2], 0.0, "Wing moment yaw should be negative"); // small yaw due to differential left dependent drag

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
			test_rolling_right();
			test_rolling_left();

	}
};

