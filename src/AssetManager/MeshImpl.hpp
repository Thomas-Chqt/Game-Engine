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
#include "Math/Matrix.hpp"
#include "Renderer/GPURessourceManager.hpp"
#include "UtilsCPP/Array.hpp"
#include "UtilsCPP/String.hpp"

namespace GE
{
struct SubMeshImpl
{
    utils::String name;
    math::mat4x4 transform = math::mat4x4(1.0F);
    GPURessource<gfx::Buffer> vertexBuffer;
    GPURessource<gfx::Buffer> indexBuffer;
    GPURessource<gfx::Buffer> modelMatrixBuffer;
};

struct MeshImpl : public Mesh
{
    utils::Array<SubMeshImpl> subMeshes;

    inline void addSubMesh(const SubMeshImpl& submesh)
    {
        MeshImpl::subMeshes.append(submesh);
        Mesh::subMeshes.clear();
        for (auto& subMesh : MeshImpl::subMeshes)
            Mesh::subMeshes.append(SubMesh{ subMesh.name, subMesh.transform });
    }

    MeshImpl() = default;
    ~MeshImpl() override = default;

public:
    MeshImpl(const MeshImpl&)              = delete;
    MeshImpl(MeshImpl&&)                   = delete;
    MeshImpl& operator = (const MeshImpl&) = delete;
    MeshImpl& operator = (MeshImpl&&)      = delete;
};

}

#endif // MESHIMPL_HPP