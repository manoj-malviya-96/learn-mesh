#pragma once
#include <array>
#include <vector>

namespace Mesh {
using Vertex = std::array<float, 3>;
using Triangle = std::array<u_int32_t, 3>;

class TriMesh {
public:
    TriMesh() = default;

    [[nodiscard]] const std::vector<Vertex>& getVertices() const { return m_vertices; }
    [[nodiscard]] const std::vector<Triangle>& getTriangles() const { return m_triangles; }

    void reserve(const std::size_t n) {
        m_vertices.reserve(n);
        m_triangles.reserve(3 * n);
    }

    [[nodiscard]] bool empty() const { return m_triangles.empty() or m_vertices.empty(); }
    [[nodiscard]] std::size_t numVertices() const { return m_vertices.size(); }
    [[nodiscard]] std::size_t numTriangles() const { return m_triangles.size(); }

    void addTriangle(const Triangle& tri) { m_triangles.push_back(tri); }
    void addVertex(const Vertex& v) { m_vertices.push_back(v); }


private:
    std::vector<Vertex> m_vertices;
    std::vector<Triangle> m_triangles;
};


} // namespace Mesh
