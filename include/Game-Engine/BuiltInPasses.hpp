/*
 * ---------------------------------------------------
 * BuiltInPasses.hpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * ---------------------------------------------------
 */

#ifndef BUILTINPASSES_HPP
#define BUILTINPASSES_HPP

#include "Game-Engine/Export.hpp"
#include "Game-Engine/FrameGraph.hpp"
#include "Game-Engine/Scene.hpp"

#include <glm/glm.hpp>

#include <vector>

namespace GE
{

class GE_API TexturedGeometryPass
{
public:
    // Optional output maps nonzero picking tokens to UUIDs at token - 1.
    TexturedGeometryPass(const Scene&, glm::mat4 viewProjectionMatrix, glm::vec3 cameraPosition, std::vector<EntityID>* pickingEntities = nullptr);

    void record(FrameGraphBuilder&) const;

private:
    const Scene* m_scene;
    std::vector<EntityID>* m_pickingEntities;
    glm::mat4 m_viewProjectionMatrix;
    glm::vec3 m_cameraPosition;
};

} // namespace GE

#endif
