#pragma once

#include "Game-Engine/EntityID.hpp"

#include <iterator>
#include <unordered_map>
#include <utility>

namespace GE
{

template<typename T>
class ECSIterator
{
public:
    using iterator_concept = std::input_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = EntityID;

private:
    std::unordered_map<EntityID, T>::const_iterator m_iterator;

public:
    ECSIterator() = default;
    ECSIterator(const ECSIterator&) = default;
    ECSIterator(ECSIterator&&) = default;

    ECSIterator(std::unordered_map<EntityID, T>::const_iterator);

    ~ECSIterator() = default;

public:
    ECSIterator& operator=(const ECSIterator&) = default;
    ECSIterator& operator=(ECSIterator&&) = default;

    EntityID operator*() const;

    ECSIterator& operator++();
    ECSIterator operator++(int);

    bool operator==(const ECSIterator&) const = default;
};

template<typename T>
ECSIterator<T>::ECSIterator(std::unordered_map<EntityID, T>::const_iterator iterator)
    : m_iterator(std::move(iterator))
{
}

template<typename T>
EntityID ECSIterator<T>::operator*() const
{
    return m_iterator->first;
}

template<typename T>
ECSIterator<T>& ECSIterator<T>::operator++()
{
    ++m_iterator;
    return *this;
}

template<typename T>
ECSIterator<T> ECSIterator<T>::operator++(int)
{
    ECSIterator previous = *this;
    ++(*this);
    return previous;
}

}
