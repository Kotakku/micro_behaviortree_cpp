#pragma once

#include "micro_behaviortree_cpp/tree_node.hpp"

namespace micro_behavior_tree_cpp
{

class KeepRunningUntilFailureNode : public DecoratorNode
{
public:
    KeepRunningUntilFailureNode(NodeConfig config) :
        DecoratorNode::DecoratorNode(config) {}

private:
    virtual NodeStatus tick() override
    {
        setStatus(NodeStatus::RUNNING);

        const NodeStatus child_state = child_node_->executeTick();

        switch (child_state)
        {
            case NodeStatus::FAILURE: {
                resetChild();
                return NodeStatus::FAILURE;
            }
            case NodeStatus::SUCCESS: {
                resetChild();
                return NodeStatus::RUNNING;
            }
            case NodeStatus::RUNNING: {
                return NodeStatus::RUNNING;
            }

            default: {
                return NodeStatus::FAILURE;
            }
        }
        return status();
    }
};

}