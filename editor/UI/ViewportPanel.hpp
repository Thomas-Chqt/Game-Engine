/*
 * ---------------------------------------------------
 * ViewportPanel.hpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/11/01 16:20:17
 * ---------------------------------------------------
 */

#ifndef VIEWPORTPANEL_HPP
#define VIEWPORTPANEL_HPP

#include <imgui.h>

#include <cstdint>
#include <functional>
#include <utility>

namespace GE_Editor
{

class ViewportPanel
{
public:
    ViewportPanel() = delete;
    ViewportPanel(const ViewportPanel&) = delete;
    ViewportPanel(ViewportPanel&&) = delete;

    ViewportPanel(std::pair<uint32_t, uint32_t> size);

    inline ViewportPanel& onResize(std::function<void(const std::pair<uint32_t, uint32_t>&)>&& f) { return m_onResize = std::move(f), *this; }

    void render();

    ~ViewportPanel() = default;

private:
    std::pair<uint32_t, uint32_t> m_size;
    std::function<void(const std::pair<uint32_t, uint32_t>&)> m_onResize;

public:
    ViewportPanel& operator=(const ViewportPanel&) = delete;
    ViewportPanel& operator=(ViewportPanel&&) = delete;
};

} // namespace GE

#endif // VIEWPORTPANEL_HPP
