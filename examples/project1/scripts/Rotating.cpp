/*
 * ---------------------------------------------------
 * Rotating.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/10/24 12:13:30
 * ---------------------------------------------------
 */

#include "Script.hpp"
#include "ScriptLib.hpp"

class Rotating : public GE::Script
{
public:
    Rotating()                  = default;
    Rotating(const Rotating&) = default;
    Rotating(Rotating&&)      = default;

    void onUpdate() override
    {
        m_entity.rotation() += { 0.0, 0.02, 0.0 };
    }

    ~Rotating() = default;

private:
    
public:
    Rotating& operator = (const Rotating&) = default;
    Rotating& operator = (Rotating&&)      = default;
};

REGISTER_SCRIPT(Rotating);