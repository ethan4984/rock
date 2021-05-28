#ifndef MAP_HPP_
#define MAP_HPP_

#include <vector.hpp>

namespace lib {

template <typename T>
class map {
public:
    map(const map &other);
    map(map &&map);
    map();

    void swap(map &a, map &b);

    map &operator= (map other);
    T &operator[] (size_t index);

    size_t insert(const T &data);
    size_t insert(T &&data);

    void remove(size_t index);
private:
    size_t _length;
    size_t _hash_index;
    T default_value;

    vector<size_t> _tag;
    vector<T> _data;
};

template <typename T>
map<T>::map(const map &other) {
    _length = other._length;
    _hash_index = other._hash_index;
    _tag = other._tag; 
    _data = other._data;
}

template <typename T>
map<T>::map(map &&other) {
    swap(*this, other);
}

template <typename T>
map<T>::map() : _length(0), _hash_index(0) { }

template <typename T>
map<T> &map<T>::operator= (map other) {
    swap(*this, other);
    return *this;
}

template <typename T> 
T &map<T>::operator[] (size_t index) {
    default_value = T();
    for(size_t i = 0; i < _tag.size(); i++) {
        if(_tag[i] == index)
            return _data[i];
    }
    return default_value;
}

template <typename T>
size_t map<T>::insert(const T &data) {
    _data.push(data);
    _tag.push(_hash_index);
    return _hash_index++;
}

template <typename T>
size_t map<T>::insert(T &&data) { 
    return insert(data);
}

template <typename T>
void map<T>::remove(size_t index) {
    for(size_t i = 0; i < _tag.size(); i++) {
        if(_tag[i] == index) {
            _data.remove(i);
            _tag.remove(i);
        }
    }
}

template <typename T>
void map<T>::swap(map &a, map &b) {
    std::swap(a._length, b.length);
    std::swap(a._hash_index, b._hash_index);
    std::swap(a._tag, b._tag);
    std::swap(a._data, b._data);
}

}

#endif
