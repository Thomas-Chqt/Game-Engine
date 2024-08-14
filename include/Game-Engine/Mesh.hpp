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

#include "Math/Matrix.hpp"
#include "UtilsCPP/Array.hpp"
#include "UtilsCPP/String.hpp"
#include "UtilsCPP/Types.hpp"

namespace GE
{


struct SubMesh
{
    utils::String& name;
    math::mat4x4& transform;
};

struct Mesh
{
    utils::String name;
    utils::Array<SubMesh> subMeshes;

    virtual ~Mesh() = default;

protected:
    Mesh() = default;

public:
    Mesh(const Mesh&)              = delete;
    Mesh(Mesh&&)                   = delete;
    Mesh& operator = (const Mesh&) = delete;
    Mesh& operator = (Mesh&&)      = delete;
};

}

#endif // MESH_HPP