/*
 * ---------------------------------------------------
 * InputManager.hpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/08/16 15:29:35
 * ---------------------------------------------------
 */

#ifndef INPUTMANAGER_HPP
#define INPUTMANAGER_HPP

#include "Game-Engine/InputContext.hpp"
#include "Graphics/Event.hpp"
#include "UtilsCPP/Array.hpp"
#include "UtilsCPP/Dictionary.hpp"
#include "UtilsCPP/Set.hpp"
#include "UtilsCPP/String.hpp"
#include "UtilsCPP/UniquePtr.hpp"

namespace GE
{

class InputManager
{
public:
    InputManager(const InputManager&) = delete;
    InputManager(InputManager&&)      = delete;

    static inline void init() { s_sharedInstance = utils::UniquePtr<InputManager>(new InputManager()); }
    static inline InputManager& shared() { return *s_sharedInstance; }
    static inline void terminate() { s_sharedInstance.clear(); }

    inline const utils::Set<int>& pressedKeys() { return m_pressedKeys; }

    inline void newInput(const utils::String& name, Input::Type type) { m_inputs.insert(name, Input(type)); }
    inline void deleteInput(const utils::String& name) { m_inputs.remove(name); }

    inline void addInputCallback(const utils::String& inputName, utils::Func<void(float)> cb) { m_inputs[inputName].callbacks.append(cb); }

    inline void newContext(const utils::String& name) { m_contexts.append(InputContext(name)); }
    inline void deleteContext(const utils::String& name) { m_contexts.remove(m_contexts.findWhere([&](const InputContext& ctx){ return ctx.name() == name; })); }
    inline void disableContext(const utils::String& name) { m_contexts.findWhere([&](const InputContext& ctx){ return ctx.name() == name; })->disable(); }
    inline void enableContext(const utils::String& name) { m_contexts.findWhere([&](const InputContext& ctx){ return ctx.name() == name; })->enable(); }

    inline const utils::Array<InputContext>& contexts() const { return m_contexts; }

    void dispach();

    ~InputManager();

private:
    InputManager();

    void onEvent(gfx::Event&);
    void onKeyDownEvent(gfx::KeyDownEvent&);
    void onKeyUpEvent(gfx::KeyUpEvent&);
    void onScrollEvent(gfx::ScrollEvent&);
    void onMouseMoveEvent(gfx::MouseMoveEvent&);
    void onMouseDownEvent(gfx::MouseDownEvent&);
    void onMouseUpEvent(gfx::MouseUpEvent&);

    static inline utils::UniquePtr<InputManager> s_sharedInstance;

    utils::Set<int> m_pressedKeys;
    utils::Set<int> m_pressedMouseButtons;

    utils::Dictionary<utils::String, Input> m_inputs;
    utils::Array<InputContext> m_contexts;
    
public:
    InputManager& operator = (const InputManager&) = delete;
    InputManager& operator = (InputManager&&)      = delete;
};

}

#endif // INPUTMANAGER_HPP