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

#include "UtilsCPP/Array.hpp"
#include "UtilsCPP/String.hpp"

struct ScriptRegistry
{
    static utils::Array<utils::String> registry;
};

#define GE_SCRIPT(name)                                    \
    extern "C"                                             \
    {                                                      \
        GE::Script* get_##name##_script()                  \
        {                                                  \
            return new name;                               \
        }                                                  \
    }                                                      \
    class ScriptRegistry_##name : public ScriptRegistry    \
    {                                                      \
    public:                                                \
        ScriptRegistry_##name()                            \
        {                                                  \
            registry.append(#name);                        \
        }                                                  \
        static ScriptRegistry_##name instance;             \
    };                                                     \
    ScriptRegistry_##name  ScriptRegistry_##name::instance \


#endif // SCRIPTLIB_HPP