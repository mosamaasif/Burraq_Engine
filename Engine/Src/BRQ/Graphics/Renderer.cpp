#include <BRQ.h>

#include "Renderer.h"

#include "Application/Window.h"
#include "Utilities/VulkanMemoryAllocator.h"

#include "Platform/Vulkan/RenderContext.h"
#include "Platform/Vulkan/VulkanCommands.h"


namespace BRQ {

    Renderer* Renderer::s_Renderer = nullptr;

    Renderer::Renderer()
        : m_RenderContext(nullptr), m_Window(nullptr) { }

    void Renderer::Init(const Window* window) {

        s_Renderer = new Renderer();
        s_Renderer->InitInternal(window);
    }

    void Renderer::Shutdown() {

        s_Renderer->DestroyInternal();

        delete s_Renderer;
    }

    void Renderer::BeginScene(const Camera& camera) {

        U32 index = m_RenderContext->GetCurrentIndex();

        const PerFrame& perframe = m_PerFrameData[index];

        VK::WaitForFence(m_RenderContext->GetDevice(), perframe.CommandBufferExecutedFence);
        VK::ResetFence(m_RenderContext->GetDevice(), perframe.CommandBufferExecutedFence);

        VK::ResetCommandPool(m_RenderContext->GetDevice(), perframe.CommandPool);

        VkCommandBuffer buffer = perframe.CommandBuffer;

        VkResult result = m_RenderContext->AcquireImageIndex(perframe.ImageAvailableSemaphore);

        if (result == VK_SUBOPTIMAL_KHR || result == VK_ERROR_OUT_OF_DATE_KHR) {

            RecreateSwapchain();
            result = m_RenderContext->AcquireImageIndex(perframe.ImageAvailableSemaphore);
        }

        if (result != VK_SUCCESS) {

            VK::QueueWaitIdle(m_RenderContext->GetPresentationQueue());
            return;
        }

        U32 imageIndex = m_RenderContext->GetAcquiredImageIndex();

        VK::CommandBufferBeginInfo beginInfo = {};
        beginInfo.CommandBuffer = buffer;
        beginInfo.Flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        VK::CommandBufferBegin(beginInfo);

        VK::RenderPassBeginInfo info = {};
        info.CommandBuffer = buffer;
        info.RenderPass = m_RenderContext->GetRenderPass();
        info.Framebuffer = m_Framebuffers[imageIndex];
        info.RenderArea.extent = m_RenderContext->GetSwapchainExtent2D();
        info.RenderArea.offset = { 0, 0 };

        VK::CommandBeginRenderPass(info);

        VkExtent2D extent = m_RenderContext->GetSwapchainExtent2D();
        VkViewport viewport = {};
        viewport.x = 0.0f;
        viewport.y = (F32)extent.height;
        viewport.width = (F32)extent.width;
        viewport.height = -(F32)extent.height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        VkRect2D scissor = {};
        scissor.offset = { 0, 0 };
        scissor.extent = extent;

        vkCmdSetViewport(buffer, 0, 1, &viewport);
        vkCmdSetScissor(buffer, 0, 1, &scissor);
    }

    void Renderer::Submit(const RenderCommand& command) {

        U32 index = m_RenderContext->GetCurrentIndex();
        VkCommandBuffer buffer = m_PerFrameData[index].CommandBuffer;

        command.Pipeline->Bind(buffer);

        VkDeviceSize offset = 0;
        vkCmdBindVertexBuffers(buffer, 0, 1, &command.Mesh->VertexBuffer.Buffer, &offset);
        vkCmdBindIndexBuffer(buffer, command.Mesh->IndexBuffer.Buffer, 0, VK_INDEX_TYPE_UINT32);

        if (command.PushConstantData && command.PushConstantSize > 0) {

            command.Pipeline->PushConstantData(buffer, command.PushConstantStage, command.PushConstantData, command.PushConstantSize, command.PushConstantOffset);
        }

        if (command.DescriptorSets && command.DescriptorSetCount > 0) {

            command.Pipeline->BindDescriptorSets(buffer, command.DescriptorSets, command.DescriptorSetCount);
        }

        vkCmdDrawIndexed(buffer, (U32)command.Mesh->IndexCount, 1, 0, 0, 0);
    }

    void Renderer::EndScene() {

        U32 index = m_RenderContext->GetCurrentIndex();

        const PerFrame& perframe = m_PerFrameData[index];

        VkCommandBuffer buffer = perframe.CommandBuffer;

        VK::CommandEndRenderPass(buffer);

        VK::CommandBufferEnd(buffer);

        VK::QueueSubmitInfo submitInfo = {};
        submitInfo.WaitSemaphoreCount = 1;
        submitInfo.WaitSemaphores = &perframe.ImageAvailableSemaphore;
        submitInfo.WaitDstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        submitInfo.CommandBufferCount = 1;
        submitInfo.CommandBuffers = &buffer;
        submitInfo.SignalSemaphoreCount = 1;
        submitInfo.SignalSemaphores = &perframe.RenderFinishedSemaphore;
        submitInfo.Queue = m_RenderContext->GetGraphicsQueue();
        submitInfo.CommandBufferExecutedFence = perframe.CommandBufferExecutedFence;

        VK::QueueSubmit(submitInfo);
    }

    void Renderer::Present() {

        U32 index = m_RenderContext->GetCurrentIndex();

        const PerFrame& perframe = m_PerFrameData[index];

        VkResult result = m_RenderContext->Present(perframe.RenderFinishedSemaphore);

        if (result == VK_SUBOPTIMAL_KHR || result == VK_ERROR_OUT_OF_DATE_KHR) {

            RecreateSwapchain();
        }
        else if (result != VK_SUCCESS) {

            BRQ_CORE_ERROR("Failed to present swapchain image.");
        }
    }

    U32 Renderer::GetCurrentFrameIndex() const {

        return m_RenderContext->GetCurrentIndex();
    }

    void Renderer::InitInternal(const Window* window) {

        m_Window = window;

        RenderContext::Init(window);

        m_RenderContext = RenderContext::GetInstance();

        CreateFramebuffers();
        CreateCommands();
        CreateSynchronizationPrimitives();
    }

    void Renderer::DestroyInternal() {

        vkDeviceWaitIdle(m_RenderContext->GetDevice());

        DestroySynchronizationPrimitives();
        DestroyCommands();
        DestroyFramebuffers();

        RenderContext::Destroy();
    }

    void Renderer::RecreateSwapchain() {

        VK_CHECK(vkDeviceWaitIdle(m_RenderContext->GetDevice()));

        DestroyFramebuffers();
        m_RenderContext->UpdateSwapchain();
        CreateFramebuffers();
    }

    void Renderer::CreateFramebuffers() {

        m_Framebuffers.resize(m_RenderContext->GetImageCount());

        VkExtent2D extent = m_RenderContext->GetSwapchainExtent2D();

        const auto& views = m_RenderContext->GetImageViews();

        for (U64 i = 0; i < m_Framebuffers.size(); i++) {

            VK::FramebufferCreateInfo info = {};
            info.Attachments = { views[i], m_RenderContext->GetDepthView() };
            info.Layers = 1;
            info.RenderPass = m_RenderContext->GetRenderPass();
            info.Height = extent.height;
            info.Width = extent.width;

            m_Framebuffers[i] = VK::CreateFramebuffer(m_RenderContext->GetDevice(), info);
        }
    }

    void Renderer::DestroyFramebuffers() {

        for (U64 i = 0; i < m_Framebuffers.size(); i++) {

            VK::DestroyFramebuffer(m_RenderContext->GetDevice(), m_Framebuffers[i]);
        }

        m_Framebuffers.clear();
    }

    void Renderer::CreateCommands() {

        for (U64 i = 0; i < FRAME_LAG; i++) {

            VK::CommandPoolCreateInfo info = {};
            info.Flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
            info.QueueFamilyIndex = m_RenderContext->GetGraphicsQueueIndex();

            m_PerFrameData[i].CommandPool = VK::CreateCommandPool(m_RenderContext->GetDevice(), info);

            VK::CommandBufferAllocateInfo allocateInfo = {};
            allocateInfo.CommandPool = m_PerFrameData[i].CommandPool;
            allocateInfo.Level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            allocateInfo.CommandBufferCount = 1;

            auto buffer = VK::AllocateCommandBuffers(m_RenderContext->GetDevice(), allocateInfo);
            m_PerFrameData[i].CommandBuffer = buffer[0];
        }
    }

    void Renderer::DestroyCommands() {

        for (U64 i = 0; i < FRAME_LAG; i++) {

#if defined(BRQ_DEBUG)
            VK::FreeCommandBuffer(m_RenderContext->GetDevice(), m_PerFrameData[i].CommandPool, m_PerFrameData[i].CommandBuffer);
#endif
            VK::DestroyCommandPool(m_RenderContext->GetDevice(), m_PerFrameData[i].CommandPool);
        }
    }

    void Renderer::CreateSynchronizationPrimitives() {

        for (U64 i = 0; i < FRAME_LAG; i++) {

            m_PerFrameData[i].ImageAvailableSemaphore = VK::CreateSemaphore(m_RenderContext->GetDevice());
            m_PerFrameData[i].RenderFinishedSemaphore = VK::CreateSemaphore(m_RenderContext->GetDevice());
            m_PerFrameData[i].CommandBufferExecutedFence = VK::CreateFence(m_RenderContext->GetDevice());
        }
    }

    void Renderer::DestroySynchronizationPrimitives() {

        for (U64 i = 0; i < FRAME_LAG; i++) {

            VK::DestroySemaphore(m_RenderContext->GetDevice(), m_PerFrameData[i].ImageAvailableSemaphore);
            VK::DestroySemaphore(m_RenderContext->GetDevice(), m_PerFrameData[i].RenderFinishedSemaphore);
            VK::DestroyFence(m_RenderContext->GetDevice(), m_PerFrameData[i].CommandBufferExecutedFence);
        }
    }
}
