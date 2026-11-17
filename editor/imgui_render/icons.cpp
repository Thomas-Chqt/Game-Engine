#include "icons.hpp"

#include <array>
#include <cmath>
#include <numbers>

namespace GE_Editor::Icons
{

namespace
{

struct Canvas
{
    ImVec2 center;
    float size{};
    ImU32 color{};
    float stroke{};
    ImDrawList* draw = ImGui::GetWindowDrawList();

    ImVec2 p(float x, float y) const { return {center.x+(x-12)*size/24, center.y+(y-12)*size/24}; }
    void line(float x, float y, float a, float b) const { draw->AddLine(p(x,y), p(a,b), color, stroke); }
};

}

void move(ImVec2 center, float size, ImU32 color, float stroke)
{
    const Canvas c{.center=center, .size=size, .color=color, .stroke=stroke};
    c.line(5,12,19,12); c.line(12,5,12,19);
    c.draw->AddTriangleFilled(c.p(2,12), c.p(6,9), c.p(6,15), color);
    c.draw->AddTriangleFilled(c.p(22,12), c.p(18,9), c.p(18,15), color);
    c.draw->AddTriangleFilled(c.p(12,2), c.p(9,6), c.p(15,6), color);
    c.draw->AddTriangleFilled(c.p(12,22), c.p(9,18), c.p(15,18), color);
}

void rotate(ImVec2 center, float size, ImU32 color, float stroke)
{
    const Canvas c{.center=center, .size=size, .color=color, .stroke=stroke};
    c.draw->PathArcTo(c.p(12,12), size*8.5f/24, .15f*std::numbers::pi_v<float>, 1.85f*std::numbers::pi_v<float>, 32);
    c.draw->PathStroke(color, 0, stroke);
    const std::array<ImVec2, 3> head = {c.p(15,9), c.p(20,9), c.p(20,4)};
    c.draw->AddPolyline(head.data(), head.size(), color, 0, stroke);
}

void scale(ImVec2 center, float size, ImU32 color, float stroke)
{
    const Canvas c{.center=center, .size=size, .color=color, .stroke=stroke};
    c.draw->AddRect(c.p(3,15), c.p(9,21), color, 0, 0, stroke);
    c.line(9,15,20,4);
    const std::array<ImVec2, 3> head = {c.p(13,4), c.p(20,4), c.p(20,11)};
    c.draw->AddPolyline(head.data(), head.size(), color, 0, stroke);
}

void axes(ImVec2 center, float size, ImU32 color, float stroke)
{
    const Canvas c{.center=center, .size=size, .color=color, .stroke=stroke};
    c.line(7,17,7,5); c.line(7,17,19,17); c.line(7,17,16,8);
    c.draw->AddTriangleFilled(c.p(7,2), c.p(4.5f,6), c.p(9.5f,6), color);
    c.draw->AddTriangleFilled(c.p(22,17), c.p(18,14.5f), c.p(18,19.5f), color);
    c.draw->AddTriangleFilled(c.p(18,6), c.p(13.5f,7), c.p(17,10.5f), color);
}

void world(ImVec2 center, float size, ImU32 color, float stroke)
{
    const Canvas c{.center=center, .size=size, .color=color, .stroke=stroke};
    c.draw->AddCircle(c.p(12,12), size*9/24, color, 32, stroke);
    for (int i=0; i<32; ++i) {
        const float a = float(i)*6.2831853f/32;
        c.draw->PathLineTo(c.p(12+std::cos(a)*4.5f, 12+std::sin(a)*9));
    }
    c.draw->PathStroke(color, ImDrawFlags_Closed, stroke);
    c.line(3,12,21,12);
}

void camera(ImVec2 center, float size, ImU32 color, float stroke)
{
    const Canvas c{.center=center, .size=size, .color=color, .stroke=stroke};
    c.draw->AddRect(c.p(2,6), c.p(15,18), color, size/24, 0, stroke);
    c.draw->PathLineTo(c.p(15,10)); c.draw->PathLineTo(c.p(22,6));
    c.draw->PathLineTo(c.p(22,18)); c.draw->PathLineTo(c.p(15,14));
    c.draw->PathStroke(color, ImDrawFlags_Closed, stroke);
}

void light(ImVec2 center, float size, ImU32 color, float stroke)
{
    const Canvas c{.center=center, .size=size, .color=color, .stroke=stroke};
    c.draw->PathLineTo(c.p(9,17));
    c.draw->PathBezierCubicCurveTo(c.p(9,14), c.p(5,14), c.p(5,9));
    c.draw->PathBezierCubicCurveTo(c.p(5,5), c.p(8,2), c.p(12,2));
    c.draw->PathBezierCubicCurveTo(c.p(16,2), c.p(19,5), c.p(19,9));
    c.draw->PathBezierCubicCurveTo(c.p(19,14), c.p(15,14), c.p(15,17));
    c.draw->PathStroke(color, ImDrawFlags_Closed, stroke);
    c.line(10,20,14,20);
}

void script(ImVec2 center, float size, ImU32 color, float stroke)
{
    const Canvas c{.center=center, .size=size, .color=color, .stroke=stroke};
    c.draw->PathLineTo(c.p(7,7)); c.draw->PathLineTo(c.p(3,12)); c.draw->PathLineTo(c.p(7,17));
    c.draw->PathStroke(color, 0, stroke);
    c.draw->PathLineTo(c.p(17,7)); c.draw->PathLineTo(c.p(21,12)); c.draw->PathLineTo(c.p(17,17));
    c.draw->PathStroke(color, 0, stroke);
    c.line(14,4,10,20);
}

void mesh(ImVec2 center, float size, ImU32 color, float stroke)
{
    const Canvas c{.center=center, .size=size, .color=color, .stroke=stroke};
    c.draw->PathLineTo(c.p(12,2)); c.draw->PathLineTo(c.p(21,7)); c.draw->PathLineTo(c.p(21,17));
    c.draw->PathLineTo(c.p(12,22)); c.draw->PathLineTo(c.p(3,17)); c.draw->PathLineTo(c.p(3,7));
    c.draw->PathStroke(color, ImDrawFlags_Closed, stroke);
    c.line(3,7,12,12); c.line(21,7,12,12); c.line(12,12,12,22);
}

void folder(ImVec2 center, float size, ImU32 color, float stroke)
{
    const auto p = [&](float x, float y) { return ImVec2(center.x+x*size, center.y+y*size); };
    auto* draw = ImGui::GetWindowDrawList();
    const std::array<ImVec2, 6> outline = {p(-.50f,-.33f), p(-.12f,-.33f), p(0,-.20f), p(.50f,-.20f), p(.50f,.36f), p(-.50f,.36f)};
    draw->AddPolyline(outline.data(), outline.size(), color, ImDrawFlags_Closed, stroke);
    draw->AddLine(p(-.50f,-.10f), p(.50f,-.10f), color, stroke);
}

void file(ImVec2 center, float size, ImU32 color, float stroke)
{
    const auto p = [&](float x, float y) { return ImVec2(center.x+x*size, center.y+y*size); };
    auto* draw = ImGui::GetWindowDrawList();
    const std::array<ImVec2, 5> outline = {p(-.32f,-.46f), p(.10f,-.46f), p(.34f,-.22f), p(.34f,.46f), p(-.32f,.46f)};
    draw->AddPolyline(outline.data(), outline.size(), color, ImDrawFlags_Closed, stroke);
    const std::array<ImVec2, 3> fold = {p(.10f,-.46f), p(.10f,-.22f), p(.34f,-.22f)};
    draw->AddPolyline(fold.data(), fold.size(), color, 0, stroke);
    draw->AddLine(p(-.18f,.07f), p(.18f,.07f), color, stroke);
    draw->AddLine(p(-.18f,.23f), p(.10f,.23f), color, stroke);
}

void more(ImVec2 center, float spacing, float radius, ImU32 color)
{
    for (int dot=-1; dot<=1; ++dot)
        ImGui::GetWindowDrawList()->AddCircleFilled(ImVec2(center.x, center.y+float(dot)*spacing), radius, color);
}
} // namespace GE_Editor::Icons
