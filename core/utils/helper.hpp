#pragma once

#include <exception>

extern HRESULT;

namespace tinyd3d {
	inline void Verify(HRESULT hr) {
		if (FAILED(hr)) {
			throw std::exception();
		}
	}
}