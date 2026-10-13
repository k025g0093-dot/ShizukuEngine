#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <d3d12.h>
#include "MeshModel.h"

//モデルの読み込みと所有を行う（同じパスは2回目から読み込まずに同じ番号を返す）
class ModelManager {
public:

    
    void Initialize(ID3D12Device* device);

    //モデルを読み込んで番号を返す
    int Load(const std::string& filePath);

    //番号からモデルを借りる
    MeshModel* Get(int index) const;

private://メンバ変数

    //外から借りているもの
    ID3D12Device* mDevice = nullptr;

    //読み込んだモデル
    std::vector<std::unique_ptr<MeshModel>> mModels;

    //パス → 番号
    std::unordered_map<std::string, int> mPathToIndex;
};