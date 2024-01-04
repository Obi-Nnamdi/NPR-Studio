#ifndef EDGEUTILS_H_
#define EDGEUTILS_H_
#include <algorithm>
#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "../../gloo/shaders/MiterOutlineShader.hpp"
#include "../npr_studio/OutlineNode.hpp"  // Includes polyline def.

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
// TODO: Simplifying Polylines by either a.) index distance, or b.) physical distance

/**
 * Splits polylines in `paths` into new ones, ensuring that no polylines over max_size are left in
 * the resulting `paths` array.
 * TODO: add 3-lengh segment between long polyline breaks to approximate "joins"?
 *
 * @param paths: The vector of polylines to be split.
 * @param max_size: The maximum size of a polyline. Must be > 1.
 *
 * Function testing: test with max_size > pathSize, max_size < pathSize, max_size = pathSize
 * For cylinder:  max_size = 4, 2, 32;
 */
void splitPolylines(std::vector<Polyline>& paths, int max_size) {
  // Enforce precondition
  if (max_size <= 1) {
    throw std::runtime_error("max_size must be > 1");
  }
  std::vector<Polyline> new_polylines;
  // Split existing polylines
  for (auto& polyline : paths) {
    int num_splits = polyline.path.size() / max_size;
    // If the polyline needed to have more than one split, then it's not a loop by definition.
    // If no splits were necessary, just use the existing loop value.
    bool areSplitsLoops = num_splits > 0 ? false : polyline.is_loop;

    // Iterate through polyline to split it, only stopping when we've reached the end of the array
    long currentIndex = 0;
    while ((currentIndex + max_size) <= polyline.path.size()) {
      // Create a new polyline with vertices "max_size" starting at currentIndex.
      Polyline new_polyline;
      long beginning_index = currentIndex;
      long end_index = beginning_index + max_size;
      new_polyline.path = std::vector<size_t>(polyline.path.begin() + beginning_index,
                                              polyline.path.begin() + end_index);
      new_polyline.is_loop = areSplitsLoops;
      new_polylines.push_back(new_polyline);
      // Start the polyline one node "back" if we're going after the first split, just so we can
      // capture the connection to the starting node.
      currentIndex += (max_size - 1);
    }
    long remainingIndices = (polyline.path.size() - 1) - currentIndex;

    // Add ending polyline segment original polyline based on splits we did
    // Don't include polyline if its size would be 0 and the polyline isn't a loop.
    if (remainingIndices > 0) {
      Polyline end_segment;
      end_segment.is_loop = areSplitsLoops;
      end_segment.path =
          std::vector<size_t>(polyline.path.begin() + currentIndex, polyline.path.end());

      // If the polyline was a loop and we split it, add additional vertex that connects the
      // original polyline head with its tail
      auto lineHead = polyline.path.front();
      if (polyline.is_loop && num_splits > 0) {
        end_segment.path.push_back(lineHead);
      }
      new_polylines.push_back(end_segment);
    } else if (remainingIndices == 0 && polyline.is_loop) {
      // We've split the polylines exactly, so push a single line that connects the head to
      // the tail.
      Polyline end_segment;
      end_segment.path = {polyline.path.front(), polyline.path.back()};
      end_segment.is_loop = areSplitsLoops;  // line segment
      new_polylines.push_back(end_segment);
    }
  }

  // Replace our original paths with our new polylines
  paths = new_polylines;
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
  // (prevents seg faulting when rendering due to too many vertices in UBO buffer)
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