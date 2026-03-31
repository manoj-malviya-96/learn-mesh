#pragma once

#include <cstddef>
#include "half_edge.h"

namespace Mesh {

struct SimplifyOptions {
    double reduceFactor = 0.5; // Target fraction of original triangles (e.g., 0.5 for 50%)
    double maxError = 1e-2;    // Also stop if the collapse cost exceeds this
};

/// Simplify a HalfEdgeMesh in-place using Quadric Error Metrics.
/// Performs edge collapses until targetTriangles are reached or maxError exceeded.
/// The link condition is checked before each collapse to preserve manifoldness.
void simplifyMesh(HalfEdgeMesh& mesh, const SimplifyOptions& options);

} // namespace Mesh