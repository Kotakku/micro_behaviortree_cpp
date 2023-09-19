#include "micro_behaviortree_cpp/micro_behaviortree_cpp.hpp"
#include <iostream>
#include <fstream>
#include <string>

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

std::string readFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::in | std::ios::binary);
    if (file) {
        file.seekg(0, std::ios::end);
        std::string contents;
        contents.resize(file.tellg());
        file.seekg(0, std::ios::beg);
        file.read(&contents[0], contents.size());
        file.close();
        return contents;
    }
    throw std::runtime_error("Failed to open file");
}

int main()
{
    auto bb = std::make_shared<BT::Blackboard>();
    BT::Builder builder(bb);

    builder.registerActionNode<SayText>("SayText");
    
    // buildフォルダの一つ上の階層にあるxmlファイルを読み込む
    std::string xml_string = readFile("../example_behaviortree/example_behaviortree.xml");

    auto tree = builder.createTreeFromText(xml_string.c_str());

    tree->rootTick();

    return 0;
}