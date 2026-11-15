#pragma once

#include <imgui.h>

namespace GE_Editor::Icons
{
using RenderFunction = void (*)(ImVec2 center, float size, ImU32 color, float stroke);

void move(ImVec2 center, float size, ImU32 color, float stroke);
void rotate(ImVec2 center, float size, ImU32 color, float stroke);
void scale(ImVec2 center, float size, ImU32 color, float stroke);
void axes(ImVec2 center, float size, ImU32 color, float stroke);
void world(ImVec2 center, float size, ImU32 color, float stroke);
void camera(ImVec2 center, float size, ImU32 color, float stroke);
void light(ImVec2 center, float size, ImU32 color, float stroke);
void script(ImVec2 center, float size, ImU32 color, float stroke);
void mesh(ImVec2 center, float size, ImU32 color, float stroke);
void folder(ImVec2 center, float size, ImU32 color, float stroke);
void file(ImVec2 center, float size, ImU32 color, float stroke);
void more(ImVec2 center, float spacing, float radius, ImU32 color);
} // namespace GE_Editor::Icons
