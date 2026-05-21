/*
  UCSB MAT Creative Motion Control -- Generators example
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

// -- Define Motion Channels --
Channel channel_a;  //Ender "x"
Channel channel_b;  // Ender "y" 
Channel channel_c;  // Ender "z"

// -- Define Encoders --
Encoder encoder_1;

// -- RPC Interface --
RPC rpc;


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
  channel_c.begin(&output_b, SIGNAL_E);



  // These ratios are for the Axidraw V3: 2032 steps correspond to 1 inch (25.4mm)
  channel_a.set_ratio(25.4, 2032);
  channel_a.invert_output(); // We do that so that the X axis points from motor A to motor B (left to right)
  channel_b.set_ratio(25.4, 2032);
  channel_b.invert_output(); // We do that so that the Y axis points down (away from the long axis)
  channel_c.set_ratio(25.4, 2032);
  channel_c.invert_output(); // We do that so that the Y axis points down (away from the long axis)



  // Team - O
  // -- Configure and start the encoders --
  encoder_1.begin(ENCODER_1); // "ENCODER_1" specifies the physical port on the PCB
  encoder_1.set_ratio(24, 2400);  // 24mm per revolution, where 1 rev == 2400 encoder pulses
  encoder_1.output.map();


  // -- Control interface (RPC) --
  rpc.begin(); 

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