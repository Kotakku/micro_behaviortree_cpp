#pragma once

#include "micro_behaviortree_cpp/tree_node.hpp"

namespace micro_behavior_tree_cpp
{

class ReactiveFallback : public ControlNode
{
public:
    ReactiveFallback(NodeConfig config) :
        ControlNode::ControlNode(config) {}

private:
    virtual NodeStatus tick() override
    {
        size_t failure_count = 0;

        for (size_t index = 0; index < childrenCount(); index++)
        {
            TreeNode::SharedPtr current_child_node = children_nodes_[index];
            const NodeStatus child_status = current_child_node->executeTick();

            switch (child_status)
            {
                case NodeStatus::RUNNING: {

                    // reset the previous children, to make sure that they are in IDLE state
                    // the next time we tick them
                    for (size_t i = 0; i < index; i++)
                    {
                        haltChild(i);
                    }
                    return NodeStatus::RUNNING;
                }

                case NodeStatus::FAILURE: {
                    failure_count++;
                }
                break;

                case NodeStatus::SUCCESS: {
                    resetChildren();
                    return NodeStatus::SUCCESS;
                }

                case NodeStatus::IDLE: {
                    return NodeStatus::FAILURE;
                }
            }   // end switch
        }     //end for

        if (failure_count == childrenCount())
        {
            resetChildren();
            return NodeStatus::FAILURE;
        }

        return NodeStatus::RUNNING;
    }
};

}