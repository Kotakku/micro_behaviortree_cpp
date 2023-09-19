#pragma once

#include "micro_behaviortree_cpp/tree_node.hpp"
#include <limits>

namespace micro_behavior_tree_cpp
{

template <typename T>
inline bool IsSame(const T& a, const T& b)
{
    return a == b;
}

inline bool IsSame(const double& a, const double& b)
{
    constexpr double EPS = static_cast<double>(std::numeric_limits<float>::epsilon());
    return std::abs(a - b) <= EPS;
}

template <typename T>
class BlackboardPreconditionNode : public DecoratorNode
{
public:
    BlackboardPreconditionNode(NodeConfig config) :
        DecoratorNode::DecoratorNode(config) {}

private:
    virtual NodeStatus tick() override
    {
        T value_A;
        T value_B;
        NodeStatus default_return_status = NodeStatus::FAILURE;

        setStatus(NodeStatus::RUNNING);

        if (getInput("value_A", value_A) && getInput("value_B", value_B) &&
            IsSame(value_A, value_B))
        {
            return child_node_->executeTick();
        }

        if (child()->status() == NodeStatus::RUNNING)
        {
            haltChild();
        }
        getInput("return_on_mismatch", default_return_status);
        return default_return_status;
    }
};

}