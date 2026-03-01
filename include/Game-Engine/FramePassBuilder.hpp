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
#include "Graphics/Enums.hpp"

#include <imgui.h>
#include <glm/glm.hpp>

#include <optional>
#include <string>
#include <utility>

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

        framePass.execute = [](gfx::CommandBuffer& cmdBuffer, gfx::ParameterBlockPool&) {
            cmdBuffer.imGuiRenderDrawData(ImGui::GetDrawData());
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

private:
    std::pair<uint32_t, uint32_t> m_renderSize;

    std::string m_colorAttachmentName = "backBuffer";
    gfx::PixelFormat m_colorAttachmentPixelFmt = gfx::PixelFormat::BGRA8Unorm;
    gfx::LoadAction m_colorAttachmentLoadAction = gfx::LoadAction::load;

    std::optional<std::string> m_depthAttachmentName;
    std::optional<gfx::PixelFormat> m_depthAttachmentPixelFmt;
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

        framePass.execute = [](gfx::CommandBuffer& cmdBuffer, gfx::ParameterBlockPool&) {};

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
