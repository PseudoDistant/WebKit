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

#include "StreamClientConnection.h"
#include "WebGPUIdentifier.h"
#include <pal/graphics/WebGPU/WebGPU.h>
#include <wtf/Deque.h>

namespace WebKit {
class GPUProcessConnection;
}

namespace WebKit::WebGPU {

class ConvertToBackingContext;

class RemoteGPUProxy final : public PAL::WebGPU::GPU {
public:
    static Ref<RemoteGPUProxy> create(GPUProcessConnection& gpuProcessConnection, ConvertToBackingContext& convertToBackingContext, WebGPUIdentifier identifier)
    {
        return adoptRef(*new RemoteGPUProxy(gpuProcessConnection, convertToBackingContext, identifier));
    }

    virtual ~RemoteGPUProxy();

    RemoteGPUProxy& root() { return *this; }

    IPC::StreamClientConnection& streamClientConnection() { return m_streamConnection; }

private:
    friend class DowncastConvertToBackingContext;

    RemoteGPUProxy(GPUProcessConnection&, ConvertToBackingContext&, WebGPUIdentifier);

    RemoteGPUProxy(const RemoteGPUProxy&) = delete;
    RemoteGPUProxy(RemoteGPUProxy&&) = delete;
    RemoteGPUProxy& operator=(const RemoteGPUProxy&) = delete;
    RemoteGPUProxy& operator=(RemoteGPUProxy&&) = delete;

    WebGPUIdentifier backing() const { return m_backing; }
    
    static inline constexpr Seconds defaultSendTimeout = 30_s;
    template<typename T>
    WARN_UNUSED_RETURN bool send(T&& message)
    {
        return root().streamClientConnection().send(WTFMove(message), backing(), defaultSendTimeout);
    }
    template<typename T>
    WARN_UNUSED_RETURN IPC::Connection::SendSyncResult sendSync(T&& message, typename T::Reply&& reply)
    {
        return root().streamClientConnection().sendSync(WTFMove(message), WTFMove(reply), backing(), defaultSendTimeout);
    }

    void requestAdapter(const PAL::WebGPU::RequestAdapterOptions&, WTF::Function<void(RefPtr<PAL::WebGPU::Adapter>&&)>&&) final;

    Deque<WTF::Function<void(RefPtr<PAL::WebGPU::Adapter>&&)>> m_callbacks;

    WebGPUIdentifier m_backing;
    Ref<ConvertToBackingContext> m_convertToBackingContext;
    IPC::StreamClientConnection m_streamConnection;
};

} // namespace WebKit::WebGPU

#endif // ENABLE(GPU_PROCESS)
