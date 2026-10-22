#include <stdio.h>

// Benchmark 13: 3D Ray-Tracing Axis-Aligned Bounding Box (AABB) Slab Intersection
int main(int argc, char* argv[]) {
    // Ray Origin and Direction Vector
    int ray_ox = 0;
    int ray_oy = 10;
    int ray_oz = -50;

    int ray_dx = 1;
    int ray_dy = 2;
    int ray_dz = 5;

    // Bounding Box Min and Max extents
    int box_min_x = -10; int box_max_x = 10;
    int box_min_y = -10; int box_max_y = 20;
    int box_min_z = -10; int box_max_z = 30;

    // Inverse Directions (CSE for slab tests)
    int inv_dx = 100 / ray_dx;
    int inv_dy = 100 / ray_dy;
    int inv_dz = 100 / ray_dz;

    // Dead volume metric
    int dead_box_volume = (box_max_x - box_min_x) * (box_max_y - box_min_y) * (box_max_z - box_min_z);

    // X-slab intervals
    int t1_x = (box_min_x - ray_ox) * inv_dx;
    int t2_x = (box_max_x - ray_ox) * inv_dx;

    // Y-slab intervals
    int t1_y = (box_min_y - ray_oy) * inv_dy;
    int t2_y = (box_max_y - ray_oy) * inv_dy;

    // Z-slab intervals
    int t1_z = (box_min_z - ray_oz) * inv_dz;
    int t2_z = (box_max_z - ray_oz) * inv_dz;

    // Entry and Exit Times
    int t_entry = t1_x + t1_y + t1_z;
    int t_exit = t2_x + t2_y + t2_z;

    // Intersection Hit Point at t = t_entry / 3
    int t_hit = t_entry / 300;
    int hit_x = ray_ox + (ray_dx * t_hit);
    int hit_y = ray_oy + (ray_dy * t_hit);
    int hit_z = ray_oz + (ray_dz * t_hit);

    int hit_hash = hit_x * 100 + hit_y * 10 + hit_z;
    printf("Intersection Hit Hash: %d\n", hit_hash);
    return 0;
}
