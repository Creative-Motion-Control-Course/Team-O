/*
  UCSB MAT Creative Motion Control -- Project 1, Version 3

  Uses TimeBasedInterpolator (TBI) to generate a continuous sequence of
  horizontal segments along X, with a WaveGenerator1D adding a sinusoidal
  Y deviation to each segment. This produces rows of curvy lines in the
  X direction of the AxiDraw — similar to try0_v1 but with TBI handling
  motion planning instead of a VelocityGenerator.

  Signal chain:
    TBI (queues X segments) ──► kinematics input_x
    TBI output_parameter (0→1 per segment) ──► WaveGenerator1D ──► kinematics input_y

  As the TBI traverses each segment, output_parameter sweeps 0 → 1.
  The WaveGenerator turns that linear sweep into one sine cycle,
  producing a smooth wave in Y that is spatially locked to each X segment.

  RPC commands (send via Serial as JSON):
    {"name": "set_speed",       "args": [10]}  — travel speed in mm/s
    {"name": "set_x_amplitude", "args": [5]}   — sine wave amplitude in Y (mm)
*/

#define module_driver   // tells compiler we're using the Stepdance Driver Module PCB

#include "stepdance.hpp"

// -- Output Ports --
OutputPort output_a;  // AxiDraw left motor
OutputPort output_b;  // AxiDraw right motor
OutputPort output_c;  // Z axis (servo, pen up/down)

// -- Motion Channels --
Channel channel_a;
Channel channel_b;
Channel channel_z;

// -- Kinematics (CoreXY) --
KinematicsCoreXY axidraw_kinematics;

// -- TimeBasedInterpolator --
// Queues pre-planned horizontal moves along X at a given speed.
// output_x        → kinematics X (position along the line)
// output_parameter → feeds the wave shape (0 at start of segment, 1 at end)
TimeBasedInterpolator tbi;

// -- Wave Generator --
// Takes tbi.output_parameter as its input (not setNoInput).
// frequency = 1.0 → one full sine cycle per segment (parameter 0→1).
// The output goes to kinematics Y: the pen traces a sine wave as it sweeps X.
WaveGenerator1D x_wave_gen;

// -- Pen control --
Button button_d1;
PositionGenerator position_gen;

// -- RPC interface --
RPC rpc;

// -- Motion parameters (adjustable via RPC) --
float32_t travel_speed_mm_s = 10.0;  // horizontal speed (mm/s); 0 = pause
float32_t segment_length_mm = 50.0;  // length of each TBI segment in X (mm)


void setup() {
  // -- Output ports --
  output_a.begin(OUTPUT_A);
  output_b.begin(OUTPUT_B);
  output_c.begin(OUTPUT_C);
  enable_drivers();

  // -- Channels --
  channel_a.begin(&output_a, SIGNAL_E);
  channel_b.begin(&output_b, SIGNAL_E);
  // AxiDraw V3: 2032 steps = 1 inch (25.4 mm)
  channel_a.set_ratio(25.4, 2032);
  channel_a.invert_output();
  channel_b.set_ratio(25.4, 2032);
  channel_b.invert_output();

  channel_z.begin(&output_c, SIGNAL_E);
  channel_z.set_ratio(1, 50);

  // -- TimeBasedInterpolator --
  // Only X is used; Y, Z, E, R, T are left at 0 (INCREMENTAL mode, so no movement).
  tbi.begin();
  tbi.output_x.map(&axidraw_kinematics.input_x);

  // -- Wave Generator --
  // tbi.output_parameter (range 0→1) drives the wave input.
  // frequency = 1.0 → the sine completes exactly one full cycle per segment.
  // Increase frequency for more waves per segment.
  x_wave_gen.frequency = 1.0;   // waves per segment
  x_wave_gen.amplitude = 0.0;   // start at 0; change via set_x_amplitude
  x_wave_gen.phase     = 0.0;
  tbi.output_parameter.map(&x_wave_gen.input);
  x_wave_gen.output.map(&axidraw_kinematics.input_y);
  x_wave_gen.begin();

  // -- Kinematics --
  axidraw_kinematics.begin();
  axidraw_kinematics.output_a.map(&channel_a.input_target_position);
  axidraw_kinematics.output_b.map(&channel_b.input_target_position);

  // -- Pen button --
  button_d1.begin(IO_D1, INPUT_PULLDOWN);
  button_d1.set_mode(BUTTON_MODE_TOGGLE);
  button_d1.set_callback_on_press(&pen_up);
  button_d1.set_callback_on_release(&pen_down);

  position_gen.output.map(&channel_z.input_target_position);
  position_gen.begin();

  // -- RPC --
  rpc.begin();
  // {"name": "set_speed", "args": [10]}
  rpc.enroll("set_speed",       set_speed);
  // {"name": "set_x_amplitude", "args": [5]}
  rpc.enroll("set_x_amplitude", set_x_amplitude);

  dance_start();
}

LoopDelay overhead_delay;

void loop() {
  overhead_delay.periodic_call(&report_overhead, 500);

  // Keep the TBI queue fed with horizontal segments.
  // Each call adds one segment of segment_length_mm at travel_speed_mm_s.
  // Segments are queued ahead of time (up to TBI_BLOCK_QUEUE_SIZE = 100).
  // When speed is 0 the queue drains naturally and stops.
  if (!tbi.queue_is_full() && travel_speed_mm_s > 0) {
    tbi.add_move(INCREMENTAL, travel_speed_mm_s,
                 segment_length_mm, 0, 0, 0, 0, 0);
  }

  dance_loop();
}

// -- Pen callbacks --
void pen_down() { position_gen.go(-4, ABSOLUTE, 100); }
void pen_up()   { position_gen.go( 4, ABSOLUTE, 100); }

// -- RPC callbacks --
void set_speed(float32_t speed) {
  travel_speed_mm_s = speed;
}

void set_x_amplitude(float32_t amplitude) {
  x_wave_gen.amplitude = amplitude;
}

void report_overhead() {}
