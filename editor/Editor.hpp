/*
 * ---------------------------------------------------
 * Editor.hpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/09/02 16:04:23
 * ---------------------------------------------------
 */

#ifndef EDITOR_HPP
#define EDITOR_HPP

#include <Game-Engine/Application.hpp>

namespace GE_Editor
{

class Editor : public GE::Application
{
public:
    Editor();
    Editor(const Editor&) = delete;
    Editor(Editor&&) = delete;

    void onUpdate() override;
    void onEvent(GE::Event& event) override;

    inline const GE::FrameGraph& frameGraph() override { return m_frameGraph; }

private:
    void rebuildFrameGraph();

    GE::FrameGraph m_frameGraph;

public:
    Editor& operator=(const Editor&) = delete;
    Editor& operator=(Editor&&) = delete;
};

} // namespace GE

#endif // EDITOR_HPP
