#include "imgui_render.hpp"

#include <cstddef>
#include <imgui.h>

namespace
{

struct AssetDebugRow
{
    GE::AssetID assetId;
    std::string_view typeName;
    std::string_view assetName;
    uint32_t loadCount;
    bool isLoaded;
    std::size_t index;
};

std::string_view assetTypeName(const GE::AssetManager& assetManager, GE::AssetID assetId)
{
    std::string_view typeName = "Unknown";
    const bool foundType = GE::anyOfType<GE::ManagableAssetTypes>([&]<typename T>() -> bool {
        if (!assetManager.assetTypeIs<T>(assetId))
            return false;
        typeName = GE::ManagableAssetTraits<T>::name;
        return true;
    });
    assert(foundType);
    return typeName;
}

} // namespace

namespace GE_Editor
{

void renderAssetManagerDebugWindow(GE::AssetManager& manager)
{
    auto assetIds = manager.assetIds();
    auto containerPaths = manager.assetContainerPaths();

    std::map<std::filesystem::path, std::vector<AssetDebugRow>> assetsByContainer;
    std::vector<AssetDebugRow> builtInAssets;
    uint32_t loadedAssetCount = 0;
    for (const GE::AssetID assetId : assetIds)
    {
        const uint32_t loadCount = manager.assetLoadCount(assetId);
        const bool isLoaded = manager.isAssetLoaded(assetId);
        const auto assetLocation = manager.assetLocation(assetId);
        if (isLoaded)
            loadedAssetCount++;

        AssetDebugRow row{
            .assetId = assetId,
            .typeName = assetTypeName(manager, assetId),
            .assetName = manager.assetName(assetId),
            .loadCount = loadCount,
            .isLoaded = isLoaded,
            .index = assetLocation ? std::visit([](const auto& typedLocation) { return typedLocation.index; }, *assetLocation) : 0
        };

        if (assetLocation)
        {
            std::visit([&](const auto& typedLocation) {
                assetsByContainer[typedLocation.containerPath].push_back(row);
            },
                       *assetLocation);
        }
        else
            builtInAssets.push_back(row);
    }

    uint32_t loadedContainerCount = 0;
    for (const auto& path : containerPaths)
    {
        if (manager.isAssetContainerLoaded(path))
            loadedContainerCount++;
    }

    if (ImGui::BeginTable("asset_manager_summary", 3, ImGuiTableFlags_SizingStretchSame))
    {
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("Assets\n%zu total / %u loaded", assetIds.size(), loadedAssetCount); // NOLINT(cppcoreguidelines-pro-type-vararg)
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("Containers\n%zu total / %u live", containerPaths.size(), loadedContainerCount); // NOLINT(cppcoreguidelines-pro-type-vararg)
        ImGui::TableSetColumnIndex(2);
        ImGui::Text("Built-in\n%zu", builtInAssets.size()); // NOLINT(cppcoreguidelines-pro-type-vararg)
        ImGui::EndTable();
    }

    ImGui::Separator();

    auto drawAssetTable = [&](const char* tableId, const std::ranges::range auto& assets) {
        if (!ImGui::BeginTable(tableId, 5, ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable | ImGuiTableFlags_SizingStretchProp))
            return;
        ImGui::TableSetupColumn("ID", ImGuiTableColumnFlags_WidthFixed, 55.0f);
        ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed, 80.0f);
        ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch, 0.35f);
        ImGui::TableSetupColumn("Index", ImGuiTableColumnFlags_WidthFixed, 55.0f);
        ImGui::TableSetupColumn("State", ImGuiTableColumnFlags_WidthFixed, 100.0f);
        ImGui::TableHeadersRow();

        for (const AssetDebugRow& asset : assets)
        {
            const std::string stateText = asset.isLoaded ? std::format("loaded({})", asset.loadCount) : std::format("not loaded({})", asset.loadCount);

            ImGui::PushID(static_cast<int>(asset.assetId));
            ImGui::TableNextRow();

            ImGui::TableSetColumnIndex(0);
            ImGui::Text("%llu", static_cast<unsigned long long>(asset.assetId)); // NOLINT(cppcoreguidelines-pro-type-vararg)

            ImGui::TableSetColumnIndex(1);
            ImGui::TextUnformatted(asset.typeName.data()); // NOLINT(bugprone-suspicious-stringview-data-usage)

            ImGui::TableSetColumnIndex(2);
            ImGui::TextUnformatted(asset.assetName.data()); // NOLINT(bugprone-suspicious-stringview-data-usage)

            ImGui::TableSetColumnIndex(3);
            ImGui::Text("%zu", asset.index); // NOLINT(cppcoreguidelines-pro-type-vararg)

            ImGui::TableSetColumnIndex(4);
            ImGui::TextUnformatted(stateText.c_str());

            if (ImGui::BeginPopupContextItem("asset_context_menu"))
            {
                if (ImGui::MenuItem("Load"))
                    manager.loadAsset(asset.assetId);

                ImGui::BeginDisabled(asset.loadCount == 0);
                if (ImGui::MenuItem("Unload"))
                    manager.unloadAsset(asset.assetId);
                ImGui::EndDisabled();

                ImGui::EndPopup();
            }

            ImGui::PopID();
        }

        ImGui::EndTable();
    };

    if (!builtInAssets.empty() && ImGui::CollapsingHeader("Built-in assets", ImGuiTreeNodeFlags_DefaultOpen))
    {
        drawAssetTable("asset_manager_builtin_table", builtInAssets);
    }

    for (const auto& path : containerPaths)
    {
        const char* statusLabel = manager.isAssetContainerLoaded(path) ? "live" : "idle";

        const auto containerAssetsIt = assetsByContainer.find(path);
        const std::size_t assetCount = containerAssetsIt == assetsByContainer.end() ? 0 : containerAssetsIt->second.size();

        const std::string header = std::format("{}  [{}]  ({} assets)", path.string(), statusLabel, assetCount);
        if (!ImGui::CollapsingHeader(header.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
            continue;

        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort))
            ImGui::SetTooltip("%s", path.string().c_str()); // NOLINT(cppcoreguidelines-pro-type-vararg)

        if (containerAssetsIt == assetsByContainer.end() || containerAssetsIt->second.empty())
            ImGui::TextDisabled("No registered assets in this container"); // NOLINT(cppcoreguidelines-pro-type-vararg)
        else
            drawAssetTable(std::format("asset_manager_container_table##{}", path.string()).c_str(), containerAssetsIt->second);
    }
}

} // namespace GE_Editor
