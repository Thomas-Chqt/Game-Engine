//
// ---------------------------------------------------
// Window.cpp
//
// Author: Thomas Choquet <thomas.publique@icloud.com>
// ---------------------------------------------------
//

#include "Game-Engine/Window.hpp"
#include "Game-Engine/Event.hpp"

#include <GLFW/glfw3.h>

#include <cassert>
#include <cstdint>
#include <optional>
#include <string>
#include <utility>

namespace GE
{

Window::Window(const Descriptor& desc)
{
    ::glfwDefaultWindowHints();
    ::glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    m_glfwWindow = ::glfwCreateWindow(desc.width, desc.height, desc.title, nullptr, nullptr);
    assert(m_glfwWindow);

    using CallbackMap = std::unordered_map<void*, std::deque<std::function<void(Event&)>>>;

    glfwSetWindowUserPointer(m_glfwWindow, this);

    ::glfwSetKeyCallback(m_glfwWindow, [](::GLFWwindow* glfwWindow, int key, int, int action, int) {
        Window& window = *static_cast<Window*>(glfwGetWindowUserPointer(glfwWindow));
        CallbackMap callbackMap = window.m_eventCallbacks;

        if (action == GLFW_PRESS)
        {
            KeyDownEvent keyDownEvent(window, key, false);
            for (auto& [key, val] : callbackMap)
            {
                for (auto& callback : val)
                    callback(keyDownEvent);
            }
        }
        else if (action == GLFW_REPEAT)
        {
            KeyDownEvent keyDownEvent(window, key, true);
            for (auto& [key, val] : callbackMap)
            {
                for (auto& callback : val)
                    callback(keyDownEvent);
            }
        }
        else if (action == GLFW_RELEASE)
        {
            KeyUpEvent keyUpEvent(window, key);
            for (auto& [key, val] : callbackMap)
            {
                for (auto& callback : val)
                    callback(keyUpEvent);
            }
        }
    });

    ::glfwSetMouseButtonCallback(m_glfwWindow, [](::GLFWwindow* glfwWindow, int button, int action, int) {
        Window& window = *static_cast<Window*>(glfwGetWindowUserPointer(glfwWindow));
        CallbackMap callbackMap = window.m_eventCallbacks;

        auto [x, y] = window.cursorPos();

        if (action == GLFW_PRESS)
        {
            MouseDownEvent mouseDownEvent(window, x, y, button);
            for (auto& [key, val] : callbackMap)
            {
                for (auto& callback : val)
                    callback(mouseDownEvent);
            }
        }
        else if (action == GLFW_RELEASE)
        {
            MouseUpEvent mouseUpEvent(window, (int)x, (int)y, button);
            for (auto& [key, val] : callbackMap)
            {
                for (auto& callback : val)
                    callback(mouseUpEvent);
            }
        }
    });

    ::glfwSetScrollCallback(m_glfwWindow, [](::GLFWwindow* glfwWindow, double xoffset, double yoffset) {
        Window& window = *static_cast<Window*>(glfwGetWindowUserPointer(glfwWindow));
        CallbackMap callbackMap = window.m_eventCallbacks;

        auto [x, y] = window.cursorPos();

        ScrollEvent scrollEvent(window, x, y, xoffset, yoffset);
        for (auto& [key, val] : callbackMap)
        {
            for (auto& callback : val)
                callback(scrollEvent);
        }
    });

    ::glfwSetCursorPosCallback(m_glfwWindow, [](::GLFWwindow* glfwWindow, double x, double y) {
        Window& window = *static_cast<Window*>(glfwGetWindowUserPointer(glfwWindow));
        CallbackMap callbackMap = window.m_eventCallbacks;

        MouseMoveEvent mouseMoveEvent(window, x, y);
        for (auto& [key, val] : callbackMap)
        {
            for (auto& callback : val)
                callback(mouseMoveEvent);
        }
    });

    ::glfwSetWindowSizeCallback(m_glfwWindow, [](::GLFWwindow* glfwWindow, int width, int height) {
        Window& window = *static_cast<Window*>(glfwGetWindowUserPointer(glfwWindow));
        CallbackMap callbackMap = window.m_eventCallbacks;

        WindowResizeEvent windowResizeEvent(window, width, height);
        for (auto& [key, val] : callbackMap)
        {
            for (auto& callback : val)
                callback(windowResizeEvent);
        }
    });

    ::glfwSetWindowCloseCallback(m_glfwWindow, [](::GLFWwindow* glfwWindow) {
        Window& window = *static_cast<Window*>(glfwGetWindowUserPointer(glfwWindow));
        CallbackMap callbackMap = window.m_eventCallbacks;

        ::glfwSetWindowShouldClose(glfwWindow, GLFW_FALSE);

        WindowRequestCloseEvent windowRequestCloseEvent(window);
        for (auto& [key, val] : callbackMap)
        {
            for (auto& callback : val)
                callback(windowRequestCloseEvent);
        }
    });

    ::glfwSetDropCallback(m_glfwWindow, [](::GLFWwindow* glfwWindow, int count, const char** paths) {
        Window& window = *static_cast<Window*>(glfwGetWindowUserPointer(glfwWindow));
        for (int i = 0; i < count; i++)
        {
            std::filesystem::path path = std::filesystem::path(paths[i]);
            assert(path.is_absolute());
            assert(std::filesystem::exists(path));
            window.m_droppedFilePool.insert(path);
        }
    });
}

std::pair<uint32_t, uint32_t> Window::size() const
{
    int w, h;
    ::glfwGetWindowSize(m_glfwWindow, &w, &h);
    return std::make_pair(static_cast<uint32_t>(w), static_cast<uint32_t>(h));
}

std::pair<uint32_t, uint32_t> Window::frameBufferSize() const
{
    int w, h;
    ::glfwGetFramebufferSize(m_glfwWindow, &w, &h);
    return std::make_pair(static_cast<uint32_t>(w), static_cast<uint32_t>(h));
}

std::pair<float, float> Window::contentScale() const
{
    float x, y;
    ::glfwGetWindowContentScale(m_glfwWindow, &x, &y);
    return std::make_pair(x, y);
}

bool Window::isPressed(KeyboardButton btn)
{
    return ::glfwGetKey(m_glfwWindow, static_cast<int>(btn)) == GLFW_PRESS;
}

bool Window::isPressed(MouseButton key)
{
    return ::glfwGetMouseButton(m_glfwWindow, static_cast<int>(key)) == GLFW_PRESS;
}

std::pair<double, double> Window::cursorPos() const
{
    double x, y;
    ::glfwGetCursorPos(m_glfwWindow, &x, &y);
    return std::make_pair(x, y);
}

void Window::setCursorPos(double x, double y)
{
    ::glfwSetCursorPos(m_glfwWindow, x, y);
}

void Window::setCursorVisibility(bool value)
{
    ::glfwSetInputMode(m_glfwWindow, GLFW_CURSOR, value ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_HIDDEN);
}

std::optional<std::filesystem::path> Window::popDroppedFile()
{
    if (m_droppedFilePool.empty())
        return std::nullopt;
    auto node = m_droppedFilePool.extract(m_droppedFilePool.begin());
    return node.value();
}

std::string Window::clipboardString() const
{
    return ::glfwGetClipboardString(m_glfwWindow);
}

void Window::setClipboardString(const std::string& str) const
{
    ::glfwSetClipboardString(m_glfwWindow, str.c_str());
}

void Window::createSurface(gfx::Instance* instance)
{
    m_surface = instance->createSurface(m_glfwWindow);
    assert(m_surface);
}

Window::~Window()
{
    ::glfwDestroyWindow(m_glfwWindow);
}

} // namespace GE
