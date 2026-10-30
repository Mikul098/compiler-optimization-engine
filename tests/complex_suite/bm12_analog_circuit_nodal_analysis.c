#include <stdio.h>

// Benchmark 12: Analog Electronics 5-Node Resistor Network Kirchhoff Nodal Analysis
int main(int argc, char* argv[]) {
    // Resistor Conductances G = 1/R (millimhos)
    int G1 = 10;
    int G2 = 20;
    int G3 = 5;
    int G4 = 15;
    int G5 = 8;

    // Independent Current Sources (mA)
    int I_source1 = 100;
    int I_source2 = 50;

    // Common Conductance Combinations (Admittance Matrix Diagonals)
    int G_node1 = G1 + G2 + G3;
    int G_node2 = G2 + G4 + G5;

    // Dead Circuit Thermal Dissipation Metric
    int dead_joule_heating = (G_node1 * G_node2 * I_source1) / 1000;

    // Nodal Matrix Determinant: det = G11*G22 - G12*G21
    int G12 = -1 * G2;
    int G21 = -1 * G2;
    int det = (G_node1 * G_node2) - (G12 * G21);

    // Solve Node Voltages V1, V2 using Cramer's Rule
    int det1 = (I_source1 * G_node2) - (I_source2 * G12);
    int V1 = (det1 * 100) / det;

    int det2 = (G_node1 * I_source2) - (G21 * I_source1);
    int V2 = (det2 * 100) / det;

    // Branch Currents
    int I_branch_G2 = (V1 - V2) * G2;
    int I_branch_G3 = V1 * G3;

    // Power Delivered
    int total_power = (V1 * I_source1) + (V2 * I_source2);

    printf("Node Voltage V1: %d\n", V1);
    printf("Node Voltage V2: %d\n", V2);
    printf("Total Circuit Power: %d\n", total_power);
    return 0;
}
