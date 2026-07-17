#include "ImGuiRenderPass.hpp"

#include <Game-Engine/FrameGraph.hpp>

#include <imgui.h>
#include <gfx_imgui/gfx_imgui.hpp>

namespace GE_Editor
{

void ImGuiRenderPass::record(GE::FrameGraphBuilder& builder) const
{
    ZoneScopedN("ImGuiPass::record");

    const GE::FrameGraph::TextureRef targetTexture = builder.texture("windowBackBuffer");
    std::map<std::string, GE::FrameGraph::TextureRef, std::less<>> textureRefs;
    std::vector<GE::FrameGraph::TextureRef> sampledTextures;

    ImDrawData* drawData = ImGui::GetDrawData();
    assert(drawData);
    for (int i = 0; i < drawData->CmdListsCount; ++i) {
        for (ImDrawCmd& cmd : drawData->CmdLists[i]->CmdBuffer) {
            if (cmd.TexRef._TexID == 0)
                continue;
            assert(cmd.TexRef._TexData == nullptr);
            auto visitor = [&](auto& v) {
                if constexpr (std::is_same_v<std::remove_cvref_t<decltype(v)>, std::string>) {
                    GE::FrameGraph::TextureRef textureRef = builder.texture(v);
                    textureRefs.try_emplace(v, textureRef);
                    if (std::ranges::find(sampledTextures, textureRef) == sampledTextures.end())
                        sampledTextures.push_back(textureRef);
                }
                else if constexpr (std::is_same_v<std::remove_cvref_t<decltype(v)>, uint64_t>) {
                }
                else
                    std::unreachable();
            };
            std::visit(std::move(visitor), *std::bit_cast<std::variant<std::string, uint64_t>*>(cmd.TexRef._TexID));
        }
    }

    GE::FramePass imguiPass = {
        .colorAttachments = {
            GE::FrameGraph::Attachment{
                .texture = targetTexture,
                .loadAction = gfx::LoadAction::clear,
                .clearValue = gfx::ClearValue::color({ 0.0f, 0.0f, 0.0f, 1.0f })
            }
        },
        .sampledTextures = std::move(sampledTextures),
        .execute = [textureRefs=std::move(textureRefs)](GE::FramePass::ExecuteContext& ctx){
            ZoneScopedN("ImguiPass::execute");

            ImDrawData* drawData = ImGui::GetDrawData();
            for (int i = 0; i < drawData->CmdListsCount; ++i) {
                for (ImDrawCmd& cmd : drawData->CmdLists[i]->CmdBuffer) {
                    if (cmd.TexRef._TexID == 0)
                        continue;
                    assert(cmd.TexRef._TexData == nullptr);
                    auto vistor = [&](auto& v) {
                        if constexpr (std::is_same_v<std::remove_cvref_t<decltype(v)>, std::string>) {
                            auto it = textureRefs.find(v);
                            assert(it != textureRefs.end());
                            std::shared_ptr<gfx::Texture> texture = ctx.texture(it->second);
                            if (gfx::imgui::textureId(*texture).has_value() == false)
                                gfx::imgui::initTextureId(*texture);
                            cmd.TexRef._TexID = *gfx::imgui::textureId(*texture);
                        }
                        else if constexpr (std::is_same_v<std::remove_cvref_t<decltype(v)>, uint64_t>)
                            cmd.TexRef._TexID = v;
                        else
                            std::unreachable();
                    };
                    std::visit(std::move(vistor), *std::bit_cast<std::variant<std::string, uint64_t>*>(cmd.TexRef._TexID));
                }
            }
            gfx::imgui::renderDrawData(ctx.commandBuffer, drawData);
        }
    };

    builder.addPass(std::move(imguiPass));
}

}
