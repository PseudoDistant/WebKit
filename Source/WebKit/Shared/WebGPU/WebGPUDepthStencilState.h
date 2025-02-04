/*
 * Copyright (C) 2021 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#if ENABLE(GPU_PROCESS)

#include "WebGPUStencilFaceState.h"
#include <optional>
#include <pal/graphics/WebGPU/WebGPUCompareFunction.h>
#include <pal/graphics/WebGPU/WebGPUIntegralTypes.h>
#include <pal/graphics/WebGPU/WebGPUTextureFormat.h>

namespace WebKit::WebGPU {

struct DepthStencilState {
    PAL::WebGPU::TextureFormat format;

    bool depthWriteEnabled;
    PAL::WebGPU::CompareFunction depthCompare;

    StencilFaceState stencilFront;
    StencilFaceState stencilBack;

    std::optional<PAL::WebGPU::StencilValue> stencilReadMask;
    std::optional<PAL::WebGPU::StencilValue> stencilWriteMask;

    PAL::WebGPU::DepthBias depthBias;
    float depthBiasSlopeScale;
    float depthBiasClamp;

    template<class Encoder> void encode(Encoder& encoder) const
    {
        encoder << format;
        encoder << depthWriteEnabled;
        encoder << depthCompare;
        encoder << stencilFront;
        encoder << stencilBack;
        encoder << stencilReadMask;
        encoder << stencilWriteMask;
        encoder << depthBias;
        encoder << depthBiasSlopeScale;
        encoder << depthBiasClamp;
    }

    template<class Decoder> static std::optional<DepthStencilState> decode(Decoder& decoder)
    {
        std::optional<PAL::WebGPU::TextureFormat> format;
        decoder >> format;
        if (!format)
            return std::nullopt;

        std::optional<bool> depthWriteEnabled;
        decoder >> depthWriteEnabled;
        if (!depthWriteEnabled)
            return std::nullopt;

        std::optional<PAL::WebGPU::CompareFunction> depthCompare;
        decoder >> depthCompare;
        if (!depthCompare)
            return std::nullopt;

        std::optional<StencilFaceState> stencilFront;
        decoder >> stencilFront;
        if (!stencilFront)
            return std::nullopt;

        std::optional<StencilFaceState> stencilBack;
        decoder >> stencilBack;
        if (!stencilBack)
            return std::nullopt;

        std::optional<std::optional<PAL::WebGPU::StencilValue>> stencilReadMask;
        decoder >> stencilReadMask;
        if (!stencilReadMask)
            return std::nullopt;

        std::optional<std::optional<PAL::WebGPU::StencilValue>> stencilWriteMask;
        decoder >> stencilWriteMask;
        if (!stencilWriteMask)
            return std::nullopt;

        std::optional<PAL::WebGPU::DepthBias> depthBias;
        decoder >> depthBias;
        if (!depthBias)
            return std::nullopt;

        std::optional<float> depthBiasSlopeScale;
        decoder >> depthBiasSlopeScale;
        if (!depthBiasSlopeScale)
            return std::nullopt;

        std::optional<float> depthBiasClamp;
        decoder >> depthBiasClamp;
        if (!depthBiasClamp)
            return std::nullopt;

        return { { WTFMove(*format), WTFMove(*depthWriteEnabled), WTFMove(*depthCompare), WTFMove(*stencilFront), WTFMove(*stencilBack), WTFMove(*stencilReadMask), WTFMove(*stencilWriteMask), WTFMove(*depthBias), WTFMove(*depthBiasSlopeScale), WTFMove(*depthBiasClamp) } };
    }
};

} // namespace WebKit::WebGPU

#endif // ENABLE(GPU_PROCESS)
