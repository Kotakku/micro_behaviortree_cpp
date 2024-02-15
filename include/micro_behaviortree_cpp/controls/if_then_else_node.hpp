#pragma once

#include "micro_behaviortree_cpp/tree_node.hpp"

namespace micro_behavior_tree_cpp
{

class IfThenElseNode : public ControlNode
{
public:
    IfThenElseNode(NodeConfig config) :
        ControlNode::ControlNode(config) {}

private:
    virtual NodeStatus tick() override
    {
        const size_t children_count = children_nodes_.size();

        assert(children_count == 2 || children_count == 3);

        setStatus(NodeStatus::RUNNING);

        if (child_idx_ == 0)
        {
            NodeStatus condition_status = children_nodes_[0]->executeTick();

            if (condition_status == NodeStatus::RUNNING)
            {
                return condition_status;
            }
            else if (condition_status == NodeStatus::SUCCESS)
            {
                child_idx_ = 1;
            }
            else if (condition_status == NodeStatus::FAILURE)
            {
                if (children_count == 3)
                {
                    child_idx_ = 2;
                }
                else
                {
                    return condition_status;
                }
            }
        }
        // not an else
        if (child_idx_ > 0)
        {
            NodeStatus status = children_nodes_[child_idx_]->executeTick();
            if (status == NodeStatus::RUNNING)
            {
                return NodeStatus::RUNNING;
            }
            else
            {
                resetChildren();
                child_idx_ = 0;
                return status;
            }
        }
        return NodeStatus::FAILURE;
    }

    virtual void halt() override
    {
        child_idx_ = 0;
        ControlNode::halt();
    }

    size_t child_idx_ = 0;
};

}
