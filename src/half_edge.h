#pragma once
#include <array>
#include <vector>
#include "tri_mesh.h"


namespace Mesh {


struct HalfEdgeVertex {
    Vertex vertex{};
    Index outHalfEdge = InvalidIndex; // Any outward Edge From the vertex.
};

struct HalfEdge {
    Index targetVertex = InvalidIndex; // Vertex at the end of this half-edge, where it's coming out from
    Index leftFace = InvalidIndex;     // Left Face
    Index nextHalfEdge = InvalidIndex; // Right Half edge next
    Index twinHalfEdge = InvalidIndex; // The half-edge in the opposite direction
};

struct HalfEdgeTriangle {
    Index halfEdge = InvalidIndex; // Any bounding HalfEdge
};

class HalfEdgeMesh {
public:
    HalfEdgeMesh() = default;
    explicit HalfEdgeMesh(const TriMesh& triMesh);

    [[nodiscard]] std::vector<HalfEdgeVertex>& vertices() { return m_vertices; }
    [[nodiscard]] const std::vector<HalfEdgeVertex>& vertices() const { return m_vertices; }
    [[nodiscard]] std::vector<HalfEdge>& halfEdges() { return m_halfEdges; }
    [[nodiscard]] const std::vector<HalfEdge>& halfEdges() const { return m_halfEdges; }
    [[nodiscard]] std::vector<HalfEdgeTriangle>& triangles() { return m_triangles; }
    [[nodiscard]] const std::vector<HalfEdgeTriangle>& triangles() const { return m_triangles; }

private:
    std::vector<HalfEdgeVertex> m_vertices;
    std::vector<HalfEdge> m_halfEdges;
    std::vector<HalfEdgeTriangle> m_triangles;
};


} // namespace Mesh