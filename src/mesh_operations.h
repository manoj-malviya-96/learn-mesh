#pragma once
#include "mat4.h"
#include "tri_mesh.h"
namespace Mesh {



Vec3 computeNormal(const TriMesh& mesh, Index triIndex);


void flipNormals(TriMesh& mesh);

void flipWinding(TriMesh& mesh);





}