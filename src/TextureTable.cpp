/*
 * ---------------------------------------------------
 * TextureTable.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * ---------------------------------------------------
 */

#include "TextureTable.hpp"
#include "Graphics/Enums.hpp"

#include <cassert>
#include <cstdint>
#include <ranges>

constexpr uint32_t MAX_TEXTURES = 4096;

namespace GE
{

TextureTable::TextureTable(gfx::ParameterBlock* parameterBlock, uint32_t bindingIndex)
    : m_parameterBlock(parameterBlock)
    , m_bindingIndex(bindingIndex)
{
    assert(parameterBlock);
    assert(parameterBlock->layout()->bindings().at(m_bindingIndex).type == gfx::BindingType::sampledTexture);
    assert(parameterBlock->layout()->bindings().at(m_bindingIndex).count == MAX_TEXTURES);

    m_freeTextureIndices = std::views::iota(uint32_t{0}, MAX_TEXTURES) | std::ranges::to<std::set>();
}

uint32_t TextureTable::textureIndex(AssetID assetId) const
{
    std::scoped_lock lock(m_mutex);
    assert(m_textureIndices.contains(assetId));
    return m_textureIndices.at(assetId);
}

void TextureTable::addTexture(AssetID assetId, const std::shared_ptr<gfx::Texture>& texture)
{
    std::scoped_lock lock(m_mutex);

    assert(texture);
    assert(m_parameterBlock);
    assert(m_textureIndices.contains(assetId) == false);
    assert(m_freeTextureIndices.empty() == false);

    const uint32_t index = m_freeTextureIndices.extract(m_freeTextureIndices.begin()).value();
    m_parameterBlock->setBinding(m_bindingIndex, index, texture);
    m_textureIndices.emplace(assetId, index);
}

void TextureTable::removeTexture(AssetID assetId)
{
    std::scoped_lock lock(m_mutex);

    assert(m_parameterBlock);
    assert(m_textureIndices.contains(assetId));

    const uint32_t index = m_textureIndices.extract(assetId).mapped();
    m_parameterBlock->clearBinding(1, index);

    [[maybe_unused]] const auto [_, inserted] = m_freeTextureIndices.insert(index);
    assert(inserted);
}

} // namespace GE::detail
