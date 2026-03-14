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

#include "Game-Engine/FrameGraph.hpp"

#include <Graphics/Device.hpp>
#include <Graphics/Surface.hpp>
#include <Graphics/Swapchain.hpp>
#include <Graphics/Texture.hpp>
#include <Graphics/Buffer.hpp>
#include <Graphics/GraphicsPipeline.hpp>

#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <set>

#define cfd m_inFlightDatas.at(m_frameIdx)

namespace GE
{

constexpr uint8_t maxFrameInFlight = 3;

class Renderer
{
public:
    Renderer() = delete;
    Renderer(const Renderer&) = delete;
    Renderer(Renderer&&) = delete;

    Renderer(gfx::Device*, gfx::Surface*);

    void renderFrame(const FrameGraph&);

    ~Renderer();

private:
    struct InFlightData
    {
        std::unique_ptr<gfx::CommandBufferPool> commandBufferPool;
        std::unique_ptr<gfx::ParameterBlockPool> parameterBlockPool;
        gfx::CommandBuffer* waitedCmdBuffer = nullptr;

        std::map<std::string, std::shared_ptr<gfx::Buffer>> constantBuffers;
        std::map<std::string, std::shared_ptr<gfx::Buffer>> structuredBuffers;

        std::set<std::pair<gfx::Texture::Descriptor, std::shared_ptr<gfx::Texture>>> transientTextures;
    };

    gfx::Device* m_device;
    gfx::Surface* m_surface;

    std::shared_ptr<gfx::ParameterBlockLayout> m_frameDataBlockLayout;
    std::shared_ptr<gfx::ParameterBlockLayout> m_materialBlockLayout;
    std::shared_ptr<gfx::GraphicsPipeline> m_gfxPipeline; // only one for now

    std::unique_ptr<gfx::Swapchain> m_swapchain;

    uint8_t m_frameIdx = 0;
    std::array<InFlightData, maxFrameInFlight> m_inFlightDatas;

public:
    Renderer& operator=(const Renderer&) = delete;
    Renderer& operator=(Renderer&&) = delete;
};

} // namespace GE

#endif // RENDERER_HPP
