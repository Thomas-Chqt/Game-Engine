/*
 * ---------------------------------------------------
 * InputManagerIntern.hpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/08/24 10:31:15
 * ---------------------------------------------------
 */

#ifndef INPUTMANAGERINTERN_HPP
#define INPUTMANAGERINTERN_HPP

#include "Game-Engine/InputManager.hpp"
#include "Graphics/Event.hpp"
#include "UtilsCPP/Dictionary.hpp"
#include "UtilsCPP/UniquePtr.hpp"

namespace GE
{

class InputManagerIntern : public InputManager
{
public:
    InputManagerIntern(const InputManagerIntern&) = delete;
    InputManagerIntern(InputManagerIntern&&)      = delete;

    static inline void init() { s_sharedInstance = utils::UniquePtr<InputManagerIntern>(new InputManagerIntern()).staticCast<InputManager>(); }
    static inline InputManagerIntern& shared() { return *(InputManagerIntern*)(InputManager*)s_sharedInstance; }
    static inline void terminate() { s_sharedInstance.clear(); }
    
    inline void deleteInput(const utils::String& name) override { m_gameInputs.remove(name); }

    inline const utils::Set<int>& pressedKeys() override { return m_pressedKeys; }

    template<typename T> inline T& newEditorInput(const utils::String& name) { return *dynamic_cast<T*>((Input*)m_editorInputs.insert(name, utils::makeUnique<T>(name).template staticCast<Input>())->val); }
    template<typename T> inline T& getEditorInput(const utils::String& name) { return *dynamic_cast<T*>((Input*)m_editorInputs[name]); }
    inline void deleteEditorInput(const utils::String& name) { m_gameInputs.remove(name); }

    void disableGameInputs();
    inline void enableGameInputs() { m_gameInputsEnabled = true; }

    void disableEditorInputs();
    inline void enableEditorInputs() { m_editorInputsEnabled = true; }

    void dispatchInputs();

    ~InputManagerIntern();

private:
    InputManagerIntern();

    void onInputEvent(gfx::InputEvent&);

    Input* newInput(const utils::String& name, utils::UniquePtr<Input>&& ipt) override { return (Input*)m_gameInputs.insert(name, std::move(ipt))->val; }
    inline Input* getInput(const utils::String& name) override { return (Input*)m_gameInputs[name]; }

    utils::Set<int> m_pressedKeys;
    utils::Set<int> m_pressedMouseButtons;

    utils::Dictionary<utils::String, utils::UniquePtr<Input>> m_gameInputs;
    utils::Dictionary<utils::String, utils::UniquePtr<Input>> m_editorInputs;

    bool m_gameInputsEnabled = false;
    bool m_editorInputsEnabled = false;

public:
    InputManagerIntern& operator = (const InputManagerIntern&) = delete;
    InputManagerIntern& operator = (InputManagerIntern&&)      = delete;
};

}

#endif // INPUTMANAGERINTERN_HPP