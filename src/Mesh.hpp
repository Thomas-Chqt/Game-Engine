/*
 * ---------------------------------------------------
 * Mesh.hpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/08/14 13:06:07
 * ---------------------------------------------------
 */

#ifndef MESH_HPP
#define MESH_HPP

#include "Graphics/Buffer.hpp"
#include "Math/Matrix.hpp"
#include "UtilsCPP/Array.hpp"
#include "UtilsCPP/SharedPtr.hpp"
#include "UtilsCPP/String.hpp"

namespace GE
{


struct SubMesh
{
    const utils::String name;
    const math::mat4x4 transform = math::mat4x4(1.0F);
    const utils::SharedPtr<gfx::Buffer> vertexBuffer;
    const utils::SharedPtr<gfx::Buffer> indexBuffer;
};

struct Mesh
{
    const utils::String name;
    const utils::Array<SubMesh> subMeshes;
};

}

#endif // MESH_HPP