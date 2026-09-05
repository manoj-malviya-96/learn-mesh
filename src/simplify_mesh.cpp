

#include "simplify_mesh.h"
#include "mat4.h"

#include <queue>
#include <unordered_set>
#include <vector>

namespace Mesh {

namespace {

constexpr std::size_t NumSharedNeighborsOnInteriorEdge = NumEdgesOrVerticesInATriangle - 1;
constexpr std::size_t NumSharedNeighborsOnBoundaryEdge = NumEdgesOrVerticesInATriangle - 2;

// Source vertex of a half-edge: target of its twin, or walk prev
// prev(he) = next(next(he))  in a triangle
Index prevHE(const HalfEdgeMesh& mesh, const Index he) {
    return mesh.halfEdges()[mesh.halfEdges()[he].nextHalfEdge].nextHalfEdge;
}

Index srcVertex(const HalfEdgeMesh& mesh, const Index he) { return mesh.halfEdges()[prevHE(mesh, he)].targetVertex; }


std::vector<Index> outgoingHalfEdges(const HalfEdgeMesh& mesh, const Index v) {
    std::vector<Index> result;
    const Index start = mesh.vertices()[v].outHalfEdge;
    if (start == InvalidIndex)
        return result;

    Index he = start;
    do {
        result.push_back(he);
        // Step CCW: prev → twin
        const Index prev = prevHE(mesh, he);
        const auto twin = mesh.halfEdges()[prev].twinHalfEdge;
        if (!twin.has_value())
            break;
        he = *twin;
    } while (he != start);

    return result;
}

// Collect the set of vertex indices in the one-ring of v (excluding v itself)
std::unordered_set<Index> oneRing(const HalfEdgeMesh& mesh, const Index v) {
    std::unordered_set<Index> ring;
    for (const Index he : outgoingHalfEdges(mesh, v))
        ring.insert(mesh.halfEdges()[he].targetVertex);
    return ring;
}

// ─────────────────────────────────────────────
//  Quadric initialisation
// ─────────────────────────────────────────────

// Compute the plane equation (a,b,c,d) for the triangle containing halfEdge `he`
Mat4 quadricForTriangle(const HalfEdgeMesh& mesh, const Index he) {
    const auto& HE = mesh.halfEdges();
    const auto& V = mesh.vertices();

    const Index v0 = HE[he].targetVertex;
    const Index v1 = HE[HE[he].nextHalfEdge].targetVertex;
    const Index v2 = srcVertex(mesh, he);

    auto toVec3 = [&](const Index vi) -> Vec3 {
        const auto& p = V[vi].vertex;
        return {p[0], p[1], p[2]};
    };

    const Vec3 p0 = toVec3(v0);
    const Vec3 p1 = toVec3(v1);
    const Vec3 p2 = toVec3(v2);

    // Normal = (p1-p0) × (p2-p0)
    const Vec3 e1{p1.x - p0.x, p1.y - p0.y, p1.z - p0.z};
    Vec3 e2{p2.x - p0.x, p2.y - p0.y, p2.z - p0.z};
    Vec3 n{e1.y * e2.z - e1.z * e2.y, e1.z * e2.x - e1.x * e2.z, e1.x * e2.y - e1.y * e2.x};
    n = n.normalised();

    const double d = -(n.x * p0.x + n.y * p0.y + n.z * p0.z);
    return Mat4::fromPlane(n.x, n.y, n.z, d);
}

// Sum quadrics from all triangles around vertex v
Mat4 computeVertexQuadric(const HalfEdgeMesh& mesh, const Index v) {
    Mat4 Q;
    for (const Index he : outgoingHalfEdges(mesh, v))
        if (mesh.halfEdges()[he].leftFace != InvalidIndex)
            Q += quadricForTriangle(mesh, he);
    return Q;
}

// ─────────────────────────────────────────────
//  Link condition check
//
//  An edge (u, v) can be safely collapsed iff:
//    N(u) ∩ N(v) == exactly the vertices shared by the two adjacent triangles
//
//  For a manifold interior edge that shared set should be exactly 2 vertices.
//  For a boundary edge it should be exactly 1.
// ─────────────────────────────────────────────

bool linkCondition(const HalfEdgeMesh& mesh, const Index he) {
    const Index u = srcVertex(mesh, he);
    const Index v = mesh.halfEdges()[he].targetVertex;

    const auto ringU = oneRing(mesh, u);
    const auto ringV = oneRing(mesh, v);

    // Count shared neighbors
    std::size_t shared = 0;
    for (const Index nb : ringU)
        if (ringV.contains(nb))
            ++shared;

    const bool hasTwin = mesh.halfEdges()[he].twinHalfEdge.has_value();
    const std::size_t expectedShared = hasTwin ? NumSharedNeighborsOnInteriorEdge : NumSharedNeighborsOnBoundaryEdge;

    return shared == expectedShared;
}

struct CollapseCandidate {
    Index halfEdge = InvalidIndex;
    double cost = 0.0;
    Vec4 optimalPos{};

    bool operator>(const CollapseCandidate& o) const { return cost > o.cost; }
};

CollapseCandidate evaluateCollapse(const HalfEdgeMesh& mesh, const Index he, const std::vector<Mat4>& quadrics) {
    const Index u = srcVertex(mesh, he);
    const Index v = mesh.halfEdges()[he].targetVertex;

    const Mat4 Q = quadrics[u] + quadrics[v];

    // Try to solve for optimal position; fall back to midpoint if singular
    Vec4 pos;
    if (const auto opt = Q.solveOptimalPosition()) {
        pos = *opt;
    } else {
        const auto& pu = mesh.vertices()[u].vertex;
        const auto& pv = mesh.vertices()[v].vertex;
        pos = Vec4{(pu[0] + pv[0]) * 0.5, (pu[1] + pv[1]) * 0.5, (pu[2] + pv[2]) * 0.5, 1.0};
    }

    return CollapseCandidate{he, Q.evaluate(pos), pos};
}


void collapseEdge(
    HalfEdgeMesh& mesh,
    const Index halfEdge,
    const Vec4& optimalPos,
    std::vector<Mat4>& quadrics,
    std::vector<bool>& deadVertex,
    std::vector<bool>& deadFace,
    std::vector<bool>& deadHE
) {
    auto& halfEdges = mesh.halfEdges();
    auto& vertices = mesh.vertices();

    const auto twin = halfEdges[halfEdge].twinHalfEdge;
    const Index u = srcVertex(mesh, halfEdge);
    const Index v = halfEdges[halfEdge].targetVertex;

    // ── Move v to optimal position ─────────────────────────────────────
    vertices[v].vertex = {
        static_cast<float>(optimalPos.x), static_cast<float>(optimalPos.y), static_cast<float>(optimalPos.z)
    };
    quadrics[v] = quadrics[u] + quadrics[v];

    // ── Retarget all half-edges pointing TO u → point to v instead ─────
    for (const Index outHE : outgoingHalfEdges(mesh, u)) {
        // The half-edge coming INTO u is prev(outHE), its target is u
        if (const Index inHE = prevHE(mesh, outHE); halfEdges[inHE].targetVertex == u)
            halfEdges[inHE].targetVertex = v;
    }

    // ── Remove the left face of halfEdge
    {
        const Index fi = halfEdges[halfEdge].leftFace;
        const Index next = halfEdges[halfEdge].nextHalfEdge;
        const Index prev = prevHE(mesh, halfEdge);

        // Relink twins of the side edges to each other (bypass the face)
        const auto nextTwin = halfEdges[next].twinHalfEdge;
        const auto prevTwin = halfEdges[prev].twinHalfEdge;
        if (nextTwin.has_value())
            halfEdges[*nextTwin].twinHalfEdge = prevTwin;
        if (prevTwin.has_value())
            halfEdges[*prevTwin].twinHalfEdge = nextTwin;

        deadHE[halfEdge] = deadHE[next] = deadHE[prev] = true;
        if (fi != InvalidIndex)
            deadFace[fi] = true;
    }

    // ── Remove the left face of twin (if it exists)
    if (twin.has_value()) {
        const Index twinIndex = *twin;
        const Index fi = halfEdges[twinIndex].leftFace;
        const Index next = halfEdges[twinIndex].nextHalfEdge;
        const Index prev = prevHE(mesh, twinIndex);

        const auto nextTwin = halfEdges[next].twinHalfEdge;
        const auto prevTwin = halfEdges[prev].twinHalfEdge;
        if (nextTwin.has_value())
            halfEdges[*nextTwin].twinHalfEdge = prevTwin;
        if (prevTwin.has_value())
            halfEdges[*prevTwin].twinHalfEdge = nextTwin;

        deadHE[twinIndex] = deadHE[next] = deadHE[prev] = true;
        if (fi != InvalidIndex)
            deadFace[fi] = true;
    }

    // ── Mark u as dead, fix v's outgoing half-edge pointer ─────────────
    deadVertex[u] = true;
    vertices[v].outHalfEdge = InvalidIndex;
    for (Index i = 0; i < halfEdges.size(); ++i) {
        if (!deadHE[i] && srcVertex(mesh, i) == v) {
            vertices[v].outHalfEdge = i;
            break;
        }
    }
}

using MinHeap = std::priority_queue<CollapseCandidate, std::vector<CollapseCandidate>, std::greater<>>;

} // namespace


/***
For each vertex v:
Q_v = sum of (ppᵀ) for each plane p of triangles around v

Priority queue ordered by collapse cost (min-heap)

While triangle count > target:
pop cheapest edge (u, v)
compute optimal new position  v* = argmin(vᵀ (Q_u + Q_v) v)
collapse u into v at v*
- redirect all half-edges from u to point to v instead
- remove the two triangles adjacent to the collapsed edge
- remove degenerate half-edges
Q_v* = Q_u + Q_v
recompute costs for all edges touching v*
***/
void simplifyMesh(HalfEdgeMesh& mesh, const SimplifyOptions& options) {
    const Index numVertices = mesh.vertices().size();
    const Index numHalfEdges = mesh.halfEdges().size();
    const Index numTriangles = mesh.triangles().size();

    std::vector<bool> deadVertex(numVertices, false);
    std::vector<bool> deadFace(numTriangles, false);
    std::vector<bool> deadHE(numHalfEdges, false);


    std::vector<Mat4> quadrics(numVertices);
    for (Index v = 0; v < numVertices; ++v)
        quadrics[v] = computeVertexQuadric(mesh, v);


    // Building Queue - One candidate per unique undirected edge (he with index < twin index)
    MinHeap collapseQueue;
    for (Index he = 0; he < numHalfEdges; ++he) {
        if (const auto twin = mesh.halfEdges()[he].twinHalfEdge;
            !twin.has_value() || he < *twin) // each undirected edge once
            collapseQueue.push(evaluateCollapse(mesh, he, quadrics));
    }

    const Index targetTriangles = options.targetTriangles > 0
        ? options.targetTriangles
        : static_cast<Index>(options.reduceFactor * static_cast<double>(numTriangles));

    Index activeTriangles = numTriangles;
    while (!collapseQueue.empty() && activeTriangles > targetTriangles) {
        const CollapseCandidate candidate = collapseQueue.top();
        collapseQueue.pop();

        const Index halfEdge = candidate.halfEdge;

        // Skip if this half-edge was already removed
        if (deadHE[halfEdge])
            continue;

        if (candidate.cost > options.maxError)
            break;

        if (!linkCondition(mesh, halfEdge))
            continue;

        const Index trianglesRemoved = mesh.halfEdges()[halfEdge].twinHalfEdge.has_value()
            ? NumEdgesOrVerticesInATriangle - 1
            : NumEdgesOrVerticesInATriangle - 2;
        collapseEdge(mesh, halfEdge, candidate.optimalPos, quadrics, deadVertex, deadFace, deadHE);
        activeTriangles -= trianglesRemoved;

        // Re-enqueue edges around the surviving vertex v
        const Index survivingVertex = mesh.halfEdges()[halfEdge].targetVertex;
        for (const Index outHalfEdge : outgoingHalfEdges(mesh, survivingVertex)) {
            if (!deadHE[outHalfEdge])
                collapseQueue.push(evaluateCollapse(mesh, outHalfEdge, quadrics));
        }
    }
}

} // namespace Mesh