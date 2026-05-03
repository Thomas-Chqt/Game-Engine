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

#include <yaml-cpp/yaml.h>

#include <map>
#include <string>
#include <type_traits>
#include <variant>

namespace GE
{

class GE_API InputContext
{
public:
    InputContext() = default;
    InputContext(const InputContext&) = default;
    InputContext(InputContext&&) = default;

    void addInput(const std::string& name, const VInput&);
    void removeInput(const std::string& name);
    bool renameInput(const std::string& oldName, const std::string& newName);

    inline const std::map<std::string, VInput>& inputs() const { return m_inputs; }
    inline std::map<std::string, VInput>& inputs() { return m_inputs; }

    template<InputType T>
    void setInputCallback(const std::string& name, const T::CallbackType&);

    void clearAllInputCallbacks();

    virtual void onInputEvent(InputEvent&);
    void dispatchInputs();

    virtual ~InputContext() = default;

private:
    std::map<std::string, VInput> m_inputs;

public:
    InputContext& operator = (const InputContext&) = default;
    InputContext& operator = (InputContext&&) = default;
};

template<InputType T>
void InputContext::setInputCallback(const std::string& name, const T::CallbackType& callback)
{
    std::visit([&](auto& input)
    {
        if constexpr (std::is_same_v<std::remove_cvref_t<decltype(input)>, T>)
            input.callback = callback;
    },
    m_inputs.at(name));
}

} // namespace GE

namespace YAML
{

template<>
struct convert<GE::InputContext>
{
    static Node encode(const GE::InputContext& rhs)
    {
        Node node;
        node["inputs"] = rhs.inputs();
        return node;
    }

    static bool decode(const Node& node, GE::InputContext& rhs)
    {
        if (!node.IsMap() || !node["inputs"])
            return false;

        rhs.inputs() = node["inputs"].as<std::map<std::string, GE::VInput>>();
        return true;
    }
};

} // namespace YAML

#endif
