/*
  UCSB MAT Creative Motion Control -- Generators example
*/

#define module_driver   // tells compiler we're using the Stepdance Driver Module PCB
                        // This configures pin assignments for the Teensy 4.1

#include "stepdance.hpp"  // Import the stepdance library
// -- Define Input Ports --
InputPort input_a;

// -- Define Output Ports --
OutputPort output_a;  // Axidraw left motor
OutputPort output_b;  // Axidraw right motor
OutputPort output_c;  // Z axis, a servo driver for the AxiDraw

// -- Define Motion Channels --
Channel channel_a;  //AxiDraw "A" axis --> left motor motion
Channel channel_b;  // AxiDraw "B" axis --> right motor motion
Channel channel_z;  // AxiDraw "Z" axis --> pen up/down

// -- Define Kinematics --
KinematicsCoreXY axidraw_kinematics;

// -- Define Encoders --
Encoder encoder_1;
Encoder encoder_2;

// -- Define Input Button --
Button button_d1;

// -- Position Generator for Pen Up/Down --
PositionGenerator position_gen;

// -- RPC Interface --
RPC rpc;

// -- Velocity Generator --
VelocityGenerator speed_y_gen;


// -- Wave Generator --
WaveGenerator1D y_wave_gen;

// -- Potentiometer to control speed y -- //
AnalogInput speed_potentiometer_a1;


void setup() {
  // -- Configure and start the output ports --
  output_a.begin(OUTPUT_A); // "OUTPUT_A" specifies the physical port on the PCB for the output.
  output_b.begin(OUTPUT_B);
  output_c.begin(OUTPUT_C);

  // Enable the output drivers
  enable_drivers();

  // -- Configure and start the channels --
  channel_a.begin(&output_a, SIGNAL_E);
  channel_b.begin(&output_b, SIGNAL_E);

  // These ratios are for the Axidraw V3: 2032 steps correspond to 1 inch (25.4mm)
  channel_a.set_ratio(25.4, 2032);
  channel_a.invert_output(); // We do that so that the X axis points from motor A to motor B (left to right)
  channel_b.set_ratio(25.4, 2032);
  channel_b.invert_output(); // We do that so that the Y axis points down (away from the long axis)

  // Team - O
  // -- Configure and start the encoders --
  encoder_1.begin(ENCODER_1); // "ENCODER_1" specifies the physical port on the PCB
  encoder_1.set_ratio(24, 2400);  // 24mm per revolution, where 1 rev == 2400 encoder pulses
  encoder_1.output.map(&axidraw_kinematics.input_x);

  encoder_2.begin(ENCODER_2); // "ENCODER_2" specifies the physical port on the PCB
  encoder_2.set_ratio(24, 2400);  // 24mm per revolution, where 1 rev == 2400 encoder pulses
  encoder_2.output.map(&axidraw_kinematics.input_y);

  // Team - O *****************
  // -- Configure the velocity generator --
 // Set the speed (start at zero, use Serial commands to change it, or change the value in code)
  speed_y_gen.begin(); // all plugins must call begin
  speed_y_gen.speed_units_per_sec = 0.0;
  // Control the motion in X axis with the velocity generator  
  speed_y_gen.output.map(&axidraw_kinematics.input_y);


  // -- Configure the wave generator --
  y_wave_gen.setNoInput();     // we will use the internal clock as the time variable
  y_wave_gen.frequency = 5.0; // frequency of oscillation (feel free to change)
  y_wave_gen.amplitude = 0.0;  // amplitude of the wave (start at 0, change through serial)
  y_wave_gen.output.map(&axidraw_kinematics.input_x);
  y_wave_gen.begin();

  // -- Configure and start the kinematics module --
  axidraw_kinematics.begin();
  axidraw_kinematics.output_a.map(&channel_a.input_target_position);
  axidraw_kinematics.output_b.map(&channel_b.input_target_position);

  // -- Configure Button --
  button_d1.begin(IO_D1, INPUT_PULLDOWN);
  button_d1.set_mode(BUTTON_MODE_TOGGLE);
  button_d1.set_callback_on_press(&pen_up);
  button_d1.set_callback_on_release(&pen_down);

  // -- Control interface (RPC) --
  rpc.begin(); 

  // Call example: {"name": "hello"}
  // expected result: serial monitor prints "hello!{"result":"ok"}"
  rpc.enroll("hello", hello_serial);

  // Call example: {"name": "set_y_amplitude", "args": [1]}
  rpc.enroll("set_y_amplitude", set_y_amplitude);

  // Call example: {"name": "set_y_frequency", "args": [1]}
  rpc.enroll("set_y_frequency", set_y_frequency);

  // Call example: {"name": "set_speed_y", "args": [1]}
  rpc.enroll("set_speed_y", set_speed_y);

  // -- Configure Position Generator --
  position_gen.output.map(&channel_z.input_target_position);
  position_gen.begin();


  // -- Configure the speed potentiometer -- //
  speed_potentiometer_a1.begin(IO_A1);
  speed_potentiometer_a1.set_floor(-15);
  speed_potentiometer_a1.set_ceiling(15);
  speed_potentiometer_a1.set_deadband(1, 509, 4); // deadband controls designated range where input movements do not produce output change to avoid jitter.
  speed_potentiometer_a1.map(&speed_y_gen.speed_units_per_sec); // map the value of the slider to the scaling filter ratio.

  // -- Start the stepdance library --
  dance_start();
}

LoopDelay overhead_delay;

void loop() {
  overhead_delay.periodic_call(&report_overhead, 500);

  dance_loop(); // Stepdance loop provides convenience functions, and should be called at the end of the main loop
}

void pen_down(){
  position_gen.go(-4, ABSOLUTE, 100);
}

void pen_up(){
  position_gen.go(4, ABSOLUTE, 100);
}

void hello_serial(){
  Serial.print("hello!");
}


void set_speed_y(float32_t speed_y){
    speed_y_gen.speed_units_per_sec = speed_y;
}

void set_y_amplitude(float32_t amplitude){
  y_wave_gen.amplitude = amplitude;
}

void set_y_frequency(float32_t frequency){ // 0 - 20 ?
  y_wave_gen.frequency = frequency;
}

void report_overhead(){
  Serial.println(speed_potentiometer_a1.read());
//  Serial.println(speed_potentiometer_a1.read_raw());

}