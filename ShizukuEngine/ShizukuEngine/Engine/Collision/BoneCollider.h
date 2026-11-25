#pragma once
#include "../Allinclude/AllVector.h"
#include "Sapaes.h"
#include <vector>
#include <cfloat>
#include "../Model/SkinningConstants.h"
#include <numeric>
#include <stdexcept>
class SkinnedModel;
class Animator;

/*OBB作成関数*/
 OBB CreateOBBForBone(const SkinnedModel& skinnedModel);
 std::vector<OBB>CreateAnimationBoneOBBs(const SkinnedModel& skinnedModel);
 /*ボーンの動きに応じて更新されるOBB*/
 std::vector<OBB>UpdateAnimationBoneOBBs(const SkinnedModel& skinnedModel, const Animator& animator, const Matrix4x4 matrix);
 /*判定計算用*/
 /* std::vector<OBB>によるボーンのついたアニメーションモデル用の当たり判定*/
 bool IsAnimationOBBsToOBBsCollision(const std::vector<OBB>& obb1, const std::vector<OBB>& obb2);

 bool IsAnimationOBBsToOBBCollision(const std::vector<OBB>& obb1, const OBB& obb2);

 bool IsOBBToOBBCollision(const OBB& obb1, const OBB& obb2);

/*ボーンを読み込みカプセルの作成を行う*/
//Capsule LoadBoneColliderCapsel(const SkinnedModel& skinnedModel);

