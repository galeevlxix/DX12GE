#pragma once
#include <unordered_map>
#include <any>
#include <stdexcept>
#include <string>

class Blackboard {
private:
    std::unordered_map<std::string, std::any> data;

public:
    template <typename T>
    void Set(const std::string& key, const T& value) {
        data[key] = value;
    }

    template <typename T>
    T Get(const std::string& key) const {
        auto it = data.find(key);
        if (it != data.end()) {
            return std::any_cast<T>(it->second);
        }
        throw std::runtime_error("Key not found: " + key);
    }

    bool Has(const std::string& key) const {
        return data.find(key) != data.end();
    }

    void Remove(const std::string& key) {
        data.erase(key);
    }

    Blackboard* Clone() const {
        auto clone = new Blackboard();
        clone->data = data;
        return clone;
    }
};