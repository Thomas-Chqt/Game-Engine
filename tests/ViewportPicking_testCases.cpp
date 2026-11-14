#include "../editor/ViewportPicking.hpp"

#include <gtest/gtest.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>

namespace
{
const std::array<GE::EntityID, 2> entities = {
    *uuids::uuid::from_string("00000000-0000-0000-0000-000000000001"),
    *uuids::uuid::from_string("00000000-0000-0000-0000-000000000002")
};

std::optional<GE::EntityID> decodeToken(uint32_t low, uint32_t high)
{
    const std::array words = {low, high};
    return GE_Editor::decodeViewportPick(std::as_bytes(std::span(words)), {1, 1}, {0, 0}, entities);
}
}

TEST(ViewportPickingTest, backgroundDoesNotSelectAnEntity)
{
    EXPECT_EQ(decodeToken(0, 0), std::nullopt);
}

TEST(ViewportPickingTest, oldSkyboxSentinelDoesNotCrashOrSelectAnEntity)
{
    constexpr auto allBits = std::numeric_limits<uint32_t>::max();
    EXPECT_EQ(decodeToken(allBits, allBits), std::nullopt);
}

TEST(ViewportPickingTest, resolvesFirstAndLastEntityTokens)
{
    EXPECT_EQ(decodeToken(1, 0), entities.front());
    EXPECT_EQ(decodeToken(2, 0), entities.back());
}

TEST(ViewportPickingTest, rejectsTokensOutsideTheCapturedEntityTable)
{
    EXPECT_EQ(decodeToken(3, 0), std::nullopt);
    EXPECT_EQ(decodeToken(1, 1), std::nullopt); // Do not truncate to the low 32-bit word.
}

TEST(ViewportPickingTest, decodesTheRequestedRowAndColumn)
{
    const std::array<uint32_t, 12> words = {0,0, 0,0, 1,0, 0,0, 0,0, 2,0};
    const auto bytes = std::as_bytes(std::span(words));
    EXPECT_EQ(GE_Editor::decodeViewportPick(bytes, {2, 3}, {0, 1}, entities), entities.front());
    EXPECT_EQ(GE_Editor::decodeViewportPick(bytes, {2, 3}, {1, 2}, entities), entities.back());
}

TEST(ViewportPickingTest, rejectsEmptyViewportsAndOutOfBoundsCoordinates)
{
    const std::array<uint32_t, 2> words = {1, 0};
    const auto bytes = std::as_bytes(std::span(words));
    EXPECT_EQ(GE_Editor::decodeViewportPick(bytes, {0, 1}, {0, 0}, entities), std::nullopt);
    EXPECT_EQ(GE_Editor::decodeViewportPick(bytes, {1, 0}, {0, 0}, entities), std::nullopt);
    EXPECT_EQ(GE_Editor::decodeViewportPick(bytes, {1, 1}, {1, 0}, entities), std::nullopt);
    EXPECT_EQ(GE_Editor::decodeViewportPick(bytes, {1, 1}, {0, 1}, entities), std::nullopt);
}

TEST(ViewportPickingTest, rejectsIncompleteReadbacksWithoutReadingPastTheBuffer)
{
    const std::array<std::byte, 7> partialPixel{};
    EXPECT_EQ(GE_Editor::decodeViewportPick(partialPixel, {1, 1}, {0, 0}, entities), std::nullopt);
    const std::array<uint32_t, 6> truncatedRows = {1,0, 2,0, 1,0};
    EXPECT_EQ(GE_Editor::decodeViewportPick(std::as_bytes(std::span(truncatedRows)), {2, 2}, {1, 1}, entities), std::nullopt);
    constexpr auto huge = std::numeric_limits<uint32_t>::max();
    EXPECT_EQ(GE_Editor::decodeViewportPick(std::as_bytes(std::span(truncatedRows)), {huge, huge}, {huge-1, huge-1}, entities), std::nullopt);
}

TEST(ViewportPickingTest, rejectsEntityTokensWhenTheCapturedSceneIsEmpty)
{
    const std::array<uint32_t, 2> words = {1, 0};
    EXPECT_EQ(GE_Editor::decodeViewportPick(std::as_bytes(std::span(words)), {1, 1}, {0, 0}, {}), std::nullopt);
}
