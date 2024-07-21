/*
 * ---------------------------------------------------
 * Game.hpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/06/20 16:57:06
 * ---------------------------------------------------
 */

#ifndef GAME_HPP
# define GAME_HPP

namespace GE
{

class Game
{
public:
    Game(const Game&) = delete;
    Game(Game&&)      = delete;

    virtual void onWindowCloseEvent();

    virtual ~Game() = default;

protected:
    Game() = default;
};

}

#endif // GAME_HPP