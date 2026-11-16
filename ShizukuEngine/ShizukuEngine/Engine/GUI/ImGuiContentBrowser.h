#pragma once
#include "ImGuiWindow.h"
#include <filesystem>
#include <string>
#include <unordered_map>

class TextureManager;

class ImGuiContentBrowser:public ImGuiUIWindow
{
public:
	ImGuiContentBrowser();
	void update(TextureManager* TextureManager);
private://関数
	bool IsImageFile(const std::filesystem::path& filePath);

private://変数

	std::filesystem::path mCurrentDirectory;
	int mDirectoryIcon = -1;
	int mFileIcon = -1;
	std::unordered_map < std::string, int>mThumbnailCache;

	TextureManager* mTextureManager;
};

