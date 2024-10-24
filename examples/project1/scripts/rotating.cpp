/*
 * ---------------------------------------------------
 * rotating.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/10/24 12:13:30
 * ---------------------------------------------------
 */

#include "Script.hpp" // IWYU pragma: keep
#include "ScriptLib.hpp"



class CubeRotate : public GE::Script
{
public:
    CubeRotate()                  = default;
    CubeRotate(const CubeRotate&) = default;
    CubeRotate(CubeRotate&&)      = default;

    void onUpdate() override
    {
        
    }

    ~CubeRotate() = default;

private:
    
public:
    CubeRotate& operator = (const CubeRotate&) = default;
    CubeRotate& operator = (CubeRotate&&)      = default;
};

GE_SCRIPT(CubeRotate);