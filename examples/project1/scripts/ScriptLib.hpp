/*
 * ---------------------------------------------------
 * ScriptLib.hpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/10/24 15:51:57
 * ---------------------------------------------------
 */

#ifndef SCRIPTLIB_HPP
#define SCRIPTLIB_HPP

#include "Script.hpp"
#include "UtilsCPP/Dictionary.hpp"
#include "UtilsCPP/Func.hpp"
#include "UtilsCPP/String.hpp"

struct ScriptRegistry
{
    static utils::Dictionary<utils::String, utils::Func<GE::Script*()>>& getRegistery()
    {
        static utils::Dictionary<utils::String, utils::Func<GE::Script*()>> registery;
        return registery;
    }
};

#define REGISTER_SCRIPT(name)                                       \
    class ScriptRegistry_##name : public ScriptRegistry             \
    {                                                               \
    public:                                                         \
        ScriptRegistry_##name()                                     \
        {                                                           \
            getRegistery().insert(#name, [](){ return new name; }); \
        }                                                           \
        static ScriptRegistry_##name s_instance;                    \
    };                                                              \
    ScriptRegistry_##name  ScriptRegistry_##name::s_instance


#endif // SCRIPTLIB_HPP