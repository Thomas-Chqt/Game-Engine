/*
 * ---------------------------------------------------
 * Event.hpp
 *
 * Author: Thomas Choquet <thomas.publique@icloud.com>
 * Date: 2023/11/17 00:53:33
 * ---------------------------------------------------
 */

#ifndef EVENT_HPP
#define EVENT_HPP

#include "Game-Engine/Window.hpp"

#include <cstdint>
#include <functional>
#include <format>
#include <utility>

namespace GE
{

class Event
{
public:
    Event(const Event&) = delete;
    Event(Event&&)      = delete;

    template<typename T>
    bool dispatch(const std::function<void(T&)>& f)
    {
        T* casted_event = dynamic_cast<T*>(this);
        if (casted_event == nullptr)
            return false;
        f(*casted_event);
        return true;
    }

    inline bool processed() { return m_processed; }
    inline void markAsProcessed() { m_processed = true; }

    virtual ~Event() = default;

protected:
    Event() = default;

private:
    friend struct std::formatter<GE::Event>;
    virtual std::string to_string() const = 0;

    bool m_processed = false;

public:
    Event& operator = (const Event&) = delete;
    Event& operator = (Event&&)      = delete;
};

class ApplicationRequestTerminationEvent final : public Event
{
public:
    ApplicationRequestTerminationEvent()                                          = default;
    ApplicationRequestTerminationEvent(const ApplicationRequestTerminationEvent&) = delete;
    ApplicationRequestTerminationEvent(ApplicationRequestTerminationEvent&&)      = delete;

    ~ApplicationRequestTerminationEvent() override = default;

private:
    inline std::string to_string() const override { return std::format("(ApplicationRequestTerminationEvent) ->"); }

public:
    ApplicationRequestTerminationEvent& operator = (const ApplicationRequestTerminationEvent&) = delete;
    ApplicationRequestTerminationEvent& operator = (ApplicationRequestTerminationEvent&&)      = delete;
};

class WindowEvent : public Event
{
public:
    WindowEvent()                   = delete;
    WindowEvent(const WindowEvent&) = delete;
    WindowEvent(WindowEvent&&)      = delete;

    inline const Window& window() const { return m_window; }

    ~WindowEvent() override = default;

protected:
    inline explicit WindowEvent(Window& window)
        : m_window(window)
    {
    }

private:
    inline std::string to_string() const override = 0;

protected:
    Window& m_window;

public:
    WindowEvent& operator = (const WindowEvent&) = delete;
    WindowEvent& operator = (WindowEvent&&)      = delete;
};

class WindowResizeEvent final : public WindowEvent
{
public:
    WindowResizeEvent()                         = delete;
    WindowResizeEvent(const WindowResizeEvent&) = delete;
    WindowResizeEvent(WindowResizeEvent&&)      = delete;

    inline WindowResizeEvent(Window& window, int w, int h)
        : WindowEvent(window)
        , m_width(w)
        , m_height(h)
    {
    }

    inline int width() const { return m_width; }
    inline int height() const { return m_height; }

    ~WindowResizeEvent() override = default;

private:
    inline std::string to_string() const override { return std::format("(WindowResizeEvent) -> Window: {:x} width: {} height: {}", (std::uintptr_t)&m_window, m_width, m_height); }

    int m_width;
    int m_height;

public:
    WindowResizeEvent& operator = (const WindowResizeEvent&) = delete;
    WindowResizeEvent& operator = (WindowResizeEvent&&)      = delete;
};

class WindowRequestCloseEvent final : public WindowEvent
{
public:
    WindowRequestCloseEvent()                               = delete;
    WindowRequestCloseEvent(const WindowRequestCloseEvent&) = delete;
    WindowRequestCloseEvent(WindowRequestCloseEvent&&)      = delete;

    inline explicit WindowRequestCloseEvent(Window& window)
        : WindowEvent(window)
    {
    }

    ~WindowRequestCloseEvent() override = default;

private:
    inline std::string to_string() const override { return std::format("(WindowRequestCloseEvent) -> Window: {:x}", (std::uintptr_t)&m_window); }

public:
    WindowRequestCloseEvent& operator = (const WindowRequestCloseEvent&) = delete;
    WindowRequestCloseEvent& operator = (WindowRequestCloseEvent&&)      = delete;
};

class InputEvent : public WindowEvent
{
public:
    InputEvent()                  = delete;
    InputEvent(const InputEvent&) = delete;
    InputEvent(InputEvent&&)      = delete;

    ~InputEvent() override = default;

protected:
    inline explicit InputEvent(Window& window)
        : WindowEvent(window)
    {
    }

private:
    inline std::string to_string() const override = 0;

public:
    InputEvent& operator = (const InputEvent&) = delete;
    InputEvent& operator = (InputEvent&&)      = delete;
};

class KeyboardEvent : public InputEvent
{
public:
    KeyboardEvent()                     = delete;
    KeyboardEvent(const KeyboardEvent&) = delete;
    KeyboardEvent(KeyboardEvent&&)      = delete;

    inline int keyCode() const { return m_keyCode; }

    ~KeyboardEvent() override = default;

protected:
    inline KeyboardEvent(Window& window, int keyCode)
        : InputEvent(window)
        , m_keyCode(keyCode)
    {
    }

private:
    inline std::string to_string() const override = 0;

protected:
    int m_keyCode;

public:
    KeyboardEvent& operator = (const KeyboardEvent&) = delete;
    KeyboardEvent& operator = (KeyboardEvent&&)      = delete;
};

class KeyDownEvent final : public KeyboardEvent
{
public:
    KeyDownEvent()                    = delete;
    KeyDownEvent(const KeyDownEvent&) = delete;
    KeyDownEvent(KeyDownEvent&&)      = delete;

    inline KeyDownEvent(Window& window, int keyCode, bool isRepeat)
        : KeyboardEvent(window, keyCode)
        , m_isRepeat(isRepeat)
    {
    }

    inline bool isRepeat() const { return m_isRepeat; }

    ~KeyDownEvent() override = default;

private:
    inline std::string to_string() const override { return std::format("(KeyDownEvent) -> Window: {:x} keyCode: {} isRepeat: {}", (std::uintptr_t)&m_window, (int)m_keyCode, m_isRepeat); }

    bool m_isRepeat;

public:
    KeyDownEvent& operator = (const KeyDownEvent&) = delete;
    KeyDownEvent& operator = (KeyDownEvent&&)      = delete;
};

class KeyUpEvent final : public KeyboardEvent
{
public:
    KeyUpEvent()                  = delete;
    KeyUpEvent(const KeyUpEvent&) = delete;
    KeyUpEvent(KeyUpEvent&&)      = delete;

    inline KeyUpEvent(Window& window, int keyCode)
        : KeyboardEvent(window, keyCode)
    {
    }

    ~KeyUpEvent() override = default;

private:
    inline std::string to_string() const override { return std::format("(KeyUpEvent) -> Window: {:x} keyCode: {}", (std::uintptr_t)&m_window, (int)m_keyCode); }

public:
    KeyUpEvent& operator = (const KeyUpEvent&) = delete;
    KeyUpEvent& operator = (KeyUpEvent&&)      = delete;
};

class MouseEvent : public InputEvent
{
public:
    MouseEvent()                  = delete;
    MouseEvent(const MouseEvent&) = delete;
    MouseEvent(MouseEvent&&)      = delete;

    inline std::pair<double, double> mousePos() const { return std::make_pair(m_pos_x, m_pos_y); }

    ~MouseEvent() override = default;

protected:
    inline MouseEvent(Window& window, double posX, double posY)
        : InputEvent(window)
        , m_pos_x(posX)
        , m_pos_y(posY)
    {
    }

private:
    inline std::string to_string() const override = 0;

protected:
    double m_pos_x;
    double m_pos_y;

public:
    MouseEvent& operator = (const MouseEvent&) = delete;
    MouseEvent& operator = (MouseEvent&&)      = delete;
};

class ScrollEvent final : public MouseEvent
{
public:
    ScrollEvent()                   = delete;
    ScrollEvent(const ScrollEvent&) = delete;
    ScrollEvent(ScrollEvent&&)      = delete;

    inline ScrollEvent(Window& window, double posX, double posY, double offsetX, double offsetY)
        : MouseEvent(window, posX, posY)
        , m_offsetX(offsetX)
        , m_offsetY(offsetY)
    {
    }

    inline double offsetX() const { return m_offsetX; }
    inline double offsetY() const { return m_offsetY; }

    ~ScrollEvent() override = default;

private:
    double m_offsetX;
    double m_offsetY;

private:
    inline std::string to_string() const override { return std::format("(ScrollEvent) -> Window: {:x} pos: {},{} offset: {},{}", (std::uintptr_t)&m_window, m_pos_x, m_pos_y, m_offsetX, m_offsetY); }

public:
    ScrollEvent& operator = (const ScrollEvent&) = delete;
    ScrollEvent& operator = (ScrollEvent&&)      = delete;
};

class MouseMoveEvent final : public MouseEvent
{
public:
    MouseMoveEvent()                      = delete;
    MouseMoveEvent(const MouseMoveEvent&) = delete;
    MouseMoveEvent(MouseMoveEvent&&)      = delete;

    inline MouseMoveEvent(Window& window, double posX, double posY)
        : MouseEvent(window, posX, posY)
    {
    }

    ~MouseMoveEvent() override = default;

private:
    inline std::string to_string() const override { return std::format("(MouseMoveEvent) -> Window: {:x} X: {} Y: {}", (std::uintptr_t)&m_window, m_pos_x, m_pos_y); }

public:
    MouseMoveEvent& operator = (const MouseMoveEvent&) = delete;
    MouseMoveEvent& operator = (MouseMoveEvent&&)      = delete;
};

class MouseButtonEvent : public MouseEvent
{
public:
    MouseButtonEvent()                        = delete;
    MouseButtonEvent(const MouseButtonEvent&) = delete;
    MouseButtonEvent(MouseButtonEvent&&)      = delete;

    inline int mouseCode() const { return m_mouseCode; }

    ~MouseButtonEvent() override = default;

protected:
    inline MouseButtonEvent(Window& window, double posX, double posY, int mouseCode)
        : MouseEvent(window, posX, posY)
        , m_mouseCode(mouseCode)
    {
    }

    int m_mouseCode;

private:
    inline std::string to_string() const override = 0;

public:
    MouseButtonEvent& operator = (const MouseButtonEvent&) = delete;
    MouseButtonEvent& operator = (MouseButtonEvent&&)      = delete;

};

class MouseDownEvent final : public MouseButtonEvent
{
public:
    MouseDownEvent()                      = delete;
    MouseDownEvent(const MouseDownEvent&) = delete;
    MouseDownEvent(MouseDownEvent&&)      = delete;

    inline MouseDownEvent(Window& window, double posX, double posY, int mouseCode)
        : MouseButtonEvent(window, posX, posY, mouseCode)
    {
    }

    ~MouseDownEvent() override = default;

private:
    inline std::string to_string() const override { return std::format("(MouseDownEvent) -> Window: {:x} mouseCode: {} X: {} Y: {}", (std::uintptr_t)&m_window, (int)m_mouseCode, m_pos_x, m_pos_y); }

public:
    MouseDownEvent& operator = (const MouseDownEvent&) = delete;
    MouseDownEvent& operator = (MouseDownEvent&&)      = delete;
};

class MouseUpEvent final : public MouseButtonEvent
{
public:
    MouseUpEvent()                    = delete;
    MouseUpEvent(const MouseUpEvent&) = delete;
    MouseUpEvent(MouseUpEvent&&)      = delete;

    inline MouseUpEvent(Window& window, double posX, double posY, int mouseCode)
        : MouseButtonEvent(window, posX, posY, mouseCode)
    {
    }

    ~MouseUpEvent() override = default;

private:
    inline std::string to_string() const override { return std::format("(MouseUpEvent) -> Window: {:x} mouseCode: {} X: {} Y: {}", (std::uintptr_t)&m_window, (int)m_mouseCode, m_pos_x, m_pos_y); }

public:
    MouseUpEvent& operator = (const MouseUpEvent&) = delete;
    MouseUpEvent& operator = (MouseUpEvent&&)      = delete;
};

}

template<>
struct std::formatter<GE::Event> {
    constexpr auto parse(std::format_parse_context& ctx) {
        auto it = ctx.begin();
        for(; it != ctx.end() && *it != '}'; it++);
        return it;
    }

    auto format(const GE::Event& event, std::format_context& ctx) const {
        return std::format_to(ctx.out(), "{}", event.to_string());
    }
};

#endif // EVENT_HPP
