namespace utils
{

template<typename T>
Storage<T>::Key Storage<T>::add(const T& value)
{
    return add(T(value));
}

template<typename T>
Storage<T>::Key Storage<T>::add(T&& value)
{
    Key key = idGenerator_.generateNewId();

    values_.emplace_back(key, std::move(value));
    keyToIndex_[key] = values_.size() - 1;

    return key;
}

template<typename T>
template<typename... Args>
Storage<T>::Key Storage<T>::emplace(Args&&... args)
{
    return add(T(std::forward<Args>(args)...));
}


template<typename T>
void Storage<T>::remove(Key key)
{
    size_t index = keyToIndex_[key];

    keyToIndex_[values_.back().key] = index;
    keyToIndex_.erase(key);

    std::swap(values_[index], values_.back());
    values_.pop_back();

    idGenerator_.removeId(key);
}

template<typename T>
T& Storage<T>::get(Key key)
{
    return values_[keyToIndex_.at(key)].value;
}

template<typename T>
const T& Storage<T>::get(Key key) const
{
    return values_[keyToIndex_.at(key)].value;
}

template<typename T>
std::span<typename Storage<T>::Value> Storage<T>::getValues()
{
    return values_;
}

template<typename T>
std::span<const typename Storage<T>::Value> Storage<T>::getValues() const
{
    return values_;
}


};