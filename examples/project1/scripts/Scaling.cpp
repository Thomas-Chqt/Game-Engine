/*
 * ---------------------------------------------------
 * Scaling.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/11/06 17:20:14
 * ---------------------------------------------------
 */

#include "Math/Vector.hpp"
#include "Script.hpp"
#include "ScriptLib.hpp"

class Scaling : public GE::Script
{
public:
    Scaling()               = default;
    Scaling(const Scaling&) = default;
    Scaling(Scaling&&)      = default;
    
    void onUpdate() override
    {
        m_entity.scale() += m_increasing ? math::vec3f{0.02, 0.02, 0.02} : math::vec3f{-0.02, -0.02, -0.02};
        if (m_entity.scale().x >= 5)
            m_increasing = false;
        if (m_entity.scale().y <= 1)
            m_increasing = true;
    }

    ~Scaling() = default;

private:
    bool m_increasing = true;


public:
    Scaling& operator = (const Scaling&) = default;
    Scaling& operator = (Scaling&&)      = default;
};

REGISTER_SCRIPT(Scaling);