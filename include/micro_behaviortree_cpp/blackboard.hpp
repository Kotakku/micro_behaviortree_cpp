#pragma once

#include <memory>
#include <vector>
#include <string>
#include <unordered_map>
#include <any>
#include <cassert>
#include "basic_types.hpp"

namespace micro_behavior_tree_cpp
{

class Blackboard
{
public:
    using SharedPtr = std::shared_ptr<Blackboard>;

    Blackboard() = default;

    template <typename T>
    bool get(const std::string& key, T& value)
    {
        if(storage_.empty())
        {
            return false;
        }
        auto it = storage_.find(key);
        if (it == storage_.end())
        {
            return false;
        }

        if(it->second.type() == typeid(T))
        {
            value = std::any_cast<T>(it->second);
            return true;
        }

        if(it->second.type() == typeid(std::string))
        {
            // try to convert to the desired type
            std::string str_value = std::any_cast<std::string>(it->second);
            return convertFromString(str_value, value);
        }
        return false;
    }

    template <typename T>
    void set(const std::string key, const T& value)
    {
        std::any any_value = value;
        storage_[key] = any_value;
    }

    std::unordered_map<std::string, std::any> getStorage() const
    {
        return storage_;
    }
    
private:
    std::unordered_map<std::string, std::any> storage_ = {};
};


}