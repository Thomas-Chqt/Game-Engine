/*
 * ---------------------------------------------------
 * Mesh.hpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/08/13 12:01:02
 * ---------------------------------------------------
 */

#ifndef MESH_HPP
#define MESH_HPP

#include "Graphics/Buffer.hpp"
#include "Math/Matrix.hpp"
#include "Renderer/GPURessourceManager.hpp"

namespace GE
{

struct SubMesh
{
    utils::String name;
    math::mat4x4 transform = math::mat4x4(1.0F);
    GPURessource<gfx::Buffer> modelMatrix;

    GPURessource<gfx::Buffer> vertexBuffer;
    GPURessource<gfx::Buffer> indexBuffer;

    utils::Array<SubMesh> childs;
};

struct Mesh
{
    utils::String name;
    utils::Array<SubMesh> subMeshes;
};

}

#endif // MESH_HPP