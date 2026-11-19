#include "BoneCollider.h"
#include "../Model/3DAnimation/SkinnedModel.h"


OBB CreateOBBForBone(
	const SkinnedModel& skinnedModel
) {

	Vector3 positionMin = { FLT_MAX, FLT_MAX, FLT_MAX };
	Vector3 positionMax = { -FLT_MAX, -FLT_MAX, -FLT_MAX };

	const auto& vertices = skinnedModel.GetVertices();
	size_t count = vertices.size();

	for (size_t j = 0; j < count; j++) {

		if (vertices[j].position.x < positionMin.x) {
			positionMin.x = vertices[j].position.x;
		}
		if (vertices[j].position.x > positionMax.x) {
			positionMax.x = vertices[j].position.x;
		}
		if (vertices[j].position.y < positionMin.y) {
			positionMin.y = vertices[j].position.y;
		}
		if (vertices[j].position.y > positionMax.y) {
			positionMax.y = vertices[j].position.y;
		}
		if (vertices[j].position.z < positionMin.z) {
			positionMin.z = vertices[j].position.z;
		}
		if (vertices[j].position.z > positionMax.z) {
			positionMax.z = vertices[j].position.z;
		}
	}

	OBB obb;
	obb.center = (positionMin + positionMax) * 0.5f;
	obb.size = (positionMax - positionMin) * 0.5f;
	obb.orientations[0] = { 1.0f,0.0f,0.0f };
	obb.orientations[1] = { 0.0f,1.0f,0.0f };
	obb.orientations[2] = { 0.0f,0.0f,1.0f };
	return obb;
}

std::vector<OBB>CreateBoneOBBs(
	const SkinnedModel& skinnedModel
) {

	const auto& vertices = skinnedModel.GetVertices();

	size_t count = vertices.size();
	size_t boneCount = skinnedModel.GetBoneCount();

	

	std::vector<OBB> obb(boneCount);

	for (const auto& boneInFoMap : skinnedModel.GetBoneInFoMap()) {
		/*OBBの最小値と最大値を入れる変数*/
		Vector3 positionMin = { FLT_MAX, FLT_MAX, FLT_MAX };
		Vector3 positionMax = { -FLT_MAX, -FLT_MAX, -FLT_MAX };


		for (size_t j = 0; j < count; j++) {

			/*ボーンの最大で大きかったウェイトとそれを入れるID*/
			float maxBoneWeights = 0;
			int maxBoneId = -1;

			/*ボーンのウェイトの一番大きいのを探す*/
			for (int k = 0; k < kMaxBoneIngluence; k++) {
				if (maxBoneWeights < vertices[j].mWeights[k]) {
					maxBoneWeights = vertices[j].mWeights[k];
					maxBoneId = vertices[j].mBoneIds[k];
				}
			}

			/*ボーンのIDがそろっていれば処理を飛ばす*/
			if (maxBoneId != boneInFoMap.second.id) {
				continue;
			}

			/*ベースとなるポジションの定義*/
			glm::vec4 boneSpacePosition = boneInFoMap.second.offset * glm::vec4(vertices[j].position, 1.0f);

			/*各ベースポジションが該当していれば値を入れ替える*/
			if (boneSpacePosition.x < positionMin.x) {
				positionMin.x = boneSpacePosition.x;
			}
			if (boneSpacePosition.x > positionMax.x) {
				positionMax.x = boneSpacePosition.x;
			}
			if (boneSpacePosition.y < positionMin.y) {
				positionMin.y = boneSpacePosition.y;
			}
			if (boneSpacePosition.y > positionMax.y) {
				positionMax.y = boneSpacePosition.y;
			}
			if (boneSpacePosition.z < positionMin.z) {
				positionMin.z = boneSpacePosition.z;
			}
			if (boneSpacePosition.z > positionMax.z) {
				positionMax.z = boneSpacePosition.z;
			}
		}

		/*ボーンが存在しない場合は以降の処理を飛ばす*/
		if (positionMin.x > positionMax.x)continue;
		if (positionMin.y > positionMax.y)continue;
		if (positionMin.z > positionMax.z)continue;

		/*各OBB作成に必要な値を格納*/
		obb[boneInFoMap.second.id].center = (positionMin + positionMax) * 0.5f;
		obb[boneInFoMap.second.id].size = (positionMax - positionMin) * 0.5f;
		obb[boneInFoMap.second.id].orientations[0] = { 1.0f,0.0f,0.0f };
		obb[boneInFoMap.second.id].orientations[1] = { 0.0f,1.0f,0.0f };
		obb[boneInFoMap.second.id].orientations[2] = { 0.0f,0.0f,1.0f };
	}

	return obb;
}


//Capsule BoneCollider::LoadBoneColliderCapsel(
//	const SkinnedModel& skinnedModel
//) {
//	mSkinnedModel = skinnedModel;
//	glm::vec3 positionMin = { FLT_MAX, FLT_MAX, FLT_MAX };
//	glm::vec3 positionMax = { -FLT_MAX, -FLT_MAX, -FLT_MAX };
//
//	Capsule capsule;
//
//	for (const auto& skinned : mSkinnedModel.GetVertices()) {
//
//		const glm::vec3 positionMinusCnter = skinned.position - positionMin;
//
//		const float segmentLength = std::abs(std::inner_product())
//
//
//			capsule.base = kvertices.position;
//
//	}
//}
