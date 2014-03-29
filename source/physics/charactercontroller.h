#ifndef CHARACTER_CONTROLLER_INCLUDED
#define CHARACTER_CONTROLLER_INCLUDED

#include "../main/MatrixX.h"

class CharacterController
{
public:
	CharacterController() : acceleration(0), topSpeed(0) {}

	Matrix44 transform;
	Vector3 velocity; //relative to orientation
	float acceleration;
	float topSpeed;
};

#endif
