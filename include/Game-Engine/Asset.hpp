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

class MeshAsset
{
public:
    MeshAsset() = default;
    MeshAsset(const MeshAsset&);
    MeshAsset(MeshAsset&&);

    MeshAsset(const utils::SharedPtr<Mesh>&);
    
    inline operator utils::SharedPtr<Mesh>& () { return m_ptr; }

    ~MeshAsset();

private:
    utils::SharedPtr<Mesh> m_ptr;
};

}

#endif // MANAGEDASSET_HPP