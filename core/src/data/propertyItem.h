#pragma once

#include "util/variant.h"

namespace Tangram {

struct PropertyItem {
    PropertyItem(std::string _key, Value _value) :
        key(std::move(_key)), value(std::move(_value)) {}

    PropertyItem(PropertyItem&&) noexcept = default;
    PropertyItem(const PropertyItem&) = default;

    PropertyItem& operator=(PropertyItem&& _other) noexcept {
        key = std::move(_other.key);
        value = std::move(_other.value);
        return *this;
    };
    PropertyItem& operator=(const PropertyItem&) = default;

    std::string key;
    Value value;
    bool operator<(const PropertyItem& _rhs) const { return key < _rhs.key; }
};

}
