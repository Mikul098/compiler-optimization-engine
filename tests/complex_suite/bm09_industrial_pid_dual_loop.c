#include <stdio.h>

// Benchmark 09: Industrial Automation Cascaded Outer-Position Inner-Velocity PID Loop
int main(int argc, char* argv[]) {
    // Outer Loop Position Gains
    int kp_pos = 10;
    int ki_pos = 2;
    int kd_pos = 5;

    // Inner Loop Velocity Gains
    int kp_vel = 4;
    int ki_vel = 1;
    int kd_vel = 2;

    // Desired Setpoint
    int pos_setpoint = 1000;
    int current_pos = 200;
    int current_vel = 30;

    // Dead Telemetry Calibration
    int dead_sensor_bias = kp_pos * kd_pos * 50;

    // Outer Loop Error Terms
    int pos_error = pos_setpoint - current_pos;
    int pos_integral = pos_error * 2;
    int pos_derivative = 0 - current_vel;

    // Outer Loop Output (Becomes velocity target)
    int vel_target = (kp_pos * pos_error) + (ki_pos * pos_integral) + (kd_pos * pos_derivative);
    vel_target = vel_target / 10;

    // Inner Loop Error Terms
    int vel_error = vel_target - current_vel;
    int vel_integral = vel_error * 2;
    int vel_derivative = -10; // Measured acceleration feedback

    // Motor Actuator Output Command
    int p_term = kp_vel * vel_error;
    int i_term = ki_vel * vel_integral;
    int d_term = kd_vel * vel_derivative;

    // Redundant power limit calculations (dead code)
    int dead_inverter_thermal_load = (p_term * p_term) / 100;

    // Final Control Command
    int motor_torque_command = p_term + i_term + d_term;

    printf("Outer Velocity Setpoint: %d\n", vel_target);
    printf("Inner Motor Command: %d\n", motor_torque_command);
    return 0;
}
