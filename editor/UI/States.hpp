/*
 * ---------------------------------------------------
 * States.hpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * ---------------------------------------------------
 */

#ifndef UI_STATES_HPP
#define UI_STATES_HPP

#include <filesystem>
#include <memory>

namespace GE_Editor::UI
{

struct States
{
    static inline std::unique_ptr<bool> projectPropertiesOpen;
    static inline std::unique_ptr<std::filesystem::path> fileBrowserSubDirectory;

    States()
    {
        projectPropertiesOpen = std::make_unique<bool>(false);
        fileBrowserSubDirectory = std::make_unique<std::filesystem::path>();
    }

    ~States()
    {
        fileBrowserSubDirectory.reset();
        projectPropertiesOpen.reset();
    }
};

} // namespace GE_Editor::UI

#endif // UI_STATES_HPP
