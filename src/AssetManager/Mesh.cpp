/*
 * ---------------------------------------------------
 * Mesh.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/08/13 12:08:02
 * ---------------------------------------------------
 */

#include "Mesh.hpp"
#include "Game-Engine/Asset.hpp"
#include <utility>

namespace GE
{

MeshAsset::MeshAsset(const MeshAsset& cp)
    : m_ptr(cp.m_ptr)
{
}

MeshAsset::MeshAsset(MeshAsset&& mv)
    : m_ptr(std::move(mv.m_ptr))
{
}

MeshAsset::MeshAsset(const utils::SharedPtr<Mesh>& ptr)
    : m_ptr(ptr)
{
}

MeshAsset::~MeshAsset()
{
}

}