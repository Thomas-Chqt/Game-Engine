/*
 * ---------------------------------------------------
 * Renderer.hpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/08/09 15:49:29
 * ---------------------------------------------------
 */

#ifndef RENDERER_HPP
#define RENDERER_HPP

#include "Game-Engine/Export.hpp"
#include "Game-Engine/FrameGraph.hpp"
#include "Game-Engine/AssetManager.hpp"
#include "Graphics/ParameterBlock.hpp"

#include <Graphics/Device.hpp>
#include <Graphics/Surface.hpp>
#include <Graphics/Swapchain.hpp>
#include <Graphics/Texture.hpp>
#include <Graphics/Buffer.hpp>
#include <Graphics/GraphicsPipeline.hpp>

#include <tracy/Tracy.hpp>
#include <gfx_tracy/gfx_tracy.hpp>

#include <cstddef>
#include <cstdint>
#include <array>
#include <map>
#include <memory>
#include <set>
#include <vector>

#define cfd m_inFlightDatas.at(m_frameIdx)

namespace GE
{

constexpr uint8_t maxFrameInFlight = 3;

class TextureTable;

class GE_API Renderer
{
public:
    Renderer() = delete;
    Renderer(const Renderer&) = delete;
    Renderer(Renderer&&) = delete;

    Renderer(gfx::Device*, AssetManager*, gfx::Surface*);

    FrameGraphBuilder newFrameGraphBuilder();
    void renderFrame(FrameGraph);

    ~Renderer();

private:
    struct PendingReadback
    {
        std::shared_ptr<gfx::Buffer> buffer;
        std::unique_ptr<FrameGraph::ReadbackBase> request;
    };

    struct InFlightData
    {
        std::unique_ptr<gfx::CommandBufferPool> commandBufferPool;
        std::unique_ptr<gfx::ParameterBlockPool> parameterBlockPool;
        gfx::CommandBuffer* waitedCmdBuffer = nullptr;

        std::map<gfx::Texture::Descriptor, std::set<std::shared_ptr<gfx::Texture>>> textureCache;
        std::map<gfx::Buffer::Descriptor, std::set<std::shared_ptr<gfx::Buffer>>> bufferCache;
        std::vector<PendingReadback> pendingReadbacks;
    };

    gfx::Device* m_device;
    AssetManager* m_assetManager;
    gfx::Surface* m_surface;
    gfx::tracy::TracyGfxCtx* m_tracyGfxCtx;

    std::shared_ptr<gfx::ParameterBlock> m_textureTableBlock;
    std::shared_ptr<TextureTable> m_textureTable;
    std::shared_ptr<gfx::ParameterBlockLayout> m_frameDataBlockLayout;

    std::shared_ptr<gfx::ParameterBlockLayout> m_texturedMaterialPBlockLayout;
    std::shared_ptr<gfx::GraphicsPipeline> m_texturedPipeline;

    std::unique_ptr<gfx::Swapchain> m_swapchain;

    uint8_t m_frameIdx = 0;
    std::array<InFlightData, maxFrameInFlight> m_inFlightDatas;

public:
    Renderer& operator=(const Renderer&) = delete;
    Renderer& operator=(Renderer&&) = delete;
};

} // namespace GE

#endif // RENDERER_HPP
