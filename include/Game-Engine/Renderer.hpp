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

#include <Graphics/Device.hpp>

#include <memory>

#define cfd m_frameDatas.at(m_frameIdx)

namespace GE
{

constexpr uint8_t maxFrameInFlight = 3;

class Renderer
{
public:
    Renderer() = delete;
    Renderer(const Renderer&) = delete;
    Renderer(Renderer&&) = delete;

    Renderer(gfx::Device*);

    ~Renderer() = default;

private:
    struct FrameData
    {
        std::unique_ptr<gfx::CommandBufferPool> commandBufferPool;
        std::unique_ptr<gfx::ParameterBlockPool> parameterBlockPool;

        gfx::CommandBuffer* lastCommandBuffer = nullptr;
    };

    gfx::Device* m_device;

    uint8_t m_frameIdx = 0;
    std::array<FrameData, maxFrameInFlight> m_frameDatas;

public:
    Renderer& operator=(const Renderer&) = delete;
    Renderer& operator=(Renderer&&) = delete;
};

} // namespace GE

#endif // RENDERER_HPP
