#pragma once
#include <vector>
#include <string>
#include <algorithm>

namespace Tangram {

// TODO
// - key builder for faster string comparison
// see also http://www.boost.org/doc/libs/1_59_0/doc/html/boost/container/flat_map.html
//

#if 0
template<typename K, typename T>
struct fastmap {
    std::vector<std::pair<K, T>> map;
    bool sorted;

    T& operator[](const K& key) {
        for (auto& a : map)
            if (key == a.first)
                return a.second;

        map.emplace_back(key, T{});

        return map.back().second;
        // auto& result = map.back().second;
        // std::sort(map.begin(), map.end())
        // return result;
    }
    void clear() { map.clear(); }
};
#endif

template<typename K, typename T>
struct fastmap {
    std::vector<std::pair<K, T>> map;
    bool sorted;

    T& operator[](const K& key) {
        for (auto& a : map)
            if (key == a.first)
                return a.second;

        map.emplace_back(key, T{});

        return map.back().second;
        // auto& result = map.back().second;
        // std::sort(map.begin(), map.end())
        // return result;
    }
    auto begin() { return map.begin(); }
    auto end() { return map.end(); }

    auto begin() const { return map.begin(); }
    auto end() const { return map.end(); }

    void clear() { map.clear(); }
};

// TODO check out
// https://realm.io/assets/news/binary-search/blog.cpp

template<typename T>
struct fastmap<std::string, T> {
    struct Key {
        size_t hash;
        std::string k;
    };

    std::vector<std::pair<Key, T>> map;
    std::vector<size_t> lengths;

    T& operator[](const std::string& key) {

        size_t hash = key.size();
        const auto it = std::lower_bound(
            map.begin(), map.end(), key,
            [&](const auto& item, const auto& key) {
                if (item.first.hash == hash) {
                    return item.first.k < key;
                }
                return item.first.hash < hash;
            });

        if (it == map.end() || it->first.k != key) {
            auto entry = map.emplace(it, Key{hash, key}, T{});
            return entry->second;
        }

        return it->second;
    }

    auto find(const std::string& key) const {
        size_t hash = key.size();
        const auto it = std::lower_bound(
            map.begin(), map.end(), key,
            [&](const auto& item, const auto& key) {
                if (item.first.hash == hash) {
                    return item.first.k < key;
                }
                return item.first.hash < hash;
            });

        if (it == map.end() || it->first.k == key) {
            return it;
        }
        return map.end();
    }
    void clear() { map.clear(); }

    auto begin() { return map.begin(); }
    auto end() { return map.end(); }

    auto begin() const { return map.begin(); }
    auto end() const { return map.end(); }

};

}
