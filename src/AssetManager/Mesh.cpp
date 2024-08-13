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

Asset<Mesh>::Asset(const Asset<Mesh>& cp)
    : m_ptr(cp.m_ptr)
{
}

Asset<Mesh>::Asset(Asset<Mesh>&& mv)
    : m_ptr(std::move(mv.m_ptr))
{
}

Asset<Mesh>::Asset(const utils::SharedPtr<Mesh>& ptr)
    : m_ptr(ptr)
{
}

Asset<Mesh>::~Asset()
{
}

}