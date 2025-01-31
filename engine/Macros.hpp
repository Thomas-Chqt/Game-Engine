/*
 * ---------------------------------------------------
 * Macros.hpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/11/13 23:12:30
 * ---------------------------------------------------
 */

#ifndef GAME_ENGINE_MACROS_HPP
#define GAME_ENGINE_MACROS_HPP

#if (defined(__GNUC__) || defined(__clang__)) && defined(GAME_ENGINE_API_EXPORT)
    #define GAME_ENGINE_API __attribute__((visibility("default")))
#elif defined(_MSC_VER) && defined(GAME_ENGINE_API_EXPORT)
    #define GAME_ENGINE_API __declspec(dllexport)
#elif defined(_MSC_VER) && defined(GAME_ENGINE_API_IMPORT)
    #define GAME_ENGINE_API __declspec(dllimport)
#else
    #define GAME_ENGINE_API
#endif

#endif // GAME_ENGINE_MACROS_HPP