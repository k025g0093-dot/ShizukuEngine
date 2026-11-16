#include "EntityManager.h"
Entity* EntityManager::CreateEntity(const std::string& pathName) {

	auto entity = std::make_unique<Entity>();
	entity->SetName(pathName);
	entity->SetDisplayName(pathName);

	Entity* ptr = entity.get();
	mEntities.push_back(std::move(entity));
	return ptr;
}

Entity* EntityManager::CloneToRuntime(const Entity& entity) {

	auto runtimeEntity = std::make_unique<Entity>();
	runtimeEntity->SetName(entity.GetName());
	runtimeEntity->SetDisplayName(entity.GetDisplayName());


	Entity* result = runtimeEntity.get();
	mRuntimeEntities.push_back(std::move(runtimeEntity));
	return result;
}

void EntityManager::DestoryEntity(Entity* entity) {

	auto it = std::find_if(mEntities.begin(), mEntities.end(),
		[entity](const auto& e) {return e.get() == entity; });
	if (it != mEntities.end()) {
		mEntities.erase(it);
	}
}

void EntityManager::UpdateAll(float dt)
{

	if (mPlayState != PlayState::playing) {
		return;
	}

	for (auto& entity : mRuntimeEntities) {
		for (const std::unique_ptr<Component>& component : entity->GetComponents()) {
			if (!component) continue;

			//if (auto* behaviour = dynamic_cast<MonoBehaviour*>(component)) {
			//	if (!behaviour->m_isStarted) {
			//		behaviour->Start();
			//	}
			//}
		}

	}

	// Update
	for (auto& entity : mRuntimeEntities) {
		for (const std::unique_ptr<Component>& component : entity->GetComponents()) {
			if (!component) continue;
			component->deltaTime = dt;
			component->Update();
		}
	}

}



	void EntityManager::StartPlay() {

		if (mPlayState != PlayState::editing)return;

		mRuntimeEntities.clear();
		for (const auto& editorEntity : mEntities) {
			CloneToRuntime(*editorEntity);
		}

		mPlayState = PlayState::playing;

	}

	void EntityManager::StopPlay() {
		if (mPlayState == PlayState::editing)return;

		mRuntimeEntities.clear();
		mPlayState = PlayState::editing;
	}

	void EntityManager::PausePlay() {
		if (mPlayState == PlayState::playing)
			mPlayState = PlayState::paused;
	}

	void EntityManager::ResumePlay() {
		if (mPlayState == PlayState::paused)
			mPlayState = PlayState::playing;
	}
