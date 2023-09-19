#pragma once

#include "micro_behaviortree_cpp/tree_node.hpp"

namespace micro_behavior_tree_cpp
{

class AlwaysFailureNode : public SyncActionNode
{
public:
    AlwaysFailureNode(NodeConfig config) :
        SyncActionNode::SyncActionNode(config) {}

private:
    virtual NodeStatus tick() override
    {
        return NodeStatus::FAILURE;
    }
};

}