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

#include "ECS/Entity.hpp"
#include "Script.hpp"
#include "UtilsCPP/Dictionary.hpp"
#include "UtilsCPP/Func.hpp"
#include "UtilsCPP/String.hpp"

#if (defined(__GNUC__) || defined(__clang__)) && defined(PROJECT1_API_EXPORT)
    #define PROJECT1_API __attribute__((visibility("default")))
#elif defined(_MSC_VER) && defined(PROJECT1_API_EXPORT)
    #define PROJECT1_API __declspec(dllexport)
#elif defined(_MSC_VER) && defined(PROJECT1_API_IMPORT)
    #define PROJECT1_API __declspec(dllimport)
#else
    #define PROJECT1_API
#endif


struct ScriptRegistry
{
    static utils::Dictionary<utils::String, utils::Func<GE::Script*(const GE::Entity&, GE::Game&)>>& getRegistery()
    {
        static utils::Dictionary<utils::String, utils::Func<GE::Script*(const GE::Entity&, GE::Game&)>> registery;
        return registery;
    }
};

#define REGISTER_SCRIPT(name)                                                  \
    class ScriptRegistry_##name : public ScriptRegistry                        \
    {                                                                          \
    public:                                                                    \
        ScriptRegistry_##name()                                                \
        {                                                                      \
            getRegistery().insert(#name, [](const GE::Entity& e, GE::Game& g){ \
                return new name(e, g);                                         \
            });                                                                \
        }                                                                      \
        static ScriptRegistry_##name s_instance;                               \
    };                                                                         \
    ScriptRegistry_##name  ScriptRegistry_##name::s_instance

#endif // SCRIPTLIB_HPP