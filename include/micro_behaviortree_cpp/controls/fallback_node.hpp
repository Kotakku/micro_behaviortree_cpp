#pragma once

#include "micro_behaviortree_cpp/tree_node.hpp"

namespace micro_behavior_tree_cpp
{

class FallbackNode : public ControlNode
{
public:
    FallbackNode(NodeConfig config) :
        ControlNode::ControlNode(config) {}

private:
    virtual NodeStatus tick() override
    {
        const size_t children_count = children_nodes_.size();

        setStatus(NodeStatus::RUNNING);

        while (current_child_idx_ < children_count)
        {
            TreeNode::SharedPtr current_child_node = children_nodes_[current_child_idx_];
            const NodeStatus child_status = current_child_node->executeTick();

            switch (child_status)
            {
                case NodeStatus::RUNNING: {
                    return child_status;
                }
                case NodeStatus::SUCCESS: {
                    resetChildren();
                    current_child_idx_ = 0;
                    return child_status;
                }
                case NodeStatus::FAILURE: {
                    current_child_idx_++;
                }
                break;

                case NodeStatus::IDLE: {
                    return NodeStatus::FAILURE;
                }
            }   // end switch
        }     // end while loop

        // The entire while loop completed. This means that all the children returned FAILURE.
        if (current_child_idx_ == children_count)
        {
            resetChildren();
            current_child_idx_ = 0;
        }

        return NodeStatus::FAILURE;
    }

    virtual void halt() override
    {
        current_child_idx_ = 0;
        ControlNode::halt();
    }

    size_t current_child_idx_ = 0;
};

}