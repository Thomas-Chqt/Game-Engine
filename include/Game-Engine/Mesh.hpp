/*
 * ---------------------------------------------------
 * Mesh.hpp
 *
 * Author: Thomas Choquet <thomas.publique@icloud.com>
 * Date: 2024/07/06 17:50:10
 * ---------------------------------------------------
 */

#ifndef MESH_HPP
# define MESH_HPP

#include "Graphics/Buffer.hpp"
#include "Graphics/BufferInstance.hpp"
#include "Math/Matrix.hpp"
#include "UtilsCPP/Array.hpp"
#include "UtilsCPP/String.hpp"
#include "UtilsCPP/SharedPtr.hpp"

struct SubMesh
{
    utils::String name;
    math::mat4x4 transform = math::mat4x4(1.0F);

    utils::SharedPtr<gfx::Buffer> vertexBuffer;
    utils::SharedPtr<gfx::Buffer> indexBuffer;
    gfx::BufferInstance<math::mat4x4> modelMatrix;

    utils::Array<SubMesh> childs;
};

struct Mesh
{
    utils::String name;
    utils::Array<SubMesh> subMeshes;
};

#endif // MESH_HPP
