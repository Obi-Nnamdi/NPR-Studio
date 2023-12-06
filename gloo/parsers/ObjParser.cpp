#include "ObjParser.hpp"

#include <iostream>
#include <fstream>
#include <sstream>

#include "gloo/utils.hpp"

namespace GLOO {
ObjParser::ParsedData ObjParser::Parse(const std::string& file_path,
                                       bool& success) {
  success = false;
  std::fstream fs(file_path);
  if (!fs) {
    std::cerr << "ERROR: Unable to open OBJ file " + file_path + "!"
              << std::endl;
    return {};
  }

  std::string base_path = GetBasePath(file_path);

  ParsedData data;
  MaterialDict material_dict;

  MeshGroup current_group;
  std::string line;
  while (std::getline(fs, line)) {
    std::stringstream ss(line);
    std::string command;
    ss >> command;
    if (command == "#" || command == "") {
      continue;
    } else if (command == "v") {
      glm::vec3 p;
      ss >> p.x >> p.y >> p.z;
      if (data.positions == nullptr)
        data.positions = make_unique<PositionArray>();
      data.positions->emplace_back(std::move(p));
    } else if (command == "vn") {
      glm::vec3 n;
      ss >> n.x >> n.y >> n.z;
      if (data.normals == nullptr)
        data.normals = make_unique<NormalArray>();
      data.normals->emplace_back(std::move(n));
    } else if (command == "vt") {
      glm::vec2 uv;
      ss >> uv.s >> uv.t;
      if (data.tex_coords == nullptr)
        data.tex_coords = make_unique<TexCoordArray>();
      data.tex_coords->emplace_back(std::move(uv));
    } else if (command == "f") {
      if (data.indices == nullptr)
        data.indices = make_unique<IndexArray>();
      for (int t = 0; t < 3; t++) {
        std::string str;
        ss >> str;
        unsigned int idx;
        if (str.find('/') == std::string::npos) {
          idx = std::stoul(str);
        } else {
          idx = std::stoul(Split(str, '/')[0]);
        }
        // Minus 1 because OBJ indices start with 1.
        data.indices->push_back(idx - 1);
      }
    } else if (command == "g") {
      if (current_group.name != "") {
        current_group.num_indices =
            data.indices->size() - current_group.start_face_index;
        data.groups.push_back(std::move(current_group));
      }
      ss >> current_group.name;
      if (data.indices == nullptr)
        current_group.start_face_index = 0;
      else
        current_group.start_face_index = data.indices->size();
    } else if (command == "usemtl") {
      ss >> current_group.material_name;
    } else if (command == "mtllib") {
      std::string mtl_file;
      ss >> mtl_file;
      material_dict = ParseMTL(base_path + mtl_file);
    } else if (command == "o" || command == "s") {
      std::cout << "Skipped command: " << command << std::endl;
    } else {
      std::cerr << "Unknown obj command: " << command << std::endl;
      success = false;
      continue;
    }
  }

  if (current_group.name != "") {
    current_group.num_indices =
        data.indices->size() - current_group.start_face_index;
    data.groups.push_back(std::move(current_group));
  }

  // Associate materials.
  for (auto& g : data.groups) {
    auto itr = material_dict.find(g.material_name);
    if (itr != material_dict.end())
      g.material = itr->second;
  }

  success = true;
  return data;
}

ObjParser::MaterialDict ObjParser::ParseMTL(const std::string& file_path) {
  std::fstream fs(file_path);
  if (!fs) {
    std::cerr << "ERROR: Unable to open MTL file " + file_path + "!"
              << std::endl;
    return {};
  }
  std::string base_path = GetBasePath(file_path);

  MaterialDict dict;
  std::string line;
  std::shared_ptr<Material> cur_mtl;
  std::string cur_name;
  while (std::getline(fs, line)) {
    std::stringstream ss(line);
    std::string command;
    ss >> command;
    if (command == "#" || command == "") {
      continue;
    } else if (command == "newmtl") {
      if (cur_mtl != nullptr) {
        dict[cur_name] = std::move(cur_mtl);
      }
      ss >> cur_name;
      cur_mtl = std::make_shared<Material>();
    } else if (command == "Ns") {
      float shininess;
      ss >> shininess;
      cur_mtl->SetShininess(shininess);
    } else if (command == "Ka" || command == "Kd" || command == "Ks") {
      glm::vec3 color;
      ss >> color.r >> color.g >> color.b;
      if (command == "Ka")
        cur_mtl->SetAmbientColor(color);
      else if (command == "Kd")
        cur_mtl->SetDiffuseColor(color);
      else {
        assert(command == "Ks");
        cur_mtl->SetSpecularColor(color);
      }
    } else if (command == "map_Ka" || command == "map_Kd" ||
               command == "map_Ks") {
      std::string image_file;
      ss >> image_file;
      auto img = Image::LoadPNG(base_path + image_file, false);
      auto texture = std::make_shared<Texture>();
      texture->UpdateImage(*img);
      if (command == "map_Ka")
        cur_mtl->SetAmbientTexture(texture);
      else if (command == "map_Kd") {
        cur_mtl->SetDiffuseTexture(texture);
      } else {
        assert(command == "map_Ks");
        cur_mtl->SetSpecularTexture(texture);
      }
    } else if (command == "map_bump") {
      // Skip bump map for now.
    } else {
      std::cerr << "Unknown mtl command: " << command << std::endl;
      continue;
    }
  }
  if (cur_mtl != nullptr) {
    dict[cur_name] = std::move(cur_mtl);
  }

  return dict;
}
}  // namespace GLOO
