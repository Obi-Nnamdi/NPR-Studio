#include <algorithm>
#include <iostream>
#include <vector>

using namespace std;

// Function to find all of the connected components of a graph
vector<vector<int>> findConnectedComponents(const vector<vector<int>>& adjacencyList) {
  vector<vector<int>> connectedComponents;
  vector<bool> visited(adjacencyList.size(), false);

  for (int i = 0; i < adjacencyList.size(); i++) {
    if (!visited[i]) {
      vector<int> connectedComponent;
      dfs(i, adjacencyList, visited, connectedComponent);
      connectedComponents.push_back(connectedComponent);
    }
  }

  return connectedComponents;
}

// Function to perform depth-first search on a graph
void dfs(int node, const vector<vector<int>>& adjacencyList, vector<bool>& visited,
         vector<int>& connectedComponent) {
  visited[node] = true;
  connectedComponent.push_back(node);

  for (int neighbor : adjacencyList[node]) {
    if (!visited[neighbor]) {
      dfs(neighbor, adjacencyList, visited, connectedComponent);
    }
  }
}

// Function to find the longest path in a connected component
vector<int> findLongestPath(const vector<vector<int>>& adjacencyList,
                            const vector<int>& connectedComponent) {
  vector<int> longestPath;
  int longestPathLength = 0;

  for (int node : connectedComponent) {
    vector<int> currentPath;
    vector<bool> visited;
    dfs(node, adjacencyList, visited, currentPath);

    if (currentPath.size() > longestPathLength) {
      longestPath = currentPath;
      longestPathLength = currentPath.size();
    }
  }

  return longestPath;
}

// Function to convert a path into a polyline
vector<int> convertPathToPolyline(const vector<int>& path) {
  vector<int> polyline;

  for (int i = 0; i < path.size() - 1; i++) {
    polyline.push_back(path[i]);
  }

  polyline.push_back(path[path.size() - 1]);

  return polyline;
}

// Function to find polylines in a graph
vector<vector<int>> findPolylines(const vector<vector<int>>& adjacencyList) {
  vector<vector<int>> polylines;

  for (const vector<int>& connectedComponent : findConnectedComponents(adjacencyList)) {
    vector<int> longestPath = findLongestPath(adjacencyList, connectedComponent);
    polylines.push_back(convertPathToPolyline(longestPath));
  }

  return polylines;
}

int main() {
  // Example usage
  vector<vector<int>> adjacencyList = {{1}, {2}, {3}, {4}, {5}, {6}, {7}, {8}};

  vector<vector<int>> polylines = findPolylines(adjacencyList);

  // Print the result
  for (const vector<int>& polyline : polylines) {
    for (int node : polyline) {
      cout << node << " ";
    }
    cout << endl;
  }

  return 0;
}
