# Casio calculator data logger extended capabilities; remote control, survey logging, human-machine-interface, IMC simulator - the $10 upgrade

Public URL: **https://MikeFentonNZ.github.io**

Updated from Michael Fenton's original work with the Casio FX-9750 in the mid 2000s. An ultra-low-cost open-source project that connects Casio FX-9750 and FX-9860 graphing calculators to external microcontrollers such as Picaxe, ESP8266, ESP32, BBC Microbit and Arduino sensor units.

These calculators cannot carry out automated time-interval data logging on their own. This project addresses that, enabling a low-cost smart data logger that also acts as a remote control and automation control interface. It enables authentic STEM investigations at school, at home, across subject areas and at all year levels. It supports out-of-field and novice STEM teachers by revealing a tool students already own permits student-led learning. Students become the expert tool builders and users, while teachers guide activities tied to curriculum goals.

## How it works

The Casio serial protocol has positions in its `RECEIVE()` exchange where the calculator waits for the attached device **without any deadline** — it raises no COM ERROR however long the wait. Four such positions were found by inserting a pause at every point in the device-side flow, line by line, and recording the result. Two of them are useful.

A microcontroller holds the calculator at one of those points while it counts out a sampling interval, so the **device** keeps time and the calculator simply receives. No specialised hardware is needed — no EA-200, no CLAB, no E-CON application.

These positions are called **host-wait windows**. The priority disclosure below uses the earlier term *READY-gated timing tolerance*; it was renamed in August 2026, before peer-reviewed publication. The finding is unchanged.

The simplest build is a Casio FX-9750 or FX-9860 graphing calculator, a microcontroller costing a few dollars, and a cross-over cable. Sensor readings arrive at a set interval and land in the calculator's own lists, where the graphing and statistics tools students already know are waiting for them.

For a few dollars more, the Casio can log data from remote sensors wirelessly and serve a web page with data to smart devices including phones, tablets, and laptops.

The calculator needs no modification or firmware change. It does not need to know what is on the other end of the cross-over cable.

## Priority and publication

* **Priority disclosure:** Fenton, M. (2026). *Casio Graphing Calculator Serial Interface: Priority Disclosure of Timing Discoveries, Encoding Invention, and Operational Modes (FX-9750 and FX-9860 Series).* Zenodo. https://doi.org/10.5281/zenodo.19303911
* **2008 classroom research:** Fenton, M. (2008). *Authentic Learning Using Mobile Sensor Technology.* New Zealand Ministry of Education E-Learning Fellowship report. Zenodo. https://doi.org/10.5281/zenodo.19302276
* **2009 international presentation:** Fenton, M. (2009). *RIGEL — Learning From Life: Communities of Learning via a Connected Curriculum.* Microsoft Partners in Learning Regional Innovative Teachers Conference, Kuala Lumpur, 27–29 May 2009. Zenodo. https://doi.org/10.5281/zenodo.19334228
* **Simultaneous synchronous multi-sensor transmission** — paper in preparation. SHA-256 of the manuscript, March 2026:
`6F0C9402CD3F0715F3660BC864322569D40205D0C96F509097A62C10A98B0B3A`
The encoding method is withheld pending peer-reviewed publication.

## Repository contents

Files are being released in stages.

* `/casio\\\_calclculator\\\_program` — the Casio BASIC datalogger program in `.txt` and `.g1m` form, a key-code utility, and installation notes. 
* `/images\\\_video` — Casio serial protocol diagrams, assembly, and use.
* `/original\\\_version` — the 2008 education research report and analysis of classroom use.
* `LICENSE`, `README.md`, `.gitattributes`

**Next release:** ESP8266 and ESP32 firmware, BBC Micro:bit firmware, wiring diagrams, protocol notes, and the experiment manual.

## Features

* **Compatible calculators:** Casio FX-9750 and FX-9860 series. All figures below were measured on an FX-9750GIII. Casio issues one firmware image for the FX-9750GIII and FX-9860GIII, so the protocol behaves the same on both.
* **Microcontrollers:** Picaxe 08M2 and 14M2, ESP8266 (Wemos D1 mini), ESP32, BBC micro:bit V1 and V2.
* **Sampling interval:** 300 seconds is a chosen working limit with margin, not a ceiling. A pause of three hours has been held, and no pause has ever failed for being too long. Battery voltage is the limiting factor. The Casio auto-power-off (APO) is disabled.
* **999 readings per session:** The calculator's list capacity, confirmed on hardware. MORE readings can be made if readings are spread across lists. Casio data storage capacity is the limiting factor.
* **Non-volitile memory:** The calculator's logged data is retained in the event of a power loss.
* **One Casio BASIC program serves every microcontroller.** The calculator never learns which device is attached.
* **Nothing is timestamped in the demonstration code:** The calculator computes elapsed time as interval x (reading number - 1). If the device runs slightly slow no error appears and no reading is dropped; the time axis is simply stretched. On a Picaxe that stretch is about 4.4 to 5.0 % at a 1-second interval and 2.5 to 3.1 % at 2 seconds. The shape of a curve survives this. A rate does not. Timestamps CAN be provided by a suitable microcontroller to permit accurate rate calculations with reasonable precision.
* **Web-based data sharing:** both ESP builds run their own WiFi access point and serve a live status page and CSV download to phones and laptops **while logging continues, with no reading lost**. Two clients at once has been tested.
* B**oth ESP boards keep exact time at 1 Hz:** Verified in trials against an external timer.
* **Optional wireless link:** on ESP32 hardware an ESP-NOW link has been proven through a house, across 20 metres and into a metal three-bay shed. In open air, 200-plus metres, measured repeatedly.
* **A DS18B20 temperature sensor costs a Picaxe 750 ms per reading:** The picaxe chip's clock stopped throughout. That is a 16 % time-axis error at a 5-second interval. Use 30 seconds or longer for a rate, or use an ESP board, which does not have this problem.
* **Interface:** Casio SB-62, 3-pin TTL serial at 9600 baud. The FX-9750GIII port is 3.3 V; older FX-9750G Plus calculators are 5 V TTL.

## More than data logging:

* **Casi II robot control:** A remake of the original Casio calculator-controlled robot, with a micro:bit in place of the Picaxe. A second micro:bit connected to the Casio sends calculator key presses by radio to the robot (see the 2008 E-Learning report https://doi.org/10.5281/zenodo.19302276).
* **Mars surface surveyor:** Attach a low-cost ultrasonic rangefinder module (Aliexpress) and map a simulated Martian surface from the air (see the 2008 E-Learning report https://doi.org/10.5281/zenodo.19302276).
* **A user-triggered field survey logger:** Combining microcontroller-maintained elapsed-time recording and user-entered categorical observation. For example, using single numeric keys 0–9 provide 10 categories; alpha keys A–Z provide more categories for applications requiring finer classification. Examples: geological substrate (1 = scoria, 2 = sand, 3 = basalt), vegetation type, land use class, or surface condition.
* **Human-machine-interface:** The calculator transmits user-entered numerical values to the microcontroller via the serial interface. The calculator’s tamper-evident keypad and display provide a secure input mechanism for applications requiring user-generated discrete values. One group sets up a model door lock with a 4-digit PIN and no lockout. A second group is asked to open it without being told the code. Teaches coding, cyber security, building systems safety, and the rule 'the secret belongs with the thing being protected, not with the thing a user is holding.
* **IMC simulator:** The Casio calculator and connected microcontroller form a closed-loop measurement and control system. The microcontroller reads one or more sensors, transmits them to the calculator for display. The student observes the live readings, makes a control decision, and transmits a control value back to the microcontroller via the calculator keypad. The microcontroller receives that value and adjusts a physical output accordingly; motor speed, heater power, valve position, or light intensity. The student then observes the effect of their intervention in the next sensor reading.
* **Over distance:** On ESP32 hardware an ESP-NOW radio link has been proven through a house, across twenty metres, and into a metal three-bay shed. That matters more than a clear-air figure, because a shed is the sort of place a measurement actually has to come from. In open air the range is 200-plus metres, measured repeatedly. Remote sensing using Microbit radio is another option.
* **Sharing live data:** Both ESP builds run their own WiFi access point and serve a status page and a CSV download to phones and laptops while logging continues, with no reading lost. Two clients at once has been tested.

## Safety and hardware notes

* **NEVER connect mains electricity (240 V / 110 V)** to the calculator, the microcontroller, or any sensor wiring. **NEVER use mains-connected equipment near water.**
* **Keep every sensor signal within 0 V to 3.3 V.** The ESP32 and ESP8266 are not 5 V tolerant. A bare ESP8266 A0 pin reads 0 to 1.0 V only; 3.3 V will destroy it. However, popular development boards like NodeMCU and Wemos D1 Mini include an onboard resistor voltage divider, which safely extends their external board tolerance to 0 to 3.2V–3.3V
* **Do not connect 5 V logic** to the Casio port unless a bidirectional level converter is used.
* **When breadboarding with a 4-pin USB serial cable, remove the red V+ wire** and power the microcontroller from its own cells.
* **Never use boiling water for temperature calibration** — it is not needed — and never work near live electrical outlets.

## Quick start for teachers

1. **Gather parts:** Casio SB-62 cable or breakout; Picaxe, ESP32 or Wemos; battery pack; one sensor; jumper wires; the 4.7 kΩ resistor and 1N4148 diode
2. **Install the calculator program** from `/casio\\\_calclculator\\\_program`.
3. **Wire and test**:

   * A calculator: FX-9750 or FX-9860 series. Casio issues one firmware image for the FX-9750GIII and FX-9860GIII, so the protocol behaves the same on both.
   * A microcontroller: Picaxe 08M2 or 14M2 - cheapest, one chip, no board. Alternatively use a ESP8266 or ESP32 development board, which cost more but adds WiFi capability and timestamping. Arduino or BBC micro:bit V2 are also suitable.
   * A cable: a Casio SB-62, or two 2.5 mm 3-pin plugs and some wire.
   * Resistor: a 4.7 k pull-up from the calculator's transmit line to +3.3 V
   * 1N4148 diode: in series on the calculators receive line./li>
   * A sensor: a thermistor or an LDR makes a good first one. Both read in about a millisecond.
4. **Install the microcontroller program**; suggested using the Picaxe for first-use simplicity. 
5. **Run a lesson:** Turn on the microcontroller, launch the data logger program on the calculator, collect samples, graph the data, and use the calculator's own analysis tools for regression and interpretation.

## Classroom lesson ideas

* **Starter:** temperature against time; students plot and calculate rates of change.
* **Calibration:** calibrate a light sensor and compare measured values with expected behaviour.
* **Project:** student-designed investigation using one or more sensors, with error analysis and presentation.
* **Cross-curricular:** science, mathematics, digital technology, and physical education — heart-rate recovery measured over days or weeks suits a logger that belongs to the student and goes home with them.

### Use it - Across subjects

A graphing calculator is bought for one reason: mathematics requires it. It is a significant purchase for a family, it is carried for three to five years, and for most of that time it does one thing. This gives the same device further uses, in further subjects, without altering it in any way.



#### Mathematics

* what it was bought for. Tradional uses cases involving calculations and data analysis (usually simulated or pre-recorded / published data). Now learners can gather authentic data at home, on farms, on marae, on holiday.

#### Science

* a data logger, with the analysis happening in the environment students were taught in, on the device they were taught on. New data collection includes field studies recording categorical data and observations, not just numerical data. Anywhere at any time.

#### Physical education

* heart-rate recovery measured after exercise and repeated over days or weeks. That wants a logger which belongs to the student and goes home with them, which is exactly what shared laboratory equipment cannot be.

#### Digital technology and computing

* the calculator as a human-machine interface, the operator's panel for a process the student has built and programmed.

The economics run the opposite way to the usual low-cost argument. Normally a cheap option is a poorer substitute for a better thing. Here the expensive item has already been bought, by the family, for a different subject. What this adds costs a few dollars, so the marginal cost of the second and third uses is close to nothing.

### Use it - Across year groups / levels

* Younger learners at Year 9 are no longer excluded from authentic investigations because science equipment is limited to senior classes. When commercial equipment is expensive, complicated or easliy damaged, it is often reserved for senior student use. When equipment is low-cost and built and coded by the learner, they see themselves as real investigators, with low floor, high ceiling opportunities available as the default state, not a special case.

### Use it - Supporting novice or out-of-field teachers

* Due to ongoing secondary teacher shortages in New Zealand, many schools are forced to have teachers work "out-of-field" or "out-of-subject"—teaching classes outside their primary area of specialist qualification. This practice is most common in high-demand subjects like science, mathematics, technology. Teachers can learn with students. Students become the technology experts with devices that enable increase learner agency for STEM investigations involving personal interest and real-world applications. This is of special relevance to the new industry-led subjects in the New Zelaand cirriculum. One example follows.

### Use it - Industrial Measurement and Control

Instrumentation technicians install, calibrate, maintain and repair the equipment that measures and controls industrial processes - dairy manufacturing, water and wastewater treatment, pulp and paper, food and beverage, metal refining, power generation. Wintec, the only New Zealand provider of the theory qualifications, describes it as "one of those careers that most people do not know exists, but is vital for industries worldwide", with technicians in high demand.

In New Zealand it begins at Level 4, and the normal route in is to qualify and work as an industrial electrician first. Dual trade - Electrical Engineering Level 4 with Industrial Measurement and Control Level 4 - is the common minimum an employer looks for. There is nothing at Level 2. The absence is structural rather than accidental: the discipline is a post-trade specialisation, and Level 4 assumes real industrial PLC and instrumentation hardware.

Which means a secondary student cannot encounter this career through the education system at all. They cannot meet it, try it, or find out whether it suits them, until after they have already committed to a different trade.

###### What a calculator and a microcontroller can actually demonstrate.

Every core idea in measurement and control is present, at a scale a student can hold:

* Measurement - a sensor turns a physical quantity into a signal, and the signal is not the quantity. Range, resolution, drift and calibration all become visible when the student built the sensor.
* Set points and limits - a value is not simply high or low, it is inside or outside a band somebody chose, and choosing that band is an engineering decision with consequences.
* Control - the microcontroller switches a heater, a fan or a vent. On and off first; proportional control once that is understood.
* The closed loop - sense, compare against the set point, act, then sense again. The loop is the whole subject, and a student can watch it run.
* Alarm and fault management - a status field reports which channel is out of range, or that the instrument itself has failed. Distinguishing "the process is wrong" from "the instrument is wrong" is what an instrumentation technician is paid for.
* The human-machine interface - the calculator becomes the operator's panel, showing live values, status and alarms, and accepting commands from the keypad.
* Fail-safe design - what should the system do when it loses contact, or loses power? A fault must never look like a normal result. That principle runs through every source file in this project, and it is the one an industry assessor will ask about.
* Data logging as process monitoring - the same record that serves a science investigation is, in an industrial context, the evidence that a process stayed within specification.



## Why the cost matters

Total hardware cost is roughly USD $6–$10, and the calculator is already in the school bag because mathematics requires it. The marginal cost of using it as a science instrument is close to nothing.

At this price the use of AI-generated or simulated data in the classroom becomes a choice requiring justification rather than a practical necessity. Researchers have raised legitimate concerns that AI training datasets may be compromised by fabricated or low-quality material in the scientific literature. Where authentic first-hand measurement can be collected cheaply and reliably, it should be.

## Prior work

* **Erik Grindheim (2001)** published the Casio CFX-9950G communications protocol. Written from a personal computer, which explains what it does not contain: one timeout of 0.5 to 1 second, and no host-wait window.
* **Michael Fenton (2004–2008)** adapted that work after correspondence with Andrew Hornblow and published Picaxe-to-Casio example code and protocol notes on the Revolution Education forum.
* **Anobium (2012)** and **nsg21 (2018)** both built on that published code and both credited it.
* **MiniExperimenter (shabaz123, 2020)** is the most technically sophisticated independent prior work identified. It uses the EA-200 hardware protocol at a different baud rate, does not address interval timing, and does not encode multiple sensors within a single transmission.

Casio's EA-100 and EA-200 data acquisition units are discontinued; the Casio New Zealand office confirmed in 2026 that the last unit was sold in this market in 2008.

## Contact and status

Project lead: Michael Fenton MRSNZ — see the project site for contact details.
Repository status: published and read-only unless otherwise noted. Files are provided for direct download. The teacher is responsible for how this material is used with learners.

## Licence

**Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International (CC BY-NC-SA 4.0)** — code and learning materials alike. See `LICENSE`.

https://creativecommons.org/licenses/by-nc-sa/4.0/

Attribution example: *Casio Calculator Data Logger: timed sensor sampling using host-wait windows in the Casio serial protocol — Michael Fenton — CC BY-NC-SA 4.0*

The multi-sensor encoding method is withheld from all licences pending peer-reviewed publication.

