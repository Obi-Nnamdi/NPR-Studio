#include "MeshLoader.hpp"

#include <iostream>
#include <algorithm>

#include "gloo/utils.hpp"

namespace GLOO {
MeshData MeshLoader::Import(const std::string& filename) {
  std::string file_path = GetAssetDir() + filename;
  bool success;
  auto parsed_data = ObjParser::Parse(file_path, success);
  if (!success) {
    std::cerr << "Load mesh file " << filename << " failed!" << std::endl;
    return {};
  }
  // Remove empty groups.
  parsed_data.groups.erase(
      std::remove_if(parsed_data.groups.begin(), parsed_data.groups.end(),
                     [](MeshGroup& g) { return g.num_indices == 0; }),
      parsed_data.groups.end());

  MeshData mesh_data;
  mesh_data.vertex_obj = make_unique<VertexObject>();
  if (parsed_data.positions) {
    mesh_data.vertex_obj->UpdatePositions(std::move(parsed_data.positions));
  }
  if (parsed_data.normals) {
    mesh_data.vertex_obj->UpdateNormals(std::move(parsed_data.normals));
  }
  if (parsed_data.tex_coords) {
    mesh_data.vertex_obj->UpdateTexCoord(std::move(parsed_data.tex_coords));
  }
  if (parsed_data.indices) {
    mesh_data.vertex_obj->UpdateIndices(std::move(parsed_data.indices));
  }

  mesh_data.groups = std::move(parsed_data.groups);

  return mesh_data;
}
}  // namespace GLOO
