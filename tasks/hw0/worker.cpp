#include "green_thread_scheduler.h"
#include <iostream>

int main() {
    Scheduler pool;
    std::vector<GraphNode> graph(5);

    graph[0].name = "a1";
    graph[0].node_dependencies = {1, 2, 3};
    graph[0].calc = [](const std::vector<int>& args) {
        return args[0] + args[1] + args[2];
    };

    graph[1].name = "a2";
    graph[1].node_dependencies = {};
    graph[1].calc = [](const std::vector<int>&) {
        return 8;
    };

    graph[2].name = "a3";
    graph[2].node_dependencies = {1};
    graph[2].calc = [](const std::vector<int>& args) {
        return args[0] + 7;
    };

    graph[3].name = "a5";
    graph[3].node_dependencies = {};
    graph[3].calc = [](const std::vector<int>&) {
        return 8;
    }; 

    graph[4].name = "a6";
    graph[4].node_dependencies = {1, 3};
    graph[4].calc = [](const std::vector<int>& args) {
        return args[0] + args[1];
    };

    std::cout << "Starting computations" << std::endl;
    pool.spawn([&graph](auto& sink) {
        calculate_dfs(sink, graph, 0);
    });
    pool.spawn([&graph](auto& sink) {
        calculate_dfs(sink, graph, 4);
    });
    pool.run();
    
    std::cout << "Results:" << std::endl;
    for (auto node: graph) {
      std::cout << node.name << " = " << node.result << std::endl;
    } 

    return 0;
}
