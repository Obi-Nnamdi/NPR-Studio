// TODO: rename and restructure edgeutil file.
// #include <iostream>
// #include <vector>

// #include "edgeutils.hpp"

// // Example usage
// int main() {
//   // Example usage
//   // std::vector<Edge> edges = {
//   //     {0, 1}, {1, 2}, {2, 3}, {3, 4}, {5, 6}, {6, 7}, {8, 1},
//   // }; // no loop

//   // Has 1 loop (8, 9)
//   std::vector<GLOO::Edge> edges = {{0, 1}, {1, 2}, {2, 3}, {3, 4}, {5, 0}, {8, 9}};

//   std::vector<GLOO::Polyline> polylines = GLOO::edgesToPolylines(edges);

//   // Print the result
//   for (const auto& polyline : polylines) {
//     std::cout << "Polyline: ";
//     for (size_t node : polyline.path) {
//       std::cout << node << " ";
//     }
//     std::cout << std::endl;
//     std::cout << "Loop: " << (polyline.is_loop ? "True" : "False");
//     std::cout << std::endl;
//   }

//   return 0;
// }