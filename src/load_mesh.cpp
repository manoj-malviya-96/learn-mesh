#include "load_mesh.h"
#include <cmath>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include "tri_mesh.h"

namespace Mesh {

namespace {

auto makeError(const LoadErrorCode code, std::string msg) { return std::unexpected(LoadError{code, std::move(msg)}); }

float readFloat(const uint8_t* p) {
    float v;
    std::memcpy(&v, p, 4);
    return v;
}
uint32_t readU32(const uint8_t* p) {
    uint32_t v;
    std::memcpy(&v, p, 4);
    return v;
}

Vertex readVertex(const uint8_t* p) { return {readFloat(p), readFloat(p + 4), readFloat(p + 8)}; }

std::string_view trim(std::string_view s) {
    const auto b = s.find_first_not_of(" \t\r\n");
    if (b == std::string_view::npos)
        return {};
    return s.substr(b, s.find_last_not_of(" \t\r\n") - b + 1);
}


bool looksLikeASCII(const std::vector<uint8_t>& data) {
    if (data.size() < 5)
        return false;
    const std::string_view head(reinterpret_cast<const char*>(data.data()), std::min(data.size(), std::size_t{256}));
    const auto pos = head.find_first_not_of(" \t\r\n");
    if (pos == std::string_view::npos || head.substr(pos, 5) != "solid")
        return false;
    const std::string_view full(reinterpret_cast<const char*>(data.data()), data.size());
    return full.find("facet") != std::string_view::npos;
}

LoadMeshResult<std::vector<uint8_t>> readFile(const std::string& path) {
    std::cout << "Reading file: " << path << std::endl;
    std::ifstream file(path, std::ios::binary | std::ios::ate);

    if (!file)
        return makeError(LoadErrorCode::FileNotFound, "File not found: " + path);

    const auto size = static_cast<std::size_t>(file.tellg());
    file.seekg(0);

    std::vector<uint8_t> data(size);
    if (!file.read(reinterpret_cast<char*>(data.data()), size))
        return makeError(LoadErrorCode::ReadError, "Read error: " + path);

    return data;
}

std::expected<TriMesh, LoadError> parseASCII(const std::string& text) {
    TriMesh mesh;
    std::istringstream ss(text);
    std::string line;

    auto nextLine = [&]() -> bool {
        while (std::getline(ss, line)) {
            line = std::string(trim(line));
            if (!line.empty())
                return true;
        }
        return false;
    };

    auto expect = [&](const std::string_view prefix) -> LoadMeshResult<void> {
        if (!nextLine() || line.rfind(prefix, 0) != 0)
            return makeError(LoadErrorCode::ParseError, "Expected '" + std::string(prefix) + "', got: " + line);
        return {};
    };

    if (auto r = expect("solid"); !r)
        return std::unexpected(r.error());

    while (nextLine()) {
        if (line.rfind("endsolid", 0) == 0)
            break;

        if (line.rfind("facet normal", 0) != 0)
            return makeError(LoadErrorCode::ParseError, "Expected 'facet normal', got: " + line);

        if (auto r = expect("outer loop"); !r)
            return std::unexpected(r.error());

        const auto base = static_cast<uint32_t>(mesh.numVertices());
        for (int v = 0; v < 3; ++v) {
            if (auto r = expect("vertex"); !r)
                return std::unexpected(r.error());
            std::istringstream ls(line.substr(6));
            Vertex vtx;
            if (!(ls >> vtx[0] >> vtx[1] >> vtx[2]))
                return makeError(LoadErrorCode::ParseError, "Bad vertex: " + line);
            mesh.addVertex(vtx);
        }

        mesh.addTriangle({base, base + 1, base + 2});

        if (auto r = expect("endloop"); !r)
            return std::unexpected(r.error());
        if (auto r = expect("endfacet"); !r)
            return std::unexpected(r.error());
    }

    return mesh;
}


LoadMeshResult<TriMesh> parseBinary(const std::vector<uint8_t>& data) {
    constexpr std::size_t kHeader = 80;
    constexpr std::size_t kTriBytes = 50;

    if (data.size() < kHeader + 4) {
        return makeError(LoadErrorCode::ParseError, "File too small for binary STL");
    }

    const uint32_t triCount = readU32(data.data() + kHeader);
    const std::size_t expected = kHeader + 4 + triCount * kTriBytes;

    if (data.size() < expected)
        return makeError(
            LoadErrorCode::ParseError,
            "Binary STL truncated: expected " + std::to_string(expected) + " bytes, got " + std::to_string(data.size())
        );

    TriMesh mesh;
    mesh.reserve(triCount);

    const uint8_t* p = data.data() + kHeader + 4;
    for (uint32_t i = 0; i < triCount; ++i, p += kTriBytes) {
        const auto base = static_cast<uint32_t>(mesh.numVertices());
        mesh.addVertex(readVertex(p + 12));
        mesh.addVertex(readVertex(p + 24));
        mesh.addVertex(readVertex(p + 36));
        mesh.addTriangle({base, base + 1, base + 2});
    }
    return mesh;
}


LoadMeshResult<TriMesh> parseSTL(const std::vector<uint8_t>& data) {
    std::cout << "Parsing STL" << std::endl;
    const auto result = looksLikeASCII(data)
        ? parseASCII(std::string(reinterpret_cast<const char*>(data.data()), data.size()))
        : parseBinary(data);

    if (!result)
        return result;

    if (result->empty())
        return makeError(LoadErrorCode::EmptyMesh, "STL contains no triangles");

    return result;
}
} // namespace

LoadMeshResult<TriMesh> loadMesh(const std::string& path) { return readFile(path).and_then(parseSTL); }


} // namespace Mesh