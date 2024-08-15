/*
 * ---------------------------------------------------
 * GPURessourceManager.hpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/08/13 13:07:58
 * ---------------------------------------------------
 */

#ifndef GPURESSOURCEMANAGER_HPP
#define GPURESSOURCEMANAGER_HPP

#include "Graphics/Buffer.hpp"
#include "Graphics/FrameBuffer.hpp"
#include "Graphics/GraphicAPI.hpp"
#include "Graphics/GraphicPipeline.hpp"
#include "Graphics/Sampler.hpp"
#include "Graphics/Shader.hpp"
#include "Graphics/Texture.hpp"
#include "UtilsCPP/SharedPtr.hpp"
#include "UtilsCPP/UniquePtr.hpp"
#include <utility>

namespace GE
{

class GPURessourceManager
{
public:
    GPURessourceManager(const GPURessourceManager&) = delete;
    GPURessourceManager(GPURessourceManager&&)      = delete;

    static void init(const utils::SharedPtr<gfx::GraphicAPI>& api);
    static inline GPURessourceManager& shared() { return *s_sharedInstance; }
    static inline void terminate() { s_sharedInstance.clear(); }

    inline gfx::GraphicAPI& graphicAPI() { return *m_graphicAPI; }


    template<typename ... Args>
    inline utils::SharedPtr<gfx::Shader> newShader(Args&& ... args) { return m_graphicAPI->newShader(std::forward<Args>(args)...); }

    template<typename ... Args>
    inline utils::SharedPtr<gfx::GraphicPipeline> newGraphicsPipeline(Args&& ... args) { return m_graphicAPI->newGraphicsPipeline(std::forward<Args>(args)...); }

    template<typename ... Args>
    inline utils::SharedPtr<gfx::Buffer> newBuffer(Args&& ... args) { return m_graphicAPI->newBuffer(std::forward<Args>(args)...); }

    template<typename ... Args>
    inline utils::SharedPtr<gfx::Texture> newTexture(Args&& ... args) { return m_graphicAPI->newTexture(std::forward<Args>(args)...); }

    template<typename ... Args>
    inline utils::SharedPtr<gfx::Sampler> newSampler(Args&& ... args) { return m_graphicAPI->newSampler(std::forward<Args>(args)...); }

    template<typename ... Args>
    inline utils::SharedPtr<gfx::FrameBuffer> newFrameBuffer(Args&& ... args) { return m_graphicAPI->newFrameBuffer(std::forward<Args>(args)...); }


    ~GPURessourceManager() = default;

private:
    GPURessourceManager(const utils::SharedPtr<gfx::GraphicAPI>& api);

    inline static utils::UniquePtr<GPURessourceManager> s_sharedInstance;

    const utils::SharedPtr<gfx::GraphicAPI> m_graphicAPI;

public:
    GPURessourceManager& operator = (const GPURessourceManager&) = delete;
    GPURessourceManager& operator = (GPURessourceManager&&)      = delete;
};

}

#endif // GPURESSOURCEMANAGER_HPP