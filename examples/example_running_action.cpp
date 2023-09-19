

#include "micro_behaviortree_cpp/micro_behaviortree_cpp.hpp"
#include <iostream>

namespace BT = micro_behavior_tree_cpp;

class WaitNode : public BT::SyncActionNode
{
public:
    WaitNode(BT::NodeConfig config) : SyncActionNode(config) {}
    BT::NodeStatus tick() override
    {
        int count_num;
        if (!getInput("count_num", count_num))
        {
            return BT::NodeStatus::FAILURE;
        }
        count_++;

        if(count_ >= count_num)
        {
            count_ = 0;
            return BT::NodeStatus::SUCCESS;
        }
        else
        {
            return BT::NodeStatus::RUNNING;
        }
    }

    virtual void halt() override
    {
        count_ = 0;
        SyncActionNode::halt();
    }

private:
    int count_ = 0;
};

class SayText : public BT::SyncActionNode
{
public:
    SayText(BT::NodeConfig config) : SyncActionNode(config) {}
    BT::NodeStatus tick() override
    {
        std::string text;
        if (!getInput("text", text))
        {
            return BT::NodeStatus::FAILURE;
        }
        std::cout << "[SayText]: " << text << std::endl;
        return BT::NodeStatus::SUCCESS;
    }
};

int main()
{
    auto bb = std::make_shared<BT::Blackboard>();
    BT::Builder builder(bb);

    auto tree = builder
        .control_sequence()
            .action<WaitNode>({{"count_num", BT::toStr(5)}})
            
            .action<SayText>({{"text", "Hello World!"}})
        .control_end()
        .build();

    for(size_t i = 0; i < 5; i++)
    {
        std::cout << "===== tick =====" << std::endl;
        tree->rootTick();
    }

    return 0;
}