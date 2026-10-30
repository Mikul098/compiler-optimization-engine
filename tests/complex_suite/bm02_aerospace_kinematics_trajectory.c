#include <stdio.h>

// Benchmark 02: Aerospace Multi-Stage Orbital Rocket Ascent & Trajectory Kinematics
int main(int argc, char* argv[]) {
    // Physical Constants and Launch Parameters
    int g0 = 9;                 // Gravity acceleration (approx 9.8 m/s^2)
    int dry_mass = 5000;        // kg
    int fuel_stage1 = 20000;    // kg
    int fuel_stage2 = 10000;    // kg
    int thrust_stage1 = 600000; // Newtons
    int isp_stage1 = 300;       // Specific Impulse (s)

    // Time steps
    int dt = 10;                // seconds
    int burn_time1 = 40;        // seconds
    int burn_time2 = 30;        // seconds

    // Atmospheric Drag Model Parameters
    int rho0 = 1;               // Air density at sea level
    int drag_coeff = 2;
    int cross_area = 5;

    // Initial State at t=0
    int altitude = 0;
    int velocity = 0;
    int mass = dry_mass + fuel_stage1 + fuel_stage2;

    // Stage 1 - Step 1
    int net_force_t1 = thrust_stage1 - (mass * g0);
    int accel_t1 = net_force_t1 / mass;
    velocity += accel_t1 * dt;
    altitude += velocity * dt;
    mass -= 5000;

    // Dynamic Pressure Candidate for CSE: q = 1/2 * rho * v^2
    int v_sq_1 = velocity * velocity;
    int dynamic_pressure_1 = (rho0 * v_sq_1 * drag_coeff) / 2;
    int drag_force_1 = dynamic_pressure_1 * cross_area;

    // Dead orbital parameters computed too early
    int dead_apogee_pred = altitude + (velocity * velocity) / (2 * g0);
    int dead_orbital_energy = -1 * mass * g0;

    // Stage 1 - Step 2
    int net_force_t2 = thrust_stage1 - (mass * g0) - drag_force_1;
    int accel_t2 = net_force_t2 / mass;
    velocity += accel_t2 * dt;
    altitude += velocity * dt;
    mass -= 5000;

    // Dynamic Pressure Candidate 2
    int v_sq_2 = velocity * velocity;
    int dynamic_pressure_2 = (rho0 * v_sq_2 * drag_coeff) / 2;

    // Stage 1 - Step 3 Burnout & Staging
    int net_force_t3 = thrust_stage1 - (mass * g0);
    int accel_t3 = net_force_t3 / mass;
    velocity += accel_t3 * dt;
    altitude += velocity * dt;

    // Staging: Eject Stage 1 empty tank
    int stage1_tank_mass = 4000;
    mass -= stage1_tank_mass;

    // Final Stage 2 Burn Ignition
    int thrust_stage2 = 200000;
    int net_force_s2 = thrust_stage2 - (mass * g0);
    int accel_s2 = net_force_s2 / mass;
    velocity += accel_s2 * dt;
    altitude += velocity * dt;

    // True Apogee Calculation
    int final_apogee = altitude + (velocity * velocity) / (2 * g0);
    int kinetic_energy = (mass * velocity * velocity) / 2;
    int potential_energy = mass * g0 * altitude;
    int total_mechanical_energy = (kinetic_energy + potential_energy) / 10000;

    printf("Telemetry Apogee: %d\n", final_apogee);
    printf("Total Energy Metric: %d\n", total_mechanical_energy);
    return 0;
}
