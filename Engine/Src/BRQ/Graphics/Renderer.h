#pragma once

#include "Events/Event.h"
#include "Camera/Camera.h"
#include "Platform/Vulkan/RenderContext.h"
#include "RenderCommand.h"

namespace BRQ {

    struct PerFrame {

        VkCommandPool                CommandPool;
        VkCommandBuffer              CommandBuffer;

        VkSemaphore                  ImageAvailableSemaphore;
        VkSemaphore                  RenderFinishedSemaphore;
        VkFence                      CommandBufferExecutedFence;
    };

    class Renderer {

    private:
        static Renderer*											s_Renderer;

        const Window*												m_Window;
        RenderContext*                                              m_RenderContext;

        PerFrame                                                    m_PerFrameData[FRAME_LAG];
        std::vector<VkFramebuffer>                                  m_Framebuffers;

    protected:
        Renderer();
        Renderer(const Renderer& renderer) = default;

    public:
        ~Renderer() = default;

        static void Init(const Window* window);
        static void Shutdown();

        static Renderer* GetInstance() { return s_Renderer;  }

        void BeginScene(const Camera& camera);
        void Submit(const RenderCommand& command);
        void EndScene();

        void Present();

        U32 GetCurrentFrameIndex() const;

    private:
        void InitInternal(const Window* window);
        void DestroyInternal();

        void RecreateSwapchain();

        void CreateFramebuffers();
        void DestroyFramebuffers();

        void CreateCommands();
        void DestroyCommands();

        void CreateSynchronizationPrimitives();
        void DestroySynchronizationPrimitives();
    };
}
