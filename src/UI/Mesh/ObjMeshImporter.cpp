//
// Created by zhou_zhengming on 2026/5/20.
//

#include "UI/Mesh/ObjMeshImporter.h"

#include <DirectXColors.h>
#include <DirectXMath.h>
#include <fstream>
#include <sstream>

using namespace z8;
using namespace std;
using namespace DirectX;

Mesh z8::ObjMeshImporter::Parse(std::string FileName) {
  ifstream ifs(FileName);
  assert(ifs);
  Mesh M;
  std::string line;
  while (std::getline(ifs, line)) {
    std::istringstream iss(line);
    std::string type;
    iss >> type;

    // 解析顶点 v
    if (type == "v") {
      float x, y, z;
      iss >> x >> y >> z;
      M.V.emplace_back(x, y, z);
    }
    // 解析面 f（只取顶点索引）
    else if (type == "f") {
      std::vector<unsigned> faceIndices;
      std::string part;

      // 读取面的每一段：1/1/1  2/2/2  3/3/3
      while (iss >> part) {
        // 只取 / 前面的顶点索引
        size_t slashPos = part.find('/');
        if (slashPos != std::string::npos) {
          part = part.substr(0, slashPos);
        }

        // OBJ 索引从 1 开始 → 转 0
        unsigned idx = static_cast<unsigned>(std::stoi(part)) - 1;
        faceIndices.push_back(idx);
      }

      // 三角化：四边形 → 两个三角形
      if (faceIndices.size() >= 3) {
        M.I.push_back(faceIndices[0]);
        M.I.push_back(faceIndices[1]);
        M.I.push_back(faceIndices[2]);

        if (faceIndices.size() == 4) {
          M.I.push_back(faceIndices[0]);
          M.I.push_back(faceIndices[2]);
          M.I.push_back(faceIndices[3]);
        }
      }
    }
  }

  return M;
}