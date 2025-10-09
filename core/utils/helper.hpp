#pragma once

#include <exception>
#include <Windows.h>

namespace tinyd3d {
	inline void Verify(HRESULT hr) {
		if (FAILED(hr)) {
			throw std::exception();
		}
	}
}