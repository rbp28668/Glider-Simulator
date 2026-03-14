#include "state_output.h"
#include <iostream>

SimObjectData::DataItem StateOutput::dataItems[] = {
	{"VELOCITY BODY X","meters per second", SIM_OUTPUT_DATATYPE},//	True lateral speed, relative to aircraft axis	Feet per second	Y -
	{"VELOCITY BODY Y","meters per second", SIM_OUTPUT_DATATYPE},//	True vertical speed, relative to aircraft axis	Feet per second	Y -
	{"VELOCITY BODY Z", "meters per second", SIM_OUTPUT_DATATYPE},//	True longitudinal speed, relative to aircraft axis	Feet per second	Y -
	{"ACCELERATION BODY X","meters per second squared", SIM_OUTPUT_DATATYPE},//	Acceleration relative to aircraft axis, in east / west direction	Feet per second squared	Y -
	{"ACCELERATION BODY Y","meters per second squared", SIM_OUTPUT_DATATYPE},//	Acceleration relative to aircraft axis, in vertical direction	Feet per second squared	Y -
	{"ACCELERATION BODY Z","meters per second squared", SIM_OUTPUT_DATATYPE},//	Acceleration relative to aircraft axis, in north / south direction	Feet per second squared	Y -
	{"ROTATION VELOCITY BODY X","radians per second", SIM_OUTPUT_DATATYPE},//	Rotation relative to aircraft axis	Radians per second	Y -
	{"ROTATION VELOCITY BODY Y","radians per second", SIM_OUTPUT_DATATYPE},//	Rotation relative to aircraft axis	Radians per second	Y -
	{"ROTATION VELOCITY BODY Z","radians per second", SIM_OUTPUT_DATATYPE},//	Rotation relative to aircraft axis	Radians per second	Y -
	{"ROTATION ACCELERATION BODY X","radians per second", SIM_OUTPUT_DATATYPE},//	Rotation acceleration relative to aircraft axis	Radians per second	Y -
	{"ROTATION ACCELERATION BODY Y","radians per second", SIM_OUTPUT_DATATYPE},//	Rotation acceleration relative to aircraft axis	Radians per second	Y -
	{"ROTATION ACCELERATION BODY Z","radians per second", SIM_OUTPUT_DATATYPE},//	Rotation acceleration relative to aircraft axis	Radians per second	Y -

//VELOCITY WORLD Z	Speed relative to earth, in North / South direction	Feet per second	Y -
//VELOCITY WORLD X	Speed relative to earth, in East / West direction	Feet per second	Y -
//VELOCITY WORLD Y	Speed relative to earth, in vertical direction	Feet per second	Y -
//ACCELERATION WORLD X	Acceleration relative to earth, in east / west direction	Feet per second squared	Y -
//ACCELERATION WORLD Y	Acceleration relative to Earth, in vertical direction	Feet per second squared	Y -
//ACCELERATION WORLD Z	Acceleration relative to earth, in north / south direction	Feet per second squared	Y -


	{"PLANE LATITUDE", "Radians", SIM_OUTPUT_DATATYPE}, //	Latitude of aircraft, North is positive, South negative	Radians	Y -
	{"PLANE LONGITUDE", "Radians", SIM_OUTPUT_DATATYPE }, //	Longitude of aircraft, East is positive, West negative	Radians	Y -
	{"PLANE ALTITUDE", "Meters", SIM_OUTPUT_DATATYPE }, //	Altitude of aircraft	Feet	Y

	{"PLANE PITCH DEGREES","Radians",SIM_OUTPUT_DATATYPE},
	{"PLANE BANK DEGREES", "Radians",SIM_OUTPUT_DATATYPE},
	{"PLANE HEADING DEGREES TRUE","Radians",SIM_OUTPUT_DATATYPE},

	//{"SIM ON GROUND","",SIMCONNECT_DATATYPE_INT32},
};

StateOutput::StateOutput(Prepar3D* p3d) : SimObjectData(p3d) {
	createDefinition();
}

SimObjectData::DataItem* StateOutput::items() {
	return dataItems;
}

int StateOutput::itemCount() {
	return sizeof(dataItems) / sizeof(dataItems[0]);
}

void StateOutput::sendData() {
	send(&data, sizeof(data), SIMCONNECT_OBJECT_ID_USER);
}
