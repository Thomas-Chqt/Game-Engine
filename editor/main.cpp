#include "Editor.hpp"

int main(int argc, const char* argv[])
{
    GE_Editor::Editor(std::span<const char*>{argv, static_cast<size_t>(argc)}).run();
}
