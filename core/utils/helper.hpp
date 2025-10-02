#pragma once

#include <filesystem>

namespace tinyd3d {
	inline std::wstring getAssetPath(const wchar_t* name) {
		std::filesystem::path src = __FILE__;
		auto path = src.parent_path() / name;
		auto test = std::filesystem::current_path();

		if (!std::filesystem::exists(path)) {
			throw std::runtime_error("shader file doesn't exist!");
		}

		return path.wstring();
	}
}