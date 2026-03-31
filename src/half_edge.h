#pragma once
#include <array>
#include <optional>
#include <vector>
#include "tri_mesh.h"


namespace Mesh {

struct HalfEdgeVertex {
    Vertex vertex;
    Index outHalfEdge;
};

struct HalfEdge {
    Index targetVertex;
    Index leftFace;
    Index nextHalfEdge;
    Index prevHalfEdge;
    std::optional<Index> twinHalfEdge;
};

struct HalfEdgeTriangle {
    Index halfEdge;
};

class HalfEdgeMesh {
public:
    HalfEdgeMesh() = default;
    explicit HalfEdgeMesh(const TriMesh& triMesh);

    [[nodiscard]] std::size_t numVertices() const { return m_vertices.size(); }
    [[nodiscard]] std::size_t numTriangles() const { return m_triangles.size(); }

    [[nodiscard]] std::vector<HalfEdgeVertex>& vertices() { return m_vertices; }
    [[nodiscard]] const std::vector<HalfEdgeVertex>& vertices() const { return m_vertices; }
    [[nodiscard]] std::vector<HalfEdge>& halfEdges() { return m_halfEdges; }
    [[nodiscard]] const std::vector<HalfEdge>& halfEdges() const { return m_halfEdges; }
    [[nodiscard]] std::vector<HalfEdgeTriangle>& triangles() { return m_triangles; }
    [[nodiscard]] const std::vector<HalfEdgeTriangle>& triangles() const { return m_triangles; }


    [[nodiscard]] TriMesh triMesh() const;

private:
    std::vector<HalfEdgeVertex> m_vertices;
    std::vector<HalfEdge> m_halfEdges;
    std::vector<HalfEdgeTriangle> m_triangles;
};


} // namespace Mesh