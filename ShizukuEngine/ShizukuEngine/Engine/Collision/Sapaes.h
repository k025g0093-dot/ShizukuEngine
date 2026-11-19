#pragma once
#include "Allinclude/AllVector.h"


/*OBB構造体*/
struct OBB
{
	Vector3 center;
	Vector3 orientations[3];
	Vector3 size;
};

/*カプセルの構造体*/
struct Capsule {
	Vector3 base;
	Vector3 tip;
	float radius;
};


