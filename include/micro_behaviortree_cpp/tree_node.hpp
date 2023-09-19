#pragma once

#include <memory>
#include <vector>
#include <string>
#include <functional>
#include <optional>
#include <unordered_map>
#include <cassert>
#include "basic_types.hpp"
#include "blackboard.hpp"

namespace micro_behavior_tree_cpp
{

using PortsRemapping = std::unordered_map<std::string, std::string>;
struct NodeConfig
{
    Blackboard::SharedPtr blackboard;
    PortsRemapping ports; // 入出力方向は考慮しない
};

class TreeNode
{
public:
    using SharedPtr = std::shared_ptr<TreeNode>;

    TreeNode(NodeConfig config) :
        status_(NodeStatus::IDLE),
        config_(config)
    {
        uid_ = getUID();
    }
    virtual ~TreeNode() = default;

    /// The method used to interrupt the execution of a RUNNING node.
    /// Only Async nodes that may return RUNNING should implement it.
    virtual void halt() {}

    /// The method that should be used to invoke tick() and setStatus();
    virtual NodeStatus executeTick()
    {
        NodeStatus new_status = status_;
        new_status = tick();
        setStatus(new_status);
        return new_status;
    }

    /// Method to be implemented by the user
    virtual NodeStatus tick() = 0;

    NodeStatus status() const { return status_; }
    bool isHalted() const { return status_ == NodeStatus::IDLE; }

    void setStatus(NodeStatus new_status)
    {
        status_ = new_status;
    }

    void resetStatus()
    {
        status_ = NodeStatus::IDLE;
    }

    uint16_t uid() const { return uid_; }

    virtual NodeType type() const = 0;

    // 値の読み込み
    // val -> portからvalを返す
    // {val} -> blackboardからvalという変数を読み込み
    template <typename T>
    bool getInput(const std::string& key, T& destination) const
    {
        std::string bb_key;
        if(is_blackboard_key(key, bb_key))
        {
            return config_.blackboard->get(bb_key, destination);
        }

        // portから読み込み
        auto it = config_.ports.find(key);
        if(it == config_.ports.end())
        {
            return false;
        }
        std::string str_value = std::any_cast<std::string>(it->second);
        return convertFromString(str_value, destination);
    }

    template <typename T>
    std::optional<T> getInput(const std::string& key) const
    {
        T out;
        auto res = getInput(key, out);
        return (res) ? std::optional<T>(out) : std::nullopt;
    }

    // blackboardに値を書き込む
    // {val}のような構文のみ使用する？
    template <typename T>
    void setOutput(const std::string& key, const T& value)
    {
        std::string bb_key;
        if(is_blackboard_key(key, bb_key))
        {
            return config_.blackboard->set(bb_key, value);
        }

        return config_.blackboard->set(key, value); // どちらでもblackboardに書き込む
    }

private:
    bool is_blackboard_key(const std::string& key, std::string& bb_key) const
    {
        if (key.size() < 2) {
            return false;
        }

        if(key.front() == '{' && key.back() == '}')
        {
            bb_key = key.substr(1, key.size() - 2);
            return true;
        }
        return false;
    }

    static uint16_t getUID()
    {
        static uint16_t uid = 1;
        return uid++;
    }

private:
    NodeStatus status_;
    NodeConfig config_;
    uint16_t uid_;
};

class LeafNode : public TreeNode
{
public:
    using TreeNode::TreeNode;
};

class ControlNode : public TreeNode
{
public:
    using SharedPtr = std::shared_ptr<ControlNode>;
    using TreeNode::TreeNode;

    /// The method used to add nodes to the children vector
    void addChild(TreeNode::SharedPtr child)
    {
        children_nodes_.push_back(child);
    }

    size_t childrenCount() const { return children_nodes_.size(); }

    const std::vector<TreeNode::SharedPtr>& children() const { return children_nodes_; }

    const TreeNode::SharedPtr child(size_t index) const
    {
        return children().at(index);
    }

    virtual void halt() override { resetChildren();}

    /// same as resetChildren()
    void haltChildren()
    {
        for (size_t i = 0; i < children_nodes_.size(); i++)
        {
            haltChild(i);
        }
    }

    void haltChild(size_t i)
    {
        auto child = children_nodes_[i];
        if (child->status() == NodeStatus::RUNNING)
        {
            child->halt();
        }
        child->resetStatus();
    }

    NodeType type() const override final { return NodeType::CONTROL; }

    /// Set the status of all children to IDLE.
    /// also send a halt() signal to all RUNNING children
    void resetChildren()
    {
        for (auto child: children_nodes_)
        {
            if (child->status() == NodeStatus::RUNNING)
            {
                child->halt();
            }
            child->resetStatus();
        }
    }

protected:
    std::vector<TreeNode::SharedPtr> children_nodes_;
};

class DecoratorNode : public TreeNode
{
public:
    using SharedPtr = std::shared_ptr<DecoratorNode>;
    using TreeNode::TreeNode;

    void setChild(TreeNode::SharedPtr child)
    {
        child_node_ = child;
    }

    const TreeNode::SharedPtr child() const
    {
        return child_node_;
    }

    TreeNode::SharedPtr child()
    {
        return child_node_;
    }

    /// The method used to interrupt the execution of this node
    virtual void halt() override
    {
        resetChild();
    }

    /// Same as resetChild()
    void haltChild()
    {
        resetChild();
    }

    NodeType type() const override final { return NodeType::DECORATOR; }

    NodeStatus executeTick() override
    {
        NodeStatus status = TreeNode::executeTick();
        NodeStatus child_status = child()->status();
        if (child_status == NodeStatus::SUCCESS || child_status == NodeStatus::FAILURE)
        {
            child()->resetStatus();
        }
        return status;
    }

    /// Set the status of the child to IDLE.
    /// also send a halt() signal to a RUNNING child
    void resetChild()
    {
        if (!child_node_)
        {
            return;
        }
        if (child_node_->status() == NodeStatus::RUNNING)
        {
            child_node_->halt();
        }
        child_node_->resetStatus();
    }

protected:
    TreeNode::SharedPtr child_node_;
};

// class ConditionNode : public LeafNode
// {
// public:
//     using SharedPtr = std::shared_ptr<ConditionNode>;
//     ConditionNode(NodeConfig config) :
//         LeafNode(config) {}

//     virtual ~ConditionNode() override = default;

//     //Do nothing
//     virtual void halt() override final
//     {}

//     virtual NodeType type() const override final
//     {
//         return NodeType::CONDITION;
//     }
// };

class ActionNodeBase : public LeafNode
{
public:
    using SharedPtr = std::shared_ptr<ActionNodeBase>;
    using LeafNode::LeafNode;

    NodeType type() const override final { return NodeType::ACTION; }
};

class SyncActionNode : public ActionNodeBase
{
public:
    using SharedPtr = std::shared_ptr<SyncActionNode>;
    using ActionNodeBase::ActionNodeBase;
};

class StatefulActionNode : public ActionNodeBase
{
public:
    using SharedPtr = std::shared_ptr<StatefulActionNode>;
    using ActionNodeBase::ActionNodeBase;

    // do not override this method
    NodeStatus tick() override final
    {
        const NodeStatus initial_status = status();

        if (initial_status == NodeStatus::IDLE)
        {
            NodeStatus new_status = onStart();
            return new_status;
        }
        //------------------------------------------
        if (initial_status == NodeStatus::RUNNING)
        {
            NodeStatus new_status = onRunning();
            return new_status;
        }
        //------------------------------------------
        return initial_status;
    }
    // do not override this method
    void halt() override final
    {
        if (status() == NodeStatus::RUNNING)
        {
            onHalted();
        }
    }

    /// method to be called at the beginning.
    /// If it returns RUNNING, this becomes an asychronous node.
    virtual NodeStatus onStart() = 0;

    /// method invoked by a RUNNING action.
    virtual NodeStatus onRunning() = 0;

    /// when the method halt() is called and the action is RUNNING, this method is invoked.
    /// This is a convenient place todo a cleanup, if needed.
    virtual void onHalted() = 0;
};

// simple node implementations

// class SimpleConditionNode : public ConditionNode
// {
// public:
//     typedef std::function<NodeStatus(TreeNode&)> TickFunctor;

//     // You must provide the function to call when tick() is invoked
//     SimpleConditionNode(NodeConfig config, TickFunctor tick_functor):
//         ConditionNode(config), tick_functor_(std::move(tick_functor)) {}

//     ~SimpleConditionNode() override = default;

// protected:
//     virtual NodeStatus tick() override
//     {
//         return tick_functor_(*this);
//     }

//     TickFunctor tick_functor_;
// };

class SimpleDecoratorNode : public DecoratorNode
{
public:
    typedef std::function<NodeStatus(NodeStatus, TreeNode&)> TickFunctor;

    // You must provide the function to call when tick() is invoked
    SimpleDecoratorNode(NodeConfig config, TickFunctor tick_functor):
        DecoratorNode(config), tick_functor_(std::move(tick_functor)) {}

    ~SimpleDecoratorNode() override = default;

protected:
    virtual NodeStatus tick() override
    {
        return tick_functor_(child()->executeTick(), *this);
    }

    TickFunctor tick_functor_;
};

class SimpleActionNode : public SyncActionNode
{
public:
    typedef std::function<NodeStatus(TreeNode&)> TickFunctor;

    // You must provide the function to call when tick() is invoked
    SimpleActionNode(NodeConfig config, TickFunctor tick_functor):
        SyncActionNode(config), tick_functor_(std::move(tick_functor)) {}

    ~SimpleActionNode() override = default;

protected:
    virtual NodeStatus tick() override final
    {
        NodeStatus prev_status = status();

        if (prev_status == NodeStatus::IDLE)
        {
            setStatus(NodeStatus::RUNNING);
            prev_status = NodeStatus::RUNNING;
        }

        NodeStatus status = tick_functor_(*this);
        if (status != prev_status)
        {
            setStatus(status);
        }
        return status;
    }

    TickFunctor tick_functor_;
};

}