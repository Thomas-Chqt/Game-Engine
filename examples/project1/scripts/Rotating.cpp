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
    using GE::Script::Script;

    void onUpdate() override
    {
        m_entity.rotation() += { 0.0, 0.02, 0.0 };
    }
};

REGISTER_SCRIPT(Rotating);