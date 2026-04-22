/*
 * ---------------------------------------------------
 * Window.hpp
 *
 * Author: Thomas Choquet <thomas.publique@icloud.com>
 * Date: 2023/11/14 17:09:45
 * ---------------------------------------------------
 */

#ifndef WINDOW_HPP
#define WINDOW_HPP

#include "Game-Engine/Export.hpp"
#include "Game-Engine/RawInput.hpp"

#include <Graphics/Instance.hpp>
#include <Graphics/Surface.hpp>

#include <cstdint>
#include <filesystem>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <set>
#include <utility>
#include <deque>

struct GLFWwindow; // glfw is not accessible outside the engine

namespace GE
{

class Event;

class GE_API Window
{
public:
    struct Descriptor
    {
        uint32_t width = 1280;
        uint32_t height = 720;
        const char* title = "";
    };

public:
    Window() = delete;
    Window(const Window&) = delete;
    Window(Window&&) = delete;

    Window(const Descriptor&);

    std::pair<uint32_t, uint32_t> size() const; // std::pair<WIDTH, HEIGHT>

    std::pair<uint32_t, uint32_t> frameBufferSize() const; // std::pair<WIDTH, HEIGHT>

    std::pair<float, float> contentScale() const; // std::pair<X, Y>

    inline void addEventCallBack(void* id, const std::function<void(Event&)>& fn) { m_eventCallbacks[id].push_back(fn); }
    inline void addEventCallBack(const std::function<void(Event&)>& cb) { addEventCallBack(nullptr, cb); }

    inline void clearCallbacks(void* id) { m_eventCallbacks.erase(id); }
    inline void clearCallbacks() { m_eventCallbacks.clear(); }

    bool isPressed(KeyboardButton);
    bool isPressed(MouseButton);

    std::pair<double, double> cursorPos() const; // std::pair<X, Y>
    void setCursorPos(double x, double y);

    void setCursorVisibility(bool);

    std::optional<std::filesystem::path> popDroppedFile();

    std::string clipboardString() const;
    void setClipboardString(const std::string&) const;

    void createSurface(gfx::Instance*);
    inline gfx::Surface* surface() const { return m_surface.get(); }

    GLFWwindow* glfwWindow() const { return m_glfwWindow; }

    ~Window();

private:
    ::GLFWwindow* m_glfwWindow = nullptr;
    std::unordered_map<void*, std::deque<std::function<void(Event&)>>> m_eventCallbacks;
    std::set<std::filesystem::path> m_droppedFilePool;

    std::unique_ptr<gfx::Surface> m_surface;

public:
    Window& operator=(const Window&) = delete;
    Window& operator=(Window&&) = delete;
};

} // namespace GE

#endif // WINDOW_HPP
