/*
 * ---------------------------------------------------
 * RessourceBrowserPanel.hpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * ---------------------------------------------------
 */

#ifndef RESSOURCEBROWSERPANEL_HPP
#define RESSOURCEBROWSERPANEL_HPP

#include "UI/Button.hpp"
#include "UI/Child.hpp"
#include "UI/Group.hpp"
#include "UI/Pane.hpp"
#include "UI/SameLine.hpp"
#include "UI/States.hpp"
#include "UI/Text.hpp"
#include "UI/TileGrid.hpp"
#include "imgui.h"

#include <cassert>
#include <filesystem>
#include <format>
#include <ranges>
#include <string>

constexpr float TILE_SIZE = 60.0f;

namespace GE_Editor::UI
{

class RessourceBrowserPanel
{
public:
    RessourceBrowserPanel(std::filesystem::path baseDir)
        : m_baseDir(std::move(baseDir))
    {
    }

    void render() const
    {
        assert(std::filesystem::exists(m_baseDir));

        if (std::filesystem::exists(m_baseDir / m_subDir) == false)
            m_subDir = "";

        Pane("Ressources",
            SameLine(
                Button("<")
                    .disabled(m_subDir.empty())
                    .onClick([&](){ m_subDir = m_subDir.parent_path(); }),
                Text(std::filesystem::path(m_baseDir.filename() / m_subDir).string())
            ),
            Child("RessourcesChild",
                TileGrid(std::filesystem::directory_iterator(m_baseDir / m_subDir) | std::views::transform([&](const std::filesystem::directory_entry& entry) {
                    return Group(
                        (entry.is_directory() ? Button(std::format("DIR##{}", entry.path().string()))
                                                    .size(TILE_SIZE, TILE_SIZE)
                                                    .onDoubleClick([&](){ m_subDir /= entry.path().filename(); })
                                              : Button(std::format("FILE##{}", entry.path().string()))
                                                    .size(TILE_SIZE, TILE_SIZE)
                                                    .dragDropSource([path=entry.path().string(), filename=entry.path().filename().string()](){
                                                        ImGui::SetDragDropPayload("resource_dnd", path.c_str(), path.size());
                                                        ImGui::Text("%s", filename.c_str());
                                                    })),
                        Text(entry.path().filename().string()).size(TILE_SIZE, ImGui::GetTextLineHeightWithSpacing())
                    );
                }))
            )
        )
        .render();
    }

private:
    std::filesystem::path m_baseDir;
    std::filesystem::path& m_subDir = *UI::States::fileBrowserSubDirectory;
};

}

#endif // RESSOURCEBROWSERPANEL_HPP
