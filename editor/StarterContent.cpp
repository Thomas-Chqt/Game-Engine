/*
 * ---------------------------------------------------
 * StarterContent.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/09/08 12:36:30
 * ---------------------------------------------------
 */

#include "StarterContent.hpp"

namespace GE
{

StarterContent::StarterContent()
{
    Scene defaultScene;
    defaultScene.ecsWorld.newEntity("cube");
    
    addScene("defaultScene", std::move(defaultScene));
    setStartScene("defaultScene");
}

}