/*
 * ---------------------------------------------------
 * MainMenuBar.hpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/10/28 18:39:45
 * ---------------------------------------------------
 */

#ifndef MAINMENUBAR_HPP
#define MAINMENUBAR_HPP

namespace GE
{

class MainMenuBar
{
public:
    MainMenuBar() = default;
    MainMenuBar(const MainMenuBar&) = default;
    MainMenuBar(MainMenuBar&&)      = default;
    

    ~MainMenuBar() = default;

private:
    
public:
    MainMenuBar& operator = (const MainMenuBar&) = default;
    MainMenuBar& operator = (MainMenuBar&&)      = default;
};

}

#endif // MAINMENUBAR_HPP