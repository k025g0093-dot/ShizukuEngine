#pragma once
#include <d3d12.h>
#include "Allinclude/allVector.h"

struct InstanceData
{
	Matrix4x4 WVP;//ワールドビューポート
	Matrix4x4 World;//ワールド座標
	Vector4 Color;//色
};