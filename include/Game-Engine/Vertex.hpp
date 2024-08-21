/*
 * ---------------------------------------------------
 * Vertex.hpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/08/20 22:35:44
 * ---------------------------------------------------
 */

#ifndef VERTEX_HPP
#define VERTEX_HPP

#include "Math/Vector.hpp"

namespace GE
{

struct Vertex
{
    math::vec3f pos;
    math::vec2f uv;
    math::vec3f normal;
    math::vec3f tangent;
};

}

#endif // VERTEX_HPP