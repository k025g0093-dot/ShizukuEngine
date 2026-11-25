#include "BoneCollider.h"
#include "../Model/3DAnimation/SkinnedModel.h"
#include "../Model/3DAnimation/Animator.h"


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

std::vector<OBB>CreateAnimationBoneOBBs(
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

std::vector<OBB>UpdateAnimationBoneOBBs(
	const SkinnedModel& skinnedModel,
	const Animator& animator,
	const Matrix4x4 matrix) {

	size_t boneCount = skinnedModel.GetBoneCount();
	const auto& vertices = skinnedModel.GetVertices();
	std::vector<OBB> obb(boneCount);
	for (size_t i = 0; i < skinnedModel.GetBoneOBBs().size(); i++) {
		Matrix4x4 localWorldMatrix{};

		/*ローカルワールド座標を各ボーンに応じて作成していく*/
		localWorldMatrix = animator.GetBoneWorldMatrix(static_cast<int>(i), matrix);

		/*各軸のレングスを計算*/
		float lengthX = std::sqrt(
			localWorldMatrix.m[0][0] * localWorldMatrix.m[0][0] +
			localWorldMatrix.m[0][1] * localWorldMatrix.m[0][1] +
			localWorldMatrix.m[0][2] * localWorldMatrix.m[0][2]
		);

		float lengthY = std::sqrt(
			localWorldMatrix.m[1][0] * localWorldMatrix.m[1][0] +
			localWorldMatrix.m[1][1] * localWorldMatrix.m[1][1] +
			localWorldMatrix.m[1][2] * localWorldMatrix.m[1][2]
		);

		float lengthZ = std::sqrt(
			localWorldMatrix.m[2][0] * localWorldMatrix.m[2][0] +
			localWorldMatrix.m[2][1] * localWorldMatrix.m[2][1] +
			localWorldMatrix.m[2][2] * localWorldMatrix.m[2][2]
		);

		/*長さが0は向きが決まらないため飛ばす*/
		if (lengthX <= 0.0f || lengthY <= 0.0f || lengthZ <= 0.0f) {
			continue;
		}

		/*中心を位置として変換*/
		obb[i].center = TransformMatrix(skinnedModel.GetBoneOBBs()[i].center, localWorldMatrix);

		/*各軸の伸びた分をかけてボーンに合わせる*/
		obb[i].size.x = skinnedModel.GetBoneOBBs()[i].size.x * lengthX;
		obb[i].size.y = skinnedModel.GetBoneOBBs()[i].size.y * lengthY;
		obb[i].size.z = skinnedModel.GetBoneOBBs()[i].size.z * lengthZ;

		/*向き：行列の各行を、その長さで割って長さ1にする*/
		obb[i].orientations[0] = {
			localWorldMatrix.m[0][0] / lengthX,
			localWorldMatrix.m[0][1] / lengthX,
			localWorldMatrix.m[0][2] / lengthX
		};
		obb[i].orientations[1] = {
			localWorldMatrix.m[1][0] / lengthY,
			localWorldMatrix.m[1][1] / lengthY,
			localWorldMatrix.m[1][2] / lengthY
		};
		obb[i].orientations[2] = {
			localWorldMatrix.m[2][0] / lengthZ,
			localWorldMatrix.m[2][1] / lengthZ,
			localWorldMatrix.m[2][2] / lengthZ
		};

	}
	return obb;
}


bool IsAnimationOBBsToOBBsCollision(const std::vector<OBB>& obb1, const std::vector<OBB>& obb2) {

	for (size_t i = 0; i < obb1.size(); i++) {
		for (size_t j = 0; j < obb2.size(); j++) {
			Vector3 axes[15];

			axes[0] = obb1[i].orientations[0];
			axes[1] = obb1[i].orientations[1];
			axes[2] = obb1[i].orientations[2];

			axes[3] = obb2[j].orientations[0];
			axes[4] = obb2[j].orientations[1];
			axes[5] = obb2[j].orientations[2];

			axes[6] = Cross(obb1[i].orientations[0], obb2[j].orientations[0]);
			axes[7] = Cross(obb1[i].orientations[0], obb2[j].orientations[1]);
			axes[8] = Cross(obb1[i].orientations[0], obb2[j].orientations[2]);
			axes[9] = Cross(obb1[i].orientations[1], obb2[j].orientations[0]);
			axes[10] = Cross(obb1[i].orientations[1], obb2[j].orientations[1]);
			axes[11] = Cross(obb1[i].orientations[1], obb2[j].orientations[2]);
			axes[12] = Cross(obb1[i].orientations[2], obb2[j].orientations[0]);
			axes[13] = Cross(obb1[i].orientations[2], obb2[j].orientations[1]);
			axes[14] = Cross(obb1[i].orientations[2], obb2[j].orientations[2]);

			bool isSeparated = false;

			for (int k = 0; k < 15; ++k) {

				if (Length(axes[k]) < 0.0001f) {
					continue;
				}

				//軸を正規化（長さを1にする）しておく
				Vector3 axis = Normalize(axes[k]);

				float L1 =
					std::abs(Dot(obb1[i].orientations[0], axis)) * (obb1[i].size.x) +
					std::abs(Dot(obb1[i].orientations[1], axis)) * (obb1[i].size.y) +
					std::abs(Dot(obb1[i].orientations[2], axis)) * (obb1[i].size.z);

				// OBB B の影の半径 L2
				float L2 =
					std::abs(Dot(obb2[j].orientations[0], axis)) * (obb2[j].size.x) +
					std::abs(Dot(obb2[j].orientations[1], axis)) * (obb2[j].size.y) +
					std::abs(Dot(obb2[j].orientations[2], axis)) * (obb2[j].size.z);

				// 軸の上での、それぞれの箱の中心の位置
				float centerA = Dot(obb1[i].center, axis);
				float centerB = Dot(obb2[j].center, axis);

				// 箱Aの影の「左端」と「右端」
				float min1 = centerA - L1;
				float max1 = centerA + L1;

				// 箱Bの影の「左端」と「右端」
				float min2 = centerB - L2;
				float max2 = centerB + L2;


				if (max1 < min2 || max2 < min1) {
					isSeparated = true;  // 隙間がある
					continue;
				}
			}
			if (!isSeparated) {
				return true;
			}
		}
	}
	return false;
}

bool IsAnimationOBBsToOBBCollision(
	const std::vector<OBB>& obb1,
	const OBB& obb2
) {

	for (size_t i = 0; i < obb1.size(); i++) {
		Vector3 axes[15];

		axes[0] = obb1[i].orientations[0];
		axes[1] = obb1[i].orientations[1];
		axes[2] = obb1[i].orientations[2];

		axes[3] = obb2.orientations[0];
		axes[4] = obb2.orientations[1];
		axes[5] = obb2.orientations[2];

		axes[6] = Cross(obb1[i].orientations[0], obb2.orientations[0]);
		axes[7] = Cross(obb1[i].orientations[0], obb2.orientations[1]);
		axes[8] = Cross(obb1[i].orientations[0], obb2.orientations[2]);
		axes[9] = Cross(obb1[i].orientations[1], obb2.orientations[0]);
		axes[10] = Cross(obb1[i].orientations[1], obb2.orientations[1]);
		axes[11] = Cross(obb1[i].orientations[1], obb2.orientations[2]);
		axes[12] = Cross(obb1[i].orientations[2], obb2.orientations[0]);
		axes[13] = Cross(obb1[i].orientations[2], obb2.orientations[1]);
		axes[14] = Cross(obb1[i].orientations[2], obb2.orientations[2]);

		bool isSeparated = false;

		for (int k = 0; k < 15; ++k) {

			if (Length(axes[k]) < 0.0001f) {
				continue;
			}

			//軸を正規化（長さを1にする）しておく
			Vector3 axis = Normalize(axes[k]);

			float L1 =
				std::abs(Dot(obb1[i].orientations[0], axis)) * (obb1[i].size.x) +
				std::abs(Dot(obb1[i].orientations[1], axis)) * (obb1[i].size.y) +
				std::abs(Dot(obb1[i].orientations[2], axis)) * (obb1[i].size.z);

			// OBB B の影の半径 L2
			float L2 =
				std::abs(Dot(obb2.orientations[0], axis)) * (obb2.size.x) +
				std::abs(Dot(obb2.orientations[1], axis)) * (obb2.size.y) +
				std::abs(Dot(obb2.orientations[2], axis)) * (obb2.size.z);

			// 軸の上での、それぞれの箱の中心の位置
			float centerA = Dot(obb1[i].center, axis);
			float centerB = Dot(obb2.center, axis);

			// 箱Aの影の「左端」と「右端」
			float min1 = centerA - L1;
			float max1 = centerA + L1;

			// 箱Bの影の「左端」と「右端」
			float min2 = centerB - L2;
			float max2 = centerB + L2;


			if (max1 < min2 || max2 < min1) {
				isSeparated = true;  // 隙間がある
				continue;
			}
		}
		if (!isSeparated) {
			return true;
		}
	}
	return false;
}

bool IsOBBToOBBCollision(const OBB& obb1, const OBB& obb2) {
	Vector3 axes[15];

	axes[0] = obb1.orientations[0];
	axes[1] = obb1.orientations[1];
	axes[2] = obb1.orientations[2];

	axes[3] = obb2.orientations[0];
	axes[4] = obb2.orientations[1];
	axes[5] = obb2.orientations[2];

	axes[6] = Cross(obb1.orientations[0], obb2.orientations[0]);
	axes[7] = Cross(obb1.orientations[0], obb2.orientations[1]);
	axes[8] = Cross(obb1.orientations[0], obb2.orientations[2]);
	axes[9] = Cross(obb1.orientations[1], obb2.orientations[0]);
	axes[10] = Cross(obb1.orientations[1], obb2.orientations[1]);
	axes[11] = Cross(obb1.orientations[1], obb2.orientations[2]);
	axes[12] = Cross(obb1.orientations[2], obb2.orientations[0]);
	axes[13] = Cross(obb1.orientations[2], obb2.orientations[1]);
	axes[14] = Cross(obb1.orientations[2], obb2.orientations[2]);

	for (int i = 0; i < 15; ++i) {

		if (Length(axes[i]) < 0.0001f) {
			continue;
		}

		// 💡 軸を正規化（長さを1にする）しておく
		Vector3 axis = Normalize(axes[i]);

		float L1 =
			std::abs(Dot(obb1.orientations[0], axis)) * (obb1.size.x / 2.0f) +
			std::abs(Dot(obb1.orientations[1], axis)) * (obb1.size.y / 2.0f) +
			std::abs(Dot(obb1.orientations[2], axis)) * (obb1.size.z / 2.0f);

		// OBB B の影の半径 L2
		float L2 =
			std::abs(Dot(obb2.orientations[0], axis)) * (obb2.size.x / 2.0f) +
			std::abs(Dot(obb2.orientations[1], axis)) * (obb2.size.y / 2.0f) +
			std::abs(Dot(obb2.orientations[2], axis)) * (obb2.size.z / 2.0f);

		// 軸の上での、それぞれの箱の中心の位置
		float centerA = Dot(obb1.center, axis);
		float centerB = Dot(obb2.center, axis);

		// 箱Aの影の「左端」と「右端」
		float min1 = centerA - L1;
		float max1 = centerA + L1;

		// 箱Bの影の「左端」と「右端」
		float min2 = centerB - L2;
		float max2 = centerB + L2;

		if (max1 < min2 || max2 < min1) {
			return false;  // 隙間がある
		}
	}
	return true;
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
