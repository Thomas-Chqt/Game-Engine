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

#include "Math/Vector.hpp"
#include "UtilsCPP/Func.hpp"
#include "UtilsCPP/String.hpp"
#include "UtilsCPP/UniquePtr.hpp"

namespace GE
{

class IMapper;

struct Input
{
    const utils::String name;
    bool triggered = false;
    utils::UniquePtr<IMapper> mappers[2];

    virtual void dispatch() = 0;

    Input(utils::String name);
    virtual ~Input();
};

struct ActionInput : public Input
{
    utils::Func<void()> callback;

    void dispatch() override;

    ActionInput(utils::String name);
    ~ActionInput() override = default;
};

struct StateInput : public Input
{
    utils::Func<void()> callback;

    void dispatch() override;

    StateInput(utils::String name);
    ~StateInput() override = default;
};

struct RangeInput : public Input
{
    utils::Func<void(float)> callback;
    float value = 0.0F;

    void dispatch() override;

    RangeInput(utils::String name);
    ~RangeInput() override = default;
};

struct Range2DInput : public Input
{
    utils::Func<void(math::vec2f)> callback;
    math::vec2f value = { 0.0F, 0.0F };

    void dispatch() override;

    Range2DInput(utils::String name);
    ~Range2DInput() override = default;
};

}

#endif // INPUT_HPP