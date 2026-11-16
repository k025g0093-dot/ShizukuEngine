#include "ImGuiContentBrowser.h"
#include "../TextureManager/TextureManager.h"

static const std::filesystem::path sAssetsPath = "resources";

ImGuiContentBrowser::ImGuiContentBrowser()
	:mCurrentDirectory(sAssetsPath){}


bool ImGuiContentBrowser::IsImageFile(const std::filesystem::path& filePath) {
	std::string ext = filePath.extension().string();
	for (auto& c : ext)c = (char)tolower(c);
	return ext == ".png" || ext == ".jpg" || ext == "jpeg";
}

void ImGuiContentBrowser::update(TextureManager* TextureManager) {

	mTextureManager = TextureManager;

	if (mDirectoryIcon == -1) {
		//フォルダの画像とか読み込み
		mDirectoryIcon = mTextureManager->LoadTexture("ContentBrowserAsset/Icons/ContentBrowser/DirectoryIcon.png");
		mFileIcon = mTextureManager->LoadTexture("ContentBrowserAsset/Icons/ContentBrowser/FileIcon2.png");
	}

	if (!show)return;

	if (begin("コンテンツブラウザ")) {

		if (mCurrentDirectory != std::filesystem::path(sAssetsPath)) {
			if (ImGui::Button("←戻る")) {
				mCurrentDirectory = mCurrentDirectory.parent_path();
			}
		}

		static float padding = 16.0f;
		static float thumbnailSize = 60;
		float cellSize = thumbnailSize + padding;

		float panelWindth = ImGui::GetContentRegionAvail().x;
		int colummCount = (int)(panelWindth / cellSize);
		if (colummCount < 1)
			colummCount = 1;
		ImGui::Columns(colummCount, 0, false);

		for (auto& directoryEntry : std::filesystem::directory_iterator(mCurrentDirectory)) {

			const auto& path = directoryEntry.path();
			auto relativePath = std::filesystem::relative(path, sAssetsPath);
			std::string filenameString = relativePath.filename().string();

			ImGui::PushID(filenameString.c_str());

			int textureIndex;
			if (directoryEntry.is_directory())
			{
				textureIndex = mDirectoryIcon;
			}
			else if (IsImageFile(path))
			{
				auto it = mThumbnailCache.find(filenameString);
				if (it != mThumbnailCache.end())
				{
					textureIndex = it->second;
				}
				else
				{
					textureIndex = mTextureManager->LoadTexture(path.string());
					mThumbnailCache[filenameString] = textureIndex;
				}

			}
			else
			{
				textureIndex = mFileIcon;
			}

			auto gpuHandle = mTextureManager->GetGPUHandle(textureIndex);
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
			ImGui::ImageButton(filenameString.c_str(), (ImTextureID)gpuHandle.ptr, { thumbnailSize,thumbnailSize });

			if (ImGui::BeginDragDropSource())
			{
				const wchar_t* itemPath = relativePath.c_str();
				ImGui::SetDragDropPayload("CONTENT_BROWSER_ITEM", itemPath, (wcslen(itemPath) + 1) * sizeof(wchar_t));
				ImGui::Text("%s", filenameString.c_str());

				ImGui::EndDragDropSource();
			}

			ImGui::PopStyleColor();

			if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
			{

				if (directoryEntry.is_directory())
					mCurrentDirectory /= path.filename();
			}
		
			ImGui::TextWrapped(filenameString.c_str());

			ImGui::NextColumn();
			ImGui::PopID();

		}

		ImGui::Columns(1);

		ImGui::SliderFloat("サムネイルサイズ", &thumbnailSize, 16, 512);
		ImGui::SliderFloat("パディング", &padding, 0, 32);

	}

	end();
}

