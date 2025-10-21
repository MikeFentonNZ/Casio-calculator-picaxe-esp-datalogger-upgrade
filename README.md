# Casio Calculator Picaxe ESP Datalogger Sensor Upgrade

Public URL: **https://MikeFentonNZ.github.io**



Updated from Michael Fenton's original work in the mid 2000's. Next-generation ultra-low-cost open-source project that connects Casio FX-9750 and FX-9860 graphing calculators to $6 USD Picaxe, ESP8266, and ESP32 microcontroller sensor units.



Michael has identified a method that allows the Casio FX 9750G and FX 9860G calculators to pause during a RECEIVE() operation without triggering a COM ERROR. This enables external sensor units to send data at preset intervals (e.g., every 10 seconds, every minute) without requiring specialized hardware such as the EA 200 Data Analyzer, CLAB or E-Con application. Michael's system realises a hundred-fold decrease in cost.



This site will archive program code, wiring diagrams, and teaching resources for classroom, outdoors, and home data collecting investigations.

## Quick links

* Project site: https://mikefentonnz.github.io/projects/casio-calculator-sensor-upgrade.html
* Download teacher lesson pack: /teaching\_resources
* Multi-sensor unit firmware examples (Picaxe, ESP8266, ESP32): /datalogger\_sensor\_unit
* Casio FX-9750GIII calculator program: /casio\_calculator\_program

## Why this matters



* Bypasses a known limitation: The FX 9750G and its relatives are notorious for their strict communication timeouts (≈ 0.1 to 1s). Normally, this prevents any long interval data logging directly over the SB-62 cable. Safely and reliably pausing mid RECEIVE() sidesteps one of the calculator’s biggest constraints.
* Enables true interval logging without extra hardware: Until now, the only practical way to do timed sampling was to use Casio’s EA 200/CBL2 data analyzer of CLAB, which has its own clock. Michael's  method allows a simple sensor unit to handle timing and still communicate directly with the calculator, eliminating the need for expensive proprietary peripherals.
* Expands the calculator’s role in experiments: In education and hobbyist science, this means the FX 9750G or FX 9860 could be used as a lightweight data logger for physics, chemistry, or environmental monitoring — something previously limited by the COM ERROR timeout.
* Potential for open source adoption: A new tool for interfacing calculators with sensors.
* This upgrade makes science data recording portable and affordable ($6 USD budget). Use in any class, take to the fields, the gym, on a walk, on holiday.
* Ultra-low-cost sensors: Works with cheap low-cost DIY / home-made and low-cost sensors; measure range (distance), water vapour, temperature, pressure, alpha radiation, visible light, ultra violet (UV) light, infra red (IR) light, water hardness, salinity, static electric fields, mobile phone signals, angles, magnetic fields, and more.
* Democratising STEM Education: This system fundamentally challenges the traditional model where "fragile" and "expensive" equipment is reserved for senior students.
* Flexible curriculum uses across music, physical education, chemistry, physics, biology, environmental science, and mathematics.

## Features

* Compatible calculators: Casio FX-9750 and FX-9860 series.
* Timestamped data collection using mid-RECEIVE() pause with no COM error.
* Microcontrollers supported: Picaxe 08M2/14M2, ESP8266 (e.g., Wemos D1 mini), ESP32.
* Web-based data sharing option: Live data served as webpages with graphs accessible on smartphones, tablets, and laptops.
* Optional wireless bi-directional communication: Over 200-metre range for remote data collection and remote equipment control. Drive a robot using your calculator!
* Interface: Casio SB-62 3-pin TTL serial at 3.3V logic.
* Provided assets: Casio BASIC datalogger program, microcontroller firmware, wiring diagrams, teacher notes, research on use in class, lesson ideas, slide deck.
* License: MIT for code; Creative Commons Attribution 4.0 for lesson materials.

## Quick start for teachers

1. Gather parts: Casio SB-62 cable or breakout; Picaxe/ESP32/Wemos; battery pack; one sensor; jumper wires.
2. Flash firmware: Copy example firmware from /firmware to the microcontroller.
3. Install calculator program: Upload the Casio BASIC datalogger from /calculator to student calculators.
4. Wire and test: Connect sensor to microcontroller and microcontroller TX/RX to the SB-62 breakout using 3.3V logic only.
5. Run lesson: Launch the datalogger on the calculator, collect samples, graph data, and use calculator analysis tools for regression and interpretation.

## Classroom lesson ideas

* Starter activity: Temperature vs time logging; students plot and calculate rates of change.
* Calibration lab: Calibrate a light sensor and compare measured values to expected behaviour.
* Project: Student-designed investigation using one or more sensors with error analysis and presentation.

## Safety and hardware notes

* Do not connect 5V logic directly to Casio ports; use 3.3V logic or a safe level shifter.
* Protect ports with series resistors as described.
* Never use boiling water for temperature calibration (it is NOT needed) or work with live electrical outlets.

## Repository layout

* /datalogger\_sensor\_unit — microcontroller example code for Picaxe, ESP8266, ESP32.
* /casio\_calculator\_program — Casio BASIC datalogger program and installation notes.
* /images\_video — wiring diagrams, assembly, using the datalogger, teaching examples.
* /teaching\_resources — worksheets, slide deck, notes.
* /original\_version — Michael's education research report and analysis on classroom use.
* README.md — this file
* LICENSE — code license and lesson material license



## Contact and status

Project lead: Michael Fenton — see project site for contact details.  
Repository status: Published and read-only unless otherwise noted. Files are provided for direct download. Teacher is responsible for how this material is used in class with learners.

