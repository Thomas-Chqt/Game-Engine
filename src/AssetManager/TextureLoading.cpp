/*
 * ---------------------------------------------------
 * TextureLoading.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/08/20 15:26:19
 * ---------------------------------------------------
 */

#include "GPURessourceManager.hpp"
#include "Game-Engine/AssetManager.hpp"
#include "Graphics/Texture.hpp"
#include "UtilsCPP/Dictionary.hpp"
#include "UtilsCPP/RuntimeError.hpp"
#include "UtilsCPP/SharedPtr.hpp"
#include "UtilsCPP/String.hpp"
#include "stb_image/stb_image.h"
#include <utility>

namespace GE
{

utils::SharedPtr<gfx::Texture> AssetManager::loadTexture(const utils::String &filePath)
{
    using Iterator = utils::Dictionary<utils::String, utils::SharedPtr<gfx::Texture>>::Iterator;
    Iterator it = m_cachedTextures.find(filePath);
    if (it != m_cachedTextures.end())
        return it->val;

    int width, height;
    stbi_uc* imgBytes = stbi_load(filePath, &width, &height, nullptr, STBI_rgb_alpha);
    if (imgBytes == nullptr)
        throw utils::RuntimeError("fail to read texture at path: " + filePath);

    gfx::Texture::Descriptor textureDescriptor;
    textureDescriptor.width = width;
    textureDescriptor.height = height;

    utils::SharedPtr<gfx::Texture> newTexture = GPURessourceManager::shared().newTexture(textureDescriptor);
    newTexture->replaceContent(imgBytes);

    stbi_image_free(imgBytes);

    return m_cachedTextures.insert(filePath, std::move(newTexture))->val;
}

void AssetManager::unloadTexture(const utils::String& filePath)
{
    m_cachedTextures.remove(filePath);
}

}