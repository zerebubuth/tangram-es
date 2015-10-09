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

    std::vector<std::pair<std::string, T>> map;
    std::vector<size_t> lengths;

    T& operator[](const std::string& key) {

        const auto it = std::lower_bound(
            map.begin(), map.end(), key,
            [](const auto& item, const auto& key) {
                int d = item.first.size() - key.size();
                if (d == 0) {
                    return item.first < key;
                }
                return d < 0;
            });

        if (it == map.end() || it->first != key) {
            auto entry = map.emplace(it, key, T{});
            return entry->second;
        }

        return it->second;
    }

    auto find(const std::string& key) const {
        const auto it = std::lower_bound(
            map.begin(), map.end(), key,
            [](const auto& item, const auto& key) {
                int d = item.first.size() - key.size();
                if (d == 0) {
                    return item.first < key;
                }
                return d < 0;
            });

        if (it == map.end() || it->first == key) {
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
