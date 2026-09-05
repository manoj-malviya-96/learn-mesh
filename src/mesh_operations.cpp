//
// Created by manoj on 3/31/26.
//

#include "mesh_operations.h"

#include <utility>

namespace Mesh {

namespace {
Vec3 toVec3(const Vertex& v) { return {v[0], v[1], v[2]}; }
} // namespace

Vec3 computeNormal(const TriMesh& mesh, Index triIndex) {
    const Triangle& t = mesh.getTriangle(triIndex);
    const Vec3 v0 = toVec3(mesh.getVertex(t[0]));
    const Vec3 v1 = toVec3(mesh.getVertex(t[1]));
    const Vec3 v2 = toVec3(mesh.getVertex(t[2]));

    const Vec3 edge1 = v1 - v0;
    const Vec3 edge2 = v2 - v0;
    return edge1.cross(edge2).normalised();
}

void flipWinding(TriMesh& mesh) {
    for (Triangle& tri : mesh.triangles())
        std::swap(tri[1], tri[2]);
}

void flipNormals(TriMesh& mesh) { flipWinding(mesh); }

}