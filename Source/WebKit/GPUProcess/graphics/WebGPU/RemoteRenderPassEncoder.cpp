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
#include "RemoteRenderPassEncoder.h"

#if ENABLE(GPU_PROCESS)

#include "WebGPUObjectHeap.h"
#include "WebGPUObjectRegistry.h"
#include <pal/graphics/WebGPU/WebGPURenderPassEncoder.h>

namespace WebKit {

RemoteRenderPassEncoder::RemoteRenderPassEncoder(PAL::WebGPU::RenderPassEncoder& renderPassEncoder, WebGPU::ObjectRegistry& objectRegistry, WebGPU::ObjectHeap& objectHeap, WebGPUIdentifier identifier)
    : m_backing(renderPassEncoder)
    , m_objectRegistry(objectRegistry)
    , m_objectHeap(objectHeap)
    , m_identifier(identifier)
{
    m_objectRegistry.addObject(m_identifier, m_backing);
}

RemoteRenderPassEncoder::~RemoteRenderPassEncoder()
{
    m_objectRegistry.removeObject(m_identifier);
}

void RemoteRenderPassEncoder::setPipeline(WebGPUIdentifier renderPipeline)
{
    auto convertedRenderPipeline = m_objectRegistry.convertRenderPipelineFromBacking(renderPipeline);
    ASSERT(convertedRenderPipeline);
    if (!convertedRenderPipeline)
        return;

    m_backing->setPipeline(*convertedRenderPipeline);
}

void RemoteRenderPassEncoder::setIndexBuffer(WebGPUIdentifier buffer, PAL::WebGPU::IndexFormat indexFormat, PAL::WebGPU::Size64 offset, std::optional<PAL::WebGPU::Size64> size)
{
    auto convertedBuffer = m_objectRegistry.convertBufferFromBacking(buffer);
    ASSERT(convertedBuffer);
    if (!convertedBuffer)
        return;

    m_backing->setIndexBuffer(*convertedBuffer, indexFormat, offset, size);
}

void RemoteRenderPassEncoder::setVertexBuffer(PAL::WebGPU::Index32 slot, WebGPUIdentifier buffer, PAL::WebGPU::Size64 offset, std::optional<PAL::WebGPU::Size64> size)
{
    auto convertedBuffer = m_objectRegistry.convertBufferFromBacking(buffer);
    ASSERT(convertedBuffer);
    if (!convertedBuffer)
        return;

    m_backing->setVertexBuffer(slot, *convertedBuffer, offset, size);
}

void RemoteRenderPassEncoder::draw(PAL::WebGPU::Size32 vertexCount, PAL::WebGPU::Size32 instanceCount,
    PAL::WebGPU::Size32 firstVertex, PAL::WebGPU::Size32 firstInstance)
{
    m_backing->draw(vertexCount, instanceCount, firstVertex, firstInstance);
}

void RemoteRenderPassEncoder::drawIndexed(PAL::WebGPU::Size32 indexCount, PAL::WebGPU::Size32 instanceCount,
    PAL::WebGPU::Size32 firstIndex,
    PAL::WebGPU::SignedOffset32 baseVertex,
    PAL::WebGPU::Size32 firstInstance)
{
    m_backing->drawIndexed(indexCount, instanceCount, firstIndex, baseVertex, firstInstance);
}

void RemoteRenderPassEncoder::drawIndirect(WebGPUIdentifier indirectBuffer, PAL::WebGPU::Size64 indirectOffset)
{
    auto convertedIndirectBuffer = m_objectRegistry.convertBufferFromBacking(indirectBuffer);
    ASSERT(convertedIndirectBuffer);
    if (!convertedIndirectBuffer)
        return;

    m_backing->drawIndirect(*convertedIndirectBuffer, indirectOffset);
}

void RemoteRenderPassEncoder::drawIndexedIndirect(WebGPUIdentifier indirectBuffer, PAL::WebGPU::Size64 indirectOffset)
{
    auto convertedIndirectBuffer = m_objectRegistry.convertBufferFromBacking(indirectBuffer);
    ASSERT(convertedIndirectBuffer);
    if (!convertedIndirectBuffer)
        return;

    m_backing->drawIndexedIndirect(*convertedIndirectBuffer, indirectOffset);
}

void RemoteRenderPassEncoder::setBindGroup(PAL::WebGPU::Index32 index, WebGPUIdentifier bindGroup,
    std::optional<Vector<PAL::WebGPU::BufferDynamicOffset>>&& dynamicOffsets)
{
    auto convertedBindGroup = m_objectRegistry.convertBindGroupFromBacking(bindGroup);
    ASSERT(convertedBindGroup);
    if (!convertedBindGroup)
        return;

    m_backing->setBindGroup(index, *convertedBindGroup, WTFMove(dynamicOffsets));
}

void RemoteRenderPassEncoder::pushDebugGroup(String&& groupLabel)
{
    m_backing->pushDebugGroup(WTFMove(groupLabel));
}

void RemoteRenderPassEncoder::popDebugGroup()
{
    m_backing->popDebugGroup();
}

void RemoteRenderPassEncoder::insertDebugMarker(String&& markerLabel)
{
    m_backing->insertDebugMarker(WTFMove(markerLabel));
}

void RemoteRenderPassEncoder::setViewport(float x, float y,
    float width, float height,
    float minDepth, float maxDepth)
{
    m_backing->setViewport(x, y, width, height, minDepth, maxDepth);
}

void RemoteRenderPassEncoder::setScissorRect(PAL::WebGPU::IntegerCoordinate x, PAL::WebGPU::IntegerCoordinate y,
    PAL::WebGPU::IntegerCoordinate width, PAL::WebGPU::IntegerCoordinate height)
{
    m_backing->setScissorRect(x, y, width, height);
}

void RemoteRenderPassEncoder::setBlendConstant(WebGPU::Color color)
{
    auto convertedColor = m_objectRegistry.convertFromBacking(color);
    ASSERT(convertedColor);
    if (!convertedColor)
        return;

    m_backing->setBlendConstant(*convertedColor);
}

void RemoteRenderPassEncoder::setStencilReference(PAL::WebGPU::StencilValue stencilValue)
{
    m_backing->setStencilReference(stencilValue);
}

void RemoteRenderPassEncoder::beginOcclusionQuery(PAL::WebGPU::Size32 queryIndex)
{
    m_backing->beginOcclusionQuery(queryIndex);
}

void RemoteRenderPassEncoder::endOcclusionQuery()
{
    m_backing->endOcclusionQuery();
}

void RemoteRenderPassEncoder::beginPipelineStatisticsQuery(WebGPUIdentifier querySet, PAL::WebGPU::Size32 queryIndex)
{
    auto convertedQuerySet = m_objectRegistry.convertQuerySetFromBacking(querySet);
    ASSERT(convertedQuerySet);
    if (!convertedQuerySet)
        return;

    m_backing->beginPipelineStatisticsQuery(*convertedQuerySet, queryIndex);
}

void RemoteRenderPassEncoder::endPipelineStatisticsQuery()
{
    m_backing->endPipelineStatisticsQuery();
}

void RemoteRenderPassEncoder::executeBundles(Vector<WebGPUIdentifier>&& renderBundles)
{
    Vector<std::reference_wrapper<PAL::WebGPU::RenderBundle>> convertedBundles;
    convertedBundles.reserveInitialCapacity(renderBundles.size());
    for (WebGPUIdentifier identifier : renderBundles) {
        auto convertedBundle = m_objectRegistry.convertRenderBundleFromBacking(identifier);
        ASSERT(convertedBundle);
        if (!convertedBundle)
            return;
        convertedBundles.uncheckedAppend(*convertedBundle);
    }
    m_backing->executeBundles(WTFMove(convertedBundles));
}

void RemoteRenderPassEncoder::endPass()
{
    m_backing->endPass();
}

void RemoteRenderPassEncoder::setLabel(String&& label)
{
    m_backing->setLabel(WTFMove(label));
}

} // namespace WebKit

#endif // ENABLE(GPU_PROCESS)
