#pragma once

#include <cstddef>
#include "half_edge.h"

namespace Mesh {

struct SimplifyOptions {
    std::size_t targetTriangles = 0; // Stop when the triangle count reaches this
    double maxError = 1e-2;   // Also stop if the collapse cost exceeds this
};

/// Simplify a HalfEdgeMesh in-place using Quadric Error Metrics.
/// Performs edge collapses until targetTriangles are reached or maxError exceeded.
/// The link condition is checked before each collapse to preserve manifoldness.
void simplifyMesh(HalfEdgeMesh& mesh, const SimplifyOptions& options);

} // namespace Mesh