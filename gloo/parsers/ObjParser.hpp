#ifndef GLOO_OBJ_PARSER_H_
#define GLOO_OBJ_PARSER_H_

#include <string>
#include <memory>
#include <unordered_map>

#include "gloo/alias_types.hpp"
#include "gloo/MeshData.hpp"

namespace GLOO {
class ObjParser {
 public:
  struct ParsedData {
    std::unique_ptr<PositionArray> positions;
    std::unique_ptr<NormalArray> normals;
    std::unique_ptr<IndexArray> indices;
    std::unique_ptr<TexCoordArray> tex_coords;

    std::vector<MeshGroup> groups;
  };

  static ParsedData Parse(const std::string& file_path, bool& success);

 private:
  using MaterialDict =
      std::unordered_map<std::string, std::shared_ptr<Material>>;

  static MaterialDict ParseMTL(const std::string& file_path);
};
}  // namespace GLOO

#endif
