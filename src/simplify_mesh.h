#pragma once

#include <cstddef>
#include "half_edge.h"

namespace Mesh {

struct SimplifyOptions {
    double reduceFactor = 0.5;    // Used when targetTriangles == 0: target fraction of original triangles
    Index targetTriangles = 0;    // Absolute triangle count to stop at; overrides reduceFactor when nonzero
    double maxError = 1e-2;       // Also stop if the collapse cost exceeds this
};

/// Simplify a HalfEdgeMesh in-place using Quadric Error Metrics.
/// Performs edge collapses until the target triangle count is reached or maxError exceeded.
/// The link condition is checked before each collapse to preserve manifoldness.
void simplifyMesh(HalfEdgeMesh& mesh, const SimplifyOptions& options);

} // namespace Mesh