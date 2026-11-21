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
 std::vector<OBB>CreateBoneOBBs(const SkinnedModel& skinnedModel);
 /*ボーンの動きに応じて更新されるOBB*/
 std::vector<OBB>UpdateBoneOBBs(const SkinnedModel& skinnedModel, const Animator& animator, const Matrix4x4 matrix);
 /*判定計算用*/
 /* std::vector<OBB>によるボーンのついたアニメーションモデル用の当たり判定*/
 bool IsOBBsToOBBsCollision(std::vector<OBB> obb1, std::vector<OBB> obb2);

/*ボーンを読み込みカプセルの作成を行う*/
//Capsule LoadBoneColliderCapsel(const SkinnedModel& skinnedModel);

