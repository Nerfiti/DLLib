#include "IdGenerator.h"

#include <iostream>
#include <limits>
#include <stdexcept>

namespace utils
{

IdGenerator::IdGenerator (): IdGenerator (std::numeric_limits<id_t>::max()) {}

IdGenerator::IdGenerator (id_t idCount): invalidId_(idCount), nextId_(0), freeIds_() {}

IdGenerator::id_t IdGenerator::generateNewId ()
{
    if (nextId_ == invalidId_)
    {
        throw std::runtime_error("Generator is out of ids");
    }

    if (freeIds_.empty())
    {
        return nextId_++;
    }
    else
    {
        return freeIds_.extract(freeIds_.begin()).value();
    }
}

void IdGenerator::removeId(id_t id)
{
    if (id >= nextId_ || freeIds_.contains(id))
    {
        std::cerr << "Can't remove id " << id << " from generator\n";
        return;
    }
    if (id == nextId_ - 1)
    {
        --nextId_;
        return;
    }
    
    freeIds_.insert(id);
}

}