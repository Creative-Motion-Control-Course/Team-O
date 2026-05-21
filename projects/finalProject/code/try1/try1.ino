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
Channel channel_a;  //Ender "x"
Channel channel_b;  // Ender "y" 
Channel channel_c;  // Ender "z"
Channel channel_d;  // Ender "z"


// -- Define Encoders --
Encoder encoder_1; // x axis
Encoder encoder_2; // y axis

// -- RPC Interface --
RPC rpc;

// -- Potentiometer -- 
AnalogInput potentiometer_a1; // z axis


void setup() {
  // -- Configure and start the output ports --
  output_a.begin(OUTPUT_A); // "OUTPUT_A" specifies the physical port on the PCB for the output.
  output_b.begin(OUTPUT_B);
  output_c.begin(OUTPUT_C);
  output_d.begin(OUTPUT_D);


  // Enable the output drivers
  enable_drivers();

  // -- Configure and start the channels --
  channel_a.begin(&output_a, SIGNAL_E);
  channel_b.begin(&output_b, SIGNAL_E);
  channel_c.begin(&output_c, SIGNAL_E);
  channel_d.begin(&output_d, SIGNAL_E);




  // These ratios are for the Ender N° ?? : 80 steps correspond to ?
  channel_a.set_ratio(1, 80);
  channel_a.invert_output(); 
  channel_b.set_ratio(1, 80);
  channel_b.invert_output(); 
  channel_c.set_ratio(1, 80);
  channel_c.invert_output(); 
  channel_d.set_ratio(1, 80);
  channel_d.invert_output(); 


  // Team - O
  // -- Configure and start the encoders --
  encoder_1.begin(ENCODER_1); // "ENCODER_1" specifies the physical port on the PCB
  encoder_1.set_ratio(24, 2400);  // 24mm per revolution, where 1 rev == 2400 encoder pulses
  encoder_1.output.map(&channel_d.input_target_position);

  encoder_2.begin(ENCODER_2); // "ENCODER_1" specifies the physical port on the PCB
  encoder_2.set_ratio(24, 2400);  // 24mm per revolution, where 1 rev == 2400 encoder pulses
  encoder_2.output.map(&channel_d.input_target_position);

  // -- Control interface (RPC) --
  rpc.begin(); 

  // -- Start the stepdance library --
  dance_start();
}

LoopDelay overhead_delay;

void loop() {
  // overhead_delay.periodic_call(&report_overhead, 500);
  dance_loop(); // Stepdance loop provides convenience functions, and should be called at the end of the main loop

}

/*
void report_overhead(){
  Serial.println("x axis : " +
  String(channel_a.input_target_position.read(ABSOLUTE)));
  
  Serial.println("y axis : " +
  String(channel_b.input_target_position.read(ABSOLUTE)));
}
*/

