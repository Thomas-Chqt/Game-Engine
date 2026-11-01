#pragma once

#include "Game-Engine/Components.hpp"

#include <algorithm>
#include <cassert>
#include <memory>
#include <vector>
#include <utility>

namespace GE
{

class AComponentVector
{
public:
    virtual size_t size() const = 0;
    virtual void reserveForAppend() = 0;

    // same type, copy the elements
    virtual std::unique_ptr<AComponentVector> clone() const = 0;

    // same type, new empty
    virtual std::unique_ptr<AComponentVector> create() const = 0;

    virtual void swapComponents(uint64_t idx1, uint64_t idx2) = 0;
    virtual void eraseLast() = 0;

    // move 1 component at the end of dst and erase the src by swaping it with the last element
    // dst must be the same type
    virtual void moveComponent(uint64_t idx, AComponentVector& dst) = 0;

    virtual ~AComponentVector() = default;
};

template<Component T>
class ComponentVector : public AComponentVector
{
public:
    ComponentVector() = default;
    ComponentVector(const ComponentVector&) = default;
    ComponentVector(ComponentVector&&) = default;

    size_t size() const override;
    void reserveForAppend() override;
    std::unique_ptr<AComponentVector> clone() const override;
    std::unique_ptr<AComponentVector> create() const override;

    void swapComponents(uint64_t idx1, uint64_t idx2) override;
    void eraseLast() override;

    void moveComponent(uint64_t idx, AComponentVector& dst) override;

    std::vector<T>& vector() { return m_vector; }
    const std::vector<T>& vector() const { return m_vector; }

    ~ComponentVector() override = default;

private:
    std::vector<T> m_vector;

public:
    ComponentVector& operator = (const ComponentVector&) = default;
    ComponentVector& operator = (ComponentVector&&) = default;
};

template<Component T>
size_t ComponentVector<T>::size() const
{
    return m_vector.size();
}

template<Component T>
void ComponentVector<T>::reserveForAppend()
{
    if (m_vector.size() == m_vector.capacity()) {
        // Preserve geometric growth rather than reallocating for every transfer.
        const auto size = m_vector.size();
        const auto growth = size == 0 ? 1 : std::min(size, m_vector.max_size() - size);
        m_vector.reserve(size + (growth == 0 ? 1 : growth));
    }
}

template<Component T>
std::unique_ptr<AComponentVector> ComponentVector<T>::clone() const
{
    return std::make_unique<ComponentVector<T>>(*this);
}

template<Component T>
std::unique_ptr<AComponentVector> ComponentVector<T>::create() const
{
    return std::make_unique<ComponentVector<T>>();
}

template<Component T>
void ComponentVector<T>::swapComponents(uint64_t idx1, uint64_t idx2)
{
    assert(idx1 < size() && idx2 < size());
    if (idx1 == idx2)
        return;
    T temporary(std::move(m_vector[idx1]));
    std::destroy_at(&m_vector[idx1]);
    std::construct_at(&m_vector[idx1], std::move(m_vector[idx2]));
    std::destroy_at(&m_vector[idx2]);
    std::construct_at(&m_vector[idx2], std::move(temporary));
}

template<Component T>
void ComponentVector<T>::eraseLast()
{
    assert(!m_vector.empty());
    m_vector.pop_back();
}

template<Component T>
void ComponentVector<T>::moveComponent(uint64_t idx, AComponentVector& aDst)
{
    assert(typeid(aDst) == typeid(ComponentVector<T>));
    auto* dst = static_cast<ComponentVector<T>*>(&aDst);
    assert(dst != this);
    assert(idx < size());
    dst->vector().emplace_back(std::move(m_vector.at(idx)));
    swapComponents(idx, size() - 1);
    m_vector.pop_back();
}

}
