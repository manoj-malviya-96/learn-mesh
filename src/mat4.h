#pragma once

#include <array>
#include <cmath>
#include <optional>

namespace Mesh {


struct Vec4 {
    double x = 0, y = 0, z = 0, w = 0;
    [[nodiscard]] double dot(const Vec4& o) const { return x * o.x + y * o.y + z * o.z + w * o.w; }
};

struct Vec3 {
    double x = 0, y = 0, z = 0;
    [[nodiscard]] double length() const { return std::sqrt(x * x + y * y + z * z); }
    [[nodiscard]] Vec3 normalised() const {
        const double len = length();
        return len > 1e-12 ? Vec3{x / len, y / len, z / len} : Vec3{};
    }
    [[nodiscard]] Vec3 operator-(const Vec3& o) const { return {x - o.x, y - o.y, z - o.z}; }
    [[nodiscard]] Vec3 cross(const Vec3& o) const {
        return {y * o.z - z * o.y, z * o.x - x * o.z, x * o.y - y * o.x};
    }
};

/**
 Mat4 — symmetric 4×4 matrix (quadric)

 Stored as the 10 upper-triangle coefficients:
   [ a00 a01 a02 a03 ]
   [     a11 a12 a13 ]
   [         a22 a23 ]
   [             a33 ]
**/

struct Mat4 {
    // Stored row-major, full 4x4 (symmetric, but we keep both sides for simplicity)
    std::array<double, 16> m{};

    double& at(const int r, const int c) { return m[r * 4 + c]; }
    [[nodiscard]] double at(const int r, const int c) const { return m[r * 4 + c]; }

    Mat4 operator+(const Mat4& o) const {
        Mat4 result;
        for (int i = 0; i < 16; ++i)
            result.m[i] = m[i] + o.m[i];
        return result;
    }

    Mat4& operator+=(const Mat4& o) {
        for (int i = 0; i < 16; ++i)
            m[i] += o.m[i];
        return *this;
    }

    // Evaluate quadric error: vᵀ M v
    [[nodiscard]] double evaluate(const Vec4& v) const {
        double sum = 0;
        for (int r = 0; r < 4; ++r)
            for (int c = 0; c < 4; ++c)
                sum += at(r, c) * (&v.x)[r] * (&v.x)[c];
        return sum;
    }

    // Build a quadric from a plane (a, b, c, d) where ax+by+cz+d=0
    // Q = ppᵀ  where p = [a, b, c, d]
    static Mat4 fromPlane(double a, double b, double c, double d) {
        double p[4] = {a, b, c, d};
        Mat4 q;
        for (int r = 0; r < 4; ++r)
            for (int cc = 0; cc < 4; ++cc)
                q.at(r, cc) = p[r] * p[cc];
        return q;
    }

    // Solve the 4x4 system to find optimal collapse position.
    // Replaces the bottom row with [0 0 0 1] to enforce homogeneous constraint,
    // then solves via Gaussian elimination.
    // Returns nullopt if the system is singular (fall back to midpoint).
    std::optional<Vec4> solveOptimalPosition() const {
        // Copy matrix, replace last row with [0, 0, 0, 1]
        double A[4][4];
        for (int r = 0; r < 4; ++r)
            for (int c = 0; c < 4; ++c)
                A[r][c] = at(r, c);
        A[3][0] = 0;
        A[3][1] = 0;
        A[3][2] = 0;
        A[3][3] = 1;

        double b[4] = {0, 0, 0, 1};

        // Gaussian elimination with partial pivoting
        for (int col = 0; col < 4; ++col) {
            // Find pivot
            int pivot = col;
            for (int row = col + 1; row < 4; ++row)
                if (std::abs(A[row][col]) > std::abs(A[pivot][col]))
                    pivot = row;

            std::swap(A[col], A[pivot]);
            std::swap(b[col], b[pivot]);

            if (std::abs(A[col][col]) < 1e-12)
                return std::nullopt; // singular

            for (int row = col + 1; row < 4; ++row) {
                double factor = A[row][col] / A[col][col];
                for (int k = col; k < 4; ++k)
                    A[row][k] -= factor * A[col][k];
                b[row] -= factor * b[col];
            }
        }

        // Back substitution
        double x[4] = {};
        for (int row = 3; row >= 0; --row) {
            x[row] = b[row];
            for (int col = row + 1; col < 4; ++col)
                x[row] -= A[row][col] * x[col];
            x[row] /= A[row][row];
        }

        return Vec4{x[0], x[1], x[2], x[3]};
    }
};

} // namespace Mesh