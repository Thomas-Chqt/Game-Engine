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

class ImguiPassBuilder
{
public:
    inline constexpr FramePass build() const
    {
        GE::FramePass framePass;

        framePass.colorAttachments = { GE::ColorAttachmentUsage{
            .name = m_colorAttachmentName,
            .loadAction = m_colorAttachmentLoadAction,
            .clearColor = { 0.0f, 0.0f, 0.0f, 0.0f },
        }};

        if (m_depthAttachmentName.has_value())
        {
            framePass.depthAttachment = GE::DepthAttachmentUsage{
                .name = *m_depthAttachmentName,
                .loadAction = gfx::LoadAction::clear,
                .clearDepth = 1.0f,
            };
        }

        framePass.sampledAttachments = m_sampledAttachmentNames;

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

    inline constexpr ImguiPassBuilder& setColorAttachment(const std::string& name, gfx::LoadAction lAc = gfx::LoadAction::load)
    {
        m_colorAttachmentName = name;
        m_colorAttachmentLoadAction = lAc;
        return *this;
    }

    inline constexpr ImguiPassBuilder& setDepthAttachment(const std::string& name)
    {
        m_depthAttachmentName = name;
        return *this;
    }

    inline constexpr ImguiPassBuilder& addSampledAttachment(const std::string& name)
    {
        m_sampledAttachmentNames.push_back(name);
        return *this;
    }

private:
    std::string m_colorAttachmentName = "backBuffer";
    gfx::LoadAction m_colorAttachmentLoadAction = gfx::LoadAction::load;

    std::optional<std::string> m_depthAttachmentName;
    std::vector<std::string> m_sampledAttachmentNames;
};

class ClearPassBuilder
{
public:
    inline constexpr FramePass build() const
    {
        GE::FramePass framePass;

        framePass.colorAttachments = { GE::ColorAttachmentUsage{
            .name = m_colorAttachmentName,
            .loadAction = gfx::LoadAction::clear,
            .clearColor = { m_clearColor.x, m_clearColor.y, m_clearColor.z, 1.0f },
        }};

        if (m_depthAttachmentName.has_value())
        {
            framePass.depthAttachment = GE::DepthAttachmentUsage{
                .name = *m_depthAttachmentName,
                .loadAction = gfx::LoadAction::clear,
                .clearDepth = m_clearDepth,
            };
        }

        framePass.execute = [](auto) {};

        return framePass;
    }

    inline constexpr ClearPassBuilder& setColorAttachment(const std::string& name)
    {
        m_colorAttachmentName = name;
        return *this;
    }

    inline constexpr ClearPassBuilder& setDepthAttachment(const std::string& name)
    {
        m_depthAttachmentName = name;
        return *this;
    }

    inline constexpr ClearPassBuilder& setClearColor(const glm::vec3& color)
    {
        m_clearColor = color;
        return *this;
    }

    inline constexpr ClearPassBuilder& setClearDepth(float value)
    {
        m_clearDepth = value;
        return *this;
    }

private:
    std::string m_colorAttachmentName = "backBuffer";
    glm::vec3 m_clearColor = glm::vec3(0.0, 0.0, 0.0);

    std::optional<std::string> m_depthAttachmentName = "depthBuffer";
    float m_clearDepth = 1.0f;
};

class FlatGeometryPassBuilder
{
public:
    inline FramePass build() const;

    inline constexpr FlatGeometryPassBuilder& setColorAttachment(const std::string& name)
    {
        m_colorAttachmentName = name;
        return *this;
    }

    inline constexpr FlatGeometryPassBuilder& setDepthAttachment(const std::string& name)
    {
        m_depthAttachmentName = name;
        return *this;
    }

    inline constexpr FlatGeometryPassBuilder& setClearColor(const glm::vec3& color)
    {
        m_clearColor = color;
        return *this;
    }

    inline constexpr FlatGeometryPassBuilder& setClearDepth(float value)
    {
        m_clearDepth = value;
        return *this;
    }

    inline constexpr FlatGeometryPassBuilder& setScene(const Scene* scene)
    {
        m_scene = scene;
        return *this;
    }

private:
    std::string m_colorAttachmentName = "backBuffer";
    glm::vec3 m_clearColor = glm::vec3(0.0, 0.0, 0.0);

    std::optional<std::string> m_depthAttachmentName = "depthBuffer";
    float m_clearDepth = 1.0f;

    const Scene* m_scene = nullptr;
};

}

#endif
