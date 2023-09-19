#pragma once

#include "micro_behaviortree_cpp/tree_node.hpp"

namespace micro_behavior_tree_cpp
{

class ForceFailureNode : public DecoratorNode
{
public:
    ForceFailureNode(NodeConfig config) :
        DecoratorNode::DecoratorNode(config) {}

private:
    virtual NodeStatus tick() override
    {
        setStatus(NodeStatus::RUNNING);

        const NodeStatus child_status = child_node_->executeTick();

        if(StatusCompleted(child_status))
        {
            resetChild();
            return NodeStatus::FAILURE;
        }

        // RUNNING
        return child_status;
    }
};

}