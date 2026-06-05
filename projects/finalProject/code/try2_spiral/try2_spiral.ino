/*
try1.ino
x axis ratio (1,80) 250
y axis ratio (1,80) 250
*/

#define module_driver   // tells compiler we're using the Stepdance Driver Module PCB
                        // This configures pin assignments for the Teensy 4.1
#include "stepdance.hpp"  // Import the stepdance library

// -- Define Input Ports --
InputPort input_a;

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

CircleGenerator circle_gen;//controls individual rosette
ScalingFilter2D circle_scale;

// -- RPC Interface --
RPC rpc;


// -- Potentiometer to control speed -- //
/*
AnalogInput speed_potentiometer_a1;
AnalogInput speed_potentiometer_a2;
AnalogInput speed_potentiometer_a3;
*/

// -- Potentiometer to control extrusion -- //

//extrusion rate .52, via potentimetere 
AnalogInput analog_a1;
AnalogInput analog_a2;

// -- Velocity Generator --
VelocityGenerator speed_x_gen;
VelocityGenerator speed_y_gen;
VelocityGenerator speed_z_gen;
VelocityGenerator speed_e_gen;

// -- Wave Generator --
WaveGenerator1D x_wave_gen;
WaveGenerator1D y_wave_gen;
WaveGenerator1D z_wave_gen;
WaveGenerator1D e_wave_gen;

PathLengthGenerator2D e_gen; //generates extruder signal

KinematicsPolarToCartesian polar_kinematics;

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
  channel_c.set_ratio(1, 400);
  channel_c.invert_output(); 
  channel_d.set_ratio(8, 200);
  channel_d.invert_output(); 


  // Team - O
  // -- Configure and start the encoders --
  encoder_1.begin(ENCODER_1); // "ENCODER_1" specifies the physical port on the PCB
  encoder_1.set_ratio(24, 2400);  // 24mm per revolution, where 1 rev == 2400 encoder pulses
  encoder_1.output.map(&channel_a.input_target_position);

  encoder_2.begin(ENCODER_2); // "ENCODER_1" specifies the physical port on the PCB
  encoder_2.set_ratio(24, 2400);  // 24mm per revolution, where 1 rev == 2400 encoder pulses
  encoder_2.output.map(&channel_b.input_target_position);


  circle_gen.rotational_speed_rev_per_sec = 1.0;
  circle_gen.setNoInput(); // Use internal frame count
  circle_gen.output_x.map(&channel_a.input_target_position);
  circle_gen.output_y.map(&channel_b.input_target_position);
  
  //analog_a2.map(&circle_gen.radius);
  circle_gen.radius = 10;
  circle_gen.disable();
  circle_gen.begin();

  // x axis
 /* speed_x_gen.begin(); // all plugins must call begin
  speed_x_gen.speed_units_per_sec = 0.0;
  speed_x_gen.output.map(&channel_a.input_target_position);

  x_wave_gen.setNoInput();     // we will use the internal clock as the time variable
  x_wave_gen.frequency = 0.0; // frequency of oscillation (feel free to change)
  x_wave_gen.amplitude = 0.0;  // amplitude of the wave (start at 0, change through serial)
  x_wave_gen.output.map(&channel_a.input_target_position);
  x_wave_gen.begin();

    // y axis
  speed_y_gen.begin(); // all plugins must call begin
  speed_y_gen.speed_units_per_sec = 0.0;
  speed_y_gen.output.map(&channel_b.input_target_position);

  y_wave_gen.setNoInput();     // we will use the internal clock as the time variable
  y_wave_gen.frequency = 0.0; // frequency of oscillation (feel free to change)
  y_wave_gen.amplitude = 0.0;  // amplitude of the wave (start at 0, change through serial)
  y_wave_gen.output.map(&channel_b.input_target_position);
  y_wave_gen.begin();

  // z axis
  speed_z_gen.begin(); // all plugins must call begin
  speed_z_gen.speed_units_per_sec = 0.0;
  speed_z_gen.output.map(&channel_c.input_target_position);

  z_wave_gen.setNoInput();     // we will use the internal clock as the time variable
  z_wave_gen.frequency = 0.0; // frequency of oscillation (feel free to change)
  z_wave_gen.amplitude = 0.0;  // amplitude of the wave (start at 0, change through serial)
  z_wave_gen.output.map(&channel_c.input_target_position);
  z_wave_gen.begin();*/


  // extrusion
  /*speed_e_gen.begin(); // all plugins must call begin
  speed_e_gen.speed_units_per_sec = 0.0;
  speed_e_gen.output.map(&channel_d.input_target_position);

  e_wave_gen.setNoInput();     // we will use the internal clock as the time variable
  e_wave_gen.frequency = 0.0; // frequency of oscillation (feel free to change)
  e_wave_gen.amplitude = 0.0;  // amplitude of the wave (start at 0, change through serial)
  e_wave_gen.output.map(&channel_d.input_target_position);
  e_wave_gen.begin();*/
  
  e_gen.begin();
  e_gen.input_1.map(&channel_a.input_target_position);
  e_gen.input_2.map(&channel_b.input_target_position);
  //e_gen.output.map(&channel_d.input_target_position);

  //extrusion knob
  analog_a1.set_floor(0, 25);
  analog_a1.set_ceiling(3, 1020);
  analog_a1.begin(IO_A1);
  
  // circle generator radius tuner knob
  analog_a2.set_floor(2, 25);
  analog_a2.set_ceiling(15, 1020);
  analog_a2.begin(IO_A2);

  /*
    // -- Configure the speed potentiometer --
  speed_potentiometer_a1.begin(IO_A1);
  speed_potentiometer_a1.set_floor(-20);
  speed_potentiometer_a1.set_ceiling(20);
  speed_potentiometer_a1.set_deadband(1, 509, 4); // deadband controls designated range where input movements do not produce output change to avoid jitter.
  speed_potentiometer_a1.map(&speed_x_gen.speed_units_per_sec); // map the value of the slider to the scaling filter ratio.

    // -- Configure the speed potentiometer --
  speed_potentiometer_a2.begin(IO_A2);
  speed_potentiometer_a2.set_floor(-20);
  speed_potentiometer_a2.set_ceiling(20);
  speed_potentiometer_a2.set_deadband(1, 509, 4); // deadband controls designated range where input movements do not produce output change to avoid jitter.
  speed_potentiometer_a2.map(&speed_y_gen.speed_units_per_sec); // map the value of the slider to the scaling filter ratio.

      // -- Configure the speed potentiometer --
  speed_potentiometer_a3.begin(IO_A3);
  speed_potentiometer_a3.set_floor(-20);
  speed_potentiometer_a3.set_ceiling(20);
  speed_potentiometer_a3.set_deadband(1, 509, 4); // deadband controls designated range where input movements do not produce output change to avoid jitter.
  speed_potentiometer_a3.map(&speed_z_gen.speed_units_per_sec); // map the value of the slider to the scaling filter ratio.
  */

  // -- Control interface (RPC) --
  rpc.begin(); 

  rpc.enroll("hello", hello_serial); // {"name": "hello"}

  // circle gen
  rpc.enroll("start_circle", start_circle);
  rpc.enroll("stop_circle", stop_circle);

  // x axis
  rpc.enroll("set_x_amplitude", set_x_amplitude);   // {"name": "set_x_amplitude", "args": [1]}
  rpc.enroll("set_x_frequency", set_x_frequency);   // {"name": "set_x_frequency", "args": [1]}
  rpc.enroll("set_speed_x", set_speed_x);           // {"name": "set_speed_x", "args": [1]}

  // y axis
  rpc.enroll("set_y_amplitude", set_y_amplitude);   // {"name": "set_y_amplitude", "args": [1]}
  rpc.enroll("set_y_frequency", set_y_frequency);   // {"name": "set_y_frequency", "args": [1]}
  rpc.enroll("set_speed_y", set_speed_y);           // {"name": "set_speed_y", "args": [1]}

  // z axis
  rpc.enroll("set_z_amplitude", set_z_amplitude);   // {"name": "set_z_amplitude", "args": [1]}
  rpc.enroll("set_z_frequency", set_z_frequency);   // {"name": "set_z_frequency", "args": [1]}
  rpc.enroll("set_speed_z", set_speed_z);           // {"name": "set_speed_z", "args": [1]}

  // extrusion rate
  rpc.enroll("set_e_amplitude", set_e_amplitude);   // {"name": "set_e_amplitude", "args": [1]}
  rpc.enroll("set_e_frequency", set_e_frequency);   // {"name": "set_e_frequency", "args": [1]}
  rpc.enroll("set_speed_e", set_speed_e);           // {"name": "set_speed_e", "args": [1]}

  // -- Start the stepdance library --
  dance_start();
}

LoopDelay overhead_delay;

void loop() {
  // Extrusion calibration
  float64_t extrusionMultiplier = analog_a1.read();
  e_gen.set_ratio(-extrusionMultiplier);

  overhead_delay.periodic_call(&report_overhead, 500);

  dance_loop(); // Stepdance loop provides convenience functions, and should be called at the end of the main loop
}


void hello_serial(){
  Serial.print("hello!");
}

void start_circle(){
  circle_gen.output_x.enable();
  circle_gen.output_y.enable();
  }

void stop_circle(){
  circle_gen.output_x.disable();
  circle_gen.output_y.disable();

} 

// x axis
void set_speed_x(float32_t speed_x){
  speed_x_gen.speed_units_per_sec = speed_x;
}

void set_x_amplitude(float32_t amplitude){
  x_wave_gen.amplitude = amplitude;
}

void set_x_frequency(float32_t frequency){ // 0 - 20 ?
  x_wave_gen.frequency = frequency;
}

// y axis
void set_speed_y(float32_t speed_y){
  speed_y_gen.speed_units_per_sec = speed_y;
}

void set_y_amplitude(float32_t amplitude){
  y_wave_gen.amplitude = amplitude;
}

void set_y_frequency(float32_t frequency){ // 0 - 20 ?
  y_wave_gen.frequency = frequency;
}

// z axis
void set_speed_z(float32_t speed_z){
  speed_z_gen.speed_units_per_sec = speed_z;
}

void set_z_amplitude(float32_t amplitude){
  z_wave_gen.amplitude = amplitude;
}

void set_z_frequency(float32_t frequency){ // 0 - 20 ?
  z_wave_gen.frequency = frequency;
}

// e axis
void set_speed_e(float32_t speed_z){
  speed_e_gen.speed_units_per_sec = speed_z;
}

void set_e_amplitude(float32_t amplitude){
  e_wave_gen.amplitude = amplitude;
}

void set_e_frequency(float32_t frequency){ // 0 - 20 ?
  e_wave_gen.frequency = frequency;
}

// --
void report_overhead(){
  Serial.println(analog_a1.read());
}


