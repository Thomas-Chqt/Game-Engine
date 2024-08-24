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

#include "UtilsCPP/Set.hpp"
#include "UtilsCPP/String.hpp"
#include "UtilsCPP/UniquePtr.hpp"
#include "Game-Engine/Input.hpp"

namespace GE
{

class InputManager
{
public:
    InputManager(const InputManager&) = delete;
    InputManager(InputManager&&)      = delete;

    static inline InputManager& shared() { return *s_sharedInstance; }

    template<typename T> inline T& newInput(const utils::String& name) { *dynamic_cast<T*>(newInput(name, utils::makeUnique<T>(name).template staticCast<Input>())); }
    template<typename T> inline T& getInput(const utils::String& name) { *dynamic_cast<T*>(getInput(name)); }
    virtual void deleteInput(const utils::String& name) = 0;

    virtual const utils::Set<int>& pressedKeys() = 0;

    virtual ~InputManager() = default;

protected:
    InputManager() = default;

    virtual Input* newInput(const utils::String& name, utils::UniquePtr<Input>&&) = 0;
    virtual Input* getInput(const utils::String& name) = 0;

    static inline utils::UniquePtr<InputManager> s_sharedInstance;

public:
    InputManager& operator = (const InputManager&) = delete;
    InputManager& operator = (InputManager&&)      = delete;
};

}

#endif // INPUTMANAGER_HPP