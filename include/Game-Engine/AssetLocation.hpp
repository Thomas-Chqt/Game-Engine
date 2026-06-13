/*
 * ---------------------------------------------------
 * AssetLocation.hpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * ---------------------------------------------------
 */

#ifndef ASSETLOCATION_HPP
#define ASSETLOCATION_HPP

#include "Game-Engine/ManagableAsset.hpp"
#include "Game-Engine/TypeList.hpp"

#include <cstddef>
#include <filesystem>
#include <variant>

namespace GE
{

template<ManagableAsset T>
struct AssetLocation
{
    using AssetType = T;
    std::filesystem::path containerPath;
    std::size_t index = 0;

    auto operator<=>(const AssetLocation&) const = default;
};

using AssetLocationTypes = ManagableAssetTypes::wrapped<AssetLocation>;
using VAssetLocation = AssetLocationTypes::into<std::variant>;

} // namespace GE

#endif // ASSETLOCATION_HPP
