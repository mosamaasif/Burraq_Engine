#include <vector>
#include <string_view>

#include <Engine.h>
#include <BRQ/Application/EntryPoint.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Sandbox : public BRQ::Application {

private:
    BRQ::Texture2D*       m_Texture2D;
    BRQ::TextureCube*     m_TextureCube;

    BRQ::GraphicsPipeline m_Pipeline;
    BRQ::GraphicsPipeline m_SkyboxPipeline;

    BRQ::Mesh             m_MeshData;
    BRQ::Skybox           m_SkyboxData;

    VkDescriptorPool      m_DescriptorPools[FRAME_LAG];
    VkDescriptorPool      m_SkyboxDescriptorPools[FRAME_LAG];

    std::vector<VkDescriptorSet> m_DescriptorSets[FRAME_LAG];
    std::vector<VkDescriptorSet> m_SkyboxDescriptorSets[FRAME_LAG];

public:
    Sandbox(const BRQ::WindowProperties& props)
        : Application(props), m_Texture2D(nullptr), m_TextureCube(nullptr)
    {
        CreateTextures();
        CreatePipelines();
        CreateDescriptorPools();
        CreateDescriptorSets();
        LoadMeshes();
    }

    ~Sandbox()
    {
        vkDeviceWaitIdle(BRQ::RenderContext::GetInstance()->GetDevice());

        m_MeshData.DestroyMesh();
        m_SkyboxData.DestroyMesh();

        DestroyPipelines();
        DestroyDescriptorPools();
        DestroyTextures();
    }

    void OnUpdate(F32 dt) override
    {
        Application::OnUpdate(dt);

        m_Renderer->BeginScene(m_CameraController.GetCamera());

        const BRQ::Camera& camera = m_CameraController.GetCamera();

        // Main mesh
        glm::mat4 pv = camera.GetProjectionMatrix() * camera.GetViewMatrix();

        U32 frameIndex = m_Renderer->GetCurrentFrameIndex();

        BRQ::RenderCommand meshCmd = {};
        meshCmd.Pipeline = &m_Pipeline;
        meshCmd.Mesh = &m_MeshData;
        meshCmd.DescriptorSets = m_DescriptorSets[frameIndex].data();
        meshCmd.DescriptorSetCount = (U32)m_DescriptorSets[frameIndex].size();
        meshCmd.PushConstantData = &pv[0];
        meshCmd.PushConstantSize = sizeof(glm::mat4);
        meshCmd.PushConstantOffset = 0;
        meshCmd.PushConstantStage = BRQ::PipelineStage::Vertex;

        m_Renderer->Submit(meshCmd);

        // Skybox
        glm::mat4 skyboxPV = camera.GetProjectionMatrix() * glm::mat4(glm::mat3(camera.GetViewMatrix()));

        const BRQ::Mesh& skyboxMesh = m_SkyboxData.GetMesh();

        BRQ::RenderCommand skyboxCmd = {};
        skyboxCmd.Pipeline = &m_SkyboxPipeline;
        skyboxCmd.Mesh = &skyboxMesh;
        skyboxCmd.DescriptorSets = m_SkyboxDescriptorSets[frameIndex].data();
        skyboxCmd.DescriptorSetCount = (U32)m_SkyboxDescriptorSets[frameIndex].size();
        skyboxCmd.PushConstantData = &skyboxPV[0];
        skyboxCmd.PushConstantSize = sizeof(glm::mat4);
        skyboxCmd.PushConstantOffset = 0;
        skyboxCmd.PushConstantStage = BRQ::PipelineStage::Vertex;

        m_Renderer->Submit(skyboxCmd);

        m_Renderer->EndScene();
        m_Renderer->Present();
    }

private:
    void CreateTextures()
    {
        m_Texture2D = new BRQ::Texture2D("Resources/Textures/Lion.jpg");

        std::vector<std::string_view> filenames = {
            "Resources/Textures/Skybox/posz.jpg",
            "Resources/Textures/Skybox/negz.jpg",
            "Resources/Textures/Skybox/negy.jpg",
            "Resources/Textures/Skybox/posy.jpg",
            "Resources/Textures/Skybox/posx.jpg",
            "Resources/Textures/Skybox/negx.jpg",
        };

        m_TextureCube = new BRQ::TextureCube(filenames);
    }

    void DestroyTextures()
    {
        delete m_TextureCube;
        m_TextureCube = nullptr;

        delete m_Texture2D;
        m_Texture2D = nullptr;
    }

    void CreatePipelines()
    {
        BRQ::BufferLayout meshLayout;
        meshLayout.PushElement(BRQ::ElementType::Vec3, 3 * sizeof(float));
        meshLayout.PushElement(BRQ::ElementType::Vec2, 2 * sizeof(float));

        BRQ::GraphicsPipelineCreateInfo meshInfo = {};
        meshInfo.Layout = meshLayout;
        meshInfo.Flags = (BRQ::GraphicsPipelineFlags)(BRQ::EnableCulling | BRQ::DepthWriteEnabled | BRQ::DepthTestEnabled | BRQ::DepthCompareLess);
        meshInfo.Shaders = { { "Resources/Shaders/shader.vert.spv" }, { "Resources/Shaders/shader.frag.spv" } };

        m_Pipeline.Init(meshInfo);

        BRQ::BufferLayout skyboxLayout;
        skyboxLayout.PushElement(BRQ::ElementType::Vec3, 3 * sizeof(float));

        BRQ::GraphicsPipelineCreateInfo skyboxInfo = {};
        skyboxInfo.Layout = skyboxLayout;
        skyboxInfo.Flags = (BRQ::GraphicsPipelineFlags)(BRQ::DepthTestEnabled | BRQ::DepthCompareLess | BRQ::DepthCompareEqual | BRQ::CullModeFrontFace | BRQ::EnableCulling);
        skyboxInfo.Shaders = { { "Resources/Shaders/skyboxShader.vert.spv" }, { "Resources/Shaders/skyboxShader.frag.spv" } };

        m_SkyboxPipeline.Init(skyboxInfo);
    }

    void DestroyPipelines()
    {
        m_Pipeline.Destroy();
        m_SkyboxPipeline.Destroy();
    }

    void CreateDescriptorPools()
    {
        VkDescriptorPoolSize size = {};
        size.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        size.descriptorCount = 1;

        BRQ::VK::DescriptorPoolCreateInfo info = {};
        info.MaxSets = 4;
        info.PoolSizeCount = 1;
        info.PoolSizes = &size;

        auto* context = BRQ::RenderContext::GetInstance();

        for (U64 i = 0; i < FRAME_LAG; i++) {

            m_DescriptorPools[i] = BRQ::VK::CreateDescriptorPool(context->GetDevice(), info);
            m_SkyboxDescriptorPools[i] = BRQ::VK::CreateDescriptorPool(context->GetDevice(), info);
        }
    }

    void DestroyDescriptorPools()
    {
        auto* context = BRQ::RenderContext::GetInstance();

        for (U64 i = 0; i < FRAME_LAG; i++) {

            BRQ::VK::DestroyDescriptorPool(context->GetDevice(), m_DescriptorPools[i]);
            BRQ::VK::DestroyDescriptorPool(context->GetDevice(), m_SkyboxDescriptorPools[i]);
        }
    }

    void CreateDescriptorSets()
    {
        auto* context = BRQ::RenderContext::GetInstance();

        std::vector<VkDescriptorSetLayout> layouts = m_Pipeline.GetDescriptorSetLayouts();
        std::vector<VkDescriptorSetLayout> skyboxLayouts = m_SkyboxPipeline.GetDescriptorSetLayouts();

        for (U64 i = 0; i < FRAME_LAG; i++) {

            BRQ::VK::DescriptorSetAllocateInfo allocInfo = {};
            allocInfo.DescriptorPool = m_DescriptorPools[i];
            allocInfo.DescriptorSetCount = (U32)layouts.size();
            allocInfo.SetLayouts = layouts.data();

            m_DescriptorSets[i] = std::move(BRQ::VK::AllocateDescriptorSets(context->GetDevice(), allocInfo));

            allocInfo.DescriptorPool = m_SkyboxDescriptorPools[i];
            allocInfo.DescriptorSetCount = (U32)skyboxLayouts.size();
            allocInfo.SetLayouts = skyboxLayouts.data();

            m_SkyboxDescriptorSets[i] = std::move(BRQ::VK::AllocateDescriptorSets(context->GetDevice(), allocInfo));

            for (U64 j = 0; j < m_DescriptorSets[i].size(); j++) {

                VkDescriptorImageInfo imageInfo = {};
                imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                imageInfo.imageView = m_Texture2D->GetImageView();
                imageInfo.sampler = m_Texture2D->GetSampler();

                VkWriteDescriptorSet descriptorWrites = {};
                descriptorWrites.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                descriptorWrites.dstSet = m_DescriptorSets[i][j];
                descriptorWrites.dstBinding = 0;
                descriptorWrites.dstArrayElement = 0;
                descriptorWrites.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                descriptorWrites.descriptorCount = 1;
                descriptorWrites.pImageInfo = &imageInfo;

                vkUpdateDescriptorSets(context->GetDevice(), 1, &descriptorWrites, 0, nullptr);
            }

            for (size_t k = 0; k < m_SkyboxDescriptorSets[i].size(); k++) {

                VkDescriptorImageInfo imageInfo = {};
                imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                imageInfo.imageView = m_TextureCube->GetImageView();
                imageInfo.sampler = m_TextureCube->GetSampler();

                VkWriteDescriptorSet descriptorWrites = {};
                descriptorWrites.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                descriptorWrites.dstSet = m_SkyboxDescriptorSets[i][k];
                descriptorWrites.dstBinding = 0;
                descriptorWrites.dstArrayElement = 0;
                descriptorWrites.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                descriptorWrites.descriptorCount = 1;
                descriptorWrites.pImageInfo = &imageInfo;

                vkUpdateDescriptorSets(context->GetDevice(), 1, &descriptorWrites, 0, nullptr);
            }
        }
    }

    void LoadMeshes()
    {
        m_MeshData.LoadMesh("Resources/Models/Lion.obj");
        m_SkyboxData.Load();
    }
};

BRQ::Application* BRQ::CreateApplication(const BRQ::WindowProperties& props) {

    return new Sandbox(props);
}
