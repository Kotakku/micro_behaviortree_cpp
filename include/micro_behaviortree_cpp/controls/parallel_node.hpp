#pragma once

#include "micro_behaviortree_cpp/tree_node.hpp"
#include <set>

namespace micro_behavior_tree_cpp
{

class ParallelNode : public ControlNode
{
public:
    ParallelNode(NodeConfig config) :
        ControlNode::ControlNode(config) {}

private:
    virtual NodeStatus tick() override
    {
        if (read_parameter_from_ports_)
        {
            if (!getInput(THRESHOLD_SUCCESS, success_threshold_))
            {
                return NodeStatus::FAILURE;
            }

            if (!getInput(THRESHOLD_FAILURE, failure_threshold_))
            {
                return NodeStatus::FAILURE;
            }
        }

        size_t success_childred_num = 0;
        size_t failure_childred_num = 0;

        const size_t children_count = children_nodes_.size();

        if (children_count < successThreshold())
        {
            return NodeStatus::FAILURE;
        }

        if (children_count < failureThreshold())
        {
            return NodeStatus::FAILURE;
        }

        // Routing the tree according to the sequence node's logic:
        for (unsigned int i = 0; i < children_count; i++)
        {
            TreeNode::SharedPtr child_node = children_nodes_[i];

            bool in_skip_list = (skip_list_.count(i) != 0);

            NodeStatus child_status;
            if (in_skip_list)
            {
                child_status = child_node->status();
            }
            else
            {
                child_status = child_node->executeTick();
            }

            switch (child_status)
            {
                case NodeStatus::SUCCESS: {
                    if (!in_skip_list)
                    {
                        skip_list_.insert(i);
                    }
                    success_childred_num++;

                    if (success_childred_num == successThreshold())
                    {
                        skip_list_.clear();
                        resetChildren();
                        return NodeStatus::SUCCESS;
                    }
                }
                break;

                case NodeStatus::FAILURE: {
                    if (!in_skip_list)
                    {
                        skip_list_.insert(i);
                    }
                    failure_childred_num++;

                    // It fails if it is not possible to succeed anymore or if
                    // number of failures are equal to failure_threshold_
                    if ((failure_childred_num > children_count - successThreshold()) ||
                        (failure_childred_num == failureThreshold()))
                    {
                        skip_list_.clear();
                        resetChildren();
                        return NodeStatus::FAILURE;
                    }
                }
                break;

                case NodeStatus::RUNNING: {
                    // do nothing
                }
                break;

                default: {
                    return NodeStatus::FAILURE;
                }
            }
        }

        return NodeStatus::RUNNING;
    }

    virtual void halt() override
    {
        skip_list_.clear();
        ControlNode::halt();
    }

    size_t successThreshold() const
    {
        return success_threshold_ < 0 ?
            std::max(children_nodes_.size() + success_threshold_ + 1, size_t(0)) :
            success_threshold_;
    }

    size_t failureThreshold() const
    {
        return failure_threshold_ < 0 ?
            std::max(children_nodes_.size() + failure_threshold_ + 1, size_t(0)) :
            failure_threshold_;
    }

    void setSuccessThreshold(int threshold_M)
    {
        success_threshold_ = threshold_M;
    }

    void setFailureThreshold(int threshold_M)
    {
        failure_threshold_ = threshold_M;
    }

    int success_threshold_ = 1;
    int failure_threshold_ = 1;

    std::set<int> skip_list_;

    bool read_parameter_from_ports_ = false;
    static constexpr const char* THRESHOLD_SUCCESS = "success_threshold";
    static constexpr const char* THRESHOLD_FAILURE = "failure_threshold";
};

}