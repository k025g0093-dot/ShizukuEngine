#pragma once
#include <vector>
#include <memory>
#include "Entity.h"
class EntityManager
{

public:

	EntityManager() = default;
	~EntityManager() = default;

	enum class PlayState {

		playing,//再生中
		paused,//一時停止中
		editing,//停止中

	};

	//エンティティリスト
	using EntityList = std::vector<std::unique_ptr<Entity>>;

	const PlayState GetPlayState() const { return mPlayState; }

	Entity* CreateEntity(const std::string& pathName);

	void DestoryEntity(Entity* entity);

	void UpdateAll(float dt);

	//プレイの制御関数
	void StartPlay();
	void StopPlay();
	void PausePlay();
	void ResumePlay();

	//GUIが参照してplay中でも編集シーンを見れるように
	const EntityList& GetEditorEntities()const {
		return mEntities;
	}

	// ゲーム実行中の描画が参照する。
	const EntityList& GetRuntimeEntities() const {
		return mRuntimeEntities;
	}

	// 通常の描画・更新用を区別する
	const EntityList& GetActiveEntities()const {
		return mPlayState == PlayState::editing
			? mEntities
			: mRuntimeEntities;
	}

private://プライベート関数

	Entity* CloneToRuntime(const Entity& entity);

private://プライベート変数

	EntityList mEntities;
	EntityList mRuntimeEntities;

	PlayState mPlayState = PlayState::editing;

};

