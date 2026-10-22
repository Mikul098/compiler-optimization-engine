#include <stdio.h>

// Benchmark 01: 3D Graphics 4x4 Affine Matrix Transform & Vertex Projection Pipeline
int main(int argc, char* argv[]) {
    // Model-View Transformation Matrix Elements
    int m00 = 2;
    int m01 = 0;
    int m02 = 1;
    int m03 = 10;

    int m10 = 0;
    int m11 = 3;
    int m12 = 0;
    int m13 = 20;

    int m20 = 1;
    int m21 = 0;
    int m22 = 2;
    int m23 = -5;

    // Perspective Projection Scaling Factors
    int fov_scale = 4;
    int aspect_ratio = 2;
    int near_plane = 10;
    int far_plane = 100;

    // Input 3D Vertices (Homogeneous coordinates x, y, z, w=1)
    int v1_x = 5;
    int v1_y = 8;
    int v1_z = 12;

    int v2_x = 10;
    int v2_y = 15;
    int v2_z = 24;

    // Vertex 1 Affine Transformation
    int v1_world_x = m00 * v1_x + m01 * v1_y + m02 * v1_z + m03;
    int v1_world_y = m10 * v1_x + m11 * v1_y + m12 * v1_z + m13;
    int v1_world_z = m20 * v1_x + m21 * v1_y + m22 * v1_z + m23;

    // Common Subexpression Candidate across vertices
    int depth_delta = far_plane - near_plane;
    int depth_sum = far_plane + near_plane;
    int proj_factor = fov_scale * aspect_ratio;

    // Redundant depth metric (dead code)
    int dead_depth_metric = depth_delta * depth_sum * 42;

    // Projected Coordinate calculations for Vertex 1
    int v1_proj_x = v1_world_x * proj_factor;
    int v1_proj_y = v1_world_y * proj_factor;
    int v1_proj_z = (v1_world_z * depth_sum) / depth_delta;

    // Dead intermediate calculations
    int unused_normal_x = v1_proj_x * 3;
    int unused_normal_y = v1_proj_y * 3;
    int unused_normal_len = unused_normal_x + unused_normal_y;

    // Vertex 2 Affine Transformation
    int v2_world_x = m00 * v2_x + m01 * v2_y + m02 * v2_z + m03;
    int v2_world_y = m10 * v2_x + m11 * v2_y + m12 * v2_z + m13;
    int v2_world_z = m20 * v2_x + m21 * v2_y + m22 * v2_z + m23;

    // Projected Coordinate calculations for Vertex 2 using cached subexpressions
    int v2_proj_x = v2_world_x * proj_factor;
    int v2_proj_y = v2_world_y * proj_factor;
    int v2_proj_z = (v2_world_z * depth_sum) / depth_delta;

    // Screen Space Center of Mass
    int center_x = (v1_proj_x + v2_proj_x) / 2;
    int center_y = (v1_proj_y + v2_proj_y) / 2;
    int center_z = (v1_proj_z + v2_proj_z) / 2;

    // Compound assignment update
    center_x += 100;
    center_y += 100;

    // Final Screen Coordinate Hash
    int screen_hash = center_x * 1000 + center_y * 10 + center_z;
    printf("3D Viewport Screen Hash: %d\n", screen_hash);
    return 0;
}
