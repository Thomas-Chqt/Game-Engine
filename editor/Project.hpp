/*
 * ---------------------------------------------------
 * Project.hpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * ---------------------------------------------------
 */

#ifndef PROJECT_HPP
#define PROJECT_HPP

#include "EditorCamera.hpp"
#include "Game-Engine/Input.hpp"

#include <Game-Engine/AssetLocation.hpp>
#include <Game-Engine/AssetManager.hpp>
#include <Game-Engine/Entity.hpp>
#include <Game-Engine/Game.hpp>
#include <Game-Engine/InputContext.hpp>
#include <Game-Engine/Scene.hpp>

#include <filesystem>
#include <optional>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

namespace GE_Editor
{

struct Project
{
    std::string name;

    std::optional<std::filesystem::path> resourceDir;
    std::optional<std::filesystem::path> scriptLibPath;

    std::vector<std::tuple<std::string, GE::VAssetLocation, GE::AssetID, std::vector<GE::AssetID>>> registeredAssets;

    std::vector<std::pair<std::string, GE::VInput>> inputs;

    std::vector<GE::Scene::Descriptor> scenes;
    std::string startSceneName;

    EditorCamera editorCamera;
    std::optional<std::string> editedSceneName;
    std::optional<GE::EntityID> selectedEntityId;

    std::string imguiSettings;
};

Project makeDefaultProject();

}

#endif // PROJECT_HPP
