#pragma once

#include "micro_behaviortree_cpp/tree_node.hpp"

namespace micro_behavior_tree_cpp
{

class SetBlackboard : public SyncActionNode
{
public:
    SetBlackboard(NodeConfig config) :
        SyncActionNode::SyncActionNode(config) {}

private:
    virtual NodeStatus tick() override
    {
        std::string key, value;
        if (!getInput("output_key", key))
        {
            return NodeStatus::FAILURE;
        }
        if (!getInput("value", value))
        {
            return NodeStatus::FAILURE;
        }

        setOutput(key, value);
        return NodeStatus::SUCCESS;
    }
};

}