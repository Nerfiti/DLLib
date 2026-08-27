#pragma once

#include <utils/IdGenerator.h>

#include <span>
#include <unordered_map>
#include <vector>


namespace utils
{

template<typename T>
class Storage final
{

public:
    using Key = IdGenerator::id_t;
    struct Value final
    {
        Key key;
        T value;
    };

public:

    Key add(const T& value);
    Key add(T&& value);

    template<typename... Args>
    Key emplace(Args&&... args);
    
    void remove(Key key);

    T& get(Key key);
    const T& get(Key key) const;

    std::span<Value> getValues();
    std::span<const Value> getValues() const;

private:
    
    std::vector<Value> values_;

    std::unordered_map<Key, size_t> keyToIndex_;

    IdGenerator idGenerator_;
};

};

#include "Storage.inl"