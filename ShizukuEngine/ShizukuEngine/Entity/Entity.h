#pragma once
#include <vector>
#include <string>
#include <memory>
#include <algorithm>
#include <iostream>
#include "Component.h"

class Entity
{

public:

	Entity();
	~Entity();



	template<typename T, typename Args>
	//コンポーネント追加
	T* AddComponent(Args&& args) {
		//引数の型がComponentを継承しているかどうかをチェックする
		T* component = std::make_unique< T>(std::forward<Args>(args)...);
		//コンポーネントのentityを対象のEntityに設定する
		component->entity = this;
		//コンポーネントをEntityのコンポーネントリストに追加する
		mComponents.push_back(component);
		return component;
	}

	template <typename T>
	//コンポーネント取得
	T* GetComponent() {
		//コンポーネントリストから指定された型を検索する
		for (const auto* component : mComponents) {
			//dynamic_castを使用して、指定された型にキャストできるかどうかを確認する
			if (T* castedComponent = dynamic_cast<T*>(component.get())) {
				return castedComponent;
			}
		}
		return nullptr;
	}

	//コンポーネント削除
	void RemoveComponent(Component* component) {

		auto isCompTarget = [component](const std::unique_ptr<Component>& comp) {
			return (comp.get() == component);
			};

		//コンポーネントリストから指定されたコンポーネントを検索する
		auto it = std::find_if(mComponents.begin(), mComponents.end(), isCompTarget);
		//コンポーネントが最後まで見つからない場合は何もしない
		if (it == mComponents.end())return;
		//コンポーネントリストから指定されたコンポーネントを削除する
		mComponents.erase(it);
	}



	const std::string& GetDisplayName()const { return mDisplayName; }

	//コンポーネントの名前を設定する
	void SetDisplayName(const std::string& name) {
		mDisplayName = name;
		strncpy_s(mDisplayNameBuf, name.c_str(), sizeof(mDisplayNameBuf));
	}

	void SetName(const std::string& newName) { mName = newName; }
	const std::string& GetName()const { return mName; }

	const std::vector<std::unique_ptr<Component>>& GetComponents() const { return mComponents; }

private:

	std::string mName;
	std::string mDisplayName;
	char mDisplayNameBuf[128]{};

	std::vector<std::unique_ptr<Component>> mComponents;

};

