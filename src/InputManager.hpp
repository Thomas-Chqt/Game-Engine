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

#include "Graphics/Event.hpp"
#include "UtilsCPP/Set.hpp"
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
    
public:
    InputManager& operator = (const InputManager&) = delete;
    InputManager& operator = (InputManager&&)      = delete;
};

}

#endif // INPUTMANAGER_HPP