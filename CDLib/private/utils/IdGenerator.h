#pragma once

#include <unordered_set>

namespace utils
{


class IdGenerator final
{

public:
    using id_t = uint64_t;

public:
    IdGenerator ();
    IdGenerator (id_t idCount);

    id_t generateNewId ();
    void removeId (id_t id);

private:
    const id_t invalidId_;

    id_t nextId_;
    std::unordered_set<id_t> freeIds_;
};


};