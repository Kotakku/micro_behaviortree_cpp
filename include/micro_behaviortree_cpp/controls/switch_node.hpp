#pragma once

#include "micro_behaviortree_cpp/tree_node.hpp"

namespace micro_behavior_tree_cpp
{

template <size_t NUM_CASES>
class SwitchNode : public ControlNode
{
public:
    SwitchNode(NodeConfig config) :
        ControlNode::ControlNode(config) {}

private:
    virtual NodeStatus tick() override
    {
        if (childrenCount() != NUM_CASES + 1)
        {
            return NodeStatus::FAILURE;
        }

        std::string variable;
        std::string value;
        int match_index = int(NUM_CASES);   // default index;

        if (getInput("variable", variable))   // no variable? jump to default
        {
            // check each case until you find a match
            for (int index = 0; index < int(NUM_CASES); ++index)
            {
                char case_key[20];
                sprintf(case_key, "case_%d", int(index + 1));
                bool found = static_cast<bool>(getInput(case_key, value));

                if (found && variable == value)
                {
                    match_index = index;
                    break;
                }
            }
        }

        // if another one was running earlier, halt it
        if (running_child_ != -1 && running_child_ != match_index)
        {
            haltChild(running_child_);
        }

        auto& selected_child = children_nodes_[match_index];
        NodeStatus ret = selected_child->executeTick();
        if (ret == NodeStatus::RUNNING)
        {
            running_child_ = match_index;
        }
        else
        {
            resetChildren();
            running_child_ = -1;
        }
        return ret;
    }

    virtual void halt() override
    {
        running_child_ = -1;
        ControlNode::halt();
    }

    int running_child_ = -1;
};

}