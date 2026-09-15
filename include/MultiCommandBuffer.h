#pragma once

#include <Metal/Metal.hpp>
#include <AppKit/AppKit.hpp>
#include <MetalKit/MetalKit.hpp>
#include <QuartzCore/CAMetalLayer.h>

#include <vector>
#include <deque>
#include <cstdint>
#include <functional>
#include <algorithm>

#include "DeletionQueue.h"
#include "metal_view_bridge.h"

class MultiCommandBuffer
{
public:
    MultiCommandBuffer() = default;

    MultiCommandBuffer(const uint8_t number, MTL::Device *metalDevice, DeletionQueue &dq)
    {
        metal4CommandBuffer.reserve(number);
        for (size_t i = 0; i < number; i++)
        {
            metal4CommandBuffer.emplace_back(metalDevice->newCommandBuffer());
        }

        dq.push_function([cmdBuffers = metal4CommandBuffer]()
                         {
            for (auto* cb : cmdBuffers)
            {
                if (cb) cb->release();
            } });
    }

    void pushback_function(std::function<void(MTL4::CommandBuffer *)> &&function)
    {
        DequecommandBuffer.push_back(std::move(function));
    }

    void Execute(MTL4::CommandAllocator *cmd_alloc, void *metalLayerHandle,
                 MTL4::CommandQueue *metal4CommandQueue)
    {
        const size_t count = std::min(DequecommandBuffer.size(), metal4CommandBuffer.size());
        CA::MetalLayer *metalLayerCpp = MetalViewBridge::AsMetalLayerCpp(metalLayerHandle);

        for (size_t i = 0; i < count; i++)
        {
            MTL4::CommandBuffer *cb = metal4CommandBuffer[i];

            cb->beginCommandBuffer(cmd_alloc);
            DequecommandBuffer[i](cb);
            cb->useResidencySet(metalLayerCpp->residencySet());
            cb->endCommandBuffer();
        }

        if (count > 0)
        {
            metal4CommandQueue->commit(metal4CommandBuffer.data(), count);
        }

        DequecommandBuffer.clear();
    }

private:
    std::vector<MTL4::CommandBuffer *> metal4CommandBuffer;
    std::deque<std::function<void(MTL4::CommandBuffer *)>> DequecommandBuffer;
};