#pragma once
#include <array>
#include <limits>
#include <vector>

namespace Mesh {

using Index = std::size_t;
constexpr Index NumEdgesOrVerticesInATriangle = 3;
using Vertex = std::array<float, NumEdgesOrVerticesInATriangle>;
using Triangle = std::array<Index, NumEdgesOrVerticesInATriangle>;

class TriMesh {
public:
    TriMesh() = default;

    [[nodiscard]] const std::vector<Vertex>& getVertices() const { return m_vertices; }
    [[nodiscard]] const std::vector<Triangle>& getTriangles() const { return m_triangles; }

    void reserve(const std::size_t numVertices) {
        m_vertices.reserve(numVertices);
        m_triangles.reserve(NumEdgesOrVerticesInATriangle * numVertices);
    }

    [[nodiscard]] bool empty() const { return m_triangles.empty() or m_vertices.empty(); }
    [[nodiscard]] std::size_t numVertices() const { return m_vertices.size(); }
    [[nodiscard]] std::size_t numTriangles() const { return m_triangles.size(); }

    [[nodiscard]] Vertex getVertex(const Index index) const { return m_vertices[index]; }
    [[nodiscard]] Triangle getTriangle(const Index index) const { return m_triangles[index]; }

    void addTriangle(const Triangle& tri) { m_triangles.emplace_back(tri); }
    void addVertex(const Vertex& v) { m_vertices.emplace_back(v); }

private:
    std::vector<Vertex> m_vertices;
    std::vector<Triangle> m_triangles;
};


} // namespace Mesh
