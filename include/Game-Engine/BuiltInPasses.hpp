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

namespace GE
{

class GE_API TexturedGeometryPass
{
public:
    TexturedGeometryPass(const Scene&, glm::mat4 viewProjectionMatrix, glm::vec3 cameraPosition);

    void record(FrameGraphBuilder&) const;

private:
    const Scene* m_scene;
    glm::mat4 m_viewProjectionMatrix;
    glm::vec3 m_cameraPosition;
};

class GE_API ImGuiPass
{
public:
    void record(FrameGraphBuilder&) const;
};

} // namespace GE

#endif
