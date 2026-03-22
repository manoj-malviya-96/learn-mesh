#pragma once
#include <expected>
#include "tri_mesh.h"


namespace Mesh {

enum class LoadErrorCode { FileNotFound, InvalidFileFormat, ParseError, ReadError, EmptyMesh, Unknown };

struct LoadError {
    LoadErrorCode code;
    std::string message;
};


template <typename T>
using LoadMeshResult = std::expected<T, LoadError>;

LoadMeshResult<TriMesh> loadMesh(const std::string& path);

} // namespace Mesh