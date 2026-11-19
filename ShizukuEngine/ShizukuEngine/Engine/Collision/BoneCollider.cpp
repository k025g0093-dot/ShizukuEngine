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
