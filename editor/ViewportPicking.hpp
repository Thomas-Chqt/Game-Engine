#pragma once

#include <Game-Engine/EntityID.hpp>

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <optional>
#include <span>
#include <utility>

namespace GE_Editor
{

inline std::optional<GE::EntityID> decodeViewportPick(std::span<const std::byte> pixels,
    std::pair<uint32_t, uint32_t> viewportSize, std::pair<uint32_t, uint32_t> mousePos,
    std::span<const GE::EntityID> pickingEntities)
{
    const auto [width, height] = viewportSize;
    const auto [x, y] = mousePos;
    if (width == 0 || height == 0 || x >= width || y >= height)
        return std::nullopt;

    constexpr size_t pixelSize = sizeof(uint32_t)*2;
    const size_t availablePixels = pixels.size()/pixelSize;
    if (width > availablePixels || y > availablePixels/width)
        return std::nullopt;
    const size_t rowStart = static_cast<size_t>(y)*width;
    if (x >= availablePixels-rowStart)
        return std::nullopt;

    std::array<uint32_t, 2> words{};
    std::memcpy(words.data(), pixels.data()+(rowStart+x)*pixelSize, pixelSize);
    const uint64_t token = uint64_t(words[0]) | (uint64_t(words[1]) << 32);
    if (token == 0 || token > pickingEntities.size())
        return std::nullopt;
    return pickingEntities[static_cast<size_t>(token-1)];
}

} // namespace GE_Editor
