/*
 * ---------------------------------------------------
 * Engine.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/06/20 17:15:45
 * ---------------------------------------------------
 */

#include "AssetManager/MeshImpl.hpp"
#include "ECS/ECSView.hpp"
#include "Game-Engine/AssetManager.hpp"
#include "Game-Engine/Components.hpp"
#include "Game-Engine/Engine.hpp"
#include "Game-Engine/Entity.hpp"
#include "Game-Engine/Game.hpp"
#include "Graphics/Event.hpp"
#include "Graphics/Platform.hpp"
#include "Math/Constants.hpp"
#include "Math/Matrix.hpp"
#include "Renderer/GPURessourceManager.hpp"
#include "Renderer/Renderer.hpp"
#include "Scene/InternalComponents.hpp"
#include "UtilsCPP/Func.hpp"
#include "UtilsCPP/String.hpp"
#include <cassert>
#include <cstring>
#include <utility>

namespace GE
{

static void scriptSystem(ECSView<ScriptComponent> view)
{
    view.onEach([](Entity, ScriptComponent& scriptComponent) {
        scriptComponent.instance->onUpdate();
    });
}

static Renderer::Camera getActiveCameraSystem(ECSView<TransformComponent, CameraComponent, ActiveCameraComponent> view)
{
    Renderer::Camera rendererCam = { math::mat4x4(1.0), math::mat4x4(1.0) };
    view.onFirst([&](Entity, TransformComponent& transform, CameraComponent& camera, ActiveCameraComponent&) {
        rendererCam.viewMatrix = ((math::mat4x4)transform).inversed();
        rendererCam.projectionMatrix = camera.projectionMatrix();
    });
    return rendererCam;
}

static void addLightsSystem(ECSView<TransformComponent, LightComponent> view)
{
    view.onEach([](Entity, TransformComponent& transform, LightComponent& light) {
        switch (light.type)
        {
        case LightComponent::Type::point:
            Renderer::shared().addPointLight({transform.position, light.color, light.intentsity});
            break;
        default:
            UNREACHABLE
        }
    });
}

static void addRenderableSystem(ECSView<TransformComponent, MeshComponent> view)
{
    view.onEach([](Entity entity, TransformComponent& transform, MeshComponent& meshComponent) {
        utils::SharedPtr<MeshImpl> mesh = meshComponent.mesh.forceDynamicCast<MeshImpl>();

        auto addSubMesh = [&](SubMeshImpl& subMesh, const math::mat4x4& transform) {
            math::mat4x4 modelMatrix = transform * subMesh.transform;
            *static_cast<math::mat4x4*>(subMesh.modelMatrixBuffer->mapContent()) = modelMatrix;
            subMesh.modelMatrixBuffer->unMapContent();

            Renderer::Renderable renderable;
            renderable.vertexBuffer = subMesh.vertexBuffer;
            renderable.indexBuffer = subMesh.indexBuffer;
            renderable.modelMatrix = subMesh.modelMatrixBuffer;

            Renderer::shared().addRenderable(renderable);
        };

        for (auto& subMesh : mesh->subMeshes)
            addSubMesh(subMesh, entity.worldTransform());
    });
}

void Engine::runGame(utils::UniquePtr<Game>&& game)
{
    m_runningGame = std::move(game);

    m_runningGame->onSetup();

    m_running = true;
    while (m_running)
    {
        gfx::Platform::shared().pollEvents();

        m_runningGame->onUpdate();

        scriptSystem(m_runningGame->activeScene());

        Renderer::Camera rendererCam = getActiveCameraSystem(m_runningGame->activeScene());

        Renderer::shared().beginScene(rendererCam);
        {
            addLightsSystem(m_runningGame->activeScene());
            addRenderableSystem(m_runningGame->activeScene());
        }
        Renderer::shared().endScene();

        Renderer::shared().render();
    }
}

Engine::~Engine()
{
    gfx::Platform::shared().clearCallbacks(this);

    AssetManager::terminate();
    GPURessourceManager::terminate();
    Renderer::terminate();
    gfx::Platform::terminate();
}

Engine::Engine()
{
    gfx::Platform::init();
    Renderer::init();
    GPURessourceManager::init();
    AssetManager::init();

    gfx::Platform::shared().addEventCallBack(utils::Func<void(gfx::Event&)>(*this, &Engine::onEvent), this);
    m_mainWindow = gfx::Platform::shared().newWindow(800, 600);

    Renderer::shared().setWindow(m_mainWindow);
    Renderer::shared().setOnImGuiRender(utils::Func<void()>(*this, &Engine::onImGuiRender));
}

void Engine::onEvent(gfx::Event& event)
{
    event.dispatch<gfx::KeyDownEvent>([&](gfx::KeyDownEvent& event) {
        if (event.isRepeat() == false)
            m_pressedKeys.insert(event.keyCode());
    });
    
    event.dispatch<gfx::KeyUpEvent>([&](gfx::KeyUpEvent& event) {
        m_pressedKeys.remove(m_pressedKeys.find(event.keyCode()));
    });

    assert(m_runningGame);
    m_runningGame->onEvent(event);
}

void Engine::onImGuiRender()
{
    if (ImGui::Begin("FPS"))
        ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
    ImGui::End();

    drawSceneGraphWindow();
    drawEntityInspectorWindow();

    assert(m_runningGame);
    m_runningGame->onImGuiRender();
}

void Engine::drawSceneGraphWindow()
{
    utils::Func<void(Entity)> sceneGraphEntityRow = [&](Entity entity) {
        bool node_open = false;
        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow |
                                   ImGuiTreeNodeFlags_OpenOnDoubleClick |
                                   ImGuiTreeNodeFlags_SpanAvailWidth |
                                   ImGuiTreeNodeFlags_DefaultOpen;

        if (m_selectedEntity == entity)
            flags |= ImGuiTreeNodeFlags_Selected;

        if (entity.has<HierarchicalComponent>() == true && entity.firstChild())
            node_open = ImGui::TreeNodeEx(entity.imGuiID(), flags, "%s", (const char*)entity.name());
        else
        {
            flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
            ImGui::TreeNodeEx(entity.imGuiID(), flags, "%s", (const char*)entity.name());
        }

        if (ImGui::IsItemClicked())
            m_selectedEntity = entity;

        if (node_open)
        {
            for (Entity curr = entity.firstChild(); curr; curr = curr.nextChild() )
                sceneGraphEntityRow(curr);
            ImGui::TreePop();
        }
    };

    if (ImGui::Begin("Scene graph"))
    {
        ECSView<NameComponent>(m_runningGame->activeScene()).onEach([&](Entity entity, NameComponent &) {
            if (entity.has<HierarchicalComponent>() == false || entity.parent() == false)
                sceneGraphEntityRow(entity);
        });
    }
    ImGui::End();
}

void Engine::drawEntityInspectorWindow()
{
    if (ImGui::Begin("Entity inspector"))
    {
        if (m_selectedEntity == false)
            ImGui::Text("No entity selected");
        else
        {
            ImGui::PushItemWidth(-100);

            NameComponent& nameComponent = m_selectedEntity.get<NameComponent>();
            char buff[32];
            std::strncpy(buff, nameComponent.name, sizeof(buff));
            ImGui::InputText("Name", buff, sizeof(buff));
            nameComponent.name = utils::String(buff);
            
            if (m_selectedEntity.has<TransformComponent>())
            {
                ImGui::SeparatorText("Transform Component");

                TransformComponent& transform = m_selectedEntity.get<TransformComponent>();
                ImGui::DragFloat3("position", (float*)&transform.position, 0.01f, -1000.0f, 1000.0f);
                ImGui::DragFloat3("rotation", (float*)&transform.rotation, 0.01f,    -2*PI,    2*PI);
                ImGui::DragFloat3("scale",    (float*)&transform.scale,    0.01f,     0.0f,   10.0f);
            }

            if (m_selectedEntity.has<CameraComponent>())
            {
                ImGui::SeparatorText("Camera Component");

                CameraComponent& cameraComponent = m_selectedEntity.get<CameraComponent>();
                if (m_selectedEntity.has<ActiveCameraComponent>())
                    ImGui::Text("Active");
                else if (ImGui::Button("Make active"))
                {
                    ECSView<ActiveCameraComponent>(m_runningGame->activeScene()).onFirst([](Entity entity, ActiveCameraComponent&){ 
                        entity.remove<ActiveCameraComponent>();
                    });
                    m_selectedEntity.emplace<ActiveCameraComponent>();
                }
                ImGui::DragFloat("fov",   &cameraComponent.fov,   0.01f,  -2*PI,     2*PI);
                ImGui::DragFloat("zFar",  &cameraComponent.zFar,  0.01f, 0.001f, 10000.0f);
                ImGui::DragFloat("zNear", &cameraComponent.zNear, 0.01f, 0.001f, 10000.0f);
            }

            if (m_selectedEntity.has<LightComponent>())
            {
                ImGui::SeparatorText("Light Component");

                LightComponent& lightComponent = m_selectedEntity.get<LightComponent>();
                ImGui::ColorEdit3("color", (float*)&lightComponent.color);
                ImGui::DragFloat("intentsity", &lightComponent.intentsity, 0.01, 0.0f, 1.0f);
            }
            
            if (m_selectedEntity.has<MeshComponent>())
            {
                ImGui::SeparatorText("Mesh Component");

                MeshComponent meshComponent = m_selectedEntity.get<MeshComponent>();
                ImGui::Text("%s", (char*)meshComponent.mesh->name);
                if (ImGui::TreeNode("Sub Meshes"))
                {
                    for (auto& subMesh : meshComponent.mesh->subMeshes)
                        ImGui::Text("%s", (char*)subMesh.name);
                    ImGui::TreePop();
                }
            }

            ImGui::PopItemWidth();
        }
    }
    ImGui::End();
}

}