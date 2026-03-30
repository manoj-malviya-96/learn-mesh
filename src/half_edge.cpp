#include "half_edge.h"

#include <unordered_map>


namespace Mesh {

namespace {

struct DirectedEdge {
    Index src;
    Index dst;

    [[nodiscard]] bool operator==(const DirectedEdge& other) const = default;
};

struct DirectedEdgeHash {
    [[nodiscard]] std::size_t operator()(const DirectedEdge& edge) const {
        const std::size_t h1 = std::hash<Index>{}(edge.src);
        const std::size_t h2 = std::hash<Index>{}(edge.dst);
        return h1 ^ (h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2));
    }
};

using EdgeMap = std::unordered_map<DirectedEdge, Index, DirectedEdgeHash>;

bool buildHalfEdgesForTriangle(
    const Index fi,
    const Triangle& tri,
    std::vector<HalfEdge>& halfEdges,
    std::vector<HalfEdgeVertex>& vertices,
    EdgeMap& edgeMap
) {
    const Index lastHalfEdge = halfEdges.size();
    for (Index e = 0; e < NumVerticesPerTriangle; ++e) {
        const Index src = tri[e];
        const Index dst = tri[(e + 1) % NumVerticesPerTriangle];
        const Index newHalfEdge = lastHalfEdge + e;

        halfEdges[newHalfEdge] = HalfEdge{
            .targetVertex = dst,
            .leftFace = fi,
            .nextHalfEdge = lastHalfEdge + (e + 1) % NumVerticesPerTriangle,
        };
        vertices[src].outHalfEdge = newHalfEdge;
        // Check if an edge already exists in the map, which would indicate a non-manifold edge. We should surface this
        // as an error.
        if (auto [it, inserted] = edgeMap.emplace(DirectedEdge{src, dst}, newHalfEdge); !inserted) {
            return false;
        }
    }
    return true;
}

void stitchTwins(const TriMesh& triMesh, std::vector<HalfEdge>& halfEdges, const EdgeMap& edgeMap) {
    for (Index i = 0; i < triMesh.numTriangles(); ++i) {
        const Triangle& tri = triMesh.getTriangle(i);
        const Index heBase = i * NumVerticesPerTriangle;

        for (Index e = 0; e < NumVerticesPerTriangle; ++e) {
            const Index src = tri[e];
            const Index dst = tri[(e + 1) % NumVerticesPerTriangle];
            const Index heIdx = heBase + e;

            if (auto it = edgeMap.find(DirectedEdge{dst, src}); it != edgeMap.end())
                halfEdges[heIdx].twinHalfEdge = it->second;
        }
    }
}


} // namespace

/***

For each triangle i with verts (a, b, c):
he0 = a→b,  he1 = b→c,  he2 = c→a
he0.next = he1,  he1.next = he2,  he2.next = he0
he0.face = he1.face = he2.face = i
triangle[i].halfEdge = he0
edgeMap[(a,b)] = he0,  [(b,c)] = he1,  [(c,a)] = he2

Second pass — twin stitching:
for each (src→dst, heIdx) in edgeMap:
if edgeMap contains (dst→src):
he[heIdx].twin = edgeMap[(dst,src)]
else:
twin = InvalidIndex  ← boundary edge

A few edge cases worth handling explicitly:
Non-manifold edges — (a→b) appears more than once. That's a corrupt mesh, we should surface it as an error.
Boundary edges — twin = InvalidIndex are valid; it means the mesh has a border.
***/

HalfEdgeMesh::HalfEdgeMesh(const TriMesh& triMesh) {
    if (triMesh.empty())
        return;
    this->m_triangles.reserve(triMesh.numTriangles());
    this->m_halfEdges.reserve(triMesh.numVertices());

    // Simply copy
    this->m_vertices = std::invoke([&triMesh] {
        std::vector<HalfEdgeVertex> vertices(triMesh.numVertices());
        for (std::size_t i = 0; i < triMesh.numVertices(); ++i)
            vertices[i].vertex = triMesh.getVertex(i);
        return vertices;
    });

    EdgeMap edgeMap;
    edgeMap.reserve(triMesh.numVertices());
    for (Index i = 0; i < triMesh.numTriangles(); ++i) {
        this->m_triangles[i].halfEdge = m_halfEdges.size();
        if (const auto check = buildHalfEdgesForTriangle(i, triMesh.getTriangle(i), m_halfEdges, m_vertices, edgeMap);
            !check) {
            throw std::runtime_error("Non-manifold edge detected in input mesh");
        }
    }
    stitchTwins(triMesh, m_halfEdges, edgeMap);
}


} // namespace Mesh