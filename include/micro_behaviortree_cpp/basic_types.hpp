#pragma once

#include <memory>
#include <vector>
#include <string>
#include <string_view>

namespace micro_behavior_tree_cpp
{

/// Enumerates the possible types of nodes
enum class NodeType
{
    UNDEFINED = 0,
    ACTION,
    CONDITION,
    CONTROL,
    DECORATOR,
    SUBTREE
};

/// Enumerates the states every node can be in after execution during a particular
/// time step.
/// IMPORTANT: Your custom nodes should NEVER return IDLE.
enum class NodeStatus
{
    IDLE = 0,
    RUNNING,
    SUCCESS,
    FAILURE
};

inline bool StatusCompleted(const NodeStatus& status)
{
    return status == NodeStatus::SUCCESS || status == NodeStatus::FAILURE;
}

std::vector<std::string_view> splitString(const std::string_view& strToSplit, char delimeter)
{
    std::vector<std::string_view> splitted_strings;
    splitted_strings.reserve(4);

    size_t pos = 0;
    while (pos < strToSplit.size())
    {
        size_t new_pos = strToSplit.find_first_of(delimeter, pos);
        if (new_pos == std::string::npos)
        {
        new_pos = strToSplit.size();
        }
        std::string_view sv = {&strToSplit.data()[pos], new_pos - pos};
        splitted_strings.push_back(sv);
        pos = new_pos + 1;
    }
    return splitted_strings;
}

template <typename T>
inline bool convertFromString(std::string_view str, T& value)
{
    return false;
}

template <>
bool convertFromString<std::string>(std::string_view str, std::string& value)
{
    value = std::string(str.data(), str.size());
    return true;
}
template <>
bool convertFromString<int>(std::string_view str, int& value)
{
    value = std::stoi(str.data());
    return true;
}

template <>
bool convertFromString<long>(std::string_view str, long& value)
{
    value = std::stol(str.data());
    return true;
}

template <>
bool convertFromString<unsigned>(std::string_view str, unsigned& value)
{
    value = std::stoul(str.data());
    return true;
}

template <>
bool convertFromString<unsigned long>(std::string_view str, unsigned long& value)
{
    value = std::stoul(str.data());
    return true;
}

template <>
bool convertFromString<double>(std::string_view str, double& value)
{
    value = std::stod(str.data());
    return true;
}

template <>
bool convertFromString<float>(std::string_view str, float& value)
{
    value = std::stof(str.data());
    return true;
}

template <>
bool convertFromString<std::vector<int>>(std::string_view str, std::vector<int>& value)
{
    auto parts = splitString(str, ';');
    value.clear();
    value.reserve(parts.size());
    for (const std::string_view& part : parts)
    {
        char* end;
        value.push_back(std::strtol(part.data(), &end, 10));
    }
    return true;
}

template <>
bool convertFromString<std::vector<double>>(std::string_view str, std::vector<double>& value)
{
    auto parts = splitString(str, ';');
    value.clear();
    value.reserve(parts.size());
    for (const std::string_view& part : parts)
    {
        char* end;
        value.push_back(std::strtod(part.data(), &end));
    }
    return true;
}

template <>
bool convertFromString<bool>(std::string_view str, bool& value)
{
    if (str.size() == 1)
    {
        if (str[0] == '0')
        {
            value = false;
            return true;
        }
        if (str[0] == '1')
        {
            value = true;
            return true;
        }
    }
    else if (str.size() == 4)
    {
        if (str == "true" || str == "TRUE" || str == "True")
        {
            value = true;
            return true;
        }
    }
    else if (str.size() == 5)
    {
        if (str == "false" || str == "FALSE" || str == "False")
        {
            value = false;
            return true;
        }
    }
    return false; // error
}

template <>
bool convertFromString<NodeStatus>(std::string_view str, NodeStatus& value)
{
    if (str == "IDLE")
    {
        value = NodeStatus::IDLE;
        return true;
    }
    if (str == "RUNNING")
    {
        value = NodeStatus::RUNNING;
        return true;
    }
    if (str == "SUCCESS")
    {
        value = NodeStatus::SUCCESS;
        return true;
    }
    if (str == "FAILURE")
    {
        value = NodeStatus::FAILURE;
        return true;
    }
    return false; // error
}

template <typename T>
std::string toStr(T value)
{
    return std::to_string(value);
}

template <>
std::string toStr<NodeStatus>(NodeStatus status)
{
    switch (status)
    {
        case NodeStatus::SUCCESS:
            return "SUCCESS";
        case NodeStatus::FAILURE:
            return "FAILURE";
        case NodeStatus::RUNNING:
            return "RUNNING";
        case NodeStatus::IDLE:
            return "IDLE";
    }
    return "";
}

std::string toStr(std::string value)
{
    return value;
}

}