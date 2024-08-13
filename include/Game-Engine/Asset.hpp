/*
 * ---------------------------------------------------
 * Asset.hpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/08/13 12:10:41
 * ---------------------------------------------------
 */

#ifndef MANAGEDASSET_HPP
#define MANAGEDASSET_HPP

#include "UtilsCPP/SharedPtr.hpp"

namespace GE
{

class Mesh;

template<typename T> class Asset;

template<>
class Asset<Mesh>
{
public:
    Asset() = default;
    Asset(const Asset<Mesh>&);
    Asset(Asset<Mesh>&&);

    Asset(const utils::SharedPtr<Mesh>&);
    
    inline operator utils::SharedPtr<Mesh>& () { return m_ptr; }

    ~Asset();

private:
    utils::SharedPtr<Mesh> m_ptr;
};

}

#endif // MANAGEDASSET_HPP