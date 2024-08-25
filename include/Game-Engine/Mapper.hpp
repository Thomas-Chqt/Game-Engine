/*
 * ---------------------------------------------------
 * Mapper.hpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/08/24 15:46:20
 * ---------------------------------------------------
 */

#ifndef MAPPER_HPP
#define MAPPER_HPP

#include "Graphics/Event.hpp"
#include "Game-Engine/Input.hpp"
#include "Game-Engine/RawInput.hpp"
#include "Math/Vector.hpp"

namespace GE
{

class IMapper
{
public:
    virtual void onInputEvent(gfx::InputEvent&) = 0;
    virtual ~IMapper() = default;
};

template<typename K, typename I> class Mapper;

template<>
class Mapper<KeyboardButton, ActionInput> : public IMapper
{
public:
    Mapper(KeyboardButton btn, ActionInput& ipt);

    void onInputEvent(gfx::InputEvent&) override;
    
    ~Mapper() override = default;

private:
    void onKeyDownEvent(gfx::KeyDownEvent&);

    const KeyboardButton m_button;
    ActionInput& m_input;
};

template<>
class Mapper<KeyboardButton, StateInput> : public IMapper
{
public:
    Mapper(KeyboardButton btn, StateInput& ipt);

    void onInputEvent(gfx::InputEvent&) override;

    ~Mapper() override = default;

private:
    void onKeyDownEvent(gfx::KeyDownEvent&);
    void onKeyUpEvent(gfx::KeyUpEvent&);

    const KeyboardButton m_button;
    StateInput& m_input;
};

template<>
class Mapper<KeyboardButton, RangeInput> : public IMapper
{
public:
    struct Descriptor
    {
        KeyboardButton button;
        float scale = 1.0F;
    };

public:
    Mapper(const Descriptor& btn, RangeInput& ipt);

    void onInputEvent(gfx::InputEvent& event) override;

    ~Mapper() override = default;

private:
    void onKeyDownEvent(gfx::KeyDownEvent&);
    void onKeyUpEvent(gfx::KeyUpEvent&);

    const KeyboardButton m_button;
    const float m_scale;
    RangeInput& m_input;
};

template<>
class Mapper<KeyboardButton, Range2DInput> : public IMapper
{
public:
    struct Descriptor
    {
        KeyboardButton xPos;
        KeyboardButton xNeg;
        KeyboardButton yPos;
        KeyboardButton yNeg;
        
        math::vec2f scale = {1.0F, 1.0F};
    };

public:
    Mapper(const Descriptor& btn, Range2DInput& ipt);

    void onInputEvent(gfx::InputEvent& event) override;

    ~Mapper() override = default;

private:
    void onKeyDownEvent(gfx::KeyDownEvent&);
    void onKeyUpEvent(gfx::KeyUpEvent&);

    const KeyboardButton m_xPos;
    const KeyboardButton m_xNeg;
    const KeyboardButton m_yPos;
    const KeyboardButton m_yNeg;

    const math::vec2f m_scale;

    Range2DInput& m_input;
};

}

#endif // MAPPER_HPP