#pragma once

#include "micro_behaviortree_cpp/tree_node.hpp"

namespace micro_behavior_tree_cpp
{

class InverterNode : public DecoratorNode
{
public:
    InverterNode(NodeConfig config) :
        DecoratorNode::DecoratorNode(config) {}

private:
    virtual NodeStatus tick() override
    {
        setStatus(NodeStatus::RUNNING);
        const NodeStatus child_status = child_node_->executeTick();

        switch (child_status)
        {
            case NodeStatus::SUCCESS: {
                resetChild();
                return NodeStatus::FAILURE;
            }

            case NodeStatus::FAILURE: {
                resetChild();
                return NodeStatus::SUCCESS;
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