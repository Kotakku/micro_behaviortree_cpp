

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

    builder.registerActionNode<SayText>("SayText");
    
    std::string xml_tree = R"(
        <?xml version="1.0"?>
        <root main_tree_to_execute="BehaviorTree">
            <!-- ////////// -->
            <BehaviorTree ID="BehaviorTree">
                <Sequence>
                    <Action ID="SayText" text="Hello World!"/>
                    <SubTree ID="MySubTree"/>
                </Sequence>
            </BehaviorTree>
            <!-- ////////// -->
            <BehaviorTree ID="MySubTree">
                <Sequence>
                    <Action ID="SayText" text="SubTree 1"/>
                    <Action ID="SayText" text="SubTree 2"/>
                </Sequence>
            </BehaviorTree>
            <!-- ////////// -->
            <TreeNodesModel>
                <SubTree ID="MySubTree"/>
                <Action ID="SayText">
                    <input_port default="default_text" name="text"/>
                </Action>
            </TreeNodesModel>
            <!-- ////////// -->
        </root>
    )";

    auto tree = builder.createTreeFromText(xml_tree.c_str());

    tree->rootTick();

    return 0;
}