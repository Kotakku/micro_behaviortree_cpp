#pragma once

#if defined(MBT_DONT_USE_DYNAMIC_CONSTRUCTION) && not defined(MBT_DONT_USE_XML)
#define MBT_DONT_USE_XML
#endif

#include <memory>
#include <vector>
#include <string>
#include <unordered_map>
#include <variant>
#include <cassert>
#include "tree_node.hpp"

#ifndef MBT_DONT_USE_XML
#include "3rdparty/tinyxml2/tinyxml2.h"
#endif

// Action nodes
#include "actions/always_failure_node.hpp"
#include "actions/always_success_node.hpp"
#include "actions/set_blackboard_node.hpp"

// Control nodes
#include "controls/fallback_node.hpp"
#include "controls/if_then_else_node.hpp"
#include "controls/parallel_node.hpp"
#include "controls/reactive_fallback.hpp"
#include "controls/reactive_sequence.hpp"
#include "controls/sequence_node.hpp"
#include "controls/sequence_star_node.hpp"
#include "controls/switch_node.hpp"
#include "controls/while_do_else_node.hpp"

// Decorator nodes
#include "decorators/blackboard_precondition.hpp"
#include "decorators/force_failure_node.hpp"
#include "decorators/force_success_node.hpp"
#include "decorators/inverter_node.hpp"
#include "decorators/keep_running_until_failure_node.hpp"
#include "decorators/repeat_node.hpp"
#include "decorators/retry_node.hpp"

namespace micro_behavior_tree_cpp
{

class Tree
{
public:
    using SharedPtr = std::shared_ptr<Tree>;

    Tree() = default;
    ~Tree() = default;

    void setRoot(TreeNode::SharedPtr root)
    {
        root_ = std::move(root);
    }

    void rootTick()
    {
        assert(rootNode() != nullptr);
        NodeStatus ret = rootNode()->executeTick();
        if (ret == NodeStatus::SUCCESS || ret == NodeStatus::FAILURE)
        {
            rootNode()->setStatus(NodeStatus::IDLE);
        }
    }

    TreeNode::SharedPtr rootNode() const
    {
        return root_;
    }

private:
    TreeNode::SharedPtr root_;
};

class Builder
{
public:
    Builder(Blackboard::SharedPtr bb)
    {
        builder_config_ = std::make_shared<BuilderConfig>();
        builder_config_->bb = bb;

#ifndef MBT_DONT_USE_DYNAMIC_CONSTRUCTION
        // default node registration
        // Action node
        registerActionNode<AlwaysFailureNode>("AlwaysFailure");
        registerActionNode<AlwaysSuccessNode>("AlwaysSuccess");
        registerActionNode<SetBlackboard>("SetBlackboard");

        // Control node
        registerControlNode<FallbackNode>("Fallback");
        registerControlNode<IfThenElseNode>("IfThenElse");
        registerControlNode<ParallelNode>("Parallel");
        registerControlNode<ReactiveFallback>("ReactiveFallback");
        registerControlNode<ReactiveSequence>("ReactiveSequence");
        registerControlNode<SequenceNode>("Sequence");
        registerControlNode<SequenceStarNode>("SequenceStar");
        registerControlNode<SwitchNode<2>>("Switch2");
        registerControlNode<SwitchNode<3>>("Switch3");
        registerControlNode<SwitchNode<4>>("Switch4");
        registerControlNode<SwitchNode<5>>("Switch5");
        registerControlNode<SwitchNode<6>>("Switch6");
        registerControlNode<WhileDoElseNode>("WhileDoElse");

        // Decorator node
        registerDecoratorNode<BlackboardPreconditionNode<bool>>("BlackboardCheckBool");
        registerDecoratorNode<BlackboardPreconditionNode<int>>("BlackboardCheckInt");
        registerDecoratorNode<BlackboardPreconditionNode<double>>("BlackboardCheckDouble");
        registerDecoratorNode<BlackboardPreconditionNode<std::string>>("BlackboardCheckString");
        registerDecoratorNode<ForceFailureNode>("ForceFailure");
        registerDecoratorNode<ForceSuccessNode>("ForceSuccess");
        registerDecoratorNode<InverterNode>("Inverter");
        registerDecoratorNode<KeepRunningUntilFailureNode>("KeepRunningUntilFailure");
        registerDecoratorNode<RepeatNode>("Repeat");
        registerDecoratorNode<RetryNode>("RetryUntilSuccessful");
#endif
    }

    template <typename T>
    Builder& action(const PortsRemapping& ports = {})
    {
        auto node = make_shared_node_static<ActionNodeBase, T>(ports);
        add_tree_node(node);
        return *this;
    }

    template <typename T>
    Builder& control(const PortsRemapping& ports = {})
    {
        auto node = make_shared_node_static<ControlNode, T>(ports);
        add_tree_node(node);
        builder_parent_.push_back({node, nullptr});
        return *this;
    }

    template <typename T>
    Builder& decorator(const PortsRemapping& ports = {})
    {
        auto node = make_shared_node_static<DecoratorNode, T>(ports);
        add_tree_node(node);
        builder_parent_.push_back({nullptr, node});
        return *this;
    }

    Builder& end()
    {
        bool is_control_child = (!builder_parent_.empty() && builder_parent_.back().control_node != nullptr);
        assert(is_control_child);
        builder_parent_.pop_back();
        return *this;
    }

    // default_nodes_alias
    Builder& always_failure() { return action<AlwaysFailureNode>({}); }
    Builder& always_success() { return action<AlwaysSuccessNode>({}); }
    Builder& set_blackboard(const std::string &output_key, const std::string &value) 
    { 
        return action<SetBlackboard>({
            {"output_key", output_key}, 
            {"value", value}}); 
    }

    Builder& fallback() { return control<FallbackNode>({}); }
    Builder& if_then_else() { return control<IfThenElseNode>({}); }
    Builder& parallel(int failure_threshold, int success_threshold) 
    { 
        return control<ParallelNode>({
            {"failure_threshold", toStr(failure_threshold)}, 
            {"success_threshold", toStr(success_threshold)}}); 
    }
    Builder& reactive_fallback() { return control<ReactiveFallback>({}); }
    Builder& reactive_sequence() { return control<ReactiveSequence>({}); }
    Builder& sequence() { return control<SequenceNode>({}); }
    Builder& sequence_star() { return control<SequenceStarNode>({}); }
    Builder& switch2(const std::string& variable, const std::string& value_1, const std::string& value_2) 
    { 
        return control<SwitchNode<2>>({
            {"variable", variable},
            {"value_1", value_1},
            {"value_2", value_2}}); 
    }
    Builder& switch3(const std::string& variable, const std::string& value_1, const std::string& value_2, const std::string& value_3) 
    { 
        return control<SwitchNode<3>>({
            {"variable", variable},
            {"value_1", value_1},
            {"value_2", value_2},
            {"value_3", value_3}}); 
    }
    Builder& switch4(const std::string& variable, const std::string& value_1, const std::string& value_2, const std::string& value_3, const std::string& value_4) 
    { 
        return control<SwitchNode<4>>({
            {"variable", variable},
            {"value_1", value_1},
            {"value_2", value_2},
            {"value_3", value_3},
            {"value_4", value_4}}); 
    }
    Builder& switch5(const std::string& variable, const std::string& value_1, const std::string& value_2, const std::string& value_3, const std::string& value_4, const std::string& value_5) 
    { 
        return control<SwitchNode<5>>({
            {"variable", variable},
            {"value_1", value_1},
            {"value_2", value_2},
            {"value_3", value_3},
            {"value_4", value_4},
            {"value_5", value_5}}); 
    }
    Builder& switch6(const std::string& variable, const std::string& value_1, const std::string& value_2, const std::string& value_3, const std::string& value_4, const std::string& value_5, const std::string& value_6) 
    { 
        return control<SwitchNode<6>>({
            {"variable", variable},
            {"value_1", value_1},
            {"value_2", value_2},
            {"value_3", value_3},
            {"value_4", value_4},
            {"value_5", value_5},
            {"value_6", value_6}}); 
    }
    Builder& while_do_else() { return control<WhileDoElseNode>({}); }

    Builder& blackboard_check_bool(bool value_A, bool value_B, NodeStatus return_on_mismatch) 
    { 
        return decorator<BlackboardPreconditionNode<bool>>({
            {"value_A", toStr(value_A)},
            {"value_B", toStr(value_B)},
            {"return_on_mismatch", toStr(return_on_mismatch)}});
    }
    Builder& blackboard_check_int(int value_A, int value_B, NodeStatus return_on_mismatch) 
    { 
        return decorator<BlackboardPreconditionNode<int>>({
            {"value_A", toStr(value_A)},
            {"value_B", toStr(value_B)},
            {"return_on_mismatch", toStr(return_on_mismatch)}});
    }
    Builder& blackboard_check_double(double value_A, double value_B, NodeStatus return_on_mismatch) 
    { 
        return decorator<BlackboardPreconditionNode<double>>({
            {"value_A", toStr(value_A)},
            {"value_B", toStr(value_B)},
            {"return_on_mismatch", toStr(return_on_mismatch)}});
    }
    Builder& blackboard_check_string(const std::string &value_A, const std::string &value_B, NodeStatus return_on_mismatch) 
    { 
        return decorator<BlackboardPreconditionNode<std::string>>({
            {"value_A", value_A},
            {"value_B", value_B},
            {"return_on_mismatch", toStr(return_on_mismatch)}});
    }
    Builder& force_failure() { return decorator<ForceFailureNode>({}); }
    Builder& force_success() { return decorator<ForceSuccessNode>({}); }
    Builder& inverter() { return decorator<InverterNode>({}); }
    Builder& keep_running_until_failure() { return decorator<KeepRunningUntilFailureNode>({}); }
    Builder& repeat(int num_cycles) { return decorator<RepeatNode>({{"num_cycles", toStr(num_cycles)}}); }
    Builder& retry_until_successful(int num_attempts) { return decorator<RetryNode>({{"num_attempts", toStr(num_attempts)}}); }

    Tree::SharedPtr build()
    {
        assert(root_ != nullptr);
        Tree::SharedPtr tree = std::make_shared<Tree>();
        tree->setRoot(root_);
        return tree;
    }

#ifndef MBT_DONT_USE_DYNAMIC_CONSTRUCTION
    template <typename T>
    void registerActionNode(std::string name)
    {
        builder_config_->action_nodes_[name] = [](NodeConfig config) -> ActionNodeBase::SharedPtr
        {
            return std::make_shared<T>(config);
        };
    }

    template <typename T>
    void registerControlNode(std::string name)
    {
        builder_config_->control_nodes_[name] = [](NodeConfig config) -> ControlNode::SharedPtr
        {
            return std::make_shared<T>(config);
        };
    }

    template <typename T>
    void registerDecoratorNode(std::string name)
    {
        builder_config_->decorator_nodes_[name] = [](NodeConfig config) -> DecoratorNode::SharedPtr
        {
            return std::make_shared<T>(config);
        };
    }

    Builder& action(std::string name, const PortsRemapping& ports = {})
    {
        auto node = make_shared_node_dynamic<ActionNodeBase>(name, ports);
        add_tree_node(node);
        return *this;
    }

    Builder& control(std::string name, const PortsRemapping& ports = {})
    {
        auto node = make_shared_node_dynamic<ControlNode>(name, ports);
        add_tree_node(node);
        builder_parent_.push_back({node, nullptr});
        return *this;
    }

    Builder& decorator(std::string name, const PortsRemapping& ports = {})
    {
        auto node = make_shared_node_dynamic<DecoratorNode>(name, ports);
        add_tree_node(node);
        builder_parent_.push_back({nullptr, node});
        return *this;
    }

#ifndef MBT_DONT_USE_XML
    Tree::SharedPtr createTreeFromText(const char * xml)
    {
        tinyxml2::XMLDocument doc;
        doc.Parse(xml);
        tinyxml2::XMLElement *root = doc.RootElement();
        assert(root != nullptr);
        createTreeFromXML(root);
        return build();
    }
#endif
#endif

protected:
    template <typename BaseNodeType, typename T>
    std::shared_ptr<BaseNodeType> make_shared_node_static(const PortsRemapping& ports)
    {
        static_assert(std::is_base_of_v<BaseNodeType, T>, "T must be derived from NodeType");
        NodeConfig config;
        config.blackboard = builder_config_->bb;
        config.ports = ports;
        return std::make_shared<T>(config);
    }

#ifndef MBT_DONT_USE_DYNAMIC_CONSTRUCTION
    template <typename BaseNodeType>
    std::shared_ptr<BaseNodeType> make_shared_node_dynamic(const std::string &name, const PortsRemapping& ports)
    {
        NodeConfig config;
        config.blackboard = builder_config_->bb;
        config.ports = ports;

        constexpr bool is_action = std::is_same_v<BaseNodeType, ActionNodeBase>;
        constexpr bool is_control = std::is_same_v<BaseNodeType, ControlNode>;
        constexpr bool is_decorator = std::is_same_v<BaseNodeType, DecoratorNode>;

        static_assert((is_action||is_control||is_decorator), "BaseNodeType must be derived from ActionNodeBase, ControlNode or DecoratorNode");

        if constexpr (is_action)
        {
            auto it = builder_config_->action_nodes_.find(name);
            assert(it != builder_config_->action_nodes_.end());
            return it->second(config);
        }
        else if constexpr (is_control)
        {
            auto it = builder_config_->control_nodes_.find(name);
            assert(it != builder_config_->control_nodes_.end());
            return it->second(config);
        }
        else if constexpr (is_decorator)
        {
            auto it = builder_config_->decorator_nodes_.find(name);
            assert(it != builder_config_->decorator_nodes_.end());
            return it->second(config);
        }
    }
#endif

    void add_tree_node(TreeNode::SharedPtr node)
    {
        bool is_root = (builder_parent_.empty());
        bool is_control_child = (!is_root && builder_parent_.back().control_node != nullptr);
        bool is_decorator_child = (!is_root && builder_parent_.back().decorator_node != nullptr);
        if(is_control_child)
        {
            auto parent = builder_parent_.back().control_node;
            parent->addChild(node);
        }
        else if(is_decorator_child)
        {
            auto parent = builder_parent_.back().decorator_node;
            parent->setChild(node);
            builder_parent_.pop_back();
        }
        else if(is_root)
        {
            root_ = node;
        }
    }

#ifndef MBT_DONT_USE_DYNAMIC_CONSTRUCTION
#ifndef MBT_DONT_USE_XML
    using XmlNodeMap = std::unordered_map<std::string, const tinyxml2::XMLNode*>;
    void createTreeFromXML(tinyxml2::XMLElement *root)
    {
        if(root->Name() != std::string("root"))
        {
            return;
        }

        // ツリーの読み取り
        std::string main_tree_to_execute = root->Attribute("main_tree_to_execute");
        XmlNodeMap trees;
        for (const tinyxml2::XMLNode* child = root->FirstChild(); child != nullptr; child = child->NextSibling()) {
            if(child->ToElement())
            {
                std::string tag = child->Value();
                if(tag == "BehaviorTree")
                {
                    std::string tree_name = child->ToElement()->Attribute("ID");
                    trees[tree_name] = child;
                }
            }
        }

        // 再帰的にツリーを構築
        createTree(trees[main_tree_to_execute], trees);
    }

    void createTree(const tinyxml2::XMLNode* walking_root, XmlNodeMap &trees)
    {
        for (const tinyxml2::XMLNode* node = walking_root->FirstChild(); node != nullptr; node = node->NextSibling()) {
            if(node->ToElement())
            {
                auto [type, name, ports] = getNodeInfo(node);

                if(type == "Action")
                {
                    action(name, ports);
                }

                if(type == "Control")
                {
                    control(name, ports);
                    createTree(node, trees);
                    control_end();
                }

                if (type == "Decorator")
                {
                    decorator(name, ports);
                    createTree(node, trees);
                }

                if(type == "SubTree")
                {
                    createTree(trees[name], trees);
                }
            }
        }
    }

    std::tuple<std::string, std::string, PortsRemapping> getNodeInfo(const tinyxml2::XMLNode* node)
    {
        std::string type = node->Value();
        std::string name;
        PortsRemapping ports;

        if(type == "SubTree")
        {
            name = node->ToElement()->Attribute("ID");
            return std::make_tuple(type, name, PortsRemapping());
        }
        else if(type == "Action" || type == "Control" || type == "Decorator")
        {
            name = node->ToElement()->Attribute("ID");
        }
        else
        {
            name = type;
            if(auto it = std::find(default_action_node_types_.begin(), default_action_node_types_.end(), name); it != default_action_node_types_.end())
                type = "Action";
            else if (auto it = std::find(default_control_node_types_.begin(), default_control_node_types_.end(), name); it != default_control_node_types_.end())
                type = "Control";
            else if (auto it = std::find(default_decorator_node_types_.begin(), default_decorator_node_types_.end(), name); it != default_decorator_node_types_.end())
                type = "Decorator";
        }

        for (const tinyxml2::XMLAttribute* attr = node->ToElement()->FirstAttribute(); attr; attr = attr->Next()) {
            if(std::string attr_str(attr->Name()); attr_str != "ID")
            {
                ports[attr_str] = std::string(attr->Value());
            }
        }

        return std::make_tuple(type, name, ports);
    }
#endif
#endif

protected:
    struct BuilderConfig
    {
        using SharedPtr = std::shared_ptr<BuilderConfig>;

        Blackboard::SharedPtr bb;

#ifndef MBT_DONT_USE_DYNAMIC_CONSTRUCTION
        using ActionNodeList = std::unordered_map<std::string, std::function<ActionNodeBase::SharedPtr(NodeConfig)>>;
        ActionNodeList action_nodes_;
        using ControlNodeList = std::unordered_map<std::string, std::function<ControlNode::SharedPtr(NodeConfig)>>;
        ControlNodeList control_nodes_;
        using DecoratorNodeList = std::unordered_map<std::string, std::function<DecoratorNode::SharedPtr(NodeConfig)>>;
        DecoratorNodeList decorator_nodes_;
#endif
    };

    BuilderConfig::SharedPtr builder_config_;
    TreeNode::SharedPtr root_;

    struct parent_t
    {
        ControlNode::SharedPtr control_node;
        DecoratorNode::SharedPtr decorator_node;
    };
    std::vector<parent_t> builder_parent_;

    inline static std::vector<std::string> default_action_node_types_ =
    {
        "AlwaysFailure",
        "AlwaysSuccess",
        "SetBlackboard",
    };

    inline static std::vector<std::string> default_control_node_types_ =
    {
        "Fallback",
        "IfThenElse",
        "Parallel",
        "ReactiveFallback",
        "ReactiveSequence",
        "Sequence",
        "SequenceStar",
        "Switch2",
        "Switch3",
        "Switch4",
        "Switch5",
        "Switch6",
        "WhileDoElse",
    };

    inline static std::vector<std::string> default_decorator_node_types_ =
    {
        "BlackboardCheckBool",
        "BlackboardCheckInt",
        "BlackboardCheckDouble",
        "BlackboardCheckString",
        "ForceFailure",
        "ForceSuccess",
        "Inverter",
        "KeepRunningUntilFailure",
        "Repeat",
        "RetryUntilSuccessful",
    };
};

}