/*
 * ---------------------------------------------------
 * ICamera.hpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * ---------------------------------------------------
 *
 * Camera interface, provide an way for the application
 * to give application specific camera to the renderer
 *
 */

#ifndef ICAMERA_HPP
#define ICAMERA_HPP

#include <glm/glm.hpp>

namespace GE
{

class ICamera
{
public:
    virtual glm::vec3 position() const = 0;
    virtual glm::mat4 viewProjectionMatrix() const = 0;

    virtual ~ICamera() = default;
};

}

#endif
