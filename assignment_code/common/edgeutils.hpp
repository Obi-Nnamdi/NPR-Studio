#ifndef EDGEUTILS_H_
#define EDGEUTILS_H_
#include <algorithm>
#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "../assignment6/OutlineNode.hpp"  // Includes polyline def.
#include "../code/gloo/shaders/MiterOutlineShader.hpp"

namespace GLOO {

// We only consider loops if they're more than 2 nodes long (3-length cycles and up)
int edge_cycle_length = 3;

// Function to perform depth-first search to find the longest chain
void dfs(size_t node, const std::unordered_map<size_t, std::unordered_set<size_t>>& adjList,
         std::unordered_set<size_t>& visited, std::vector<size_t>& currentPath,
         std::vector<size_t>& connectedComponent, std::vector<Polyline>& paths) {
  visited.insert(node);
  connectedComponent.push_back(node);
  currentPath.push_back(node);

  int counter = 0;
  for (size_t neighbor : adjList.at(node)) {
    // TODO: if we have multiple neighbors, find a way to combine the paths produced by all of
    // them together somehow
    if (visited.find(neighbor) == visited.end()) {
      dfs(neighbor, adjList, visited, currentPath, connectedComponent, paths);
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
      // We only consider loops if they're more than 2 nodes long (3-length cycles and up)
      line.is_loop =
          adjList.at(firstElt).count(lastElt) > 0 && currentPath.size() >= edge_cycle_length;
      line.path = currentPath;
      paths.push_back(line);
    }
  }

  // Backtrack
  currentPath.pop_back();
}

/**
 * Splits polylines in `paths` into new ones, ensuring that no polylines over max_size are left in
 * the resulting `paths` array.
 *
 * @param paths: The vector of polylines to be split.
 * @param max_size: The maximum size of a polyline. Must be > 0.
 */
void splitPolylines(std::vector<Polyline>& paths, int max_size) {
  // Enforce precondition
  if (max_size <= 0) {
    throw std::runtime_error("max_size must be > 0");
  }

  for (auto& polyline : paths) {
    if (polyline.path.size() > max_size) {
      int num_splits = polyline.path.size() / max_size;
      for (int i = 0; i < num_splits; i++) {
        Polyline new_polyline;
        new_polyline.path = std::vector<size_t>(polyline.path.begin() + i * max_size,
                                                polyline.path.begin() + (i + 1) * max_size);
        // If the polyline needed to have more than one split, then it's not a loop by definition.
        // If no splits were necessary, just use the existing loop parameter.
        new_polyline.is_loop = num_splits > 0 ? false : polyline.is_loop;
        paths.push_back(new_polyline);
        // Also add mini-polyline that connects the two polylines we just split ()
        if (polyline.path.size() > (i + 1) * max_size) {
          Polyline mini_polyline;
          mini_polyline.path = {new_polyline.path.back(), polyline.path[(i + 1) * max_size]};
          mini_polyline.is_loop = true;  // it's a loop by definition.
          paths.push_back(mini_polyline);
        }
      }
      polyline.path =
          std::vector<size_t>(polyline.path.begin() + num_splits * max_size, polyline.path.end());
    }
  }
}

// Performs dfs on `node` using give adjList, but visits every edge instead of just visiting every
// node.
void edgeDfs(size_t node, const std::unordered_map<size_t, std::unordered_set<size_t>>& adjList,
             std::unordered_set<Edge, pairhash, KeyEqual>& visited,
             std::unordered_set<size_t>& finished, std::vector<size_t>& currentPath,
             std::vector<Polyline>& paths) {
  currentPath.push_back(node);

  // Go through each node's connection
  int counter = 0;
  for (size_t neighbor : adjList.at(node)) {
    Edge connection = Edge(node, neighbor);

    // TODO: if we have multiple neighbors, find a way to combine the paths produced by all of them
    // together somehow
    if (visited.find(connection) == visited.end()) {
      // Mark connection as visited
      visited.insert(connection);
      edgeDfs(neighbor, adjList, visited, finished, currentPath, paths);
      counter++;
    }
  }
  // Once we're done exploring the node's connections, mark it as finished
  finished.insert(node);

  // Emit path if we're at a leaf
  // TODO: add some way to prune the path to prevent too much work?
  if (counter == 0) {
    if (currentPath.size() > 0) {
      Polyline line;
      auto firstElt = currentPath.front();
      auto lastElt = currentPath.back();
      // We only consider loops if they're more than 2 nodes long (3-length cycles and up)
      line.is_loop = firstElt == lastElt && currentPath.size() >= edge_cycle_length;
      line.path = currentPath;
      // Remove the last element from the polyline if the line is a loop since it's the same as the
      // first (allows for easier logic later)
      if (line.is_loop) {
        line.path.pop_back();
      }
      paths.push_back(line);
    }
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
  // Perform edge-DFS to find the longest path for each connected component
  // Use edge-dfs to ensure all connections between nodes get rendered, not just each node.
  std::unordered_set<size_t> finished;
  std::unordered_set<Edge, pairhash, KeyEqual> visited;

  // Find connected components
  for (const auto& entry : adjList) {
    // Go through each node that hasn't been finished yet
    size_t node = entry.first;
    if (finished.find(node) == finished.end()) {
      std::vector<size_t> currentPath;
      edgeDfs(node, adjList, visited, finished, currentPath, paths);
    }
  }

  // Split polylines into paths less than the maximum UBO array size defined in MiterOutlineShader.
  // (prevents seg faulting when rendering)
  // TODO: the sponza_low scene still errors when rendering a lot of lines due to a std::length
  // error.
  int splitLength = (int)maxUBOArraySize - 10;
  if (splitLength <= 0) {
    throw std::runtime_error("Polyline split length <= 0. Adjust maxUBOArraySize");
  }
  splitPolylines(paths, splitLength);

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
#endif