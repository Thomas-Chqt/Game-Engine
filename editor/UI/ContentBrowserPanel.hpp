/*
 * ---------------------------------------------------
 * ContentBrowserPanel.hpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * ---------------------------------------------------
 */

#ifndef CONTENTBROWSERPANEL_HPP
#define CONTENTBROWSERPANEL_HPP

#include <imgui.h>

#include <cstddef>
#include <ranges>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace GE_Editor
{

template<typename T>
concept ContentBrowserElementRange = std::ranges::range<T> && std::is_same_v<std::ranges::range_value_t<T>, std::pair<std::string, const void*>>;

class ContentBrowserPanel
{
public:
    ContentBrowserPanel(const std::string& name, const char* payloadType, std::size_t payloadSize)
        : m_name(name)
        , m_payloadType(payloadType)
        , m_payloadSize(payloadSize)
    {
    }

    void render(const ContentBrowserElementRange auto& elements)
    {
        if (ImGui::Begin(m_name.c_str()))
        {
            for (auto [name, payload] : elements)
                renderElement(name, payload);
        }
        ImGui::End();
    }

private:
    void renderElement(const std::string& name, const void* payload);

    std::string m_name;
    const char* m_payloadType;
    std::size_t m_payloadSize;
    std::vector<std::pair<std::string, const void*>> m_elements;

    float m_lineWith = 0.0F;
};

}

#endif
