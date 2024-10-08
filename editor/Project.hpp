/*
 * ---------------------------------------------------
 * Project.hpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/09/10 11:16:48
 * ---------------------------------------------------
 */

#ifndef PROJECT_HPP
#define PROJECT_HPP

#include "EditorCamera.hpp"
#include "InputManager/InputContext.hpp"
#include "Scene.hpp"
#include "UtilsCPP/Dictionary.hpp"
#include "UtilsCPP/Set.hpp"
#include "UtilsCPP/String.hpp"
#include <nlohmann/json.hpp>

namespace GE
{

struct Project
{
    utils::String name;
    utils::String ressourcesDir;
    Scene* startScene = nullptr;
    InputContext editorInputContext;
    utils::String imguiSettings;

    utils::Set<Scene> scenes;
    utils::Dictionary<utils::String, EditorCamera> editorCameras;
    
    Scene* editedScene = nullptr;

    Entity selectedEntity;
};

void to_json(nlohmann::json&, const Project&);
void from_json(const nlohmann::json&, Project&);

}

#endif // PROJECT_HPP