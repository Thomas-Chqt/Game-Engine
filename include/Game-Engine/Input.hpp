/*
 * ---------------------------------------------------
 * Input.hpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/08/24 15:45:24
 * ---------------------------------------------------
 */

#ifndef INPUT_HPP
#define INPUT_HPP

#include "Graphics/Event.hpp"
#include "Math/Vector.hpp"
#include "UtilsCPP/Func.hpp"
#include "UtilsCPP/String.hpp"
#include "UtilsCPP/UniquePtr.hpp"

namespace GE
{

class IMapper;

class Input
{
public:
    Input(utils::String name);

    void setMapper0(utils::UniquePtr<IMapper>&& map);
    void setMapper1(utils::UniquePtr<IMapper>&& map);

    void onInputEvent(gfx::InputEvent& event);

    virtual bool hasCallback() const = 0;

    inline void trigger() { m_triggered = true; }
    inline void unTrigger() { m_triggered = false; }
    inline bool isTriggered() { return m_triggered; }

    virtual void dispatch() = 0;

    virtual ~Input();

protected:
    const utils::String m_name;
    bool m_triggered = false;
    utils::UniquePtr<IMapper> m_mappers[2];
};

class ActionInput : public Input
{
public:
    ActionInput(utils::String name);

    inline bool hasCallback() const override { return m_callback == true; }
    inline void dispatch() override { m_callback(); m_triggered = false; }

    inline void setCallback(const utils::Func<void()>& f) { m_callback = f; }
    inline void removeCallback() { m_callback = utils::Func<void()>(); }

    ~ActionInput() override = default;

private:
    utils::Func<void()> m_callback;
};

class StateInput : public Input
{
public:
    StateInput(utils::String name);

    inline bool hasCallback() const override { return m_callback == true; }
    inline void dispatch() override { m_callback(); }

    inline void setCallback(const utils::Func<void()>& f) { m_callback = f; }
    inline void removeCallback() { m_callback = utils::Func<void()>(); }

    ~StateInput() override = default;

private:
    utils::Func<void()> m_callback;
};

class RangeInput : public Input
{
public:
    RangeInput(utils::String name);

    inline bool hasCallback() const override { return m_callback == true; }
    inline void dispatch() override { m_callback(m_value); }

    inline void setCallback(const utils::Func<void(float)>& f) { m_callback = f; }
    inline void removeCallback() { m_callback = utils::Func<void(float)>(); }

    float value() { return m_value; }
    void setValue(float v) { m_value = v; }

    ~RangeInput() override = default;

private:
    float m_value = 0.0F;
    utils::Func<void(float)> m_callback;
};

class Range2DInput : public Input
{
public:
    Range2DInput(utils::String name);

    inline bool hasCallback() const override { return m_callback == true; }
    inline void dispatch() override { m_callback(m_value); }

    inline void setCallback(const utils::Func<void(math::vec2f)>& f) { m_callback = f; }
    inline void removeCallback() { m_callback = utils::Func<void(math::vec2f)>(); }

    math::vec2f value() { return m_value; }
    void setValue(math::vec2f v) { m_value = v; }

    ~Range2DInput() override = default;

private:
    math::vec2f m_value = {0.0F, 0.0F};
    utils::Func<void(math::vec2f)> m_callback;
};

}

#endif // INPUT_HPP