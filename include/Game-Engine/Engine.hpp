/*
 * ---------------------------------------------------
 * Engine.hpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/06/20 16:28:28
 * ---------------------------------------------------
 */

#ifndef ENGINE_HPP
# define ENGINE_HPP

#include "UtilsCPP/UniquePtr.hpp"

namespace GE
{ 
    class Game;

    namespace Engine
    {    
        void init();

        void runGame(utils::UniquePtr<Game>&&);
        void terminateGame();

        void terminate();
    }
}

#endif // ENGINE_HPP