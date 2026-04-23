/*
  UCSB MAT Creative Motion Control -- Project 1, Version 2

  Two encoders drive curvy (sinusoidal) motion on the AxiDraw.
  Instead of mapping each encoder linearly to an axis, each encoder's
  output is fed into a WaveGenerator1D. The generator applies a sine
  function to the encoder value, so turning a knob produces a curved
  trajectory rather than a straight horizontal or vertical line.

    Encoder 1 → WaveGenerator1D → kinematics input_x  (curvy X motion)
    Encoder 2 → WaveGenerator1D → kinematics input_y  (curvy Y motion)

  This mirrors the behaviour seen in the sine-wave simulation:
  the pen path is sinusoidal rather than rectilinear.

  RPC commands (send via Serial as JSON):
    {"name": "set_freq_x", "args": [0.05]}   — spatial frequency of the X wave
    {"name": "set_freq_y", "args": [0.05]}   — spatial frequency of the Y wave
    {"name": "set_amp_x",  "args": [20]}     — amplitude of the X wave (mm)
    {"name": "set_amp_y",  "args": [20]}     — amplitude of the Y wave (mm)
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

// -- Encoders --
Encoder encoder_1;  // controls X-axis wave
Encoder encoder_2;  // controls Y-axis wave

// -- Wave Generators --
// Each encoder drives the *input* of a WaveGenerator1D.
// WaveGenerator1D computes: output = amplitude * sin(2π * frequency * input + phase)
// So the encoder value becomes the argument of a sine — the path is curvy, not linear.
WaveGenerator1D wave_x;  // encoder 1 → sine-shaped X output
WaveGenerator1D wave_y;  // encoder 2 → sine-shaped Y output

// -- Pen control --
Button button_d1;
PositionGenerator position_gen;

// -- RPC interface --
RPC rpc;


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

  // -- Encoders --
  encoder_1.begin(ENCODER_1);
  encoder_1.set_ratio(24, 2400);  // 24 mm per revolution

  encoder_2.begin(ENCODER_2);
  encoder_2.set_ratio(24, 2400);

  // -- Wave Generator X --
  // Encoder 1 feeds into wave_x.input. As encoder 1 value changes,
  // the generator computes a sine and sends it to the X axis.
  // Result: turning knob 1 draws a sinusoidal curve in X.
  wave_x.frequency = 0.05;   // cycles per mm of encoder travel (lower = wider curves)
  wave_x.amplitude = 20.0;   // peak deviation in mm
  wave_x.phase     = 0.0;
  encoder_1.output.map(&wave_x.input);
  wave_x.output.map(&axidraw_kinematics.input_x);
  wave_x.begin();

  // -- Wave Generator Y --
  // Encoder 2 feeds into wave_y.input → curvy Y motion.
  wave_y.frequency = 0.05;
  wave_y.amplitude = 20.0;
  wave_y.phase     = 0.0;
  encoder_2.output.map(&wave_y.input);
  wave_y.output.map(&axidraw_kinematics.input_y);
  wave_y.begin();

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
  // {"name": "set_freq_x", "args": [0.05]}
  rpc.enroll("set_freq_x", set_freq_x);
  // {"name": "set_freq_y", "args": [0.05]}
  rpc.enroll("set_freq_y", set_freq_y);
  // {"name": "set_amp_x", "args": [20]}
  rpc.enroll("set_amp_x",  set_amp_x);
  // {"name": "set_amp_y", "args": [20]}
  rpc.enroll("set_amp_y",  set_amp_y);

  dance_start();
}

LoopDelay overhead_delay;

void loop() {
  overhead_delay.periodic_call(&report_overhead, 500);
  dance_loop();
}

// -- Pen callbacks --
void pen_down() { position_gen.go(-4, ABSOLUTE, 100); }
void pen_up()   { position_gen.go( 4, ABSOLUTE, 100); }

// -- RPC callbacks --
void set_freq_x(float32_t freq) { wave_x.frequency = freq; }
void set_freq_y(float32_t freq) { wave_y.frequency = freq; }
void set_amp_x(float32_t amp)   { wave_x.amplitude = amp;  }
void set_amp_y(float32_t amp)   { wave_y.amplitude = amp;  }

void report_overhead() {}
