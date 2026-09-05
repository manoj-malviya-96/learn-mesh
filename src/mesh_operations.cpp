//
// Created by manoj on 3/31/26.
//

#include "mesh_operations.h"


namespace Mesh {

Vec3 computeNormal(const TriMesh& mesh, Index triIndex) {
    const Triangle& t = mesh.getTriangle(triIndex);
    const Vec3& v0 = mesh.getVertex(t[0]);
    const Vec3& v1 = mesh.getVertex(t[1]);
    const Vec3& v2 = mesh.getVertex(t[2]);

    const Vec3 edge1 = v1 - v0;
    const Vec3 edge2 = v2 - v0;
    return edge1.cross(edge2).normalised();
}




}