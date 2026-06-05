
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


// -- RPC Interface --
RPC rpc;



// -- Potentiometer to control extrusion -- //

//extrusion rate .52, via potentimetere 
AnalogInput analog_a1;
AnalogInput analog_a2;


PathLengthGenerator2D e_gen; //generates extruder signal

KinematicsPolarToCartesian polar_kinematics;
VelocityGenerator angle_speed;
PositionGenerator radius_change;

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


 
  polar_kinematics.output_x.map(&channel_a.input_target_position);
  polar_kinematics.output_y.map(&channel_b.input_target_position);
  polar_kinematics.begin();

  angle_speed.output.map(&polar_kinematics.input_angle);
  angle_speed.begin();

  radius_change.output.map(&polar_kinematics.input_radius);
  radius_change.begin();



  circle_gen.rotational_speed_rev_per_sec = 1.0;
  circle_gen.setNoInput(); // Use internal frame count
  //circle_gen.output_x.map(&channel_a.input_target_position);
  //circle_gen.output_y.map(&channel_b.input_target_position);
  
  //analog_a2.map(&circle_gen.radius);
  circle_gen.radius = 10;
  circle_gen.disable();
  circle_gen.begin();

  e_gen.begin();
  e_gen.input_1.map(&channel_a.input_target_position);
  e_gen.input_2.map(&channel_b.input_target_position);
  e_gen.output.map(&channel_d.input_target_position);

  //extrusion knob
  analog_a1.set_floor(1, 25);
  analog_a1.set_ceiling(10, 1020);
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
  angle_speed.speed_units_per_sec = 5;
  radius_change.go(20,INCREMENTAL,1);
  //circle_gen.output_x.enable();
  //circle_gen.output_y.enable();
  }

void stop_circle(){
  polar_kinematics.input_radius.reset(0);
  angle_speed.speed_units_per_sec = 0;
  //circle_gen.output_x.disable();
  //circle_gen.output_y.disable();

} 
// --
void report_overhead(){
  Serial.println(analog_a1.read());
  Serial.println(polar_kinematics.input_radius.read_absolute());
  Serial.println(polar_kinematics.input_angle.read_absolute());
}


