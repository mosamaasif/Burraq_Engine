#pragma once

#include "Platform/Vulkan/VulkanHelpers.h"
#include "GraphicsPipeline.h"
#include "Mesh.h"

namespace BRQ {

    struct RenderCommand {

        const GraphicsPipeline*  Pipeline;
        const Mesh*              Mesh;
        const VkDescriptorSet*   DescriptorSets;
        U32                      DescriptorSetCount;
        const void*              PushConstantData;
        U32                      PushConstantSize;
        U32                      PushConstantOffset;
        PipelineStage            PushConstantStage;
    };
}
