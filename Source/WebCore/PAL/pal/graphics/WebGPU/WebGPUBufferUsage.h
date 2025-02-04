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

#include "WebGPUIntegralTypes.h"
#include <cstdint>
#include <wtf/RefCounted.h>

namespace PAL::WebGPU {

using BufferUsageFlags = uint32_t;
class BufferUsage : public RefCounted<BufferUsage> {
public:
    static constexpr FlagsConstant MAP_READ      = 0x0001;
    static constexpr FlagsConstant MAP_WRITE     = 0x0002;
    static constexpr FlagsConstant COPY_SRC      = 0x0004;
    static constexpr FlagsConstant COPY_DST      = 0x0008;
    static constexpr FlagsConstant INDEX         = 0x0010;
    static constexpr FlagsConstant VERTEX        = 0x0020;
    static constexpr FlagsConstant UNIFORM       = 0x0040;
    static constexpr FlagsConstant STORAGE       = 0x0080;
    static constexpr FlagsConstant INDIRECT      = 0x0100;
    static constexpr FlagsConstant QUERY_RESOLVE = 0x0200;
};

} // namespace PAL::WebGPU
