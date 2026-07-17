#pragma once

#include <Game-Engine/FrameGraph.hpp>

namespace GE_Editor
{

class ImGuiRenderPass
{
public:
    void record(GE::FrameGraphBuilder&) const;
};

}
