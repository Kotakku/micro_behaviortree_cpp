#pragma once

#include "micro_behaviortree_cpp/tree_node.hpp"

namespace micro_behavior_tree_cpp
{

class WhileDoElseNode : public ControlNode
{
public:
    WhileDoElseNode(NodeConfig config) :
        ControlNode::ControlNode(config) {}

private:
    virtual NodeStatus tick() override
    {
        const size_t children_count = children_nodes_.size();

        if (children_count != 3)
        {
            return NodeStatus::FAILURE;
        }

        setStatus(NodeStatus::RUNNING);

        NodeStatus condition_status = children_nodes_[0]->executeTick();

        if (condition_status == NodeStatus::RUNNING)
        {
            return condition_status;
        }

        NodeStatus status = NodeStatus::IDLE;

        if (condition_status == NodeStatus::SUCCESS)
        {
            haltChild(2);
            status = children_nodes_[1]->executeTick();
        }
        else if (condition_status == NodeStatus::FAILURE)
        {
            haltChild(1);
            status = children_nodes_[2]->executeTick();
        }

        if (status == NodeStatus::RUNNING)
        {
            return NodeStatus::RUNNING;
        }
        else
        {
            resetChildren();
            return status;
        }
    }
};

}