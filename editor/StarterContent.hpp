/*
 * ---------------------------------------------------
 * StarterContent.hpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/09/08 12:35:53
 * ---------------------------------------------------
 */

#ifndef STARTERCONTENT_HPP
#define STARTERCONTENT_HPP

#include "Game.hpp"

namespace GE
{

class StarterContent : public Game
{
public:
    StarterContent();
    StarterContent(const StarterContent&) = default;
    StarterContent(StarterContent&&)      = default;
    
    ~StarterContent() override = default;

private:
    
public:
    StarterContent& operator = (const StarterContent&) = default;
    StarterContent& operator = (StarterContent&&)      = default;
};

}

#endif // STARTERCONTENT_HPP