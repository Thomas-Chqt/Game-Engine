/*
 * ---------------------------------------------------
 * Export.hpp
 *
 * Author: Thomas Choquet <thomas.publique@icloud.com>
 * ---------------------------------------------------
 */

#ifndef EXPORT_HPP
#define EXPORT_HPP

#if defined(_WIN32)
    #if defined(GE_DLL_EXPORT)
        #define GE_API __declspec(dllexport)
    #elif defined(GE_DLL_IMPORT)
        #define GE_API __declspec(dllimport)
    #else
        #define GE_API
    #endif
#elif defined(__GNUC__) || defined(__clang__)
    #define GE_API __attribute__((visibility("default")))
#else
    #define GE_API
#endif

#endif // EXPORT_HPP
