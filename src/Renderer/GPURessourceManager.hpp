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
#include "Graphics/GraphicAPI.hpp"
#include "UtilsCPP/SharedPtr.hpp"
#include "UtilsCPP/UniquePtr.hpp"
#include <utility>

namespace GE
{

template<typename T> using GPURessource = utils::SharedPtr<T>;

class GPURessourceManager
{
public:
    GPURessourceManager(const GPURessourceManager&) = delete;
    GPURessourceManager(GPURessourceManager&&)      = delete;

    static inline void init() { s_sharedInstance = utils::UniquePtr<GPURessourceManager>(new GPURessourceManager()); }
    static inline GPURessourceManager& shared() { return *s_sharedInstance; }
    static inline void terminate() { s_sharedInstance.clear(); }

    inline void setGraphicAPI(const utils::SharedPtr<gfx::GraphicAPI>& api) { m_graphicAPI = api; }

    template<typename ... Args>
    GPURessource<gfx::Buffer> newManagedBuffer(Args&& ... args)
    {
        return m_graphicAPI->newBuffer(std::forward<Args>(args)...);
    }

    ~GPURessourceManager() = default;

private:
    GPURessourceManager() = default;

    inline static utils::UniquePtr<GPURessourceManager> s_sharedInstance;

    utils::SharedPtr<gfx::GraphicAPI> m_graphicAPI;

public:
    GPURessourceManager& operator = (const GPURessourceManager&) = delete;
    GPURessourceManager& operator = (GPURessourceManager&&)      = delete;
};

}

#endif // GPURESSOURCEMANAGER_HPP