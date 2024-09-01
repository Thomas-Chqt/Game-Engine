/*
 * ---------------------------------------------------
 * StarterContent.hpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/09/01 12:29:31
 * ---------------------------------------------------
 */

#ifndef STARTERCONTENT_HPP
#define STARTERCONTENT_HPP

#include "Game-Engine/Game.hpp"

namespace GE
{

class StarterContent : public Game
{
public:
    StarterContent()                      = default;
    StarterContent(const StarterContent&) = delete;
    StarterContent(StarterContent&&)      = delete;
    
    ~StarterContent() = default;
    
public:
    StarterContent& operator = (const StarterContent&) = delete;
    StarterContent& operator = (StarterContent&&)      = delete;
};

}

#endif // STARTERCONTENT_HPP