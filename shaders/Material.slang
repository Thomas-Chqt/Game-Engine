/*
 * ---------------------------------------------------
 * Material.slang
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * ---------------------------------------------------
 */

#ifndef MATERIAL_SLANG
#define MATERIAL_SLANG

#include "shader.h"

SLANG_MODULE_DEF(material);

namespace shader
{

SLANG_PUBLIC struct FlatColorMaterial
{
    SLANG_PUBLIC float4 diffuseColor;
    SLANG_PUBLIC float3 specularColor; FLOAT3_PADDING(0);
    SLANG_PUBLIC float shininess;
};

}

#endif
