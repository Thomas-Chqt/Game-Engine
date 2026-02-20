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

#include <Graphics/Buffer.hpp>

#include <glm/glm.hpp>

#include <string>
#include <memory>
#include <vector>

namespace GE
{

struct Vertex
{
    glm::vec3 pos;
    glm::vec2 uv;
    glm::vec3 normal;
    glm::vec3 tangent;
};

struct SubMesh
{
    std::string name;
    glm::mat4x4 transform;
    std::shared_ptr<gfx::Buffer> vertexBuffer;
    std::shared_ptr<gfx::Buffer> indexBuffer;
    // std::shared_ptr<Material> material;
    std::vector<SubMesh> subMeshes;
};

struct Mesh
{
    std::string name;
    glm::vec3 bBoxMin;
    glm::vec3 bBoxMax;
    std::vector<SubMesh> subMeshes;
};

}

#endif // MESH_HPP
