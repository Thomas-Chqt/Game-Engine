/*
 * ---------------------------------------------------
 * ImGuiInputContext.hpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * ---------------------------------------------------
 */

#ifndef IMGUIINPUTCONTEXT_HPP
#define IMGUIINPUTCONTEXT_HPP

#include <Game-Engine/InputContext.hpp>

namespace GE_Editor
{

class ImGuiInputContext : public GE::InputContext
{
public:
    void onInputEvent(GE::InputEvent& event) override;
};

}

#endif
