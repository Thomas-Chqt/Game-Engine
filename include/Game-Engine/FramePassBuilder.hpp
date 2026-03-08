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

#include <Graphics/Enums.hpp>

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

        auto colorAttachmentDesc = GE::AttachmentDescriptor{
            .name = m_colorAttachmentName,
            .size = m_renderSize,
            .pixelFormat = m_colorAttachmentPixelFmt,
            .loadAction = m_colorAttachmentLoadAction,
            .clearColor = { 0.0f, 0.0f, 0.0f, 0.0f },
        };
        framePass.colorAttachments = { colorAttachmentDesc };

        if (m_depthAttachmentName.has_value() && m_depthAttachmentPixelFmt.has_value())
        {
            auto depthAttachmentDesc = GE::AttachmentDescriptor{
                .name = *m_depthAttachmentName,
                .size = m_renderSize,
                .pixelFormat = *m_depthAttachmentPixelFmt,
                .loadAction = gfx::LoadAction::clear,
                .clearDepth = 1.0f,
            };
            framePass.depthAttachment = depthAttachmentDesc;
        }

        framePass.sampledAttachments = m_sampledAttachmentNames;

        framePass.execute = [](FramePassContext& ctx) {
            ImDrawData* drawData = ImGui::GetDrawData();
            for (int i = 0; i < drawData->CmdListsCount; ++i) {
                ImDrawList* list = drawData->CmdLists[i];
                for (ImDrawCmd& cmd : list->CmdBuffer) {
                    if (cmd.TexRef._TexID == 0)
                        continue;
                    assert(cmd.TexRef._TexData == nullptr);
                    auto vistor = [&](auto& v) {
                        if constexpr (std::is_same_v<std::remove_cvref_t<decltype(v)>, std::string>) {
                            if (ctx.textureMap[v]->imTextureId().has_value() == false)
                                ctx.textureMap[v]->initImTextureId();
                            cmd.TexRef._TexID = *ctx.textureMap[v]->imTextureId();
                        }
                        else if constexpr (std::is_same_v<std::remove_cvref_t<decltype(v)>, uint64_t>) {
                            cmd.TexRef._TexID = v;
                        }
                    };
                    std::visit(std::move(vistor), *std::bit_cast<std::variant<std::string, uint64_t>*>(cmd.TexRef._TexID));
                }
            }
            ctx.commandBuffer.imGuiRenderDrawData(drawData);
        };

        return framePass;
    }

    inline constexpr ImguiPassBuilder& setRenderSize(const std::pair<uint32_t, uint32_t>& s)
    {
        return m_renderSize = s, *this;
    }

    inline constexpr ImguiPassBuilder& setColorAttachment(const std::string& name, const gfx::PixelFormat& pxFmt = gfx::PixelFormat::BGRA8Unorm, gfx::LoadAction lAc = gfx::LoadAction::load)
    {
        m_colorAttachmentName = name;
        m_colorAttachmentPixelFmt = pxFmt;
        m_colorAttachmentLoadAction = lAc;
        return *this;
    }

    inline constexpr ImguiPassBuilder& setDepthAttachment(const std::string& name, const gfx::PixelFormat& pxFmt = gfx::PixelFormat::Depth32Float)
    {
        m_depthAttachmentName = name;
        m_depthAttachmentPixelFmt = pxFmt;
        return *this;
    }

    inline constexpr ImguiPassBuilder& addSampledAttachment(const std::string& name)
    {
        m_sampledAttachmentNames.push_back(name);
        return *this;
    }

private:
    std::pair<uint32_t, uint32_t> m_renderSize;

    std::string m_colorAttachmentName = "backBuffer";
    gfx::PixelFormat m_colorAttachmentPixelFmt = gfx::PixelFormat::BGRA8Unorm;
    gfx::LoadAction m_colorAttachmentLoadAction = gfx::LoadAction::load;

    std::optional<std::string> m_depthAttachmentName;
    std::optional<gfx::PixelFormat> m_depthAttachmentPixelFmt;
    std::vector<std::string> m_sampledAttachmentNames;
};

class ClearPassBuilder
{
public:
    inline constexpr FramePass build() const
    {
        GE::FramePass framePass;

        auto colorAttachmentDesc = GE::AttachmentDescriptor{
            .name = m_colorAttachmentName,
            .size = m_renderSize,
            .pixelFormat = m_colorAttachmentPixelFmt,
            .loadAction = gfx::LoadAction::clear,
            .clearColor = { m_clearColor.x, m_clearColor.y, m_clearColor.z, 1.0f },
        };
        framePass.colorAttachments = { colorAttachmentDesc };

        if (m_depthAttachmentName.has_value() && m_depthAttachmentPixelFmt.has_value())
        {
            auto depthAttachmentDesc = GE::AttachmentDescriptor{
                .name = *m_depthAttachmentName,
                .size = m_renderSize,
                .pixelFormat = *m_depthAttachmentPixelFmt,
                .loadAction = gfx::LoadAction::clear,
                .clearDepth = m_clearDepth,
            };
            framePass.depthAttachment = depthAttachmentDesc;
        }

        framePass.execute = [](auto) {};

        return framePass;
    }

    inline constexpr ClearPassBuilder& setRenderSize(const std::pair<uint32_t, uint32_t>& s)
    {
        m_renderSize = s;
        return *this;
    }

    inline constexpr ClearPassBuilder& setColorAttachment(const std::string& name, const gfx::PixelFormat& pxFmt = gfx::PixelFormat::BGRA8Unorm)
    {
        m_colorAttachmentName = name;
        m_colorAttachmentPixelFmt = pxFmt;
        return *this;
    }

    inline constexpr ClearPassBuilder& setDepthAttachment(const std::string& name, const gfx::PixelFormat& pxFmt = gfx::PixelFormat::Depth32Float)
    {
        m_depthAttachmentName = name;
        m_depthAttachmentPixelFmt = pxFmt;
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
    std::pair<uint32_t, uint32_t> m_renderSize;

    std::string m_colorAttachmentName = "backBuffer";
    gfx::PixelFormat m_colorAttachmentPixelFmt = gfx::PixelFormat::BGRA8Unorm;
    glm::vec3 m_clearColor = glm::vec3(0.0, 0.0, 0.0);

    std::optional<std::string> m_depthAttachmentName= "depthBuffer";
    std::optional<gfx::PixelFormat> m_depthAttachmentPixelFmt = gfx::PixelFormat::Depth32Float;
    float m_clearDepth = 1.0f;
};

}

#endif
