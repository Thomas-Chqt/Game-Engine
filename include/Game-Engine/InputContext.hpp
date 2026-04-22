/*
 * ---------------------------------------------------
 * InputContext.hpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * ---------------------------------------------------
 */

#ifndef INPUTCONTEXT_HPP
#define INPUTCONTEXT_HPP

#include "Game-Engine/Export.hpp"
#include "Game-Engine/Event.hpp"
#include "Game-Engine/Input.hpp"

#include <vector>

namespace GE
{

class GE_API InputContext
{
public:
    InputContext() = default;
    InputContext(const InputContext&) = default;
    InputContext(InputContext&&) = default;

    void addInput(const VInput&);

    virtual void onInputEvent(InputEvent&);
    void dispatchInputs();

    virtual ~InputContext() = default;

private:
    std::vector<VInput> m_inputs;

public:
    InputContext& operator = (const InputContext&) = default;
    InputContext& operator = (InputContext&&) = default;
};

}

#endif
