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

#include "config.h"
#include "RemoteQueue.h"

#if ENABLE(GPU_PROCESS)

#include "WebGPUObjectHeap.h"
#include "WebGPUObjectRegistry.h"
#include <pal/graphics/WebGPU/WebGPUQueue.h>

namespace WebKit {

RemoteQueue::RemoteQueue(PAL::WebGPU::Queue& queue, WebGPU::ObjectRegistry& objectRegistry, WebGPU::ObjectHeap& objectHeap, WebGPUIdentifier identifier)
    : m_backing(queue)
    , m_objectRegistry(objectRegistry)
    , m_objectHeap(objectHeap)
    , m_identifier(identifier)
{
    m_objectRegistry.addObject(m_identifier, m_backing);
}

RemoteQueue::~RemoteQueue()
{
    m_objectRegistry.removeObject(m_identifier);
}

void RemoteQueue::submit(Vector<WebGPUIdentifier>&& commandBuffers)
{
    Vector<std::reference_wrapper<PAL::WebGPU::CommandBuffer>> convertedCommandBuffers;
    convertedCommandBuffers.reserveInitialCapacity(commandBuffers.size());
    for (WebGPUIdentifier identifier : commandBuffers) {
        auto convertedCommandBuffer = m_objectRegistry.convertCommandBufferFromBacking(identifier);
        ASSERT(convertedCommandBuffer);
        if (!convertedCommandBuffer)
            return;
        convertedCommandBuffers.uncheckedAppend(*convertedCommandBuffer);
    }
    m_backing->submit(WTFMove(convertedCommandBuffers));
}

void RemoteQueue::onSubmittedWorkDone(WTF::CompletionHandler<void()>&& callback)
{
    m_backing->onSubmittedWorkDone([callback = WTFMove(callback)] () mutable {
        callback();
    });
}

void RemoteQueue::writeBuffer(
    WebGPUIdentifier buffer,
    PAL::WebGPU::Size64 bufferOffset,
    Vector<uint8_t>&& data)
{
    auto convertedBuffer = m_objectRegistry.convertBufferFromBacking(buffer);
    ASSERT(convertedBuffer);
    if (!convertedBuffer)
        return;

    m_backing->writeBuffer(*convertedBuffer, bufferOffset, data.data(), data.size(), 0, std::nullopt);
}

void RemoteQueue::writeTexture(
    const WebGPU::ImageCopyTexture& destination,
    Vector<uint8_t>&& data,
    const WebGPU::ImageDataLayout& dataLayout,
    const WebGPU::Extent3D& size)
{
    auto convertedDestination = m_objectRegistry.convertFromBacking(destination);
    ASSERT(convertedDestination);
    auto convertedDataLayout = m_objectRegistry.convertFromBacking(dataLayout);
    ASSERT(convertedDestination);
    auto convertedSize = m_objectRegistry.convertFromBacking(size);
    ASSERT(convertedSize);
    if (!convertedDestination || !convertedDestination || !convertedSize)
        return;

    m_backing->writeTexture(*convertedDestination, data.data(), data.size(), *convertedDataLayout, *convertedSize);
}

void RemoteQueue::copyExternalImageToTexture(
    const WebGPU::ImageCopyExternalImage& source,
    const WebGPU::ImageCopyTextureTagged& destination,
    const WebGPU::Extent3D& copySize)
{
    auto convertedSource = m_objectRegistry.convertFromBacking(source);
    ASSERT(convertedSource);
    auto convertedDestination = m_objectRegistry.convertFromBacking(destination);
    ASSERT(convertedDestination);
    auto convertedCopySize = m_objectRegistry.convertFromBacking(copySize);
    ASSERT(convertedCopySize);
    if (!convertedDestination || !convertedDestination || !convertedCopySize)
        return;

    m_backing->copyExternalImageToTexture(*convertedSource, *convertedDestination, *convertedCopySize);
}

void RemoteQueue::setLabel(String&& label)
{
    m_backing->setLabel(WTFMove(label));
}

} // namespace WebKit

#endif // ENABLE(GPU_PROCESS)
