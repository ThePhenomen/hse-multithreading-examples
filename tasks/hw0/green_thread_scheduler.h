#pragma once

#include <vector>
#include <string>
#include <iostream>
#include <functional>
#include <memory>
#include <stdexcept>
#include <boost/coroutine2/all.hpp>

enum class State { Unvisited, WaitingChildren, Calculated };

struct GraphNode {
    std::string name;
    std::vector<int> node_dependencies;
    std::function<int(const std::vector<int>&)> calc;
    State state = State::Unvisited;
    int result = 0;
};

inline void calculate_dfs(boost::coroutines2::coroutine<int>::push_type& sink, std::vector<GraphNode>& graph, int root_node) {
    std::vector<int> stack;
    stack.push_back(root_node);

    while (!stack.empty()) {
        int node = stack.back();

        if (graph[node].state == State::Calculated) {
            stack.pop_back();
            continue;
        }

        if (graph[node].state == State::Unvisited) {
            graph[node].state = State::WaitingChildren; 
            
            for (auto it = graph[node].node_dependencies.rbegin(); it != graph[node].node_dependencies.rend(); ++it) {
                int node_dependency_idx = *it;

                if (graph[node_dependency_idx].state == State::Unvisited) {
                    stack.push_back(node_dependency_idx);
                } else if (graph[node_dependency_idx].state == State::WaitingChildren) {
                    throw std::runtime_error("Cycle detected involving " + graph[node_dependency_idx].name);
                }
            }
        } 
        else if (graph[node].state == State::WaitingChildren) {
            std::vector<int> node_dependencies_values;
            for (int dep : graph[node].node_dependencies) {
                node_dependencies_values.push_back(graph[dep].result);
            }
            
            graph[node].result = graph[node].calc(node_dependencies_values);
            graph[node].state = State::Calculated;
            
            sink(graph[node].result);
            stack.pop_back();
        }
    }
}

class Scheduler {
private:
    std::vector<boost::coroutines2::coroutine<int>::pull_type> tasks;
public:
    using TaskFunc = std::function<void(boost::coroutines2::coroutine<int>::push_type&)>;

    void spawn(TaskFunc func) {
        tasks.emplace_back(boost::context::fixedsize_stack(64 * 1024), std::move(func));
    }

    void run() {
        bool active = true;
        while (active) {
            active = false;
            for (auto& coro : tasks) {
                if (coro) {
                    active = true;
                    try {
                        coro.get();
                        coro();
                    } catch (const std::exception& e) {
                        std::cout << "Scheduler Error: " << e.what() << std::endl;
                    }
                }
            }
        }
    }
};

inline void calc_graph(std::vector<GraphNode>& graph) {
    std::vector<bool> is_dependency(graph.size(), false);
    for (const auto& node : graph) {
        for (int dep : node.node_dependencies) {
            is_dependency[dep] = true;
        }
    }

    Scheduler pool;
    int spawned_tasks = 0;

    for (size_t i = 0; i < graph.size(); ++i) {
        if (!is_dependency[i]) {
            pool.spawn([&graph, i](auto& sink) {
                calculate_dfs(sink, graph, i);
            });
            spawned_tasks++;
        }
    }

    if (spawned_tasks == 0 && !graph.empty()) {
        std::cout << "No independent roots found" << std::endl;
        return;
    }

    pool.run();
}
