/*
 * ---------------------------------------------------
 * Archetype.hpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/07/20 19:00:39
 * ---------------------------------------------------
 */

#ifndef ARCHETYPE_HPP
# define ARCHETYPE_HPP

#include "UtilsCPP/Func.hpp"
#include "UtilsCPP/Types.hpp"

namespace GE
{

class Archetype
{
public:
    using ID = utils::uint64;
    using Index = utils::uint64;

public:
    Archetype()                 = default;
    Archetype(const Archetype&) = delete;
    Archetype(Archetype&&)      = delete;
    

    ~Archetype() = default;

private:
    
public:
    Archetype& operator = (const Archetype&) = delete;
    Archetype& operator = (Archetype&&)      = delete;
};

}

#endif // ARCHETYPE_HPP