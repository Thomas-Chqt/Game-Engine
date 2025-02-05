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
#include "InputManager/Input.hpp"
#include "InputManager/RawInput.hpp"
#include "Math/Vector.hpp"
#include "Macros.hpp"

namespace GE
{

class GAME_ENGINE_API IMapper
{
public:
    virtual utils::UniquePtr<IMapper> clone(Input*) const = 0;
    virtual void onInputEvent(gfx::InputEvent&) = 0;
    virtual ~IMapper() = default;
};

template<typename K, typename I> class Mapper;

template<>
class GAME_ENGINE_API Mapper<KeyboardButton, ActionInput> : public IMapper
{
public:
    Mapper(KeyboardButton btn, ActionInput& ipt);

    utils::UniquePtr<IMapper> clone(Input*) const override;
    void onInputEvent(gfx::InputEvent&) override;
    
    ~Mapper() override = default;

private:
    void onKeyDownEvent(gfx::KeyDownEvent&);

    const KeyboardButton m_button;
    ActionInput& m_input;
};

template<>
class GAME_ENGINE_API Mapper<KeyboardButton, StateInput> : public IMapper
{
public:
    Mapper(KeyboardButton btn, StateInput& ipt);

    utils::UniquePtr<IMapper> clone(Input*) const override;
    void onInputEvent(gfx::InputEvent&) override;

    ~Mapper() override = default;

private:
    void onKeyDownEvent(gfx::KeyDownEvent&);
    void onKeyUpEvent(gfx::KeyUpEvent&);

    const KeyboardButton m_button;
    StateInput& m_input;
};

template<>
class GAME_ENGINE_API Mapper<KeyboardButton, RangeInput> : public IMapper
{
public:
    struct Descriptor
    {
        KeyboardButton button;
        float scale = 1.0F;
    };

public:
    Mapper(const Descriptor& btn, RangeInput& ipt);

    utils::UniquePtr<IMapper> clone(Input*) const override;
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
class GAME_ENGINE_API Mapper<KeyboardButton, Range2DInput> : public IMapper
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

    utils::UniquePtr<IMapper> clone(Input*) const override;
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