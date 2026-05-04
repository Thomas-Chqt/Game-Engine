/*
 * ---------------------------------------------------
 * Light.slang
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * ---------------------------------------------------
 */

#ifndef LIGHT_SLANG
#define LIGHT_SLANG

#include "shader.h"

SLANG_MODULE_DEF(Light);

namespace shader
{

SLANG_PUBLIC struct alignas(16) DirectionalLight
{
    SLANG_PUBLIC float3 position; FLOAT3_PADDING(0);
    SLANG_PUBLIC float3 color;    FLOAT3_PADDING(1);
};

SLANG_PUBLIC struct alignas(16) PointLight
{
    SLANG_PUBLIC float3 position;   FLOAT3_PADDING(0);
    SLANG_PUBLIC float3 color;      FLOAT3_PADDING(1);
    SLANG_PUBLIC float attenuation; FLOAT1_PADDING(0);
};

}

#endif
