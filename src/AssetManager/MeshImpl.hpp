/*
 * ---------------------------------------------------
 * MeshImpl.hpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/08/14 13:10:18
 * ---------------------------------------------------
 */

#ifndef MESHIMPL_HPP
#define MESHIMPL_HPP

#include "Game-Engine/Mesh.hpp"
#include "Graphics/Buffer.hpp"
#include "Renderer/GPURessourceManager.hpp"
#include "UtilsCPP/Array.hpp"

namespace GE
{

struct MeshImpl : public Mesh
{
    utils::Array<GPURessource<gfx::Buffer>> buffers;  

    ~MeshImpl() override = default;
};

}

#endif // MESHIMPL_HPP