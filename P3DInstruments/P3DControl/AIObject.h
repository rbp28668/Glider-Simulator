#pragma once

class Prepar3D;

class AIObject
{
	Prepar3D* p3d;
public:
	AIObject(Prepar3D* p3d);
	~AIObject();

	void createTestObject();
};

