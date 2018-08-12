#pragma once


#include "../../Common/Common/common.h"
using namespace common;
#include "../../Common/Graphic11/graphic11.h"
#include "../../Common/Framework11/framework11.h"


using Microsoft::WRL::ComPtr;
using namespace DirectX;

template <typename T, uint32_t N>
constexpr uint32_t count_of(T(&)[N]) { return N; }

