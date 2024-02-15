

#include "micro_behaviortree_cpp/micro_behaviortree_cpp.hpp"
#include <iostream>

namespace BT = micro_behavior_tree_cpp;

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
        .sequence()
            .action<SayText>({{"text", "Hello World!"}})
            
            .action<SayText>({{"text", "test1"}})
            // if(true) { SayText("True"); } else { SayText("False"); }
            .if_then_else()
                .always_success()
                .action<SayText>({{"text", "True"}})
                .action<SayText>({{"text", "False"}})
            .end()

            .action<SayText>({{"text", "test2"}})

            // if(!true) { SayText("True"); } else { SayText("False"); }
            .if_then_else()
                .inverter()
                    .always_success()
                .action<SayText>({{"text", "True"}})
                .action<SayText>({{"text", "False"}})
            .end()
        .end()
        .build();

    tree->rootTick();

    return 0;
}