#pragma once

#include "micro_behaviortree_cpp/tree_node.hpp"

namespace micro_behavior_tree_cpp
{

class RetryNode : public DecoratorNode
{
public:
    RetryNode(NodeConfig config) :
        DecoratorNode::DecoratorNode(config) {}

private:
    virtual NodeStatus tick() override
    {
        if (read_parameter_from_ports_)
        {
            if (!getInput(NUM_ATTEMPTS, max_attempts_))
            {
                return NodeStatus::FAILURE;
            }
        }

        setStatus(NodeStatus::RUNNING);

        while (try_count_ < max_attempts_ || max_attempts_ == -1)
        {
            NodeStatus child_state = child_node_->executeTick();
            switch (child_state)
            {
                case NodeStatus::SUCCESS: {
                    try_count_ = 0;
                    resetChild();
                    return (NodeStatus::SUCCESS);
                }

                case NodeStatus::FAILURE: {
                    try_count_++;
                    resetChild();
                }
                break;

                case NodeStatus::RUNNING: {
                    return NodeStatus::RUNNING;
                }

                default: {
                    return NodeStatus::FAILURE;
                }
            }
        }

        try_count_ = 0;
        return NodeStatus::FAILURE;
    }

    virtual void halt() override
    {
        try_count_ = 0;
        DecoratorNode::halt();
    }

    int max_attempts_ = 0;
    int try_count_ = 0;

    bool read_parameter_from_ports_ = true;
    static constexpr const char* NUM_ATTEMPTS = "num_attempts";
};

}