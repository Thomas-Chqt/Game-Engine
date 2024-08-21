/*
 * ---------------------------------------------------
 * Material.hpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/08/19 16:41:32
 * ---------------------------------------------------
 */

#ifndef MATERIAL_HPP
#define MATERIAL_HPP

#include "Game-Engine/RenderMethod.hpp"
#include "Graphics/Texture.hpp"
#include "Math/Vector.hpp"
#include "UtilsCPP/Dictionary.hpp"
#include "UtilsCPP/SharedPtr.hpp"
#include "UtilsCPP/String.hpp"

namespace GE
{

struct Material
{
    using TextureMap = utils::Dictionary<utils::String, utils::SharedPtr<gfx::Texture>&>;
    using RGBAMap = utils::Dictionary<utils::String, math::rgba&>;
    using FloatMap = utils::Dictionary<utils::String, float&>;

    utils::String name;
    utils::SharedPtr<RenderMethod> renderMethod;
    virtual TextureMap textures() = 0;
    virtual RGBAMap rgbaOptions() = 0;
    virtual FloatMap floatOptions() = 0;
    virtual ~Material() = default;
};

}

#endif // MATERIAL_HPP