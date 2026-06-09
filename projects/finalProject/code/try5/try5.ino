/*
try5.ino
Builds on try4: adds set_tip_type() RPC so the HTML UI can switch
channel_c ratio and extrusion multiplier at runtime.
*/

#define module_driver   // tells compiler we're using the Stepdance Driver Module PCB
                        // This configures pin assignments for the Teensy 4.1
#include "stepdance.hpp"  // Import the stepdance library

// -- Define Output Ports --
OutputPort output_a;  // x
OutputPort output_b;  // y
OutputPort output_c;  // z
OutputPort output_d;  // e

// -- Define Motion Channels --
Channel channel_a;  // Ender "x"
Channel channel_b;  // Ender "y"
Channel channel_c;  // Ender "z"
Channel channel_d;  // Extrusion stepper "e"

// -- Define Encoders --
Encoder encoder_1; // x axis
Encoder encoder_2; // y axis

// -- RPC Interface --
RPC rpc;

// -- Potentiometers --
AnalogInput analog_a1; // extrusion rate knob
AnalogInput analog_a2;

// -- Path length extrusion generator --
PathLengthGenerator2D e_gen;

// -- Spiral / polar kinematics --
KinematicsPolarToCartesian polar_kinematics;
VelocityGenerator angle_speed;
PositionGenerator  radius_change;

// -- Z jog and E jog --
PositionGenerator z_gen_jog;
PositionGenerator e_gen_jog;

// -- Wave generators --
WaveGenerator1D x_wave_gen;
WaveGenerator1D y_wave_gen;
WaveGenerator1D z_wave_gen;
WaveGenerator1D e_wave_gen;

// -- Velocity generators --
VelocityGenerator speed_x_gen;
VelocityGenerator speed_y_gen;
VelocityGenerator speed_z_gen;
VelocityGenerator speed_e_gen;

ScalingFilter1D z_lift;

// -- Tip type extrusion multiplier (set by set_tip_type RPC) --
// Type 1: Regular tip  → 0.48
// Type 2: Big star     → 0.98
// Type 3: Small star   → 0.78
float64_t extrusionMultiplier = 0.98;

void setup() {
  output_a.begin(OUTPUT_A);
  output_b.begin(OUTPUT_B);
  output_c.begin(OUTPUT_C);
  output_d.begin(OUTPUT_D);

  enable_drivers();

  channel_a.begin(&output_a, SIGNAL_E);
  channel_b.begin(&output_b, SIGNAL_E);
  channel_c.begin(&output_c, SIGNAL_E);
  channel_d.begin(&output_d, SIGNAL_E);

  channel_a.set_ratio(1, 80);
  channel_a.invert_output();
  channel_b.set_ratio(1, 80);
  channel_b.invert_output();
  channel_d.set_ratio(8, 200);
  channel_d.invert_output();
  channel_c.set_ratio(1, 200);    // Default: Type 1 Regular tip (overridden by set_tip_type)
  channel_c.invert_output();

  // -- Encoders --
  encoder_1.begin(ENCODER_1);
  encoder_1.set_ratio(24, 2400);
  encoder_1.output.map(&channel_a.input_target_position);

  encoder_2.begin(ENCODER_2);
  encoder_2.set_ratio(24, 2400);
  encoder_2.output.map(&channel_b.input_target_position);

  // -- Polar kinematics (spiral) --
  polar_kinematics.output_x.map(&channel_a.input_target_position);
  polar_kinematics.output_y.map(&channel_b.input_target_position);
  polar_kinematics.begin();

  angle_speed.output.map(&polar_kinematics.input_angle);
  angle_speed.begin();

  radius_change.output.map(&polar_kinematics.input_radius);
  radius_change.begin();

  // z_lift
  z_lift.begin();
  z_lift.set_ratio(2, TWO_PI);
  z_lift.input.map(&polar_kinematics.input_angle);
  z_lift.output.map(&channel_c.input_target_position);

  // -- Z jog --
  z_gen_jog.begin();
  z_gen_jog.output.map(&channel_c.input_target_position);

  // -- E jog --
  e_gen_jog.begin();
  e_gen_jog.output.map(&channel_d.input_target_position);

  // -- Path length extrusion --
  e_gen.begin();
  e_gen.input_1.map(&channel_a.input_target_position);
  e_gen.input_2.map(&channel_b.input_target_position);
  e_gen.output.map(&channel_d.input_target_position);

  // -- Wave generators (x, y, z, e) --
  x_wave_gen.setNoInput();
  x_wave_gen.frequency = 0.0;
  x_wave_gen.amplitude = 0.0;
  x_wave_gen.output.map(&channel_a.input_target_position);
  x_wave_gen.begin();

  y_wave_gen.setNoInput();
  y_wave_gen.frequency = 0.0;
  y_wave_gen.amplitude = 0.0;
  y_wave_gen.output.map(&channel_b.input_target_position);
  y_wave_gen.begin();

  z_wave_gen.setNoInput();
  z_wave_gen.frequency = 0.0;
  z_wave_gen.amplitude = 0.0;
  z_wave_gen.output.map(&channel_c.input_target_position);
  z_wave_gen.begin();

  e_wave_gen.setNoInput();
  e_wave_gen.frequency = 0.0;
  e_wave_gen.amplitude = 0.0;
  e_wave_gen.output.map(&channel_d.input_target_position);
  e_wave_gen.begin();

  // -- Velocity generators (x, y, z, e) --
  speed_x_gen.begin();
  speed_x_gen.speed_units_per_sec = 0.0;
  speed_x_gen.output.map(&channel_a.input_target_position);

  speed_y_gen.begin();
  speed_y_gen.speed_units_per_sec = 0.0;
  speed_y_gen.output.map(&channel_b.input_target_position);

  speed_z_gen.begin();
  speed_z_gen.speed_units_per_sec = 0.0;
  speed_z_gen.output.map(&channel_c.input_target_position);

  speed_e_gen.begin();
  speed_e_gen.speed_units_per_sec = 0.0;
  speed_e_gen.output.map(&channel_d.input_target_position);

  // -- Analog inputs --
  analog_a1.set_floor(1, 25);
  analog_a1.set_ceiling(10, 1020);
  analog_a1.begin(IO_A1);

  analog_a2.set_floor(2, 25);
  analog_a2.set_ceiling(15, 1020);
  analog_a2.begin(IO_A2);

  // -- RPC --
  rpc.begin();

  rpc.enroll("hello",           hello_serial);

  // tip type (must be called before start_spiral)
  rpc.enroll("set_tip_type",    set_tip_type);

  // spiral control
  rpc.enroll("start_spiral",    start_spiral);
  rpc.enroll("stop_spiral",     stop_spiral);
  rpc.enroll("step_z_jog",      step_z_jog);
  rpc.enroll("step_e_jog",      step_e_jog);
  rpc.enroll("disable_e",       disable_e);
  rpc.enroll("enable_e",        enable_e);
  rpc.enroll("set_spiral_speed",set_spiral_speed);

  // x axis
  rpc.enroll("set_speed_x", set_speed_x);

  // y axis
  rpc.enroll("set_speed_y", set_speed_y);

  // z axis
  rpc.enroll("set_speed_z", set_speed_z);

  // e axis
  rpc.enroll("set_speed_e", set_speed_e);

  dance_start();
}

LoopDelay overhead_delay;

void loop() {
  e_gen.set_ratio(-extrusionMultiplier);
  overhead_delay.periodic_call(&report_overhead, 500);
  dance_loop();
}

// -- Hello --
void hello_serial() {
  Serial.print("hello!");
}

// -- Tip type --
// Call this from the UI before starting a print.
// type 1 = Regular, type 2 = Big Star, type 3 = Small Star
void set_tip_type(float32_t type) {
  int t = (int)type;
  if (t == 1) {
    channel_c.set_ratio(1, 200);
    extrusionMultiplier = 0.48;
  } else if (t == 2) {
    channel_c.set_ratio(1, 600);
    extrusionMultiplier = 0.98;
  } else if (t == 3) {
    channel_c.set_ratio(1, 400);
    extrusionMultiplier = 0.78;
  }
}

// -- Spiral --
void start_spiral(float32_t speed, float32_t radius) {
  channel_d.enable();
  angle_speed.speed_units_per_sec = speed;
  polar_kinematics.input_angle.reset(0);
  polar_kinematics.input_radius.reset(radius);
  radius_change.go(-radius, INCREMENTAL, 1);
}

void stop_spiral() {
  polar_kinematics.input_radius.reset(0);
  angle_speed.speed_units_per_sec = 0;
  // z sequence is handled from HTML with timed delays
}

void step_z_jog(float32_t height) {
  z_gen_jog.go(height, INCREMENTAL, 10);
}

void step_e_jog() {
  e_gen_jog.go(1000, INCREMENTAL, 50);
}

void disable_e() {
  channel_d.disable();
}

void enable_e() {
  channel_d.enable();
}

// -- Spiral live controls --
void set_spiral_speed(float32_t speed) {
  angle_speed.speed_units_per_sec = speed;
}

// -- X axis --
void set_speed_x(float32_t speed) { speed_x_gen.speed_units_per_sec = speed; }

// -- Y axis --
void set_speed_y(float32_t speed) { speed_y_gen.speed_units_per_sec = speed; }

// -- Z axis --
void set_speed_z(float32_t speed) { speed_z_gen.speed_units_per_sec = speed; }

// -- E axis --
void set_speed_e(float32_t speed) { speed_e_gen.speed_units_per_sec = speed; }

// -- Reporting --
void report_overhead() {
  Serial.print("A1:"); Serial.println(analog_a1.read());
  Serial.print("Z:");  Serial.println(channel_c.input_target_position.read_absolute());
}
