/*
 * ---------------------------------------------------
 * TextureTable.hpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * ---------------------------------------------------
 */

#ifndef TEXTURETABLE_HPP
#define TEXTURETABLE_HPP

#include "Game-Engine/AssetManager.hpp"

#include <Graphics/Device.hpp>
#include <Graphics/ParameterBlock.hpp>
#include <Graphics/ParameterBlockLayout.hpp>
#include <Graphics/ParameterBlockPool.hpp>
#include <Graphics/Sampler.hpp>
#include <Graphics/Texture.hpp>

#include <cstdint>
#include <map>
#include <memory>
#include <mutex>
#include <set>

namespace GE
{

class TextureTable
{
public:
    TextureTable() = delete;
    TextureTable(const TextureTable&) = delete;
    TextureTable(TextureTable&&) = delete;

    TextureTable(gfx::ParameterBlock*, uint32_t bindingIndex);

    uint32_t textureIndex(AssetID) const;

    void addTexture(AssetID, const std::shared_ptr<gfx::Texture>&);
    void removeTexture(AssetID);

    ~TextureTable() = default;

private:
    gfx::ParameterBlock* m_parameterBlock;
    uint32_t m_bindingIndex;

    std::map<AssetID, uint32_t> m_textureIndices;
    std::set<uint32_t> m_freeTextureIndices;

    mutable std::mutex m_mutex;

public:
    TextureTable& operator=(const TextureTable&) = delete;
    TextureTable& operator=(TextureTable&&) = delete;
};

} // namespace GE

#endif // TEXTURETABLE_HPP
