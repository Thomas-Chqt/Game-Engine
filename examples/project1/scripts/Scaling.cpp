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
    using GE::Script::Script;
    
    void onUpdate() override
    {
        m_entity.scale() += m_increasing ? math::vec3f{0.02, 0.02, 0.02} : math::vec3f{-0.02, -0.02, -0.02};
        if (m_entity.scale().x >= 5)
            m_increasing = false;
        if (m_entity.scale().y <= 1)
            m_increasing = true;
    }

private:
    bool m_increasing = true;
};

REGISTER_SCRIPT(Scaling);