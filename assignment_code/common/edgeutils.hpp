#include <algorithm>
#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "../assignment6/OutlineNode.hpp"

namespace GLOO {

/**
 * Struct defining a polyline with a path, that can also be a loop.
 * A polyline with a loop won't have any of its path indices duplicated,
 * but its first element and last element are connected.
 */
struct Polyline {
  std::vector<size_t> path;
  bool is_loop;
};

// Function to perform depth-first search to find the longest chain
void dfs(size_t node, const std::unordered_map<size_t, std::unordered_set<size_t>>& adjList,
         std::unordered_set<size_t>& visited, std::vector<size_t>& currentPath,
         std::vector<size_t>& longestPath, std::vector<Polyline>& paths) {
  visited.insert(node);
  currentPath.push_back(node);

  int counter = 0;
  for (size_t neighbor : adjList.at(node)) {
    if (visited.find(neighbor) == visited.end()) {
      dfs(neighbor, adjList, visited, currentPath, longestPath, paths);
      counter++;
    }
  }

  // Emit path if we're at a leaf
  // TODO: add some way to prune the path to prevent too much work?
  if (counter == 0) {
    if (currentPath.size() > 0) {
      Polyline line;
      auto firstElt = currentPath.front();
      auto lastElt = currentPath.back();
      // We only consider loops if they're more than 1 node long (2-length cycles and up)
      int edge_cycle_length = 2;
      line.is_loop =
          adjList.at(firstElt).count(lastElt) > 0 && currentPath.size() >= edge_cycle_length;
      line.path = currentPath;
      paths.push_back(line);
    }
  }

  // Update the longest path
  // TODO: longestPath really isn't needed.
  if (currentPath.size() > longestPath.size()) {
    longestPath = currentPath;
  }

  // Backtrack
  currentPath.pop_back();
}

/**
 * Function to transform edges to polylines. A polyline is a consecutive list of vertices that
 * traverse a "chain" of connected vertices in a graph. Every vertex is guaranteed to be represented
 * at aleast once in the list of polylines. Unfortunately, there's no guarantee that the polylines
 * are efficient, i.e. that the longest polyline represents the longest possible chain length of the
 * graph (since that is an NP-hard problem).
 */
std::vector<Polyline> edgesToPolylines(const std::vector<Edge>& edges) {
  // Build an adjacency list from the edges
  std::unordered_map<size_t, std::unordered_set<size_t>> adjList;
  for (const Edge& edge : edges) {
    adjList[edge.first].insert(edge.second);
    adjList[edge.second].insert(edge.first);
  }

  std::vector<Polyline> paths;
  // Perform DFS to find the longest path for each connected component
  std::unordered_set<size_t> visited;
  std::vector<std::vector<size_t>> result;

  for (const auto& entry : adjList) {
    // Go through each node that hasn't been visted yet
    size_t node = entry.first;
    if (visited.find(node) == visited.end()) {
      std::vector<size_t> currentPath, longestPath;
      dfs(node, adjList, visited, currentPath, longestPath, paths);
      result.push_back(longestPath);
    }
  }

  // for (const auto& path : paths) {
  //   std::cout << "Path: ";
  //   for (size_t node : path) {
  //     std::cout << node << " ";
  //   }
  //   std::cout << std::endl;
  // }

  return paths;
}

int main() {
  // Example usage
  // std::vector<Edge> edges = {
  //     {0, 1}, {1, 2}, {2, 3}, {3, 4}, {5, 6}, {6, 7}, {8, 1},
  // }; // no loop

  // Has 1 loop (8, 9)
  std::vector<Edge> edges = {{0, 1}, {1, 2}, {2, 3}, {3, 4}, {5, 0}, {8, 9}};

  std::vector<Polyline> polylines = edgesToPolylines(edges);

  // Print the result
  for (const auto& polyline : polylines) {
    std::cout << "Polyline: ";
    for (size_t node : polyline.path) {
      std::cout << node << " ";
    }
    std::cout << std::endl;
    std::cout << "Loop: " << (polyline.is_loop ? "True" : "False");
    std::cout << std::endl;
  }

  return 0;
}
}  // namespace GLOO