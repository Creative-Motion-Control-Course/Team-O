---
layout: default
title: "Edible Patterns"
---

# Milestone 2: Mid-point Class Review

## 1. Adapt the 3D printer Ender and connect it to Stepdance and Teensy
We worked on adapting the Ender machine with Emily. We used what we learned from the analog slider about crimping and electronics. We do not need an encoder to control the Z axis and the extrusion system; that is the most important conclusion of this step.

## 2. First idea for the syringe holder
After talking with the instructors, we defined that the final design is to place the syringe beside the stepper motor.
1. We decided to use the same system Jennifer showed: a stepper motor moving the system up and down with a lead screw.
2. We evaluated the two designs Alejandro proposed and selected the one where the stepper motor is mounted on top of the metal plate.
3. We tested the weight of the whole lead screw motion system on the plate to see if it could resist. It performed very well.
4. We figured out how we are going to hold the screw motion system, but we still need to solve how we are going to hold the syringe.

## 3. 3D printing v1
We designed the first motor holder piece using Fusion. We printed it and noted some issues, so we need to iterate. With Alejandro's help we changed the design and printed again.

### 4. 3D printing v2
The result was more resistent than before and now we are able to mount the stepper motor. We still have some problems with the model so we need to model and print again.

# Challenges and proposals

## Syringe
Since we do not have the syringe yet, we decided to buy one. That way we can try a cardboard mockup before modeling.

## Syringe holder
We are considering to use threaded insert to join the syringe.

## Joystick control
Since we need to adapt the joystick to receive two encoder signals, we decided to use keyboard arrows first as a preliminary step.

# Pictures and video

![Cheese prototype](/assets/cheese_v1.jpeg)
![Holder 1](/assets/Holder1.jpeg)
![Holder 2](/assets/Holder2.jpeg)
![Measuring 1](/assets/measuring1.jpeg)
![Measuring 2](/assets/measuring2.jpeg)
![Measuring 3](/assets/measuring3.jpeg)
![Measuring 4](/assets/measuring4.jpeg)

   
   
# Milestone 3: Peer Review

We had the meeting with Team Fish. First they showed us their work and next steps, and then we presented ours. Our suggestions to them were related mostly with the UI and presets. Regarding our work, they told us that the joystick is a good idea because it is more intuitive to control the machine. They also suggested paying special attention to the syringe and the non-linear extrusion that it generates due to its mechanism. Finally, they recommended trying as soon as possible with frosting cake to observe the behaviour of the whole system — material, machine head, syringe, and control — all together.

---

# Final Project: Edible Patterns — Piping Machine

## System Overview

The Edible Patterns machine is a modified Ender 3 3D printer adapted for food extrusion. The extrusion head holds a syringe mounted beside a stepper motor, driven by a lead screw that pushes the syringe plunger down. The Teensy 4.1 microcontroller runs the [Stepdance](https://stepdance.cc) library and communicates with a browser-based control UI over USB serial at 115200 baud using a JSON RPC protocol.

The current firmware (`try5.ino`) exposes the following controls:

- **Tip type selection** — `set_tip_type(1/2/3)` switches the Z-axis channel ratio and extrusion multiplier at runtime, letting the operator swap heads without reflashing.
- **Spiral pattern** — `start_spiral(speed, radius)` uses polar-to-Cartesian kinematics to generate an inward spiral from a given radius. `stop_spiral()` halts the pattern; the timed Z lift-and-retract sequence runs in the HTML layer to avoid generator override conflicts.
- **Manual jogging** — `set_speed_x/y/z/e` drives individual axes by velocity, used for positioning and manual piping points.
- **Path-length extrusion** — a `PathLengthGenerator2D` ties extrusion rate to XY travel distance automatically.

The control UI (`Piping Control.html`) runs fully in the browser via the Web Serial API — no server required. It handles tip selection, spiral parameters, keyboard and gamepad jog control, and the timed stop sequence.

---

## Iterations

**Mechanical prototyping with Alejandro and Emily**
Emily and Alejandro were key to ensuring the system worked well. Emily helped us adapt the Ender 3 wiring and figure out which axes needed encoders (conclusion: Z and E do not). Alejandro proposed and iterated the motor mount designs, moving through two rounds of 3D printing until the holder was rigid enough to support the lead screw assembly under load. We evaluated his two bracket proposals and selected the one with the motor mounted on top of the metal plate.

**CAD ↔ cardboard back-and-forth**
Before committing to 3D printed parts we went through a physical prototyping phase with cardboard mockups of the syringe holder geometry. This back-and-forth between Fusion 360 and cardboard let us resolve fit issues cheaply before printing.

**Pencil test for spiral behaviour**
Before using any food material we clamped a pencil in place of the syringe to trace spirals on paper. This let us tune the speed, radius, and Z-lift parameters without wasting material or making a mess.

**Spiral kinematics with Jennifer**
Jennifer introduced us to the `KinematicsPolarToCartesian` approach in Stepdance. This replaced an earlier circle generator and gave us true inward-spiral motion by combining a constant angular velocity generator with a decaying radius position generator.

**UI with Claude Code**
The final control interface was built iteratively using Claude Code (Claude Sonnet 4.6). We started from a rough layout sketch and went through multiple rounds of design review — restructuring the step flow, merging the preview into the parameters panel, wiring the gamepad correctly, and stripping dead code — until the UI matched the physical workflow.

---

## Tip Types

The machine supports three interchangeable piping tips. Selecting a tip in the UI sends `set_tip_type` over serial, which updates both the Z stepper gear ratio and the extrusion multiplier in real time.

| Tip | Z ratio | Extrusion multiplier |
|-----|---------|---------------------|
| Regular | 1 : 200 | 0.48 |
| Big Star | 1 : 600 | 0.98 |
| Small Star | 1 : 400 | 0.78 |

![Regular tip](/assets/type1_regular.png)
![Big star tip](/assets/type2_big_star.png)
![Small star tip](/assets/type3_small_star.png)

---

## Control Interface

The UI is a single HTML file with no dependencies beyond Web Serial. The operator follows five numbered steps: connect, pick a tip, set spiral parameters, run/jog via keyboard or gamepad, and access advanced motor controls.

![Control UI joystick mapping](/assets/joystick.png)

---

## Videos

**Final controls demo**

<video src="assets/final_controls.mp4" controls width="100%"></video>

**Final result**

<video src="assets/final_result.mp4" controls width="100%"></video>

---

## Relevance, Usability, and Responsiveness

### Relevance
Cake decorating is a skilled craft where precision, repeatability, and speed matter. Professional pipers spend years developing muscle memory for consistent spiral patterns, rosettes, and dot work. Our machine makes this level of output accessible to anyone — a bakery operator can dial in a spiral size, select the right tip, and run repeatable decorations without manual piping skill. The output is immediately meaningful: it produces real edible decorations on real food, closing the loop between the machine and its creative purpose.

### Usability
The interface is designed around the physical workflow rather than the machine's internal architecture. The operator does not need to know about stepper ratios or RPC calls — they pick a tip from a picture, set a radius on a slider with a live preview, and press Run. The keyboard jog controls map to spatial intuition (arrows = XY, W/S = Z up/down), and the gamepad mapping mirrors the same layout for operators who prefer physical controllers. Both input methods do exactly the same thing, giving users a familiar entry point regardless of their background. The system builds on skills that bakers already have — understanding tip shapes and approximate sizes — while the machine handles the precision.

### Responsiveness
The spiral speed slider sends `set_spiral_speed` live while a print is running, so the operator can slow down at corners or speed up on straight sections without stopping. The joystick axes respond with minimal latency (50 ms polling loop) so manual point work feels direct. The timed stop sequence — Z retract, pause, Z lift — runs automatically on Stop so the operator does not need to manually manage the nozzle position between prints.

---

## Future Development

- **Quick-change syringe mount** — The current syringe holder requires unscrewing to refill. A bayonet or quarter-turn locking collar would let operators swap a pre-filled syringe in seconds, which is critical for production use.

- **Automated point generation and curves** — Currently all non-spiral motion is manual jogging. A natural next step is to let the operator define a path (sequence of XY points or a Bézier curve) in the UI and have the machine execute it automatically with consistent speed and extrusion.

- **Spherical / vertical-surface control** — A more ambitious extension would add two rotational axes to the head mount, giving spherical reach. Combined with a more viscous or fast-setting material this would allow extrusion on vertical surfaces — sides of cakes, 3D forms — rather than just the top plane.
