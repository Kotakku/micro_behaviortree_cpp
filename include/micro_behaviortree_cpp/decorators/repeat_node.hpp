#pragma once

#include "micro_behaviortree_cpp/tree_node.hpp"

namespace micro_behavior_tree_cpp
{

class RepeatNode : public DecoratorNode
{
public:
    RepeatNode(NodeConfig config) :
        DecoratorNode::DecoratorNode(config) {}

private:
    virtual NodeStatus tick() override
    {
        if (read_parameter_from_ports_)
        {
            if (!getInput(NUM_CYCLES, num_cycles_))
            {
                return NodeStatus::FAILURE;
            }
        }

        setStatus(NodeStatus::RUNNING);

        while (repeat_count_ < num_cycles_ || num_cycles_ == -1)
        {
            NodeStatus child_state = child_node_->executeTick();

            switch (child_state)
            {
                case NodeStatus::SUCCESS: {
                    repeat_count_++;
                    resetChild();
                }
                break;

                case NodeStatus::FAILURE: {
                    repeat_count_ = 0;
                    resetChild();
                    return (NodeStatus::FAILURE);
                }

                case NodeStatus::RUNNING: {
                    return NodeStatus::RUNNING;
                }

                default: {
                    return NodeStatus::FAILURE;
                }
            }
        }

        repeat_count_ = 0;
        return NodeStatus::SUCCESS;
    }

    virtual void halt() override
    {
        repeat_count_ = 0;
        DecoratorNode::halt();
    }

    int num_cycles_ = 0;
    int repeat_count_ = 0;

    bool read_parameter_from_ports_ = false;
    static constexpr const char* NUM_CYCLES = "num_cycles";

};

}