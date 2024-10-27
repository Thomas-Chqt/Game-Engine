/*
 * ---------------------------------------------------
 * ScriptLib.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/10/24 21:09:33
 * ---------------------------------------------------
 */

#include "ScriptLib.hpp"
#include "Script.hpp"
#include "UtilsCPP/Array.hpp"

extern "C" void getScriptNames(char*** names, unsigned long* count)
{
    static utils::Array<char*> nameArray;
    nameArray.clear();
    nameArray.setCapacity(ScriptRegistry::getRegistery().size() + 1);
    for (auto& [name, _] : ScriptRegistry::getRegistery())
        nameArray.append((char*)name);
    nameArray.append(nullptr);
    *names = &nameArray[0];
    *count = ScriptRegistry::getRegistery().size();
}

extern "C" GE::Script* makeScriptInstance(char* name)
{
    return ScriptRegistry::getRegistery()[name]();
}