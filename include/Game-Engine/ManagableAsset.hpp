/*
 * ---------------------------------------------------
 * ManagableAsset.hpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/08/14 13:06:07
 * ---------------------------------------------------
 */

#ifndef MANAGABLEASSET_HPP
#define MANAGABLEASSET_HPP

#include "Game-Engine/TypeList.hpp"
#include "Game-Engine/Mesh.hpp"

#include <Graphics/Texture.hpp>

namespace GE
{

using ManagableAssetTypes = TypeList<Mesh, gfx::Texture>;

template<typename T>
concept ManagableAsset = IsTypeInList<std::remove_cvref_t<T>, ManagableAssetTypes>;

} // namespace GE

#endif // MANAGABLEASSET_HPP
