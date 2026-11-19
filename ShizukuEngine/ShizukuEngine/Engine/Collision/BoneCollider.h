#pragma once
#include "../Allinclude/AllVector.h"
#include "Sapaes.h"
#include <vector>
#include <cfloat>
#include "../Model/SkinningConstants.h"
#include <numeric>

class SkinnedModel;


/*OBB作成関数*/
 OBB CreateOBBForBone(const SkinnedModel& skinnedModel);

 std::vector<OBB>CreateBoneOBBs(const SkinnedModel& skinnedModel);

/*ボーンを読み込みカプセルの作成を行う*/
//Capsule LoadBoneColliderCapsel(const SkinnedModel& skinnedModel);

