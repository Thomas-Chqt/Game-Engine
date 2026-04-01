/*
 * ---------------------------------------------------
 * FramePassBuilder.hpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * ---------------------------------------------------
 */

#ifndef FRAMEPASSBUILDER_HPP
#define FRAMEPASSBUILDER_HPP

#include "Game-Engine/FrameGraph.hpp"
#include "Game-Engine/Scene.hpp"


#include <Graphics/Enums.hpp>
#include <Graphics/ParameterBlock.hpp>

#include <imgui.h>
#include <glm/glm.hpp>

#include <optional>
#include <string>
#include <utility>
#include <functional>
#include <variant>

namespace GE
{

class AssetManager;

template<typename Derived>
class FramePassBuilderBase
{
public:
    inline constexpr FramePass build() const
    {
        GE::FramePass framePass;
        framePass.colorAttachments = { m_colorAttachment };
        framePass.depthAttachment = m_depthAttachment;
        framePass.sampledTextures = m_sampledTextureNames;
        framePass.usedBuffers = m_usedBufferNames;
        return framePass;
    }

    inline constexpr Derived& setColorAttachment(const std::string& name, gfx::LoadAction loadAction = gfx::LoadAction::clear, std::array<float, 4> clearColor = {0.0f, 0.0f, 0.0f, 1.0f})
    {
        m_colorAttachment.texture = name;
        m_colorAttachment.loadAction = loadAction;
        m_colorAttachment.clearColor = clearColor;
        return static_cast<Derived&>(*this);
    }

    inline constexpr Derived& setDepthAttachment(const std::string& name, gfx::LoadAction loadAction = gfx::LoadAction::clear, float clearDepth = 1.0f)
    {
        m_depthAttachment.emplace(AttachmentDescriptor{.texture = name, .loadAction = loadAction, .clearDepth = clearDepth});
        return static_cast<Derived&>(*this);
    }

    inline constexpr Derived& addSampledTexture(const std::string& name)
    {
        m_sampledTextureNames.push_back(name);
        return static_cast<Derived&>(*this);
    }

    inline constexpr Derived& addUsedBuffer(const std::string& name)
    {
        m_usedBufferNames.push_back(name);
        return static_cast<Derived&>(*this);
    }

protected:
    AttachmentDescriptor m_colorAttachment = AttachmentDescriptor{.clearColor={0.0f, 0.0f, 0.0f, 1.0f}};
    std::optional<AttachmentDescriptor> m_depthAttachment;
    std::vector<std::string> m_sampledTextureNames;
    std::vector<std::string> m_usedBufferNames;
};

class ImguiPassBuilder : public FramePassBuilderBase<ImguiPassBuilder>
{
public:
    inline constexpr FramePass build() const
    {
        GE::FramePass framePass = FramePassBuilderBase<ImguiPassBuilder>::build();
        framePass.execute = [](FramePassExecuteContext& ctx) {
            ImDrawData* drawData = ImGui::GetDrawData();
            for (int i = 0; i < drawData->CmdListsCount; ++i) {
                for (ImDrawCmd& cmd : drawData->CmdLists[i]->CmdBuffer) {
                    if (cmd.TexRef._TexID == 0)
                        continue;
                    assert(cmd.TexRef._TexData == nullptr);
                    auto vistor = [&](auto& v) {
                        if constexpr (std::is_same_v<std::remove_cvref_t<decltype(v)>, std::string>) {
                            if (ctx.textureMap[v]->imTextureId().has_value() == false)
                                ctx.textureMap[v]->initImTextureId();
                            cmd.TexRef._TexID = *ctx.textureMap[v]->imTextureId();
                        }
                        else if constexpr (std::is_same_v<std::remove_cvref_t<decltype(v)>, uint64_t>)
                            cmd.TexRef._TexID = v;
                        else
                            std::unreachable();
                    };
                    std::visit(std::move(vistor), *std::bit_cast<std::variant<std::string, uint64_t>*>(cmd.TexRef._TexID));
                }
            }
            ctx.commandBuffer.imGuiRenderDrawData(drawData);
        };
        return framePass;
    }
};

class ClearPassBuilder : public FramePassBuilderBase<ClearPassBuilder>
{
public:
    inline constexpr FramePass build() const
    {
        GE::FramePass framePass = FramePassBuilderBase<ClearPassBuilder>::build();
        framePass.execute = [](FramePassExecuteContext&) {};
        return framePass;
    }
};

class FlatGeometryPassBuilder : public FramePassBuilderBase<FlatGeometryPassBuilder>
{
public:
    FlatGeometryPassBuilder(const Scene* scene);

    FramePass build() const;

private:
    const Scene* m_scene;
};

}

#endif
