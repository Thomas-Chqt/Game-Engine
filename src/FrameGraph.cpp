/*
 * ---------------------------------------------------
 * FrameGraph.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * ---------------------------------------------------
 */

#include "Game-Engine/FrameGraph.hpp"
#include "Graphics/Enums.hpp"
#include "TextureTable.hpp"

#include <Graphics/Buffer.hpp>
#include <Graphics/Texture.hpp>

#include <algorithm>
#include <cassert>
#include <optional>
#include <ranges>
#include <tracy/Tracy.hpp>
#include <utility>

namespace GE
{

FrameGraphBuilder::FrameGraphBuilder(TextureTable* textureTable, AssetManager* assetManager)
    : m_textureTable(textureTable)
    , m_assetManager(assetManager)
{
    assert(m_textureTable);
    assert(m_assetManager);
}

FrameGraph::TextureRef FrameGraphBuilder::newTexture(const gfx::Texture::Descriptor& desc)
{
    FrameGraph::TextureRef newTextureRef = m_frameGraph.textures.size();
    m_frameGraph.textures.emplace_back(desc);
    return newTextureRef;
}

FrameGraph::TextureRef FrameGraphBuilder::newTexture(std::pair<uint32_t, uint32_t> size, gfx::PixelFormat pixelFormat)
{
    return newTexture(gfx::Texture::Descriptor{
        .type = gfx::TextureType::texture2d,
        .width = size.first,
        .height = size.second,
        .pixelFormat = pixelFormat,
        .usages = gfx::TextureUsage{0},
        .storageMode = gfx::ResourceStorageMode::deviceLocal
    });
}

FrameGraph::TextureRef FrameGraphBuilder::newTexture(std::string name, const gfx::Texture::Descriptor& desc)
{
    FrameGraph::TextureRef textureRef = newTexture(desc);
    aliasTexture(std::move(name), textureRef);
    return textureRef;
}

FrameGraph::TextureRef FrameGraphBuilder::newTexture(std::string name, std::pair<uint32_t, uint32_t> size, gfx::PixelFormat pixelFormat)
{
    return newTexture(std::move(name), gfx::Texture::Descriptor{
        .type = gfx::TextureType::texture2d,
        .width = size.first,
        .height = size.second,
        .pixelFormat = pixelFormat,
        .usages = gfx::TextureUsage{0},
        .storageMode = gfx::ResourceStorageMode::deviceLocal
    });
}

void FrameGraphBuilder::aliasTexture(std::string name, FrameGraph::TextureRef textureRef)
{
    assert(textureRef < m_frameGraph.textures.size());
    [[maybe_unused]] auto [_, inserted] = m_textureNames.emplace(std::move(name), textureRef);
    assert(inserted);
}

FrameGraph::TextureRef FrameGraphBuilder::texture(std::string_view name) const
{
    auto it = m_textureNames.find(name);
    assert(it != m_textureNames.end());
    return it->second;
}

const std::map<std::string, FrameGraph::TextureRef, std::less<>>& FrameGraphBuilder::textureNames() const
{
    return m_textureNames;
}

void FrameGraphBuilder::setBackBuffer(FrameGraph::TextureRef textureRef)
{
    assert(m_frameGraph.textures.size() > textureRef);
    m_frameGraph.backBuffer = textureRef;
}

void FrameGraphBuilder::addPass(FramePass pass)
{
    assert(std::ranges::all_of(pass.colorAttachments, [&](auto& attachment){ return m_frameGraph.textures.size() > attachment.texture; }));
    assert(pass.depthAttachment.has_value() == false || m_frameGraph.textures.size() > pass.depthAttachment->texture);
    assert(std::ranges::all_of(pass.sampledTextures, [&](auto& textureRef){ return m_frameGraph.textures.size() > textureRef; }));

    for (FrameGraph::Texture& texture : pass.colorAttachments | std::views::transform([&](auto& attachement) -> FrameGraph::Texture& { return m_frameGraph.textures.at(attachement.texture); }))
        texture.descriptor.usages |= gfx::TextureUsage::colorAttachment;

    if (pass.depthAttachment)
        m_frameGraph.textures.at(pass.depthAttachment->texture).descriptor.usages |= gfx::TextureUsage::depthStencilAttachment;

    for (FrameGraph::Texture& texture : pass.sampledTextures | std::views::transform([&](auto& textureRef) -> FrameGraph::Texture& { return m_frameGraph.textures.at(textureRef); }))
        texture.descriptor.usages |= gfx::TextureUsage::shaderRead;

    m_frameGraph.passes.push_back(std::move(pass));
}

AssetManager& FrameGraphBuilder::assetManager() const
{
    return *m_assetManager;
}

uint32_t FrameGraphBuilder::textureIndex(AssetID assetId) const
{
    return m_textureTable->textureIndex(assetId);
}

FrameGraph FrameGraphBuilder::build() &&
{
    assert(m_frameGraph.backBuffer < m_frameGraph.textures.size());
    return std::move(m_frameGraph);
}

} // namespace GE
