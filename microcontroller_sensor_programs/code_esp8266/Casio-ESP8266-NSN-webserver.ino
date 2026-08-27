/*
 ===================================================================
  CASIO FX-9750 <-> ESP8266   NSN DATALOGGER
  Normalised scientific notation - ONE value per Receive(, SIGNED.

 (C) Michael Fenton, MRSNZ, 2026
 Licence: Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International 
 (CC BY-NC-SA 4.0)  

 Two RECEIVE() windows (GAP 1 and GAP 2) discovered 2007/2008
 using Picaxe 18X connected to a Casio FX-9750G Plus. See the author's
 New Zealand Ministry of Education E-Learning Fellowship report and 
 conference paper; 

 1) Authentic Learning Using Mobile Sensor Technology (2008): https://doi.org/10.5281/zenodo.19302276
 2) RIGEL - Learning From Life, Kuala Lumpur (2009): https://doi.org/10.5281/zenodo.19334228

 https://mikefentonnz.github.io/projects/casio-calculator-data-logger-hack.html
 ================================================================= 
 Version 1.0; 03/12/2025
 (Update of Picaxe 2.0 rework 10/10/2025,
  original version 1.0 code for Picaxe 18X invented 2007)

 Ported from Casio-NSN-14M2.bas (PICAXE 14M2), which was
 bench-validated in 2025 / reconfirmed 2026 with 0% packet failure.

  *** STATUS: PROOF OF CONCEPT - A FOUNDATION TO BUILD ON ***
  Validated against an FX-9750GIII and FX-9750G Plus: 
  Receive(N), Send(T), then repeated
  Receive(A)/(B)/(C) sampling at a set interval, values landing in the
  calculator's Lists across multiple samples. The ESP8266 is the second
  validated platform.

 This is a reference implementation based on a validated method.
 It is deliberately minimal so that every line can be read and
 understood. It is NOT a finished classroom product and will not
 suit every use case. MODIFICATION IS EXPECTED.

 The webserver includes lots of diagnostic data; instead show a chart
 of sensor readings once this code is trusted based on 0% errors. 
 ===================================================================
  THE ESP8266 PROBLEM, AND HOW THIS BUILD SOLVES IT
 ===================================================================
  The ESP8266 has exactly ONE analogue input. Not three. One.

  That looks fatal for a three-sensor logger, and it is the reason
  the ESP8266 is often written off for this kind of work. The usual
  answers are an external multiplexer (CD4051) or an external
  converter (ADS1115) - both meaning more parts, more wiring and
  more to go wrong in a classroom.

  This build needs neither, because it stops assuming that "sensor"
  means "analogue voltage". The board has three DIFFERENT ways of
  talking to sensors, and each can carry one channel:

      Channel 1   1-Wire      DS18B20 thermometer      (D7)
      Channel 2   Analogue    the single ADC pin       (A0)
      Channel 3   I2C         BME280 sensor module     (D1/D2)

  Three simultaneous readings, no multiplexer, no extra converter.

  It is also a better teaching arrangement than three identical
  analogue sensors would be, because a student meets all three ways
  a microcontroller can be given a number - one voltage, one digital
  bus with a single device, one addressed bus that could hold many -
  inside a single working program.

  THE ANALOGUE PIN IS A0, AND IT IS THE ONLY ONE.
  There is no second analogue pin hiding anywhere on this board.

 ===================================================================
  Consecutive RECEIVE() requests 
 ===================================================================
  Each sensor is sent as its OWN value, in its own Receive(
  transaction, in normalised scientific notation. The calculator
  stores what arrives straight into a List:

      Receive(N)   how many sensors    -> N
      Send(T)      the interval        -> T
      Receive(A)   sensor 1            -> List 2
      Receive(B)   sensor 2            -> List 3   (only if N > 1)
      Receive(C)   sensor 3            -> List 4   (only if N > 2)

  SIGNED VALUES ARE NATIVE. Byte 14 of the value packet is the
  sign/info byte: bit 0 says the magnitude is >= 1, and bits 6 and 4
  together say NEGATIVE. So -12.5 travels as a negative number and
  arrives as one. 

  A fourth RECEIVE(D) could act as a diagnosttic status indicator

  If a DS18B20 is not connected a 0 reading may be a fault or
  mistaken for a genuine zero degress.
  The Picaxe BASIC code checked the one-wire serial number to detect
  if a DS18B20 was disconnected if configuration code said it was present.

  Not enabled here yet...

 ===================================================================
  WARNING
 ===================================================================
  NEVER connect mains electricity (240 V / 110 V) to the calculator,
  to this board, or to any sensor wiring. NEVER use mains-connected
  equipment near water.

  *** THE A0 VOLTAGE LIMIT IS NOT THE ONE YOU EXPECT ***
  The bare ESP8266 chip reads 0 to 1.0 V ONLY. Feeding it 3.3 V
  will destroy it. The Wemos D1 mini adds an onboard 220k/100k
  divider, so the A0 PIN ON THIS BOARD accepts 0 to about 3.2 V.
  That protection is on the board, not in the chip - if you move
  this code to a bare module or a different ESP8266 board, check
  before you connect anything.

  Modern Casio calculators are 3 - 3.3 V logic, which matches the
  ESP8266 directly. Keep the resistors and diodes below in place.
 ===================================================================
  HARDWARE (Wemos D1 mini) - Wire colours users choice
  -----------------------------------------------------------------
  THERE ARE TWO WIRINGS. Which one applies is set by
  CASIO_TRANSPORT_UART below. Wire to match the setting, or the
  board will simply sit there saying nothing.

  ---- CASIO_TRANSPORT_UART 0 : SoftwareSerial (the 2026 build) ----
  - GPIO 14 (D5) -> to Casio RX   [RING of 2.5mm TRS, BLUE wire]
                    via 1N4148 diode, bar towards Wemos
  - GPIO 12 (D6) <- from Casio TX [TIP of 2.5mm TRS, YELLOW wire]
  - GPIO 12 (D6) -> 10k IN SERIES to the pin  *** REQUIRED ***
                    plus 4.7k pull-up and a 1N5711 - see below
  - GPIO 13 (D7) -> DS18B20 data, with 4.7k pull-up to 3.3 V

  ---- CASIO_TRANSPORT_UART 1 : hardware UART0 (for WiFi work) ----
  - GPIO 15 (D8) -> to Casio RX   [RING, BLUE]  via 1N4148, BAR to board
  - GPIO 13 (D7) <- from Casio TX [TIP, YELLOW]
  - GPIO 13 (D7) -> 10k IN SERIES to the pin  *** REQUIRED ***
                    plus 4.7k pull-up and a 1N5711 - see below
  - GPIO 14 (D5) -> DS18B20 data, with 4.7k pull-up to 3.3 V

     *** D8 AND THE BOOT PROBLEM - READ THIS ***
     GPIO 15 (D8) is a boot-mode strapping pin. It MUST be LOW at
     the instant the board powers up or the ESP8266 will not start.
     The D1 mini has a pull-down fitted for exactly this reason, and
     the 1N4148 to the calculator keeps the calculator from overriding
     it. The diode does this BETTER than the 1k it replaced: its 0.6 V
     forward drop subtracts from the calculator's pull-up before any
     current can reach D8, so D8 sits LOWER at boot than it did with
     the resistor. Tested 2025 - boots normally.

     If the board stops booting once the cable is attached: unplug
     the calculator, power up, then plug in. If that cures it, add
     a 4.7k pull-down from D8 to GND and it will behave.

     *** AND THE UNEXPECTED BENEFIT OF THE SAME ARRANGEMENT ***
     THE CALCULATOR CAN STAY PLUGGED IN WHILE YOU REPROGRAM.

     UART0 lives on the USB pins for the whole upload. swap() does
     not move it to D8/D7 until setup() runs, which is after the
     board has booted, so the calculator never sees a byte of the
     upload. 

     This matters more than it sounds. A 2.5mm jack does not enjoy
     being cycled, and a class set gets unplugged hundreds of times
     over a term. It also shortens the edit-flash-test loop, which
     is most of what debugging actually is.

  *** THE RECEIVE NETWORK - SETTLED 2026 ***
  Use the universal interface. Four parts, one circuit, and it serves
  both calculator generations on a 3.3 V board and on a 5 V board:

      Casio TIP --+-- 4.7k --- 3.3 V     (the BOARD's own supply)
                  |
                  +-- 10k ---+--- RX pin
                             |
                             +--|<|--- 3.3 V   1N5711, BAND to 3.3 V

  WHY: an FX-9750G Plus holds its transmit line at 4.75 V, measured.
  That is above a 3.3 V pin's supply, so the 10k limits the current to
  about 110 uA and the Schottky - 0.3 V forward - conducts before the
  chip's own protection diode at 0.6 V and takes the current first.
  An FX-9750GIII holds its line at 2.75 V, so on a 3.3 V board the
  Schottky never conducts and the pull-up does the work. On a 5 V
  board the 4.7k goes to +5 V instead and lifts the GIII's 2.75 V to
  about 3.9 V, clearing a 5 V input's 3.0 V threshold.

  THE 10k IS IN SERIES ONLY - nothing goes from the pin to GND.
  An earlier build here used a 10k/20k divider to ground. It works on
  a 3.3 V board and both calculators logged with it, but it is NOT a
  general answer: a GIII through that divider gives about 1.8 V, below
  a 5 V input's threshold entirely. The circuit above replaces it and
  is what the manual, the website and the diagram now specify.
  Use a SMALL-SIGNAL Schottky - BAT85 or BAT43 will do. NOT a 1N5817:
  power Schottkys leak enough to lift the LOW level.

  BOTH WIRINGS:
  - GND          -> Casio GND     [SLEEVE of 2.5mm TRS, BLACK wire]

     NOTE ON THE JACK: a 2.5mm TRS plug has three parts - TIP,
     RING and SLEEVE, in that order from the end. The tip and the
     sleeve are different contacts. Ground is the SLEEVE, the one
     nearest the cable.

  SENSORS (unchanged by the transport setting):
  - A0           -> analogue sensor  (channel 2) - THE ONLY ADC PIN
  - GPIO 5  (D1) -> I2C SCL  \  BME280 module
  - GPIO 4  (D2) -> I2C SDA  /  (channel 3)
  - GPIO 2  (D4) -> built-in LED, and the debug output when the
                    hardware UART is carrying the calculator

  An SB-62 cross-over cable has male 2.5mm TRS plugs at both ends.

 ===================================================================
  FOUR THINGS TO BE AWARE OF ON THE ESP8266 AND NOT ON THE ESP32
 ===================================================================

  1. THE WATCHDOG WILL RESET THE BOARD IF YOU LET IT.
     This program deliberately sits and waits - for up to five
     minutes - while the calculator is inside Receive(). That is
     Fenton's unbounded host wait and it is the whole point. But the
     ESP8266 runs a watchdog timer that reboots the chip after a
     few seconds of code that never gives the system a turn.

     delay() hands control back and feeds the watchdog. So every
     waiting loop in this file uses delay(), never an empty
     while() spinning on millis(). Replace a delay() with a bare
     loop and the board will reset in the middle of a run, and it
     will look like a cable fault.

  2. THE LED IS BACK TO FRONT.
     On the D1 mini the built-in LED is wired to 3.3 V, so LOW
     turns it ON and HIGH turns it OFF. LED_ON / LED_OFF below
     hide this, but it surprises everyone once.

  3. THE ADC IS 10-BIT AND IT WANDERS WHEN THE RADIO IS ON.
     analogRead() returns 0 to 1023, not 0 to 4095. And when WiFi
     is active the readings fluctuate slightly - the radio draws
     current in bursts, the supply rail dips, and the converter
     measures against that rail. Readings can move by a few counts
     with nothing connected changing at all.

     MEASURED 2026, on a Wemos D1 mini in access point
     mode with a phone connected: A0 held at mid-scale by a resistor
     divider moved by ONE COUNT out of 1024 - about 3 mV on a 3.2 V
     span. Single readings, no averaging. That is far less than the
     paragraph above would lead you to expect.

     Treat it as an upper bound rather than a figure. One count is
     the quantisation limit, so a value sitting near a code boundary
     will flip by one with no real change at all, and a single
     analogRead() cannot tell the two apart. Averaging sixteen
     readings would settle it, and would also be the right thing to
     do in any build that cares about the third digit.

     The practical conclusion: on this board the radio's effect on
     the ADC is at or below one count, which is negligible for
     classroom measurement. Average anyway if the reading matters.

  4. THERE IS NO analogReadMilliVolts() HERE.
     That advice in the ESP32 version does not apply. The ESP8266
     has no factory calibration data to draw on, so you convert
     counts to volts yourself, using the divider ratio.

 ===================================================================
  SIGNED VALUES ARE NATIVE - NO OFFSET
 ===================================================================
  Byte 14 of the value packet is the sign/info byte:
      bit 0        the magnitude is >= 1
      bits 6 and 4 the value is NEGATIVE
  So 0x01 is a positive number and 0x51 a negative one.

  A DS18B20 below freezing travels as a negative number and arrives
  as one. 
 ===================================================================
  *** WHAT TO CHANGE - this code is a starting point ***
 ===================================================================
  1. scale_to_physical_1/2/3()  <-- THE MAIN EDIT POINT.
     Where a raw reading becomes a real quantity in real units.

  2. The I2C device. BME280 is used here because it is common and
     cheap. Any I2C sensor works - the channel just needs a number.

  3. The units. This build uses TENTHS, so 23.4 travels as 234.

  4. compute_application_flag(), to raise your own alarm codes.

  LIBRARIES NEEDED (Arduino Library Manager):
     EspSoftwareSerial   (by Dirk Kaar / Peter Lerup)
     OneWire             (by Paul Stoffregen)
     DallasTemperature   (by Miles Burton)
     Adafruit BME280     (+ Adafruit Unified Sensor)

 ===================================================================
  EVERY SWITCH IN THIS FILE, IN ONE PLACE
 ===================================================================
  Each one has its full reasoning written beside it further down.
  This is the index, so you can see what exists without reading
  two thousand lines to find out.

   SETTING                 DEFAULT   WHAT IT DOES
   ---------------------   -------   ---------------------------------
   CASIO_TRANSPORT_UART    1         1 = hardware UART on D8/D7
                                     0 = SoftwareSerial on D5/D6
                                     MUST match how you have wired it.

   WIFI_ENABLED            1         1 = the board makes its own
                                     network, so phones can watch.
                                     0 = radio off entirely.

   WEB_SERVER_ENABLED      1         Only read when WIFI_ENABLED is 1.
                                     Kept separate so the radio can be
                                     tested without the server.

   AP_SSID / AP_PASSWORD             Network name and password. Give
                                     each unit its own name.

   DEBUG_TRACE             1         1 = protocol bytes to Serial1
                                     (D4). Costs the status LED.

   DIAG_HEAP_ON_CH3        0         1 = channel 3 reports the board's
                                     free memory instead of a sensor.
                                     A way to watch the logger itself.

   DS18B20_RESOLUTION      9         9 = 0.5 C steps, 94 ms
                                     12 = 0.0625 C steps, 750 ms
                                     9 matches the sensor's accuracy.

   HISTORY_SIZE            999       Readings kept for the web page.
                                     999 is what the calculator holds.

   CH1_NAME .. CH3_UNIT              What the web page calls each
   CH1_CSV .. CH3_CSV                channel, and the CSV column
                                     headings. Edit to match what you
                                     actually connected.

 =================================================================
  KNOWN LIMITS
 =================================================================
    About four phones is the most an ESP8266 access point holds -
      an estimate, not a measurement; two is what has been shown
    A long run needs fresh calculator batteries - see below
    300 seconds is the longest sampling INTERVAL this code allows
    but sampling sessions have been successful over hours.

 ===================================================================
  THE FAILURE THAT WILL NOT LOOK LIKE ITSELF
 ===================================================================
  LOW CALCULATOR BATTERIES PRESENT AS A COMMUNICATIONS FAULT.

  The calculator's transmit line is a HIGH-IMPEDANCE source - about
  5k - though its voltage is fine: a steady 2.75 V measured during a
  host-wait window, 15 Aug 2026. (An earlier note here said "about
  1.4 V at idle". WITHDRAWN - that was a multimeter averaging a line
  switching between 0 V and 2.75 V.) The 4.7k pull-up is required
  because the port goes HIGH IMPEDANCE when not in use, and without
  it the line floats to 0 V - a permanent break condition. As
  the cells fall, that drive weakens further, and THE LINK FAILS
  BEFORE ANYTHING ELSE ABOUT THE CALCULATOR LOOKS WRONG. The display
  is still bright, the keys still work, and the logger stops with a
  Com ERROR.

  If a session stops early and nothing in the panel explains it,
  change the batteries before you change anything else.

  THERE IS NO EARLY WARNING. The failure is ABRUPT. The error counters
  do not climb first - timeouts, bad checksums, short packets and
  stray bytes all stay at zero right up to the moment the calculator
  shows Com ERROR. Nothing in software sees it coming, which is why
  this note exists instead of a counter.

 ===================================================================
  THE DESIGN PRINCIPLE THIS FILE IS BUILT ON
 ===================================================================
  A FAULT MUST NEVER RESEMBLE A RESULT.

  Where a failure cannot be prevented, it must be made visible. A
  logger that stops is a nuisance. A logger that carries on and
  quietly returns plausible numbers is worse than no logger at all,
  because the experiment continues and the error is found - if ever -
  long after the apparatus has been put away.

  THE POINT OF THE WHOLE PROJECT is that a student should be able to
  watch the reaction, the cooling curve or the weather and FORGET
  what is doing the recording. An instrument can only be forgotten if
  it is trustworthy in a particular way: not that it usually works,
  but that WHEN IT DOES NOT WORK, THIS IS APPARENT. Trust that has to
  be checked is not trust.

  Several things in this file exist only for that reason, and will
  look like overcaution until you know why:

    clamp_to_range()      a field over 9999 corrupts its neighbour
                          silently. The clamp is always recorded in
                          the status flag - never applied quietly.

    I fixed at 1          any other value is visible on the display
                          and proves the frame is wrong.

    the clamp record       a clamped channel is shown on the web
                          arrive together or not at all. Sent as
                          three separate transactions, a failure in
                          the third misaligns the lists permanently
                          and nothing anywhere records it.

    the counters          every fault found in 2026 was found
                          by arithmetic on them, not by reading code.

  IF YOU EXTEND THIS FILE: prefer a loud failure to a quiet one, even
  at the cost of stopping. Where a failure cannot be loud, make it
  countable. And do not add defensive code against
  faults nobody has observed: it is untested, it hides the mechanism
  a learner is meant to read, and it does not make anything visible.

 ===================================================================
*/

#include <ESP8266WiFi.h>      // radio control, and the access point
#include <ESP8266WebServer.h> // the status page
#include <SoftwareSerial.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h>
#include <Adafruit_BME280.h>

// ===================================================================
// DIAGNOSTIC TRACE
//
// Set to 1 to print every protocol byte to the USB serial monitor at
// 115200 baud. Costs nothing when 0 - the compiler removes it.
//
// This is the fastest way to find out where a Com ERROR happens. A
// healthy Receive(N) looks like this:
//
//     ATT 15  -> sent 13
//     HDR 3A 52 45 51 00 56 4D FF FF FF FF 4E   cmd=R vname=N
//     REQ 3A 52 45 51 00 56 4D FF FF FF FF 4E   cmd=R vname=N
//     ACK1 ok    ACK2 ok
//     sent plain value 3
//     END sent
//
// If ACK1 shows FF you are reading leftover request bytes - the
// drain in loop() is missing or has been removed.
// ===================================================================
// ===================================================================
// WHICH SERIAL PORT CARRIES THE CALCULATOR
//
//   0 = SoftwareSerial on D5/D6. The configuration validated in
//       2026 with the radio OFF. Leaves the USB serial
//       monitor free for debugging. Use this build for teaching and
//       for any run where the record matters.
//
//   1 = hardware UART0, moved to D8/D7 by Serial.swap(). Use this
//       when WiFi is switched on.
//
// WHY THE CHOICE EXISTS. SoftwareSerial is bit-banged: the chip
// counts out each bit in software, and one bit at 9600 baud lasts
// 104 microseconds. The WiFi stack disables interrupts for periods
// of comparable length whenever the radio has something to do. The
// result is a link that runs perfectly on the bench and then loses
// a byte when an ARP request arrives - an intermittent fault that
// looks exactly like a bad cable.
//
// The hardware UART shifts out and samples in bits in silicon and
// does not care what the processor is doing. That is the whole
// reason for the rewire.
//
// The 0% COM error record in the technical summary was earned with
// this set to 0 and the radio off. It does not transfer to setting
// 1 until setting 1 has been tested for as long.
//
// COST OF SETTING 1: the USB serial monitor no longer carries
// debug output, because UART0 has moved to the calculator. Debug
// goes out on Serial1 (GPIO 2 / D4) instead, which is TX-only and
// is also the LED pin - so with DEBUG_TRACE on, the LED becomes a
// traffic light rather than a status light. Read it with a USB-TTL
// adaptor on D4, or leave DEBUG_TRACE off and use the web page.
// ===================================================================
#define CASIO_TRANSPORT_UART 1

#define DEBUG_TRACE 0

// ===================================================================
// DIAGNOSTICS ON CHANNEL 3  -  using the logger to watch itself
//
// Set to 1 to replace the channel 3 sensor with the board's FREE HEAP,
// in units of 100 bytes. 40000 bytes free arrives as 400.
//
// WHY THIS EXISTS. Once the hardware UART carries the calculator there
// is no USB serial monitor to print to - Serial IS the calculator. So
// the calculator becomes the instrument, and the thing being measured
// is the microcontroller. A quantity that ought to be constant is
// logged, graphed and inspected using exactly the machinery built for
// temperature and light.
//
// WHY UNITS OF 100. The value packet carries four digits, and with
// offset the usable signed range is -4095 to +4095. Free heap is tens
// of thousands of bytes and will not fit. Divided by 100 it fits with
// room to spare, and 100-byte resolution is ample: a leak worth
// finding loses far more than that over half an hour.
//
// WHAT TO LOOK FOR.
//   A FLAT LINE is a healthy board. The absolute value does not
//   matter much - roughly 380 to 450 with the radio off, lower once
//   WiFi is running. What matters is that it does not fall.
//
//   A LINE THAT SLOPES DOWNWARD is a leak. Note the gradient: counts
//   per sample x 100 gives bytes per sample.
//
//   A SAWTOOTH is normal for WiFi work - buffers taken and given
//   back. Judge it by whether the peaks stay level, not by the dips.
//
// Set back to 0 when you are done. Channel 3 then returns to being a
// sensor, and nothing else in the program has changed.
// ===================================================================
#define DIAG_HEAP_ON_CH3 0

// ===================================================================
// THE RADIO
//
//   0 = OFF. The configuration every reliability figure in the
//       technical summary was earned with. Leave it here unless you
//       are deliberately testing the radio.
//
//   1 = ON, as an ACCESS POINT. The board makes its own network. No
//       server yet - Stage 3 of the plan turns the radio on and does
//       nothing with it, deliberately, so that any lost byte can be
//       blamed on the radio itself rather than on HTTP traffic.
//
// WHY AN ACCESS POINT AND NOT A CLIENT. A group of students will
// often have one calculator between three or four of them. The
// others follow the same readings on a phone they already carry.
// An access point needs no school network, no credentials typed by
// a student, no IT department, and works on a field trip with no
// coverage at all.
//
// FOUR CLIENTS, NOT THIRTY. An ESP8266 access point holds about
// four connections and each one costs heap. This suits a working
// group. It does NOT suit a class pointing thirty phones at one
// logger - that needs several loggers, which is what a class set is
// for. Say so in any worksheet, or it will be discovered during a
// lesson.
//
// PHONES FIGHT NETWORKS WITH NO INTERNET. Android and iOS both
// notice that this network cannot reach the internet, and may warn,
// or quietly fall back to mobile data and take the browser with
// them. Students will report that it does not work when in fact the
// phone left. Tell them to choose "stay connected".
//
// WHAT TO EXPECT WHEN YOU TURN THIS ON. Free heap drops sharply -
// the radio and its buffers are not small. Watch it on channel 3
// with DIAG_HEAP_ON_CH3 set to 1, and record the new figure: it
// decides how large HISTORY_SIZE may safely be.
// ===================================================================
#define WIFI_ENABLED 1

#define AP_SSID        "CASIO-ESP8266-1"   // one per unit: -1 to -8
#define AP_PASSWORD    ""    // 8 characters minimum, or
                                       // "" for an open network
#define AP_CHANNEL     1
#define AP_MAX_CLIENTS 4

// ===================================================================
// THE STATUS PAGE
//
//   0 = radio only, no server. Stage 3 of the plan.
//   1 = serve a status page at http://192.168.4.1/
//
// Kept separate from WIFI_ENABLED on purpose. If a byte ever goes
// missing, turning the server off without turning the radio off
// tells you which of the two was responsible. Two switches, two
// questions - one switch would only answer half of one.
//
// WHAT THIS PAGE IS FOR. A group of students will often have one
// calculator between three or four of them. This lets the others
// see the same readings on a phone they already carry. It is
// READ-ONLY by design: it observes and cannot configure, calibrate
// or control, so no phone can spoil another student's experiment.
//
// WHEN IT IS SERVED. Only while the board is waiting - idle between
// sessions, or during the long part of a sampling interval. It is
// NEVER served between starting the sensor conversions and
// transmitting. That window belongs to the calculator, and the
// calculator does not wait its turn.
//
// PLAIN HTML, NO JAVASCRIPT. It has to load over a slow local link
// onto whatever phone a student happens to own, possibly an old one,
// with no internet available to fetch anything else from.
// ===================================================================
#define WEB_SERVER_ENABLED 1

// Turns a #define into a string for the page: STR(999) -> "999".
#define STR_(x) #x
#define STR(x)  STR_(x)

// ---- what the three channels are called on the page ---------------
// EDIT THESE to match what you have actually connected. The page
// shows values in tenths of the unit, as the packets carry them.
#define CH1_NAME  "Temperature"
#define CH1_UNIT  "&deg;C"
#define CH2_NAME  "Analogue A0"
#define CH2_UNIT  "counts"
#if DIAG_HEAP_ON_CH3
  #define CH3_NAME  "Free heap"
  #define CH3_UNIT  "&times;100 bytes"
#else
  #define CH3_NAME  "Humidity"
  #define CH3_UNIT  "%"
#endif

// Column headings for the CSV file. Separate from the page names
// because a spreadsheet wants plain ASCII, no spaces and no HTML
// entities - and because the unit belongs IN the column name where a
// file has no room for a second row to explain itself.
#define CH1_CSV  "temperature_C"
#define CH2_CSV  "analogue_counts"
#if DIAG_HEAP_ON_CH3
  #define CH3_CSV  "free_heap_x100B"
#else
  #define CH3_CSV  "humidity_pct"
#endif


// Where debug text goes. With the hardware UART carrying the
// calculator, Serial is no longer available for it - see the note at
// CASIO_TRANSPORT_UART.
#if CASIO_TRANSPORT_UART
  #define TRACEPORT Serial1
#else
  #define TRACEPORT Serial
#endif

#if DEBUG_TRACE
  #define TRACE(x)      TRACEPORT.print(x)
  #define TRACELN(x)    TRACEPORT.println(x)
  #define TRACEHEX(x)   do { if ((x) < 16) TRACEPORT.print('0'); \
                             TRACEPORT.print((x), HEX); TRACEPORT.print(' '); } while (0)
#else
  #define TRACE(x)
  #define TRACELN(x)
  #define TRACEHEX(x)
#endif

// ===================================================================
// PIN ASSIGNMENTS  (Wemos D1 mini)
// ===================================================================
#if CASIO_TRANSPORT_UART
  // UART0 after Serial.swap(): TX is fixed at GPIO 15, RX at GPIO 13.
  // Those pins are not a choice - the silicon decides them. The
  // DS18B20 moves to D5, which SoftwareSerial has just given up.
  #define CASIO_TX_PIN   15   // D8 -> Casio RX (blue, ring) via 1N4148, BAR to board
  #define CASIO_RX_PIN   13   // D7 <- Casio TX (yellow, tip) via divider
  #define ONEWIRE_PIN    14   // D5 -- DS18B20 data          (channel 1)
#else
  #define CASIO_TX_PIN   14   // D5 -> Casio RX (blue, ring) via 1N4148, BAR to board
  #define CASIO_RX_PIN   12   // D6 <- Casio TX (yellow, tip) via divider
  #define ONEWIRE_PIN    13   // D7 -- DS18B20 data          (channel 1)
#endif

#define ANALOG_PIN     A0     // ------ THE ONLY ADC PIN --- (channel 2)
#define I2C_SCL_PIN     5     // D1 \_ BME280                (channel 3)
#define I2C_SDA_PIN     4     // D2 /
#define LED_PIN         2     // D4 -- built-in LED, INVERTED

// The D1 mini's LED is wired to 3.3 V: LOW lights it.
#define LED_ON   LOW
#define LED_OFF  HIGH

// GPIO 2 is both the LED and the Serial1 transmit pin. They cannot
// both use it, so the LED stands down when debug output needs it.
#if DEBUG_TRACE && CASIO_TRANSPORT_UART
  #define LED_IN_USE 0
#else
  #define LED_IN_USE 1
#endif

inline void led(uint8_t state) {
#if LED_IN_USE
  digitalWrite(LED_PIN, state);
#else
  (void)state;
#endif
}

// ===================================================================
// PROTOCOL CONSTANTS  (identical to the PICAXE and ESP32 versions)
// ===================================================================
const uint8_t CASIO_ATTENTION = 0x15;   // calculator: "are you there?"
const uint8_t ESP_PRESENT     = 0x13;   // our reply:  "yes, ready"
const uint8_t CASIO_ACK       = 0x06;   // acknowledge
// Every request and description packet is exactly this long. The
// value packet is 16. Named so the read loop and the checksum call
// cannot disagree about it.
const uint8_t REQUEST_PACKET_LEN = 50;
const uint8_t VALUE_PACKET_LEN   = 16;

// ===================================================================
// DO WE REJECT A PACKET WHOSE CHECKSUM FAILS?
//
//   0 = COUNT the mismatch and act on the packet anyway.
//   1 = reject the packet and flush.        <-- NOW THE DEFAULT
//
// RX, BECAUSE IT GOVERNS BOTH PACKETS THE BOARD RECEIVES: the
// 50-byte REQUEST that asks for a reading, and the 16-byte VALUE
// packet that carries the logging interval from the calculator.
//
// ---- WHAT IT CATCHES THAT NOTHING ELSE DOES --------------------
// The length test catches a truncated packet. The preamble test
// catches most desynchronisation. What survives both is a 50-byte
// packet that begins with 0x3A and is corrupt in the MIDDLE - a bit
// flipped by noise, or a read that slipped by one and happened to
// land on a 0x3A.
//
// Byte 11 of the request is the VARIABLE NAME. Corrupt that, and
// this board answers a request for sensor A with sensor B's reading.
// The calculator stores a perfectly valid wrong number in the right
// list, and NOTHING MARKS IT. That is the failure this project
// exists to prevent: a fault that resembles a result.
//
// With strict mode on, the board stays silent instead, the
// calculator times out, and the session ends with a Com ERROR. One
// reading lost, loudly. That is the trade this project has already
// made everywhere else.
//
// ---- IT IS EXPECTED NEVER TO FIRE ------------------------------
// Bench experience suggests the only noise this link sees comes from
// LOW CALCULATOR BATTERIES, and that failure already announces
// itself as a Com ERROR without any help from a checksum. Strict
// mode is therefore insurance against a case not yet observed, and
// it costs nothing while that remains true.
//
// The evidence is from ONE calculator model. The rule agrees with
// Grindheim's for the CFX-9950G, but that is his measurement, not
// one made here.
//
// IF IT EVER FIRES:
//   * Bad checksum climbing while logging is otherwise fine
//       -> the rule is wrong for that calculator. Set this to 0,
//          keep logging, and record the model.
//   * A session dying at its FIRST transaction on a calculator that
//     used to work
//       -> check the batteries FIRST. If they are good, suspect the
//          model and set this to 0.
//
// ---- A NOTE FOR TEACHING ---------------------------------------
// This switch is worth showing to students, because it makes an
// abstract idea concrete. A checksum answers a real question - how
// does a machine know a message arrived intact? - and this constant
// asks the question that follows it, which is the more interesting
// one: WHEN A MACHINE CAN TELL A MESSAGE IS DAMAGED, WHAT SHOULD IT
// DO ABOUT IT?
//
// Set it to 0 and the logger keeps going and may record something
// untrue. Set it to 1 and the logger stops and says so. Both are
// defensible; engineers choose between them constantly; and the
// choice is a design VALUE rather than a technical fact. The Bad
// checksum counter on the status page makes the whole thing
// observable while it happens.
// ===================================================================
#define RX_CHECKSUM_STRICT 1


const uint8_t CASIO_PREAMBLE  = 0x3A;   // ':' starts every packet

const uint8_t CMD_RECEIVE     = 'R';    // ":REQ..." it wants a value
const uint8_t CMD_SEND        = 'V';    // ":VAL..." it is giving us one

const uint8_t VNAME_N         = 'N';    // how many sensors?
const uint8_t VNAME_A         = 'A';    // sensor 1
const uint8_t VNAME_B         = 'B';    // sensor 2
const uint8_t VNAME_C         = 'C';    // sensor 3
const uint8_t VNAME_T         = 'T';    // sampling interval, via Send(T)

// Byte 14 of the value packet, the sign/info byte.
//   bit 0        the magnitude is >= 1
//   bits 6 and 4 the value is NEGATIVE
const uint8_t SIGN_POSITIVE   = 0x01;
const uint8_t SIGN_NEGATIVE   = 0x51;

// ===================================================================
// THE LARGEST VALUE THIS BUILD WILL TRANSMIT
// ===================================================================
// Four significant digits is what the value packet carries in the
// form used here. A reading beyond this is CLAMPED and the clamp is
// recorded, so the web page can report it - see clamp_to_range().
//
// Note the ESP8266's converter is only 10-bit (0-1023), so a raw
// analogue reading uses a fraction of this. The room is there for
// scaled values, not for raw counts.
constexpr int16_t NSN_MAX_VALUE = 9999;

const uint8_t  SENSOR_COUNT = 3;             // fixed at 3, as on the PICAXE
// ===================================================================
// THE TIMING BUDGET  -  why a slow sensor must not be read late
//
// A reading is due on a particular second, and the packet must leave
// on that second. Anything the board must do BEFORE it can transmit
// has to happen in the time set aside beforehand, or the packet
// leaves late - every single time, on every sample.
//
// The DS18B20 is the problem child. A 12-bit conversion takes 750 ms,
// which is longer than any other operation here by two orders of
// magnitude. Read it in the ordinary blocking way at 600 ms before
// the due moment and it returns 150 ms AFTER it, and the wait-until-
// due loop below falls straight through. The logger still works. It
// is simply late, always, and by an amount nobody thinks to measure.
//
// THE FIX IS TO SEPARATE STARTING A CONVERSION FROM COLLECTING IT.
//
//   due - 900 ms   request a conversion, and do NOT wait for it
//   due - 50 ms    collect the result, which is by now ready, and
//                  read the fast sensors
//   due            transmit
//
// The slow conversion now happens during time that was being spent
// waiting anyway. Collection is instant, so the readings are taken
// 50 ms before the timestamp rather than 600 ms before it - the data
// is more accurate as well as more punctual.
//
// A GENERAL RULE WORTH TAKING AWAY. Prefer fast sensors where timing
// reliability matters. The DS18B20 is kept here because it is the
// clearest teaching example of a signed, sub-zero, real-world
// quantity, and because meeting a slow sensor is itself instructive.
// A build that cared only about timing would use the analogue and
// I2C channels and leave 1-Wire alone.
//
// IF YOU ADD A SLOWER SENSOR, RAISE SENSOR_READ_OFFSET_MS TO SUIT,
// and keep it comfortably below the shortest interval you intend to
// use. At 900 ms a 2-second interval still has 1.1 seconds spare.
// ===================================================================
// ===================================================================
// DS18B20 RESOLUTION  -  and why the default is the LOWEST one
//
//   bits   step        conversion   suits
//   ----   ---------   ----------   ---------------------------------
//     9    0.5 C        94 ms       school investigations  <-- DEFAULT
//    10    0.25 C      188 ms       small temperature changes
//    11    0.125 C     375 ms       rarely justified here
//    12    0.0625 C    750 ms       the library default; see below
//
// THE SENSOR IS ACCURATE TO PLUS OR MINUS 0.5 DEGREES. That is the
// datasheet figure for -10 to +85 C, widening to +/-2 C at the
// extremes of its range. At 12 bits the part therefore reports steps
// of 0.0625 C while being uncertain by 0.5 - a figure eight times
// finer than anything it knows. Printing those digits does not make
// the measurement better; it invites a learner to believe it is.
//
// School thermometers read to about 1 degree and are trusted to
// perhaps 0.5. Matching that is honest, and it happens to be EIGHT
// TIMES FASTER, which buys back timing headroom for the whole
// program - the read-ahead below falls from 900 ms to about 250.
//
// WHEN TO USE MORE BITS. Resolution finer than accuracy is still
// useful for measuring CHANGE. A cooling curve that falls two
// degrees shows as four steps at 9 bits and sixteen at 11, and the
// second is the better graph even though neither knows the absolute
// temperature any better. Absolute accuracy and repeatability are
// different properties: a sensor can resolve a change it cannot
// locate. If an investigation tracks a small change rather than a
// large one, raise this - and say in the write-up that the extra
// digits describe the change, not the temperature.
// ===================================================================
#define DS18B20_RESOLUTION 9

#if   DS18B20_RESOLUTION == 9
  const uint32_t DS18B20_CONVERSION_MS = 94;
#elif DS18B20_RESOLUTION == 10
  const uint32_t DS18B20_CONVERSION_MS = 188;
#elif DS18B20_RESOLUTION == 11
  const uint32_t DS18B20_CONVERSION_MS = 375;
#elif DS18B20_RESOLUTION == 12
  const uint32_t DS18B20_CONVERSION_MS = 750;
#else
  #error "DS18B20_RESOLUTION must be 9, 10, 11 or 12"
#endif

// Start conversions this far ahead of the due moment. The margin
// covers the 1-Wire bus transaction and any jitter; it is deliberate
// slack, not a guess at the conversion time.
const uint32_t SENSOR_READ_OFFSET_MS = DS18B20_CONVERSION_MS + 150;
const uint32_t FINAL_READ_MS         = 50;   // collect them here

// ===================================================================
// GLOBAL STATE
// ===================================================================
// The rest of the file says CasioSerial and never needs to know which
// port that is. With the hardware UART it IS Serial, after swap().
#if CASIO_TRANSPORT_UART
  #define CasioSerial Serial
#else
  SoftwareSerial CasioSerial;
#endif

// ===================================================================
// THE TURNAROUND DELAY  -  a 2007 lesson, relearned twice
// ===================================================================
// The calculator does not switch from transmitting to listening
// instantly. Reply into that turnaround and the first bits land while
// its port is still changing direction: it mishears the byte and
// answers 0x22.
//
// Found on a PICAXE in 2007. 
// An ESP32 answers an FX-9750G Plus fast enough to trip it.
// MEASURED 2026. Without this delay the trace reads:
//     ATT 15  -> sent 13
//     SHORT PACKET 1  bytes: 22
// The calculator rejected our $13 and never sent the request packet.
// A GIII tolerates the fast reply; a G Plus does not.
//
// Applied INSIDE every send so a new code path cannot forget it.
// ===================================================================
#define TURNAROUND_MS 5

static inline void turnaround() {
#if TURNAROUND_MS > 0
  delay(TURNAROUND_MS);
#endif
}

// ===================================================================
// SENDING A PACKET
// ===================================================================
// One bulk write. The UART decides the spacing, and that is correct
// on this platform - SEE THE STOP-BIT NOTE IN setup().
//
// The FX-9750G Plus needs about one bit period of idle between bytes.
// 8N2 supplies it in the framing, so nothing has to be added here.
// A board that CANNOT send two stop bits has to add the gap by hand:
// the PICAXE builds emit one byte at a time, and the Arduino Uno
// build uses an explicit ~104 us delay, because neither an 08M2
// EUSART nor SoftwareSerial can be told to send 8N2.
//
// AN EARLIER VERSION OF THIS FILE PACED THE BYTES HERE. It was
// unnecessary - the stop bit was already doing the work - and it cost
// ~134 ms of BUSY-WAIT per transaction, during which loop() never ran
// and the web server was unreachable. Removed 2026.
//
// *** IF YOU CHANGE THE FRAMING TO 8N1, THIS ROUTINE MUST PACE. ***
// ===================================================================
static void casio_send_packet(const uint8_t *buf, size_t len) {
  turnaround();
  CasioSerial.write(buf, len);
}

OneWire           oneWire(ONEWIRE_PIN);
DallasTemperature ds18b20(&oneWire);
Adafruit_BME280   bme;

bool bmePresent = false;

uint16_t timeInterval = 10;    // seconds between readings (2 - 300)
uint32_t nextSendTime = 0;     // when the next reading is DUE, in seconds
bool     firstReading = true;

int16_t  physicalValue[3];     // the real quantity, signed
uint8_t  saturatedMask = 0;    // bits 0,1,2 set if that channel clamped
uint32_t lastExchangeMs = 0;   // when the calculator last spoke to us

// ===================================================================
// SESSION TIMING  -  separating the board's life from the run
//
// Uptime counts from power-up, and a good deal of that is a person
// walking back to the calculator and starting the program. Comparing
// samples against UPTIME therefore always shows a shortfall, and the
// shortfall means nothing.
//
// Comparing them against the SESSION is the useful measurement. If a
// session has been running 146 seconds at a 2-second interval there
// should be 73 readings. If there are 68, the board lost ten seconds
// somewhere and that is worth knowing - it is the difference between
// "it stopped" and "it was late five times and then stopped".
//
// A gap of more than three intervals is taken to end a session, so
// starting a second run does not compare against the first.
// ===================================================================
uint32_t sessionStartS  = 0;   // when the first Receive(A) arrived
uint32_t lastSampleS    = 0;   // when the most recent one arrived
uint32_t sessionSamples = 0;   // readings sent during THIS session

// ===================================================================
// THE SAMPLE HISTORY
//
// Every reading is kept here as well as being sent to the calculator,
// so the board can answer for itself later - over a web page, or as a
// CSV file - without the calculator being involved.
//
// WHAT THIS IS FOR. A group of students will often have one Casio
// between three or four of them. The history lets the others follow
// the same measurements on a phone they already carry. It also means
// somebody who joins ten minutes late still sees the whole run.
//
// IT STORES physicalValue - real units, signed, as the sensors read
// them. Nothing is conditioned for transport on the way to a screen,
// because nothing needs to be.
//
// SIZING. A Sample is 4 + 6 + 1 bytes, which the compiler pads to
// 12. 999 samples costs 11,988 bytes, against about 46,600 free with
// the radio up and a phone connected - measured, not assumed.
//
// WHY 999 AND NOT A ROUND NUMBER. It is what the CALCULATOR holds. A
// Casio list takes 999 elements, and a session uses four lists, so
// 999 samples is exactly the calculator's own limit. The board and
// the calculator fill up at the same moment, which is one fact for a
// student to learn instead of two.
//
// It is a STATIC array deliberately. malloc() on an ESP8266 that has
// been running for hours returns fragmented memory or nothing at all,
// and a logger that dies at 3 a.m. has lost the only run that
// mattered. A static array either fits when you compile it or fails
// to build. The build error is the better failure.
// ===================================================================
#define HISTORY_SIZE 999

struct Sample {
  uint32_t t;        // seconds since boot
  int16_t  v[3];     // PHYSICAL values, signed, offset already removed
  uint8_t  sat;      // saturation mask: bit 0,1,2 set if that channel clamped
};

Sample   history[HISTORY_SIZE];
uint16_t historyCount = 0;     // filled slots, stops at HISTORY_SIZE
uint16_t historyHead  = 0;     // where the next sample goes

// ===================================================================
// LINK COUNTERS
//
// Cheap to keep, and the only evidence that survives. A timeout forty
// minutes ago leaves no other trace, and "it stopped working" is not
// a fault report anybody can act on.
// ===================================================================
// ===================================================================
// THE SHORTEST AND LONGEST INTERVAL THIS BOARD WILL ACCEPT
// ===================================================================
// The calculator sends the interval it wants. These clamp it, because
// a device that acts on an unclamped number arriving over a wire is a
// device that can be stopped by a typing error.
//
// MIN_INTERVAL_S WAS 2. It is now 1.
//
// WHERE A SECOND ACTUALLY GOES. One Receive() transaction moves about
// 165 bytes - the 50-byte request in, the description and 16-byte
// value out, the end packet, and the acknowledgements. At 9600 baud
// with 10 to 11 bits per byte that is ABOUT 180 ms, and it is 180 ms
// on every platform in this project: a 240 MHz ESP32 and a 16 MHz
// PICAXE shift bits at the same rate. Add a 9-bit DS18B20 conversion
// at 94 ms and this board is committed for under 300 ms of the 1000.
//
// SO THE DEVICE IS NOT WHAT DECIDES THIS. The calculator is - its
// BASIC program has to take each value in, write it to a list and
// refresh the display between one request and the next, and that has
// never been timed.
//
// *** AT 1 SECOND, DS18B20_RESOLUTION MUST STAY AT 9. ***
// At 12 bits the conversion alone is 750 ms. Add 180 ms of serial and
// a 1-second interval has about 70 ms left. It would fail, and it
// would look like a protocol fault rather than a sensor setting.
//
// HOW IT FAILS IF THE CALCULATOR CANNOT KEEP UP - and this is why the
// experiment is safe. The board sets the pace by holding the
// calculator inside the host-wait window. A calculator that is slow
// simply reaches its next Receive() late, and this board, already
// waiting, answers at once. NOTHING CORRUPTS. The samples just sit
// further apart than asked, nextSendTime falls behind, and the
// resynchronise branch in wait_for_interval() fires - which is now
// COUNTED, so the board reports the fact instead of leaving it to be
// inferred from a stopwatch.
//
// Watch "Resynchronised" on the status page. At 2 seconds it should
// stay at 0. If it climbs at 1 second, the calculator is the limit
// and the display-refresh optimisation is the thing to try next.
// ===================================================================
#define MIN_INTERVAL_S 1
#define MAX_INTERVAL_S 300

struct LinkStats {
  uint32_t valuePackets;   // sensor value packets sent
  uint32_t endPackets;     // END packets sent
  uint32_t timeouts;       // waitForByte() gave up - total
  uint32_t timeoutAck1;    // ...waiting for the first ACK
  uint32_t timeoutAck2;    // ...waiting for the second ACK
  uint32_t timeoutFinal;   // ...waiting for the optional closing ACK
  uint32_t badAttention;   // a byte arrived that was not 0x15
  uint32_t requestPackets; // complete, checksummed request packets
  uint32_t shortPackets;   // fewer than 50 bytes arrived
  uint32_t badPreamble;    // byte 0 was not ':'
  uint32_t badChecksum;    // the calculator's own checksum failed
  uint32_t csvDownloads;   // CSV files served to a GET
  uint32_t csvOtherReq;    // HEAD and anything else asking for it
  uint32_t extraBytes;     // bytes beyond the 38 - a different fault
  uint32_t flushed;        // bytes cleared after an unexpected event
  uint32_t abortedAck1;    // wrong byte where the first ACK belongs
  uint32_t abortedAck2;    // wrong byte where the second ACK belongs
  uint32_t resyncs;        // schedule fell a whole interval behind -
                           // the calculator could not keep up
  uint32_t turnLast;       // the calculator's own loop time, ms
  uint32_t turnMin;        // the fastest seen - the best estimate
  uint32_t turnMax;        // the slowest; may include human delay
  uint32_t turnCount;      // how many turnarounds have been timed
  uint8_t  lastBadByte;    // WHAT arrived instead - the whole question
  uint8_t  lastBadStage;   // 1 = ACK1, 2 = ACK2, 3 = attention
  uint8_t  lastSat;        // most recent saturation mask
  uint8_t  saturatedNow;   // current saturation mask
};
// Empty braces zero EVERY member, however many there are - see the
// note in the ESP32 build. A list of literal zeros has to be
// recounted each time a counter is added.
LinkStats stats = {};

// ===================================================================
// HOW LONG DOES THE CALCULATOR ITSELF TAKE?
// ===================================================================
// Added 2026. This measures something no part of this
// project could measure before: THE CASIO'S OWN LOOP TIME. Not the
// link, not this board - the calculator finishing Receive(, decoding
// the value it received, writing a list element,
// refreshing the display, and coming back to ask again.
//
// HOW. The board notes the millisecond it finished a transaction and
// the millisecond the next attention byte arrives. The gap between
// them is the calculator, because nothing else runs in between: the
// interval wait happens BEFORE the transmission, inside the unbounded host wait
// window, and the web server is barred from this gap (see loop()).
//
// WHY IT IS WORTH A COUNTER
//   * It BOUNDS THE MINIMUM INTERVAL ON EVERY PLATFORM. No device can
//     log faster than the calculator comes back, whatever it is.
//   * A teacher choosing an interval has never had an answer to
//     "is my calculator program fast enough?". Now the board says so.
//   * A Casio program that is edited - more display work, more lists -
//     gets slower, and this makes that visible instead of mysterious.
//
// A PICAXE ESTIMATE PUT THIS AT ABOUT 300 ms, obtained by subtraction
// from clock loss and a hand-counted stopwatch, so +/- perhaps 50.
// THIS IS THE DIRECT MEASUREMENT that replaces it, to the millisecond.
//
// READ THE MINIMUM, NOT THE MAXIMUM. The fastest turnaround seen is
// the cleanest estimate of the calculator's own work. The maximum
// includes any moment the user was pressing keys, reading the screen,
// or had walked away, so it is not a property of the program.
// ===================================================================
static uint32_t txDoneMs = 0;     // when the last transaction finished

static void note_turnaround() {
  if (txDoneMs == 0) return;                 // no previous transaction
  uint32_t gap = millis() - txDoneMs;
  if (gap > 60000UL) return;                 // idle between sessions,
                                             // not a measurement
  stats.turnLast = gap;
  stats.turnCount++;
  if (gap > stats.turnMax) stats.turnMax = gap;
  if (stats.turnMin == 0 || gap < stats.turnMin) stats.turnMin = gap;
}


#if WIFI_ENABLED && WEB_SERVER_ENABLED
ESP8266WebServer server(80);
#endif

// Forward declarations, so this file also compiles as ordinary C++
// (which is how the packet tests are run on a PC).
int16_t  scale_to_physical_1();
int16_t  scale_to_physical_2();
int16_t  scale_to_physical_3();
int16_t  clamp_to_range(int32_t value, uint8_t channel);
uint8_t  calculate_checksum(const uint8_t *packet);
uint8_t  checksum_over(const uint8_t *packet, uint8_t len);
void     read_all_sensors();
void     start_slow_conversions();
void     note_sample_request();
uint16_t flush_line(uint32_t idleMs, uint32_t capMs);
void     handle_status_page();
void     handle_csv_download();
void     history_add();
const Sample& history_at(uint16_t i);
void     send_nsn_value(int16_t signedValue);
void     send_description(uint8_t vname);
void     send_end_packet();
uint16_t denormalise_value(uint8_t intDigit, uint8_t dec1, uint8_t dec2,
                           uint8_t signInfo, uint8_t exponent);

// ===================================================================
// SECONDS SINCE POWER-UP
//
// millis() wraps at about 49.7 days. Far more headroom than the
// PICAXE's 18 hours, but not infinite - worth knowing rather than
// discovering.
// ===================================================================
uint32_t nowSeconds() {
  return millis() / 1000UL;
}

// ===================================================================
// STAGE 2 of 4:  RAW READING  ->  REAL QUANTITY        *** EDIT ME ***
//
// Return the value in TENTHS of your unit, so one decimal place
// survives without needing any decimals in the packet:
//     23.4 degrees  ->  return 234
//     -3.7 degrees  ->  return -37
//
// The value may be NEGATIVE. That is the point of this build.
// ===================================================================

// ---- Channel 1: DS18B20 on the 1-Wire bus (D7) ---------------------
// The sub-zero demonstrator. Chosen because it returns a SIGNED
// temperature natively - the negative number is real, not
// manufactured by subtracting a midpoint from a positive reading.
//
// Range -55 to +125 C, so -550 to +1250 in tenths, comfortably
// inside +/-4095.
//
// TO TEST THE NEGATIVE PATH: ice with salt stirred through it
// reaches about -10 C. That reads -100 and travels as 4900,
// genuinely below the zero point, so the sign is actually exercised
// rather than assumed. A domestic freezer at -18 C gives 3200.
int16_t scale_to_physical_1() {
  // NO requestTemperatures() HERE. The conversion was started 900 ms
  // ago by start_slow_conversions(), so the answer is already
  // waiting and this call returns at once. See THE TIMING BUDGET.
  float celsius = ds18b20.getTempCByIndex(0);

  if (celsius <= -100.0f) {     // library returns -127 if absent
    return 0;
  }
  return (int16_t)lroundf(celsius * 10.0f);
}

// ---- Channel 2: the single analogue pin (A0) -----------------------
// analogRead() returns 0 to 1023 on this chip - TEN bits, not twelve.
// On the D1 mini that span represents about 0 to 3.2 V, because of
// the onboard divider.
//
// Placeholder: raw count, centred so it can swing either way.
// Replace with your own conversion.
//
// If WiFi is ever switched on, average several readings here - the
// radio's current draw disturbs the supply the converter measures
// against.
int16_t scale_to_physical_2() {
  int raw = analogRead(ANALOG_PIN);      // 0 - 1023
  return (int16_t)(raw - 512);           // -512 to +511
}

// ---- Channel 3: BME280 on the I2C bus (D1/D2) ---------------------
// Relative humidity in tenths of a percent: 0 to 1000. Fits easily,
// and needs no convention of its own - a good place to start.
//
// THE FIX IS THE SAME TRICK AS THE OFFSET ITSELF: move the origin.
// Subtract 900 hPa before scaling, and add 900 back in the Casio
// code. The block below is ready to use - swap the two returns.
//
//    encode (here):    tenths = (hPa - 900) x 10
//    decode (Casio):   hPa    = value / 10 + 900
//
// WHY 900 AND NOT 1000? Both fit. 900 is the better choice because
// it puts ORDINARY WEATHER IN POSITIVE NUMBERS:
//
//      870 hPa   record low (Typhoon Tip, 1979)     ->   -300
//      900 hPa   the new origin                     ->      0
//      980 hPa   deep low                           ->    800
//     1013.2     standard sea-level pressure        ->   1132
//     1050 hPa   strong high                        ->   1500
//     1084 hPa   record high (Siberia, 1968)        ->   1840
//
// Subtracting 1000 would work too, but normal weather would then
// straddle zero and half your readings would carry a minus sign
// while nothing unusual was happening. With 900 a negative number
// means something: pressure below 900 hPa is a severe storm. The
// sign becomes information rather than noise.
//
// AND NOTICE WHAT HAPPENS THEN. That -300 still reaches the
// calculator perfectly well, because the value packet carries the
// sign itself - nothing has to be added or taken away to make a
// negative number sendable.
//
// The whole project is in that idea. A field is a fixed size, and
// the skill is choosing what to put in it.
int16_t scale_to_physical_3() {
#if DIAG_HEAP_ON_CH3
  // Free heap in units of 100 bytes. See DIAG_HEAP_ON_CH3 above.
  // Divide BEFORE the cast - the raw figure overflows an int16_t.
  return (int16_t)(ESP.getFreeHeap() / 100);
#endif

  if (!bmePresent) {
    return 0;
  }

  // ---- humidity, tenths of a percent (0 to 1000) ----
  float humidity = bme.readHumidity();
  return (int16_t)lroundf(humidity * 10.0f);

  // ---- OR pressure, tenths of a hPa above 900 ----
  // Comment out the two lines above and uncomment these two.
  // REMEMBER TO ADD 900 BACK IN YOUR CASIO PROGRAM.
  // float hPa = bme.readPressure() / 100.0f;      // Pa -> hPa
  // return (int16_t)lroundf((hPa - 900.0f) * 10.0f);
}

// ===================================================================
// STAGE 3 of 4:  KEEP THE VALUE INSIDE THE FIELD
//
// If a reading is out of range we do NOT silently pretend it was at
// the limit. We clamp it AND we record that we did, so the calculator 
// is told. A silent clamp is a lie the size of the error.
// ===================================================================
int16_t clamp_to_range(int32_t value, uint8_t channel) {
  if (value >  NSN_MAX_VALUE) {
    saturatedMask |= (1 << channel);
    return  NSN_MAX_VALUE;
  }
  if (value < -NSN_MAX_VALUE) {
    saturatedMask |= (1 << channel);
    return -NSN_MAX_VALUE;
  }
  return (int16_t)value;
}

// ===================================================================
// READ ALL THREE SENSORS
//
// All three are read together, one after another with no waiting in
// between, so the three readings in a packet belong to the same
// instant. That is what makes comparing them meaningful.
//
// Note the three come from three different buses and take different
// amounts of time - the DS18B20 conversion is the slow one. They are
// still far closer together than any human could manage with three
// instruments, which is the point.
// ===================================================================
// ===================================================================
// THE STATUS PAGE
// ===================================================================
#if WIFI_ENABLED && WEB_SERVER_ENABLED

// A signed value in tenths, as "23.4" or "-3.7". Written out by hand
// because the obvious v/10.0 would drag in floating point printing
// for no reason, and because -0.7 must not print as "0.-7".
static void fmt_tenths(char *buf, size_t n, int16_t v) {
  int16_t whole = v / 10;
  int16_t frac  = v % 10;
  if (frac < 0) frac = -frac;
  if (v < 0 && whole == 0) snprintf(buf, n, "-0.%d", frac);
  else                     snprintf(buf, n, "%d.%d", whole, frac);
}

static void fmt_hms(char *buf, size_t n, uint32_t secs) {
  snprintf(buf, n, "%lu:%02lu:%02lu",
           (unsigned long)(secs / 3600UL),
           (unsigned long)((secs / 60UL) % 60UL),
           (unsigned long)(secs % 60UL));
}

// ===================================================================
// *** NEVER PASS AN EMPTY STRING TO server.sendContent(). ***
//
// This response is sent with chunked transfer encoding, and in that
// encoding a ZERO-LENGTH CHUNK IS THE END-OF-RESPONSE MARKER. An
// empty sendContent() does not send nothing - it ends the page.
//
// The damage is worse than a truncated page. Every sendContent()
// after the terminator writes to a connection the browser has now
// closed, and each of those can block until the server's send
// timeout, which is FIVE SECONDS by default. Fifteen of them in a
// row leaves the board unresponsive long enough to miss the
// calculator's attention byte, and logging stops with a COM ERROR.
//
// Observed 2026: three rows passed "" as their unit, the
// page ended silently at the first of them, and the link died
// roughly two minutes later. One empty string, two symptoms, and
// neither of them pointing at the cause.
//
// send() guards every string, so the mistake cannot be made again
// by editing the rows.
// ===================================================================
static void send(const char *text) {
  if (text && text[0]) server.sendContent(text);
}

// ===================================================================
// CLOSE THE CONNECTION WHEN THE RESPONSE IS FINISHED
//
// An ESP8266 has only a handful of TCP control blocks. A phone left
// joined to the network opens connections on its own - both Android
// and iOS probe periodically to decide whether a network reaches the
// internet, and this one never will. If those connections are not
// closed they accumulate, and when they run out the board stops
// answering anything, including the calculator.
//
// This was learned by removing the call and watching what happened:
//
//   stop() present, page truncated, phone refreshing hard
//        -> 241 samples, no error
//   stop() removed, page correct, NOBODY touching the phone
//        -> COM ERROR after 153 samples
//
// It had been removed on the theory that it was truncating the
// response. It was not - the truncation was an empty chunk, found
// separately. The lesson is narrow and worth keeping: when a change
// is made to test a theory and the theory turns out to be wrong, put
// the change back.
// ===================================================================
static void close_client() {
  server.client().stop();
}

static void row(const char *label, const char *value, const char *unit) {
  server.sendContent(F("<tr><th>"));
  send(label);
  server.sendContent(F("</th><td>"));
  send(value);
  server.sendContent(F(" </td><td class=\"u\">"));
  send(unit);
  server.sendContent(F("</td></tr>"));
}

static void rowNum(const char *label, uint32_t v, const char *unit) {
  char b[16];
  snprintf(b, sizeof b, "%lu", (unsigned long)v);
  row(label, b, unit);
}

// The page is sent in CHUNKS rather than built in a String first.
// A String would need the whole page contiguous in a heap that also
// has to hold the sample history and the WiFi buffers, and heap
// fragmentation on an ESP8266 is how long-running sketches die.
// Chunking also happens to be exactly what the CSV download will
// need, so the habit is worth forming here.
void handle_status_page() {
  char buf[24];

  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, F("text/html"), F(""));

  server.sendContent(F(
    "<!DOCTYPE html><html><head><meta charset=\"utf-8\">"
    "<meta name=\"viewport\" content=\"width=device-width,initial-scale=1\">"
    "<title>" AP_SSID "</title>"));

  // Auto-refresh only where it cannot compete with sampling. See the
  // refresh policy in ESP8266_WiFi_Plan.md.
  if (timeInterval > 60) {
    snprintf(buf, sizeof buf, "%u", (unsigned)timeInterval);
    server.sendContent(F("<meta http-equiv=\"refresh\" content=\""));
    server.sendContent(buf);
    server.sendContent(F("\">"));
  }

  server.sendContent(F(
    "<style>"
    "body{font-family:system-ui,sans-serif;margin:0;padding:14px;"
    "background:#f4f6f8;color:#182028;font-size:17px}"
    "h1{font-size:20px;margin:0 0 2px}"
    "p.s{margin:0 0 16px;color:#5a6672;font-size:14px}"
    "h2{font-size:15px;margin:20px 0 6px;color:#5a6672;"
    "text-transform:uppercase;letter-spacing:.06em}"
    "table{width:100%;border-collapse:collapse;background:#fff;"
    "border-radius:8px;overflow:hidden}"
    "th,td{padding:9px 11px;border-bottom:1px solid #e6eaee;text-align:left}"
    "th{font-weight:500;color:#5a6672;width:52%}"
    "td{font-variant-numeric:tabular-nums;font-weight:600}"
    "td.u{font-weight:400;color:#8b96a2;width:22%;font-size:14px}"
    "tr:last-child th,tr:last-child td{border-bottom:none}"
    "a{display:inline-block;margin-top:18px;padding:11px 20px;"
    "background:#2b6cb0;color:#fff;text-decoration:none;border-radius:8px}"
    "a.alt{background:#fff;color:#2b6cb0;border:1px solid #cfd8e0}"
    "footer{margin-top:22px;color:#8b96a2;font-size:13px;line-height:1.5}"
    "</style></head><body>"));

  server.sendContent(F("<h1>" AP_SSID "</h1>"
                       "<p class=\"s\">Casio NSN datalogger &mdash; "
                       "read only</p>"));

  // ---- the readings ------------------------------------------------
  server.sendContent(F("<h2>Latest reading</h2><table>"));
  if (historyCount == 0) {
    server.sendContent(F("<tr><td colspan=\"3\">No readings yet. The "
                         "calculator has not started a session.</td></tr>"));
  } else {
    const Sample &s = history_at(historyCount - 1);
    fmt_tenths(buf, sizeof buf, s.v[0]); row(CH1_NAME, buf, CH1_UNIT);
    fmt_tenths(buf, sizeof buf, s.v[1]); row(CH2_NAME, buf, CH2_UNIT);
    fmt_tenths(buf, sizeof buf, s.v[2]); row(CH3_NAME, buf, CH3_UNIT);
    snprintf(buf, sizeof buf, "%s%s%s",
             (s.sat & 1) ? "1 " : "", (s.sat & 2) ? "2 " : "", (s.sat & 4) ? "3 " : "");
    row("Channels clamped", s.sat ? buf : "none",
        s.sat ? "OUT OF RANGE" : "all in range");
    fmt_hms(buf, sizeof buf, s.t);
    row("Taken at", buf, "since start");
  }
  server.sendContent(F("</table>"));

  // ---- the session -------------------------------------------------
  server.sendContent(F("<h2>Session</h2><table>"));
  rowNum("Samples stored", historyCount, "of " STR(HISTORY_SIZE));
  rowNum("Sampling interval", timeInterval, "seconds");

  if (sessionStartS == 0) {
    row("Session", "not started", "no Receive(A) yet");
  } else {
    fmt_hms(buf, sizeof buf, sessionStartS);
    row("Session began", buf, "after power-up");

    uint32_t runS = lastSampleS - sessionStartS;
    fmt_hms(buf, sizeof buf, runS);
    row("Session length", buf, "h:mm:ss");

    rowNum("Readings this session", sessionSamples, "");

    // The stall detector. Expected counts the reading at t = 0.
    //
    // ALWAYS SHOWN, EVEN WHEN IT IS ZERO. It used to appear only
    // when readings had been lost, which meant a healthy logger
    // displayed nothing at all - indistinguishable from a panel that
    // had no such check. Somebody looking for reassurance found an
    // absence and could not tell what it meant.
    //
    // That is the design principle of this project inverted. A fault
    // must never resemble a result; equally, a HEALTHY state must be
    // visible, or the panel cannot be trusted to report either. A
    // zero that is displayed says "checked, nothing lost". A row
    // that is missing says nothing whatsoever.
    uint32_t expected = (timeInterval ? runS / timeInterval : 0) + 1;
    rowNum("Readings expected", expected, "");
    uint32_t missing = (expected > sessionSamples)
                     ? (expected - sessionSamples) : 0;
    rowNum("Missing", missing,
           missing ? "readings the board did not send" : "none lost");
  }

  fmt_hms(buf, sizeof buf, nowSeconds());
  row("Logger uptime", buf, "since power-up");
  server.sendContent(F("</table>"));

  // ---- the link ----------------------------------------------------
  server.sendContent(F("<h2>Link to calculator</h2><table>"));
  rowNum("Readings sent", stats.valuePackets, "packets");
  rowNum("Sessions ended", stats.endPackets, "END packets");
  rowNum("Timeouts", stats.timeouts, "total");
  rowNum("  at first ACK", stats.timeoutAck1, "");
  rowNum("  at second ACK", stats.timeoutAck2, "");
  rowNum("  at closing ACK", stats.timeoutFinal, "optional, harmless");
  rowNum("Unexpected bytes", stats.badAttention, "");
  rowNum("Request packets", stats.requestPackets, "complete + checksummed");
  rowNum("Short packets", stats.shortPackets, "should stay 0");
  rowNum("Bad preamble", stats.badPreamble, "should stay 0");
  rowNum("Bad checksum", stats.badChecksum, "should stay 0");
  rowNum("CSV files served", stats.csvDownloads, "");
  rowNum("CSV other requests", stats.csvOtherReq, "HEAD etc");
  rowNum("Extra bytes", stats.extraBytes, "should stay 0");
  rowNum("Bytes flushed", stats.flushed, "after surprises");
  rowNum("Aborted at first ACK", stats.abortedAck1, "");
  rowNum("Aborted at second ACK", stats.abortedAck2, "");
  rowNum("Resynchronised", stats.resyncs, "calculator too slow for the interval");
  rowNum("Calculator turnaround, ms", stats.turnLast, "its own loop time - last");
  rowNum("  fastest seen", stats.turnMin, "the best estimate of it");
  rowNum("  slowest seen", stats.turnMax, "may include time you spent at the keys");
  rowNum("  timed", stats.turnCount, "how many were measured");
  {
    // The byte that should have been 06. Everything about this fault
    // turns on what it actually was: 0x22 means the calculator chose
    // to abort, anything else means the byte stream has slipped and
    // we are reading the middle of something.
    char b[24];
    const char *where = stats.lastBadStage == 1 ? "at first ACK"
                      : stats.lastBadStage == 2 ? "at second ACK"
                      : stats.lastBadStage == 3 ? "as attention byte" : "";
    if (stats.lastBadStage) {
      snprintf(b, sizeof b, "0x%02X", (unsigned)stats.lastBadByte);
      row("Last wrong byte", b, where);
    } else {
      row("Last wrong byte", "none", "");
    }
  }
  server.sendContent(F("</table>"));

  // ---- the board ---------------------------------------------------
  // WHY THE RESET REASON IS ON THIS PAGE.
  // A board that restarts mid-session looks exactly like a board that
  // hung: the calculator shows a COM error either way, and every
  // counter here begins again from zero, which makes a five-minute
  // run look like a two-minute one. 

  server.sendContent(F("<h2>Board</h2><table>"));
  row("Last restart", ESP.getResetReason().c_str(), "");
  rowNum("Free memory", ESP.getFreeHeap(), "bytes");
  rowNum("Devices connected", WiFi.softAPgetStationNum(), "of " STR(AP_MAX_CLIENTS));
  server.sendContent(F("</table>"));

  server.sendContent(F("<a href=\"/\">Refresh</a> "
                       "<a href=\"/data.csv\" class=\"alt\">Download CSV</a>"));

  server.sendContent(F(
    "<footer>This page shows what the calculator is recording. It "
    "cannot change anything.<br>Readings update when the calculator "
    "asks for them, not when this page is refreshed.<br><br>"
    "The CSV holds every reading of this session, in the same units "
    "shown above. Downloading during a fast run may delay one "
    "reading; nothing is lost."));
  if (timeInterval <= 5) {
    server.sendContent(F("<br><br><b>Automatic refresh is off at this "
                         "sampling interval.</b> Readings are arriving "
                         "every few seconds and the logger needs that "
                         "time for the calculator. Refresh by hand."));
  }
  server.sendContent(F("</footer></body></html>"));
  server.sendContent(F(""));      // terminates the chunked response
  close_client();
}

// ===================================================================
// THE CSV DOWNLOAD
//
// Serves the whole sample history as a file the student can open in
// a spreadsheet. This is the part that outlives the lesson: the
// calculator's lists stay on the calculator, but a CSV goes home.
//
// *** WHERE IT IS SERVED MATTERS MORE THAN HOW LONG IT TAKES. ***
//
// 999 rows is roughly 25 KB - a few hundred milliseconds over a local
// link, sometimes longer if the phone stalls or a packet is resent.
//
// Served during the long wait inside wait_for_interval(), a slow
// download delays one reading and nothing more. Served in the gap
// BETWEEN transactions, it causes a COM ERROR, because that gap
// contains the attention-byte handshake and no host-wait window has ever
// been demonstrated there. The guard is in loop(); read the note
// there before changing where serve_web() is called.
//
// Inside the host-wait window, a late reading is harmless:
//
//   * The calculator is sitting inside Receive() and will WAIT. That
//     is the unbounded host wait. 300 seconds is the specified
//     limit - three hours has been achieved and no ceiling has been
//     established, so 300 is a chosen margin rather than the edge. A
//     packet arriving late is not a packet that fails.
//
//   * nextSendTime advances ARITHMETICALLY, so one late sample does
//     not push the ones after it. The schedule reasserts itself.
//
//   * If the delay were gross, the resynchronise branch in
//     wait_for_interval() catches it.
//
// So the cost of a download during a fast session is at most one
// late sample in the recorded series, and no lost data. That is a
// fair price for taking the results home, and it is stated here so
// that nobody has to rediscover it by watching a graph.
//
// Downloading between sessions costs nothing at all.
// ===================================================================
void handle_csv_download() {
  // A browser may ask for this file more than once - a HEAD to learn
  // the type before a GET to fetch it, or a download manager
  // repeating the request the page has already made. Android does
  // one of these: the counter advanced by two per download while
  // every file arrived complete and well formed.
  //
  // Count the ones that carry data, and count the others separately
  // rather than hiding them. A counter that says 2 when a student
  // downloaded 1 file teaches them to distrust the panel, and the
  // panel is the only instrument this board has.
  if (server.method() == HTTP_GET) stats.csvDownloads++;
  else                             stats.csvOtherReq++;

  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.sendHeader(F("Content-Disposition"),
                    F("attachment; filename=\"" AP_SSID ".csv\""));
  server.send(200, F("text/csv"), F(""));

  server.sendContent(F("time_s," CH1_CSV "," CH2_CSV "," CH3_CSV ",flag\r\n"));

  // Rows are batched into a fixed buffer and flushed. One
  // sendContent() per row would work and would spend more time in
  // protocol overhead than in data.
  char buf[512];
  char v1[12], v2[12], v3[12];
  size_t used = 0;
  buf[0] = '\0';

  for (uint16_t i = 0; i < historyCount; i++) {
    if (used > sizeof(buf) - 80) {      // flush BEFORE it can overflow
      server.sendContent(buf);
      used = 0;
      buf[0] = '\0';
      yield();                          // feeds the watchdog
    }

    const Sample &s = history_at(i);
    fmt_tenths(v1, sizeof v1, s.v[0]);
    fmt_tenths(v2, sizeof v2, s.v[1]);
    fmt_tenths(v3, sizeof v3, s.v[2]);

    int n = snprintf(buf + used, sizeof(buf) - used,
                     "%lu,%s,%s,%s,%02u\r\n",
                     (unsigned long)s.t, v1, v2, v3, (unsigned)s.sat);
    if (n <= 0) break;
    used += (size_t)n;
  }

  if (used) server.sendContent(buf);

  server.sendContent(F(""));      // terminates the chunked response
  close_client();
}

// Called from the waiting loops only. Cheap when nobody is connected.
inline void serve_web() {
  server.handleClient();
}
#else
inline void serve_web() {}
#endif

// ===================================================================
// START THE SLOW SENSORS CONVERTING
//
// Called SENSOR_READ_OFFSET_MS before a reading is due. It returns
// immediately - the DS18B20 gets on with its conversion while
// the board waits for the due second, which it was going to do
// anyway. Add any other slow sensor's "begin measurement" call here.
// ===================================================================
void start_slow_conversions() {
  ds18b20.requestTemperatures();      // non-blocking; see setup()
}

void read_all_sensors() {
  saturatedMask = 0;

  physicalValue[0] = clamp_to_range(scale_to_physical_1(), 0);
  physicalValue[1] = clamp_to_range(scale_to_physical_2(), 1);
  physicalValue[2] = clamp_to_range(scale_to_physical_3(), 2);

  stats.lastSat      = saturatedMask;
  stats.saturatedNow = saturatedMask;
  history_add();
}

// ===================================================================
// NOTE THAT THE CALCULATOR HAS ASKED FOR A READING
// ===================================================================
void note_sample_request() {
  uint32_t now = nowSeconds();
  bool newSession = (sessionStartS == 0) ||
                    (lastSampleS && (now - lastSampleS) > (uint32_t)timeInterval * 3);
  if (newSession) {
    sessionStartS  = now;
    sessionSamples = 0;
  }
  lastSampleS = now;
}

// ===================================================================
// KEEPING THE READING
//
// The oldest sample is overwritten once the buffer is full. A logger
// that stops recording when it fills is worse than one that keeps the
// most recent window, because the recent data is what somebody is
// looking at.
//
// Called from ONE place - the end of read_all_sensors(), immediately
// above. A sample recorded anywhere else would drift out of step with
// what was actually sent.
// ===================================================================
void history_add() {
  Sample &s = history[historyHead];
  s.t    = nowSeconds();
  s.v[0] = physicalValue[0];
  s.v[1] = physicalValue[1];
  s.v[2] = physicalValue[2];
  s.sat  = saturatedMask;

  historyHead = (historyHead + 1) % HISTORY_SIZE;
  if (historyCount < HISTORY_SIZE) historyCount++;
}

// Oldest first, for reading the buffer out in order.
const Sample& history_at(uint16_t i) {
  uint16_t start = (historyCount == HISTORY_SIZE) ? historyHead : 0;
  return history[(start + i) % HISTORY_SIZE];
}

// ===================================================================
// THE SAME CHECKSUM RULE, FOR A PACKET OF ANY LENGTH
//
// Add every byte AFTER the ':' preamble and BEFORE the checksum
// itself. The checksum is whatever brings that total back to zero in
// eight bits. One rule for the 16-byte value packet, the 50-byte
// description and END packets, and the 50-byte request packets the
// CALCULATOR sends us - which is what lets us check those instead of
// trusting them.
// ===================================================================
uint8_t checksum_over(const uint8_t *packet, uint8_t len) {
  uint8_t sum = 0;
  for (uint8_t i = 1; i < len - 1; i++) sum += packet[i];
  return (uint8_t)(0u - sum);          // two's complement of the sum
}

uint8_t calculate_checksum(const uint8_t *packet) {
  // The 16-byte value packet. Kept as its own name because that is
  // what every other line of this file calls, and because the
  // traditional form below is the one the published descriptions
  // use. Both give the same byte; checksum_over() is simply the
  // general case.
  uint8_t sum = 0;
  for (uint8_t i = 0; i < 15; i++) {
    sum += packet[i];
  }
  uint8_t t = sum - 0x3A;
  t = 255 - t;
  return t + 1;
}

// ===================================================================
// SEND ONE VALUE  -  normalised scientific notation, SIGNED
//
// An ordinary number in scientific notation, the same form students
// meet in Year 9-10 maths: I.DDDD x 10^E. The sign lives in byte 14.
// ===================================================================
void send_nsn_value(int16_t signedValue) {
  bool     negative = (signedValue < 0);
  uint16_t value    = (uint16_t)(negative ? -(int32_t)signedValue : signedValue);

  if (value == 0) {
    // Zero has its own packet, with a fixed checksum of 0xFE.
    const uint8_t zeroPacket[16] = {
      CASIO_PREAMBLE, 0x00, 0x01, 0x00, 0x01,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0xFE
    };
    casio_send_packet(zeroPacket, 16);
    return;
  }

  uint8_t intDigit = 0, dec1 = 0, dec2 = 0, exponent = 0;

  if (value < 10) {
    intDigit = value;
    exponent = 0;
  } else if (value < 100) {
    intDigit = value / 10;
    dec1     = (value % 10) << 4;
    exponent = 1;
  } else if (value < 1000) {
    intDigit = value / 100;
    uint16_t r = value % 100;
    dec1     = ((r / 10) << 4) | (r % 10);
    exponent = 2;
  } else {
    intDigit = value / 1000;
    uint16_t r = value % 1000;
    dec1     = ((r / 100) << 4) | ((r / 10) % 10);
    dec2     = (r % 10) << 4;
    exponent = 3;
  }

  uint8_t packet[16];
  packet[0] = CASIO_PREAMBLE;
  packet[1] = 0x00;
  packet[2] = 0x01;
  packet[3] = 0x00;
  packet[4] = 0x01;
  packet[5] = intDigit;
  packet[6] = dec1;
  packet[7] = dec2;
  for (uint8_t i = 8; i < 13; i++) packet[i] = 0x00;
  packet[13] = negative ? SIGN_NEGATIVE : SIGN_POSITIVE;
  packet[14] = exponent;
  packet[15] = calculate_checksum(packet);

  casio_send_packet(packet, 16);
  stats.valuePackets++;
}

// ===================================================================
// SEND THE DESCRIPTION PACKET  -  50 bytes
//
// Tells the calculator WHICH variable is about to arrive. Every byte
// matters: the checksum is the constant 273 - vname, and that
// constant is only correct because the rest of the packet never
// changes. Alter the padding and the checksum silently becomes wrong.
// ===================================================================
void send_description(uint8_t vname) {
  uint8_t packet[50];

  // ':' V A L 00 V M 00 01 00 01
  const uint8_t head[11] = { CASIO_PREAMBLE, 0x56, 0x41, 0x4C, 0x00,
                             0x56, 0x4D, 0x00, 0x01, 0x00, 0x01 };
  memcpy(packet, head, 11);

  packet[11] = vname;

  for (uint8_t i = 12; i <= 18; i++) packet[i] = 0xFF;   // 7 bytes

  // "VariableR\n" - ten ASCII bytes that must be exactly here
  const uint8_t tag[10] = { 0x56, 0x61, 0x72, 0x69, 0x61,
                            0x62, 0x6C, 0x65, 0x52, 0x0A };
  memcpy(packet + 19, tag, 10);

  for (uint8_t i = 29; i <= 48; i++) packet[i] = 0xFF;   // 20 bytes

  packet[49] = (uint8_t)(273 - vname);

  casio_send_packet(packet, 50);
}

// ===================================================================
// SEND THE END PACKET  -  ":END" + 45 x 0xFF + the constant 0x56
// ===================================================================
void send_end_packet() {
  uint8_t packet[50];
  packet[0] = CASIO_PREAMBLE;
  packet[1] = 'E';
  packet[2] = 'N';
  packet[3] = 'D';
  for (uint8_t i = 4; i <= 48; i++) packet[i] = 0xFF;    // 45 bytes
  packet[49] = 0x56;
  casio_send_packet(packet, 50);
  stats.endPackets++;
}

// ===================================================================
// RESYNCHRONISE  -  clear the line after anything unexpected
//
// Every path that gives up on a transaction must leave the line
// EMPTY. If it does not, the bytes it walked away from are read one
// at a time by the next pass of loop(), each one counted as an
// unexpected byte, and the board spends the next several transactions
// one packet behind. A single glitch becomes a cascade.
//
// That is how 0x56 - byte 5 of a request header - kept appearing
// where an attention byte belongs, even after the drain itself was
// made exact. The drain was correct; the paths that skipped it were
// not.
//
// Silence is the right signal here, unlike the drain, because we do
// NOT know how many bytes are left - the whole point is that
// something unexpected happened.
// ===================================================================
uint16_t flush_line(uint32_t idleMs = 15, uint32_t capMs = 300) {
  uint32_t start  = millis();
  uint32_t lastRx = millis();
  uint16_t n = 0;
  while ((millis() - lastRx) < idleMs && (millis() - start) < capMs) {
    if (CasioSerial.available()) {
      CasioSerial.read();
      n++;
      lastRx = millis();
    } else {
      delay(1);
    }
  }
  stats.flushed += n;
  return n;
}

// ===================================================================
// WAIT FOR A BYTE, OR GIVE UP
//
// delay(1) rather than a bare spin: it feeds the watchdog and lets
// the WiFi stack (even when idle) have its turn. See note 1 in the
// header.
// ===================================================================
bool waitForByte(uint8_t &b, uint32_t timeoutMs) {
  uint32_t start = millis();
  while (millis() - start < timeoutMs) {
    if (CasioSerial.available()) {
      b = CasioSerial.read();
      return true;
    }
    delay(1);
  }
  stats.timeouts++;
  return false;
}

// ===================================================================
// WAIT FOR THE INTERVAL, THEN READ  -  drift-free scheduling
//
// THE IMPORTANT LINE IN THIS FUNCTION IS:
//
//        nextSendTime = nextSendTime + timeInterval;
//
// and NOT
//
//        nextSendTime = now + timeInterval;      <-- WRONG
//
// The second version looks equivalent and is not. It measures each
// interval from the moment the last one finished, so every scrap of
// delay - the packet, the handshake, the sensor read - is added to
// the next interval and kept forever. Over an hour the error grows
// without limit.
//
// The first version works from a schedule decided in advance. If one
// reading is late, the next is still due at its original time, and
// the lateness is absorbed rather than banked. This is why the
// PICAXE version holds +/-1 second over hours.
//
// SENSOR READ TIMING: slow conversions are STARTED one read-offset
// before the due moment and COLLECTED 50 ms before it, so the packet
// leaves on the second and the readings are as fresh as they can be.
// See THE TIMING BUDGET above.
//
// WATCHDOG: every wait below uses delay(). This function can sit
// here for five minutes, and on the ESP8266 that is only safe
// because delay() gives the system its turn. See note 1 in the
// header - this is the single most important difference from the
// ESP32 version.
// ===================================================================
void wait_for_interval() {
  // The very first reading happens immediately, defining t = 0.
  // Nothing has been scheduled yet, so the conversion has to be
  // started and waited out here - the one place where the conversion
  // time is spent openly rather than hidden inside a wait.
  if (firstReading) {
    firstReading = false;
    start_slow_conversions();
    delay(DS18B20_CONVERSION_MS);
    read_all_sensors();
    nextSendTime = nowSeconds() + timeInterval;
    return;
  }

  uint32_t dueMs = (uint32_t)(nextSendTime * 1000UL);

  // --- 1. hold until the slow conversions must be started ---------
  // THE ONLY PLACE THE WEB SERVER IS ALLOWED TO RUN DURING A SESSION.
  // From start_slow_conversions() onwards the board is committed to
  // transmitting on the second, and an HTTP request is not welcome.
  uint32_t convStartMs = dueMs - SENSOR_READ_OFFSET_MS;
  while ((int32_t)(millis() - convStartMs) < 0) {
    serve_web();
    delay(5);                    // feeds the watchdog
  }
  start_slow_conversions();

  // --- 2. hold while they convert, then collect ------------------
  // The DS18B20 is working during this wait. Collecting is instant,
  // so the readings belong to a moment 50 ms before the timestamp
  // rather than 600 ms before it.
  uint32_t readAtMs = dueMs - FINAL_READ_MS;
  while ((int32_t)(millis() - readAtMs) < 0) {
    delay(5);                    // feeds the watchdog
  }

  read_all_sensors();

  // --- 3. hold the last few ms so the packet leaves on the second -
  while ((int32_t)(millis() - dueMs) < 0) {
    delay(1);                    // feeds the watchdog
  }

  // Arithmetic schedule - see the note above.
  nextSendTime = nextSendTime + timeInterval;

  // If we have somehow fallen a whole interval behind, resynchronise
  // rather than sprinting to catch up.
  //
  // This branch used to fire silently,
  // which made "the calculator cannot keep up at this interval" a
  // thing you had to notice with a stopwatch. It is the ONE symptom
  // of an interval set shorter than the calculator's own loop, so it
  // belongs in a counter like every other fault in this file.
  if (nextSendTime < nowSeconds()) {
    nextSendTime = nowSeconds() + timeInterval;
    stats.resyncs++;
  }
}

// ===================================================================
// THE CALCULATOR WANTS A VALUE  -  Receive(A), Receive(N)
// ===================================================================
void handle_receive(uint8_t vname) {
  uint8_t b;

  turnaround();
  CasioSerial.write(CASIO_ACK);

  // A TIMEOUT and a WRONG BYTE are not the same thing, and the PICAXE
  // treats them differently. Match it:
  //   timeout    -> the calculator has stopped listening. Close the
  //                 transaction politely with an END packet.
  //   wrong byte -> the calculator has abandoned the exchange (AC key,
  //                 or it sent 0x22 to abort). Send nothing further.
  if (!waitForByte(b, 2000)) { stats.timeoutAck1++;
                               TRACELN(F("ACK1 TIMEOUT")); send_end_packet(); return; }
  if (b != CASIO_ACK)        { stats.abortedAck1++; stats.lastBadByte = b;
                               stats.lastBadStage = 1;
                               TRACE(F("ACK1 got ")); TRACEHEX(b); TRACELN(F("(expected 06)"));
                               flush_line(); return; }
  TRACE(F("ACK1 ok  "));

  // == HOST-WAIT WINDOW: THE DESCRIPTION WINDOW (GAP 2) ==
  // Pausing here during RECEIVE does not trigger a COM ERROR. This
  // window was the one found in 2007 and used in the 2008 classroom
  // trials to wait for a student to press a key.
  //
  // INTERVAL LOGGING COULD EQUALLY HAVE BEEN BUILT HERE. The value window is
  // used instead, and the reason is DATA FRESHNESS: the value window sits
  // immediately before the value packet, so the reading is taken as
  // late as possible before it is transmitted and timestamped. A
  // reading taken in the description window would be older by the whole length of
  // the description-packet exchange by the time it was sent.
  //
  // The same principle runs through the rest of this file - the
  // sensor read-ahead collects conversions 50 ms before the due
  // moment rather than 600 ms before it, for exactly this reason.
  // Choosing where to wait is choosing how stale the data is.

  send_description(vname);

  if (!waitForByte(b, 2000)) { stats.timeoutAck2++;
                               TRACELN(F("ACK2 TIMEOUT")); send_end_packet(); return; }
  if (b != CASIO_ACK)        { stats.abortedAck2++; stats.lastBadByte = b;
                               stats.lastBadStage = 2;
                               TRACE(F("ACK2 got ")); TRACEHEX(b); TRACELN(F("(expected 06)"));
                               flush_line(); return; }
  TRACELN(F("ACK2 ok"));

  // == FENTON 2025 HOST-WAIT WINDOW: THE VALUE WINDOW (GAP 3) ==
  // THIS is the heart of the discovery. The calculator is sitting
  // inside Receive() waiting for a number, and it will wait - for
  // five minutes if we ask it to - without raising the COM ERROR
  // that every reference says should happen. That patience is what
  // turns a calculator into a datalogger.
  //
  // Five minutes is the SPECIFIED limit, not the observed one.
  // Pauses of up to three hours have succeeded. Shorter maxima were
  // recorded afterwards - around an hour - and were long read as the
  // protocol behaving inconsistently. They were not: the calculator's
  // cells had never been changed after the three-hour run, and a
  // falling supply ends a session with a Com ERROR and no warning.
  // NO CEILING HAS BEEN ESTABLISHED. 300 seconds is chosen with
  // margin, and a successful long pause still does not make the next
  // one safe - fit fresh cells before any long unattended run.

  if (vname == VNAME_N) {
    send_nsn_value((int16_t)SENSOR_COUNT);
  } else if (vname == VNAME_A) {
    // *** THE INTERVAL WAIT BELONGS TO CHANNEL A ONLY. ***
    // A, B and C are three transactions within ONE sample. Waiting in
    // B or C as well would multiply the interval by the number of
    // sensors and stretch the time axis silently. All three sensors
    // are read together inside wait_for_interval(), so B and C return
    // values taken at the same instant as A.
    note_sample_request();
    wait_for_interval();          // <-- the long pause lives in here
    send_nsn_value(physicalValue[0]);
    sessionSamples++;
  } else if (vname == VNAME_B) {
    send_nsn_value(physicalValue[1]);
  } else if (vname == VNAME_C) {
    send_nsn_value(physicalValue[2]);
  } else {
    send_nsn_value(0);            // unknown variable: zero
  }

  // The closing ACK is OPTIONAL - the PICAXE does not require it
  // either. Counted separately so that a total of 1 never again has
  // to be interpreted without knowing which wait it came from.
  if (!waitForByte(b, 1000)) stats.timeoutFinal++;
  send_end_packet();
  TRACELN(F("END sent"));
}

// ===================================================================
// TURN A RECEIVED PACKET BACK INTO A WHOLE NUMBER
//
// The calculator sends numbers as  I.DDDDDDDDDDDDDD x 10^E. This
// undoes that. It follows the Technical Reference Part II section
// I.2, and the validated PICAXE routine decode_casio_value, exactly.
//
// THE CHECK THAT IS EASY TO MISS is the first one. Bit 0 of the
// sign/info byte is clear when the magnitude is BELOW 1, and then the
// whole-number part is zero however big the digits look. Without that
// test a value of 0.5 arrives as 5.000 x 10^-1 and is read as "5".
// ===================================================================
uint16_t denormalise_value(uint8_t intDigit, uint8_t dec1, uint8_t dec2,
                           uint8_t signInfo, uint8_t exponent) {
  if ((signInfo & 0x01) == 0) {
    return 0;                       // magnitude below 1
  }
  if (exponent == 0) return intDigit;
  if (exponent == 1) return intDigit * 10  + (dec1 >> 4);
  if (exponent == 2) return intDigit * 100 + (dec1 >> 4) * 10
                                           + (dec1 & 0x0F);
  if (exponent == 3) return intDigit * 1000 + (dec1 >> 4) * 100
                                            + (dec1 & 0x0F) * 10
                                            + (dec2 >> 4);
  // 10,000 or more - far outside any usable sampling interval.
  // Saturate rather than compute a number the caller would clamp
  // anyway. (The PICAXE computes it and lets its word arithmetic
  // wrap; the outcome after clamping is identical.)
  return 65535;
}

// ===================================================================
// THE CALCULATOR IS SENDING US A NUMBER  -  Send(T)
//
// Sets the sampling interval from the calculator, so a student never
// has to re-flash the board to change it.
// ===================================================================
void handle_incoming() {
  turnaround();
  CasioSerial.write(CASIO_ACK);

  // ===============================================================
  // COUNT BOTH PACKETS. DO NOT GUESS AT EITHER.
  //
  // This function used to take three short cuts, and they are worth
  // naming because they were the last of their kind in the file:
  //
  //   1. It read the 16-byte value packet and IGNORED its checksum.
  //      A corrupted interval that still landed between 2 and 300
  //      would have been accepted in silence, and the logger would
  //      have sampled at the wrong rate for a whole session with
  //      nothing anywhere recording why.
  //
  //   2. On a short read it returned WITHOUT clearing the line, so
  //      whatever had arrived stayed behind to desynchronise the
  //      next transaction.
  //
  //   3. It discarded the calculator's 50-byte END packet with
  //      delay(80) and then read whatever had turned up. Fifty
  //      bytes take 57 ms at 9600 baud with two stop bits, so that
  //      was 23 ms of margin - the same fixed-delay pattern as the
  //      drain that failed in 2026, with a larger cushion.
  //
  // STOP BITS ARE MODEL-DEPENDENT - see the header.
  //   FX-9750GIII:   8N1 and 8N2 both work.
  //   FX-9750G Plus: 8N1 is REFUSED unless ~104 us of idle is added
  //                  between bytes. 8N2 supplies that for free.
  // Measured 2026 by changing only the framing bit.
  // THIS BUILD DEPENDS ON SERIAL_8N2.
  // ===============================================================

  // --- the value packet: exactly 16 bytes -----------------------
  uint8_t packet[VALUE_PACKET_LEN];
  uint8_t got = 0;
  uint32_t start = millis();
  while (got < VALUE_PACKET_LEN && millis() - start < 2000) {
    if (CasioSerial.available()) packet[got++] = CasioSerial.read();
    else                         delay(1);
  }

  if (got != VALUE_PACKET_LEN) {
    stats.shortPackets++;
    flush_line();
    return;
  }
  if (packet[0] != CASIO_PREAMBLE) {
    stats.badPreamble++;
    flush_line();
    return;
  }
  if (packet[VALUE_PACKET_LEN - 1] !=
      checksum_over(packet, VALUE_PACKET_LEN)) {
    stats.badChecksum++;
    TRACELN(F("INTERVAL PACKET CHECKSUM FAILED"));
#if RX_CHECKSUM_STRICT
    flush_line();
    return;
#endif
  }

  uint16_t value = denormalise_value(packet[5], packet[6], packet[7],
                                     packet[13], packet[14]);

  // Clamp. A calculator can send any number the protocol allows, and
  // a device that acts on an unclamped value arriving over a wire is
  // a device that can be stopped by a typing error. See the note at
  // MIN_INTERVAL_S for why the floor is 1 second and what decides it.
  if (value < MIN_INTERVAL_S) value = MIN_INTERVAL_S;
  if (value > MAX_INTERVAL_S) value = MAX_INTERVAL_S;
  timeInterval = value;

  turnaround();
  CasioSerial.write(CASIO_ACK);

  // --- the END packet: exactly 50 bytes, counted ----------------
  uint8_t endGot = 0;
  start = millis();
  while (endGot < REQUEST_PACKET_LEN && millis() - start < 300) {
    if (CasioSerial.available()) { CasioSerial.read(); endGot++; }
    else                         delay(1);
  }
  if (endGot != REQUEST_PACKET_LEN) stats.shortPackets++;

  flush_line();     // nothing should remain; if it does, take it
}

// ===================================================================
// SETUP
// ===================================================================
void setup() {
  // 9600 baud, 8 data bits, no parity, TWO stop bits.
  //
  // *** 8N2 IS REQUIRED, NOT MERELY CUSTOMARY. ***
  //
  // FX-9750GIII:   8N1 and 8N2 both work.
  // FX-9750G Plus: a gapless 8N1 stream is REFUSED. The calculator
  //                needs about ONE BIT PERIOD - 104 us at 9600 - of
  //                idle line between bytes, and a second stop bit is
  //                exactly that. An explicit ~104 us inter-byte delay
  //                does the same job; see casio_send_packet.
  //
  // MEASURED 2026 on an ESP32 by changing only the framing
  // bit, bulk writes both times: 8N1 fails, 8N2 works. Confirmed on
  // PICAXE and on an Arduino Uno, where the threshold was bisected to
  // between 75 and 100 us.
  //
  // Grindheim (2001) reports the link as asymmetric - two stop bits
  // FROM the calculator, one TO it and that remains true of what the
  // calculator will PARSE. It is not the same statement as what it will
  // accept in sequence.

#if CASIO_TRANSPORT_UART
  // UART0 starts life on GPIO 1/3 (the USB pins) and carries the
  // bootloader's own chatter at 74880 baud. swap() moves it to
  // GPIO 15/13 AFTER the board has booted, so the calculator never
  // sees any of that - it is asleep at power-up regardless.
  Serial.begin(9600, SERIAL_8N2);
  Serial.swap();

  #if DEBUG_TRACE
    // TX-only, on GPIO 2. Also the LED pin - see LED_IN_USE.
    Serial1.begin(115200);
  #endif
#else
  // USB serial monitor stays free for debugging, because the
  // calculator is on SoftwareSerial. The larger buffer gives the
  // 50-byte packets room to land while we are busy elsewhere.
  Serial.begin(115200);
  delay(300);
  CasioSerial.begin(9600, SWSERIAL_8N2, CASIO_RX_PIN, CASIO_TX_PIN,
                    false, 128);
#endif

#if WIFI_ENABLED
  // Access point. See the note at WIFI_ENABLED for why, and for what
  // it costs.
  //
  // persistent(false) stops the SDK writing the WiFi settings to
  // flash on every single boot. They are set in code here, so the
  // stored copy is never read - and flash has a finite number of
  // write cycles that a classroom device should not be spending on
  // nothing.
  WiFi.persistent(false);
  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID,
              (sizeof(AP_PASSWORD) > 1) ? AP_PASSWORD : nullptr,
              AP_CHANNEL, false, AP_MAX_CLIENTS);

  // No modem sleep. Sleep saves power and pays for it in latency
  // spikes, and latency is exactly what a serial link cannot afford.
  // The board is on a USB lead, not a battery.
  WiFi.setSleepMode(WIFI_NONE_SLEEP);

  #if WEB_SERVER_ENABLED
    server.on("/", handle_status_page);
    server.on("/data.csv", handle_csv_download);
    server.onNotFound(handle_status_page);   // any address shows the page
    server.begin();
  #endif
#else
  // WiFi OFF. This board can do WiFi, but the radio disturbs the
  // analogue reading (header note 3) and nothing here needs it yet.
  // Turning this on is a deliberate act - do it AFTER the transport
  // above has been proven on its own.
  WiFi.mode(WIFI_OFF);
  WiFi.forceSleepBegin();
  delay(10);
#endif

  ds18b20.begin();
  ds18b20.setResolution(DS18B20_RESOLUTION);
  // Do not block inside requestTemperatures(). The conversion runs
  // while we are waiting for the due second anyway - 94 ms at the
  // 9-bit default, 750 ms if you raise it to 12.
  ds18b20.setWaitForConversion(false);

  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
  // Cheap BME280 modules use 0x76; some use 0x77. Try both.
  bmePresent = bme.begin(0x76) || bme.begin(0x77);

  // pinMode AFTER Serial1.begin(), or it would take GPIO 2 back off
  // the debug port. LED_IN_USE settles which of the two owns the pin.
#if LED_IN_USE
  pinMode(LED_PIN, OUTPUT);
#endif
  led(LED_OFF);

  // *** THE BANNER GOES TO THE DEBUG PORT, NEVER TO Serial. ***
  // With CASIO_TRANSPORT_UART set, Serial IS the calculator. Printing
  // a greeting there sends 300 bytes of ASCII down a wire that is
  // waiting for a protocol, and the calculator answers with a COM
  // ERROR that looks like a wiring fault. This is the single easiest
  // mistake to make after the transport change.
#if DEBUG_TRACE || !CASIO_TRANSPORT_UART
  TRACEPORT.println();
  TRACEPORT.println(F("Casio ESP8266 NSN datalogger - ready"));
  TRACEPORT.println(F("Ch1 DS18B20 (1-Wire)  Ch2 A0 (analogue)  Ch3 BME280 (I2C)"));
  TRACEPORT.print  (F("BME280: "));
  TRACEPORT.println(bmePresent ? F("found") : F("NOT FOUND - channel 3 sends zero"));
  TRACEPORT.println(F("NSN: one value per Receive(, sign carried in the packet."));
  TRACEPORT.println(F("NO OFFSET applied. The Casio stores values as sent."));
  TRACEPORT.println(F("NSN has NO status field - see the web page for clamps."));
  #if CASIO_TRANSPORT_UART
    TRACEPORT.println(F("Transport: hardware UART0, swapped to D8/D7."));
  #else
    TRACEPORT.println(F("Transport: SoftwareSerial on D5/D6."));
  #endif
#endif
}

// ===================================================================
// MAIN LOOP
//
// Nothing happens until the calculator says something. The whole
// program is a reply to a question.
// ===================================================================
void loop() {
  if (!CasioSerial.available()) {
    // ===============================================================
    // *** THE WEB SERVER IS NOT ALLOWED HERE DURING A SESSION. ***
    //
    // This is the gap between one transaction and the next, and it is
    // the one place in the whole exchange where NO host-wait window
    // has ever been demonstrated. The calculator sends its attention
    // byte 0x15 and expects 0x13 straight back. Fenton's unbounded host wait
    // covers the pause INSIDE Receive(), after the second ACK - not
    // this handshake.
    //
    // Serve a 25 KB file here and the attention byte goes unanswered
    // for as long as the download takes. That is a COM ERROR, and it
    // was observed on 2026 before this guard existed.
    //
    // So: serve only when the calculator has been quiet for longer
    // than two sampling intervals, which means no session is running
    // and the board genuinely has nothing else to do. During a
    // session the long wait inside wait_for_interval() - which IS
    // inside the host-wait window - is the only place HTTP is served.
    // ===============================================================
    uint32_t quietMs = (uint32_t)timeInterval * 2000UL + 4000UL;
    if (millis() - lastExchangeMs > quietMs) {
      serve_web();
    }
    delay(1);                     // feeds the watchdog while idle
    return;
  }

  uint8_t inByte = CasioSerial.read();
  if (inByte != CASIO_ATTENTION) {
    stats.badAttention++;
    stats.lastBadByte  = inByte;
    stats.lastBadStage = 3;
    flush_line();               // do not leave the rest for next time
    return;
  }

  note_turnaround();        // the gap since we last spoke IS the calculator
  lastExchangeMs = millis();

  led(LED_ON);

  turnaround();
  CasioSerial.write(ESP_PRESENT);
  TRACE(F("\nATT 15  -> sent 13\n"));

  // The calculator now sends a 50-byte packet. Both kinds put the
  // command letter first and the variable name at byte 11:
  //     ":REQ..."  -> command 'R', it wants a value from us
  //     ":VAL..."  -> command 'V', it is giving us a value
  //
  // NOTE THE POSITIONS. Byte 0 is the ':' itself, so the command is
  // byte 1 and the variable name is byte 11. Getting this wrong by
  // one produces a logger that handshakes perfectly and then answers
  // the wrong question.
  // ===============================================================
  // READ THE WHOLE 50-BYTE REQUEST PACKET, THEN CHECK IT
  //
  // The idea came from the 2025 micro:bit draft,
  // which had it right: the packet is 50 bytes, so read 50 bytes.
  //
  // WHY IT IS SAFER, AND NOT MERELY TIDIER:
  //
  //   The old code counted the 38-byte tail and recorded a
  //   shortfall - and then CARRIED ON ANYWAY. A packet whose tail
  //   had not arrived was acted upon. Here a packet that is not
  //   complete is not used at all.
  //
  //   Having all 50 bytes means the CHECKSUM can be verified. The
  //   calculator sends one; until now this code ignored it and
  //   trusted the packet's shape instead. A desynchronised read -
  //   almost never checksums, so this catches the condition rather 
  //   than inferring it later from a byte that should have been an ACK.
  //
  //   There is one length, one timeout and one buffer instead of
  //   two of each, so there are fewer indices to get wrong. The
  //   command is byte 1 and the variable name is byte 11, counted
  //   from the ':' - which is now plainly visible rather than
  //   implied by a 12-byte header read.
  //
  // ANY failure flushes the line and abandons the transaction. One
  // bad packet then costs one reading instead of desynchronising
  // everything that follows.
  // ===============================================================
  uint8_t packet[REQUEST_PACKET_LEN];
  uint8_t got = 0;
  uint32_t start = millis();
  while (got < REQUEST_PACKET_LEN && millis() - start < 2000) {
    if (CasioSerial.available()) {
      packet[got++] = CasioSerial.read();
    } else {
      delay(1);
    }
  }

  if (got != REQUEST_PACKET_LEN) {
    stats.shortPackets++;
    TRACE(F("SHORT PACKET ")); TRACE(got); TRACE(F("  bytes: "));
    for (uint8_t d = 0; d < got; d++) { TRACEHEX(packet[d]); }
    TRACELN(F(""));
    flush_line();
    led(LED_OFF);
    return;
  }
  if (packet[0] != CASIO_PREAMBLE) {
    stats.badPreamble++;
    flush_line();
    led(LED_OFF);
    return;
  }
  if (packet[REQUEST_PACKET_LEN - 1] !=
      checksum_over(packet, REQUEST_PACKET_LEN)) {
    stats.badChecksum++;
    TRACELN(F("REQUEST CHECKSUM FAILED"));
#if RX_CHECKSUM_STRICT
    flush_line();
    led(LED_OFF);
    return;
#endif
  }

  stats.requestPackets++;

  // Nothing should remain. If anything does it was not part of this
  // packet, and too many bytes is a different fault from too few.
  {
    uint16_t extra = 0;
    while (CasioSerial.available()) { CasioSerial.read(); extra++; }
    if (extra) stats.extraBytes += extra;
  }

  uint8_t command = packet[1];      // byte 0 is the ':' preamble
  uint8_t vname   = packet[11];

  TRACE(F("REQ "));
  for (uint8_t i = 0; i < 12; i++) { TRACEHEX(packet[i]); }
  TRACE(F("  cmd=")); TRACE((char)command);
  TRACE(F(" vname=")); TRACELN((char)vname);

  if (command == CMD_RECEIVE) {
    handle_receive(vname);
  } else if (command == CMD_SEND) {
    handle_incoming();
  }

  led(LED_OFF);
  txDoneMs = millis();     // the calculator's own work starts here
}
