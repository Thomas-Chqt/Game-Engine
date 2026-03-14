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

template<typename Derived>
class FramePassBuilderBase
{
public:
    inline constexpr Derived& setColorAttachment(const std::string& name, gfx::LoadAction lAc = gfx::LoadAction::clear)
    {
        m_colorAttachmentName = name;
        m_colorAttachmentLoadAction = lAc;
        return static_cast<Derived&>(*this);
    }

    inline constexpr Derived& setClearColor(const glm::vec3& color)
    {
        m_clearColor = { color.x, color.y, color.z, 1.0f };
        return static_cast<Derived&>(*this);
    }

    inline constexpr Derived& setDepthAttachment(const std::string& name)
    {
        m_depthAttachmentName = name;
        return static_cast<Derived&>(*this);
    }

    inline constexpr Derived& setClearDepth(float value)
    {
        m_clearDepth = value;
        return static_cast<Derived&>(*this);
    }

    inline constexpr Derived& addSampledAttachment(const std::string& name)
    {
        m_sampledAttachmentNames.push_back(name);
        return static_cast<Derived&>(*this);
    }

protected:
    inline constexpr void buildAttachments(FramePass& framePass) const
    {
        framePass.colorAttachments = { AttachmentUsage{
            .name = m_colorAttachmentName,
            .loadAction = m_colorAttachmentLoadAction,
            .clearColor = m_clearColor,
        }};

        if (m_depthAttachmentName.has_value())
        {
            framePass.depthAttachment = AttachmentUsage{
                .name = *m_depthAttachmentName,
                .loadAction = gfx::LoadAction::clear,
                .clearDepth = m_clearDepth,
            };
        }

        framePass.sampledAttachments = m_sampledAttachmentNames;
    }

    std::string m_colorAttachmentName = "backBuffer";
    gfx::LoadAction m_colorAttachmentLoadAction = gfx::LoadAction::clear;
    std::array<float, 4> m_clearColor = {0.0f, 0.0f, 0.0f, 0.0f};

    std::optional<std::string> m_depthAttachmentName = "depthBuffer";
    float m_clearDepth = 1.0f;

    std::vector<std::string> m_sampledAttachmentNames;
};

class ImguiPassBuilder : public FramePassBuilderBase<ImguiPassBuilder>
{
public:
    inline constexpr FramePass build() const
    {
        GE::FramePass framePass;
        buildAttachments(framePass);

        framePass.execute = [](FramePassContext& ctx) {
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
        GE::FramePass framePass;
        buildAttachments(framePass);
        framePass.execute = [](auto) {};
        return framePass;
    }
};

class FlatGeometryPassBuilder : public FramePassBuilderBase<FlatGeometryPassBuilder>
{
public:
    inline FramePass build() const;

    inline constexpr FlatGeometryPassBuilder& setScene(const Scene* scene)
    {
        m_scene = scene;
        return *this;
    }

private:
    const Scene* m_scene = nullptr;
};

}

#endif
