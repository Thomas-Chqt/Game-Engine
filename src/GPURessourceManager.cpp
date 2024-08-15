/*
 * ---------------------------------------------------
 * GPURessourceManager.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/08/15 15:49:40
 * ---------------------------------------------------
 */

#include "GPURessourceManager.hpp"
#include "UtilsCPP/SharedPtr.hpp"
#include "UtilsCPP/UniquePtr.hpp"

namespace GE
{

void GPURessourceManager::init(const utils::SharedPtr<gfx::GraphicAPI>& api)
{
    s_sharedInstance = utils::UniquePtr<GPURessourceManager>(new GPURessourceManager(api));
}

GPURessourceManager::GPURessourceManager(const utils::SharedPtr<gfx::GraphicAPI>& api)
    : m_graphicAPI(api)
{
}

}