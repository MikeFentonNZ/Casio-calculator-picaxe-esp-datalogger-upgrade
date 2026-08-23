/*
 ===================================================================
  CASIO FX-9750 <-> ARDUINO UNO R3   NSN DATALOGGER
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
 Version 1.0; 03/03/2026
 (Update of Picaxe 2.0 rework 10/10/2025,
  original version 1.0 code for Picaxe 18X invented 2007)

 Ported from Casio-NSN-14M2.bas (PICAXE 14M2), which was
 bench-validated in 2025 / reconfirmed 2026 with 0% packet failure.

  *** STATUS: PROOF OF CONCEPT - A FOUNDATION TO BUILD ON ***
 This is a reference implementation based on a validated method.
 It is deliberately minimal so that every line can be read and
 understood. It is NOT a finished classroom product and will not
 suit every use case. MODIFICATION IS EXPECTED.

 The Uno is the fifth validated platform.

 ===================================================================
  THE UNO HAS ONE HARDWARE UART AND THE USB PORT IS USING IT
 ===================================================================
  This is the same problem the micro:bit V2 solved by claiming UARTE1
  and the ESP8266 solved by swapping pins. On the Uno there is no
  second UART to find, so there are two honest choices:

  CASIO_TRANSPORT_SOFT 1  (DEFAULT)
      SoftwareSerial on D8 and D9. The USB serial monitor stays, which
      matters on a board with no display and no other way to report.
      At 9600 baud on a 16 MHz AVR with nothing else running this is
      reliable - the ESP8266's bit-banging failure was an 80 MHz part
      with WiFi interrupts competing, which is not this situation.

  CASIO_TRANSPORT_SOFT 0
      The hardware UART on D0/D1. Better timing, but D0/D1 are also
      the USB bridge: the serial monitor is gone, AND THE CABLE MUST BE
      UNPLUGGED TO UPLOAD A SKETCH. In a classroom that is a trap, so
      it is not the default.

  *** THE SOFTWARESERIAL RECEIVE BUFFER IS 64 BYTES. ***
  The calculator's request packet is 50. That fits, but only just, and
  anything left on the line from a previous transaction eats into the
  margin. Every abandoned transaction here calls flush_line() for that
  reason. If short-packet counts start climbing on this board before
  any other, suspect the buffer first.

 ===================================================================
  THE SENSORS
 ===================================================================
  sensor 1 = A0 analogue     EXTERNAL - thermistor, LDR, your choice
  sensor 2 = A1 analogue     EXTERNAL
  sensor 3 = A2 analogue     EXTERNAL

  All three need a wire. The Uno has nothing to measure on its own,
  unlike a micro:bit - so unlike that board, a first lesson here needs
  a breadboard.

  Readings are CENTRED (raw - 512) so that an unconnected pin returns
  an obviously wrong number rather than a plausible zero. Replace the
  scale_to_physical_*() functions with your own conversion; return
  TENTHS of your unit so one decimal place survives.

 ===================================================================
  SIMULATED DATA - READ THIS BEFORE USING IT
 ===================================================================
  SIMULATE_SENSORS 1 replaces every reading with a generated value, so
  the protocol can be exercised with no sensors wired at all.

  *** THIS IS A LOADED GUN AND IT IS POINTED AT YOUR DATA. ***

  So the simulation here is deliberately made HARD TO SHIP BY ACCIDENT:

    - it is a compile-time switch, not a runtime one
    - the startup banner says so, in capitals, every time
    - the built-in LED flashes a distinctive triple-blink whenever a
      simulated value is sent, so a running board LOOKS wrong across
      the room
    - the waveform is a triangle that visibly reverses, not a ramp
      that could pass for a cooling curve or a rising temperature

  *** TURN IT OFF BEFORE ANY REAL RUN. ***
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

  Not enables here yet...

 ===================================================================
  HARDWARE  -  wire colours user choice
 ===================================================================
  - D8   <- from Casio TX [TIP of 2.5mm TRS, YELLOW]
  - D8   -> 4.7k pull-up to +5 V   *** REQUIRED, NOT OPTIONAL ***
  - D9   -> to Casio RX   [RING of 2.5mm TRS, BLUE] via 1N4148 diode,
            BAR (cathode) TOWARD THE ARDUINO
  - GND  -> Casio GND     [SLEEVE of 2.5mm TRS, BLACK]

  WHY A DIODE AND NOT A SERIES RESISTOR. The diode makes the output
  OPEN-DRAIN: this board can only ever pull the line LOW, and the
  calculator raises it with its own internal pull-up. So the board's
  supply no longer sets the calculator's high level, and one cable
  serves a 3.3 V FX-9750GIII and a 5 V FX-9750G Plus alike. Do NOT fit
  a series resistor as well - it shares the current path and pushes
  the LOW level back up, which is what breaks the link.

  *** ON A 3.3 V CALCULATOR THE DIODE IS NOT OPTIONAL. ***
  The Uno drives 5 V. An FX-9750GIII input is 3.3 V CMOS and is NOT
  5 V tolerant, so a direct connection can damage it. With the diode
  fitted the board only ever SINKS and never applies 5 V to anything.
  Here the diode protects the calculator; on a 5 V G Plus, where both
  ends share a domain, it is what lets one cable serve both.

  WHY THE PULL-UP. When the calculator's port is not in use it goes
  HIGH IMPEDANCE and the line floats to 0 V. Serial lines idle HIGH,
  so a board already listening reads that as a permanent break and
  logs junk until the calculator wakes its port. The pull-up supplies
  the idle state the calculator does not.

  *** PULL UP TO +5 V, NOT 3.3 V, AND THIS IS NOT A PREFERENCE. ***
  An FX-9750GIII drives its mark to only 2.75 V, which is BELOW the
  Uno's 3.0 V input threshold - bare, this board cannot read that
  calculator at all. The 4.7k pull-up to +5 V lifts the line to about
  3.9 V and makes the link possible. Pulled to 3.3 V it reaches 3.03 V,
  which is 30 mV of margin and is not a design.

  D8 AND D9 ARE CHOSEN, NOT ARBITRARY. SoftwareSerial receive needs a
  pin-change interrupt. On the Uno, D8-D13 are all PCINT0 (PORTB), so
  D8 is safe. D2-D7 are PCINT2 and also work; A0-A5 are PCINT1. Move
  the RX pin somewhere without PCINT and reception silently stops.

 ===================================================================
  WARNING
 ===================================================================
  NEVER connect mains electricity (240 V / 110 V) to the calculator,
  to this board, or to any sensor wiring. NEVER use mains-connected
  equipment near water. Keep every sensor input within 0 V and +5 V.

*/

#include <Arduino.h>

// ===================================================================
// SWITCHES
// ===================================================================
#define CASIO_TRANSPORT_SOFT 1   // 1 = SoftwareSerial D8/D9 (keeps USB)
                                 // 0 = hardware UART D0/D1 (loses USB)

#define SIMULATE_SENSORS     0   // 1 = GENERATED DATA. Read the header.

#define DEBUG_TRACE          1   // protocol notes to the USB monitor

// ===================================================================
// PINS
// ===================================================================
#define CASIO_RX_PIN    8    // from Casio TX (yellow, tip) + 4.7k pull-up
#define CASIO_TX_PIN    9    // to   Casio RX (blue, ring) via 1N4148, BAR to board

#define SENSOR1_PIN    A0
#define SENSOR2_PIN    A1
#define SENSOR3_PIN    A2

#define STATUS_LED     13    // the built-in one

// ===================================================================
// TRANSPORT
// ===================================================================
#if CASIO_TRANSPORT_SOFT
  #include <SoftwareSerial.h>
  SoftwareSerial CasioSerial(CASIO_RX_PIN, CASIO_TX_PIN);
  #define TRACEPORT Serial
#else
  #define CasioSerial Serial
  // No monitor is available in this mode. TRACE goes nowhere, and the
  // macros below compile to nothing so the strings are not even
  // stored - which matters on a board with 2 kB of SRAM.
  #undef  DEBUG_TRACE
  #define DEBUG_TRACE 0
#endif

// Every literal is wrapped in F() so it stays in flash. On a 2 kB
// board, a dozen forgotten strings is the difference between working
// and mysteriously not working.
// TRACEHEX prints one byte as two hex digits with a leading zero and a
// trailing space. It exists because TRACE() takes ONE argument, and
// TRACE(b, HEX) is a compile error - "macro passed 2 arguments, but
// takes just 1". The same macro is in the ESP and micro:bit builds.
#if DEBUG_TRACE
  #define TRACE(x)    TRACEPORT.print(x)
  #define TRACELN(x)  TRACEPORT.println(x)
  #define TRACEHEX(x) do { if ((x) < 16) TRACEPORT.print('0');       \
                           TRACEPORT.print((x), HEX);                \
                           TRACEPORT.print(' '); } while (0)
#else
  #define TRACE(x)
  #define TRACELN(x)
  #define TRACEHEX(x)
#endif

// ===================================================================
// PROTOCOL CONSTANTS  (identical to every other platform)
// ===================================================================
const uint8_t CASIO_ATTENTION = 0x15;
const uint8_t DEVICE_PRESENT  = 0x13;
const uint8_t CASIO_ACK       = 0x06;
const uint8_t CASIO_PREAMBLE  = 0x3A;
const uint8_t CMD_RECEIVE     = 'R';
const uint8_t CMD_SEND        = 'V';

const uint8_t VNAME_N = 'N';   // how many sensors?
const uint8_t VNAME_A = 'A';   // sensor 1
const uint8_t VNAME_B = 'B';   // sensor 2
const uint8_t VNAME_C = 'C';   // sensor 3

const uint8_t REQUEST_PACKET_LEN = 50;
const uint8_t VALUE_PACKET_LEN   = 16;

// Byte 14 of the value packet, the sign/info byte.
//   bit 0        the magnitude is >= 1
//   bits 6 and 4 the value is NEGATIVE
const uint8_t SIGN_POSITIVE = 0x01;
const uint8_t SIGN_NEGATIVE = 0x51;

// The largest magnitude the value packet carries in the form used here.
const int16_t NSN_MAX_VALUE = 9999;

const uint8_t SENSOR_COUNT = 3;

#define MIN_INTERVAL_S 1
#define MAX_INTERVAL_S 300

// ===================================================================
// THE TURNAROUND DELAY  -  a 2007 lesson 
// ===================================================================
// *** WAIT BEFORE REPLYING. THE CALCULATOR IS NOT READY YET. ***
//
// The calculator does not switch instantly from transmitting to
// listening. Reply into that turnaround and the first bits land while
// its port is still changing direction, so it receives a mangled byte
// and rejects the exchange.
//
// THIS WAS FOUND ON A PICAXE IN 2007 ona FX-9750G Plus. 
// The symptom looks like a wiring fault rather than a timing
// one: the calculator replies 0x22 - "I received something and it was
// the wrong kind of thing" - which sends you hunting the cable.
//
// The delay is applied INSIDE every send function rather than at each
// call site, so a new code path cannot forget it.
//
// 5 ms is the 2007 figure. It costs 5 ms per exchange against a
// 180 ms transaction - under 3 % - and nothing in this project is
// timing-critical at that scale, because the interval wait dwarfs it.
// Set to 0 to test whether it is still needed on your hardware.
// ===================================================================
#define TURNAROUND_MS 5

static inline void turnaround() {
#if TURNAROUND_MS > 0
  delay(TURNAROUND_MS);
#endif
}

// ===================================================================
// STATE
// ===================================================================
int16_t  physicalValue[3];
uint8_t  saturatedMask = 0;

uint16_t timeInterval = 10;
uint32_t nextSendTime = 0;
bool     firstReading = true;

// ===================================================================
// COUNTERS
// ===================================================================
struct LinkStats {
  uint16_t valuePackets;
  uint16_t endPackets;
  uint16_t requestPackets;
  uint16_t shortPackets;
  uint16_t badPreamble;
  uint16_t badChecksum;
  uint16_t badAttention;
  uint16_t timeouts;
  uint16_t flushed;
  uint16_t resyncs;
  uint8_t  lastSat;
  uint8_t  lastBadByte;
  uint8_t  lastBadStage;   // 1 = ACK1, 2 = ACK2, 3 = attention
};
LinkStats stats;           // zero-initialised as a global

// ===================================================================
// SENSORS                                        *** EDIT ME ***
// Return TENTHS of your unit. Values may be NEGATIVE - the packet
// carries the sign, so there is nothing to add or subtract here.
// ===================================================================
#if SIMULATE_SENSORS
// A TRIANGLE, NOT A RAMP, AND THAT IS DELIBERATE.
//
// A sawtooth that wraps looks like a plausible physical process -
// a filling vessel, a warming room, a discharging capacitor - right
// up until the wrap, which a student may not even capture. A triangle
// that visibly turns around cannot be mistaken for any of them, and
// the three channels run at different rates and opposite phases so
// the graph is obviously manufactured.
int16_t simulated(uint8_t channel) {
  static int16_t v[3]     = { 0, 2000, -1500 };
  static int8_t  dir[3]   = { 1, -1, 1 };
  const  int16_t step[3]  = { 37, 91, 143 };
  const  int16_t limit    = 3000;

  v[channel] += (int16_t)(dir[channel] * step[channel]);
  if (v[channel] >  limit) { v[channel] =  limit; dir[channel] = -dir[channel]; }
  if (v[channel] < -limit) { v[channel] = -limit; dir[channel] = -dir[channel]; }
  return v[channel];
}
#endif

// ---- Sensor 1: A0 --------------------------------------------------
// Centred, so an unconnected pin reads about -512 rather than 0.
// Zero is a legal reading; an obviously wrong number is not.
int16_t scale_to_physical_1() {
#if SIMULATE_SENSORS
  return simulated(0);
#else
  return (int16_t)(analogRead(SENSOR1_PIN) - 512);
#endif
}

// ---- Sensor 2: A1 --------------------------------------------------
int16_t scale_to_physical_2() {
#if SIMULATE_SENSORS
  return simulated(1);
#else
  return (int16_t)(analogRead(SENSOR2_PIN) - 512);
#endif
}

// ---- Sensor 3: A2 --------------------------------------------------
int16_t scale_to_physical_3() {
#if SIMULATE_SENSORS
  return simulated(2);
#else
  return (int16_t)(analogRead(SENSOR3_PIN) - 512);
#endif
}

// ===================================================================
// KEEP THE VALUE INSIDE WHAT THE PACKET CAN CARRY
// A silent clamp is a lie the size of the error, so it is recorded.
// ===================================================================
int16_t clamp_to_range(int32_t value, uint8_t channel) {
  if (value >  NSN_MAX_VALUE) { saturatedMask |= (1 << channel); return  NSN_MAX_VALUE; }
  if (value < -NSN_MAX_VALUE) { saturatedMask |= (1 << channel); return -NSN_MAX_VALUE; }
  return (int16_t)value;
}

// All three are read together, one after another with no waiting in
// between, so the three values a sample carries belong to the same
// instant even though they travel in three separate transactions.
void read_all_sensors() {
  saturatedMask = 0;
  physicalValue[0] = clamp_to_range(scale_to_physical_1(), 0);
  physicalValue[1] = clamp_to_range(scale_to_physical_2(), 1);
  physicalValue[2] = clamp_to_range(scale_to_physical_3(), 2);
  stats.lastSat = saturatedMask;
}

// ===================================================================
// THE SIMULATION TELL-TALE
//
// Three quick blinks whenever a generated value goes out. A board
// running on fabricated data should be visibly different from across
// a room, without anybody having to open the serial monitor.
// ===================================================================
void simulation_tell_tale() {
#if SIMULATE_SENSORS
  for (uint8_t i = 0; i < 3; i++) {
    digitalWrite(STATUS_LED, HIGH); delay(40);
    digitalWrite(STATUS_LED, LOW);  delay(40);
  }
#endif
}

// ===================================================================
// CHECKSUM  -  one rule, any packet length
//
// Add every byte AFTER the ':' preamble and BEFORE the checksum
// itself; the checksum is whatever brings that total back to zero in
// eight bits.
// ===================================================================
uint8_t checksum_over(const uint8_t *packet, uint8_t len) {
  uint8_t sum = 0;
  for (uint8_t i = 1; i < len - 1; i++) sum += packet[i];
  return (uint8_t)(0u - sum);
}

uint8_t calculate_checksum(const uint8_t *packet) {
  uint8_t sum = 0;
  for (uint8_t i = 0; i < 15; i++) sum += packet[i];
  uint8_t t = sum - 0x3A;
  t = 255 - t;
  return t + 1;
}

// ===================================================================
// SERIAL HELPERS
//
// Timeouts use UNSIGNED SUBTRACTION on millis(), never a comparison
// against a stored end time. millis() wraps after about 49 days, and
// an end-time comparison set just before the wrap waits forever.
// That exact bug was written and caught in this project in August
// 2026, in a door lock that would have failed OPEN.
// ===================================================================
bool waitForByte(uint8_t &b, uint16_t timeoutMs) {
  uint32_t t0 = millis();
  while ((millis() - t0) < timeoutMs) {
    if (CasioSerial.available()) { b = (uint8_t)CasioSerial.read(); return true; }
  }
  stats.timeouts++;
  return false;
}

uint8_t read_block(uint8_t *dst, uint8_t len, uint16_t timeoutMs) {
  uint8_t  got = 0;
  uint32_t t0  = millis();
  while (got < len && (millis() - t0) < timeoutMs) {
    if (CasioSerial.available()) { dst[got++] = (uint8_t)CasioSerial.read(); t0 = millis(); }
  }
  return got;
}

// Every path that abandons a transaction must leave the line EMPTY,
// or the bytes it walked away from are read one at a time afterwards
// and the board spends the next transactions one packet behind. On
// this board it matters twice over: the SoftwareSerial buffer is only
// 64 bytes and a 50-byte packet nearly fills it.
uint16_t flush_line() {
  uint16_t n = 0;
  uint32_t idle = millis();
  while ((millis() - idle) < 15 && n < 200) {
    if (CasioSerial.available()) { CasioSerial.read(); n++; idle = millis(); }
  }
  stats.flushed += n;
  return n;
}

// ===================================================================
// SEND ONE VALUE  -  normalised scientific notation, SIGNED
//
// The magnitude is split into a leading digit and up to three more,
// packed two to a byte, with the exponent saying where the point
// belongs. The sign lives in byte 14.
//
// Zero has its own packet with a fixed checksum, because a zero
// mantissa cannot be normalised.
// ===================================================================
void send_nsn_value(int16_t signedValue) {
  turnaround();
  uint8_t packet[VALUE_PACKET_LEN];

  if (signedValue == 0) {
    memset(packet, 0, VALUE_PACKET_LEN);
    packet[0]  = CASIO_PREAMBLE;
    packet[2]  = 0x01;
    packet[4]  = 0x01;
    packet[15] = 0xFE;
    CasioSerial.write(packet, VALUE_PACKET_LEN);
    stats.valuePackets++;
    return;
  }

  bool     negative = (signedValue < 0);
  uint16_t value    = (uint16_t)(negative ? -(int32_t)signedValue : signedValue);

  uint8_t intDigit = 0, dec1 = 0, dec2 = 0, exponent = 0;

  if (value < 10) {
    intDigit = (uint8_t)value;
    exponent = 0;
  } else if (value < 100) {
    intDigit = (uint8_t)(value / 10);
    dec1     = (uint8_t)((value % 10) << 4);
    exponent = 1;
  } else if (value < 1000) {
    intDigit = (uint8_t)(value / 100);
    uint16_t r = value % 100;
    dec1     = (uint8_t)(((r / 10) << 4) | (r % 10));
    exponent = 2;
  } else {
    intDigit = (uint8_t)(value / 1000);
    uint16_t r = value % 1000;
    dec1     = (uint8_t)(((r / 100) << 4) | ((r / 10) % 10));
    dec2     = (uint8_t)((r % 10) << 4);
    exponent = 3;
  }

  memset(packet, 0, VALUE_PACKET_LEN);
  packet[0] = CASIO_PREAMBLE;
  packet[1] = 0x00; packet[2] = 0x01; packet[3] = 0x00; packet[4] = 0x01;
  packet[5] = intDigit;
  packet[6] = dec1;
  packet[7] = dec2;
  packet[13] = negative ? SIGN_NEGATIVE : SIGN_POSITIVE;
  packet[14] = exponent;
  packet[15] = calculate_checksum(packet);

  CasioSerial.write(packet, VALUE_PACKET_LEN);
  stats.valuePackets++;
}

// ===================================================================
// THE DESCRIPTION PACKET  -  50 bytes
//
// The checksum is the constant 273 - vname, and that is only correct
// because every other byte never changes. Alter the padding and it
// silently becomes wrong.
//
// The fixed parts live in PROGMEM. On a 2 kB board there is no reason
// to spend SRAM on constants that never change.
// ===================================================================
const uint8_t PROGMEM DESC_HEAD[11] = {
  CASIO_PREAMBLE, 0x56, 0x41, 0x4C, 0x00, 0x56, 0x4D, 0x00, 0x01, 0x00, 0x01
};
const uint8_t PROGMEM DESC_TAG[10] = {
  0x56, 0x61, 0x72, 0x69, 0x61, 0x62, 0x6C, 0x65, 0x52, 0x0A   // "VariableR\n"
};

void send_description(uint8_t vname) {
  turnaround();
  uint8_t packet[REQUEST_PACKET_LEN];
  memcpy_P(packet, DESC_HEAD, 11);
  packet[11] = vname;
  for (uint8_t i = 12; i <= 18; i++) packet[i] = 0xFF;
  memcpy_P(packet + 19, DESC_TAG, 10);
  for (uint8_t i = 29; i <= 48; i++) packet[i] = 0xFF;
  packet[49] = (uint8_t)(273 - vname);
  CasioSerial.write(packet, REQUEST_PACKET_LEN);
}

void send_end_packet() {
  turnaround();
  uint8_t packet[REQUEST_PACKET_LEN];
  packet[0] = CASIO_PREAMBLE;
  packet[1] = 'E'; packet[2] = 'N'; packet[3] = 'D';
  for (uint8_t i = 4; i <= 48; i++) packet[i] = 0xFF;
  packet[49] = 0x56;
  CasioSerial.write(packet, REQUEST_PACKET_LEN);
  stats.endPackets++;
}

// ===================================================================
// DRIFT-FREE SCHEDULING
//     nextSendTime = nextSendTime + timeInterval;   <-- correct
//     nextSendTime = now + timeInterval;            <-- WRONG
// The second measures each interval from when the last one finished,
// so every scrap of delay is added to the next and kept forever. The
// first works from a schedule decided in advance: a late reading does
// not push the ones after it.
// ===================================================================
void wait_for_interval() {
  if (firstReading) {
    firstReading = false;
    read_all_sensors();
    nextSendTime = (millis() / 1000UL) + timeInterval;
    return;
  }

  uint32_t dueMs = nextSendTime * 1000UL;
  while ((int32_t)(millis() - dueMs) < 0) { delay(2); }
  read_all_sensors();

  nextSendTime += timeInterval;
  // This branch firing is the ONE symptom of an interval shorter than
  // the calculator's own loop time. It is counted rather than hidden.
  if (nextSendTime < (millis() / 1000UL)) {
    nextSendTime = (millis() / 1000UL) + timeInterval;
    stats.resyncs++;
  }
}

// ===================================================================
// THE CALCULATOR WANTS A VALUE  -  Receive(
// ===================================================================
void handle_receive(uint8_t vname) {
  uint8_t b;
  turnaround();
  CasioSerial.write(CASIO_ACK);

  // A TIMEOUT and a WRONG BYTE are not the same thing:
  //   timeout    -> the calculator stopped listening. Close politely.
  //   wrong byte -> it abandoned the exchange. Say nothing further.
  if (!waitForByte(b, 2000)) { send_end_packet(); return; }
  if (b != CASIO_ACK) {
    stats.lastBadByte = b; stats.lastBadStage = 1; flush_line(); return;
  }

  // == HOST-WAIT WINDOW: THE DESCRIPTION WINDOW (GAP 2) ==
  // A pause here is tolerated. The value window is used instead,
  // because it sits immediately before the value packet and so the
  // reading is freshest when it is sent.
  send_description(vname);

  if (!waitForByte(b, 2000)) { send_end_packet(); return; }
  if (b != CASIO_ACK) {
    stats.lastBadByte = b; stats.lastBadStage = 2; flush_line(); return;
  }

  // == HOST-WAIT WINDOW: THE VALUE WINDOW (GAP 3) ==
  // The calculator sits inside Receive() waiting for a number and it
  // will wait - for five minutes if asked - without the COM ERROR
  // every reference says should follow. 300 s is a chosen margin, not
  // an observed ceiling; a three-hour pause has been held on other
  // hardware and no upper bound has been established. Fit fresh cells
  // before a long unattended run: a falling supply ends a session with
  // a COM ERROR and no warning at all.
  //
  // *** THE INTERVAL WAIT BELONGS TO CHANNEL A ONLY. ***
  // A, B and C are three transactions within ONE sample. Waiting in B
  // or C as well would multiply the interval by the number of sensors
  // and stretch the time axis silently. All three sensors are read
  // together inside wait_for_interval(), so B and C return values
  // taken at the same instant as A.
  if (vname == VNAME_N) {
    send_nsn_value((int16_t)SENSOR_COUNT);
  } else if (vname == VNAME_A) {
    wait_for_interval();
    send_nsn_value(physicalValue[0]);
    simulation_tell_tale();
  } else if (vname == VNAME_B) {
    send_nsn_value(physicalValue[1]);
  } else if (vname == VNAME_C) {
    send_nsn_value(physicalValue[2]);
  } else {
    send_nsn_value(0);
  }

  waitForByte(b, 1000);        // the closing ACK is optional
  send_end_packet();
}

// ===================================================================
// THE CALCULATOR IS SENDING US A NUMBER  -  Send(T)
// ===================================================================
void handle_incoming() {
  turnaround();
  CasioSerial.write(CASIO_ACK);

  uint8_t packet[VALUE_PACKET_LEN];
  if (read_block(packet, VALUE_PACKET_LEN, 2000) != VALUE_PACKET_LEN) {
    stats.shortPackets++; flush_line(); return;
  }
  if (packet[0] != CASIO_PREAMBLE) { stats.badPreamble++; flush_line(); return; }
  if (packet[VALUE_PACKET_LEN - 1] != checksum_over(packet, VALUE_PACKET_LEN)) {
    stats.badChecksum++;   // counted, not acted on - the rule is not yet
  }                        // verified against traffic the calculator sends

  // Decode I.DDD x 10^E. The check that is easy to miss is the first
  // one: bit 0 of the sign/info byte is CLEAR when the magnitude is
  // below 1, and then the whole-number part is zero however big the
  // digits look. Without it, 0.5 arrives as 5.
  //
  // The interval is always positive, so the negative test is not
  // needed here. A build that RECEIVES user-entered values must also
  // check bits 6 and 4 - see the remote-control build.
  uint16_t value = 0;
  uint8_t I = packet[5], d1 = packet[6], d2 = packet[7];
  uint8_t signInfo = packet[13], E = packet[14];
  if (signInfo & 0x01) {
    if      (E == 0) value = I;
    else if (E == 1) value = I * 10   + (d1 >> 4);
    else if (E == 2) value = I * 100  + (d1 >> 4) * 10 + (d1 & 0x0F);
    else if (E == 3) value = (uint16_t)(I * 1000L + (d1 >> 4) * 100L
                                        + (d1 & 0x0F) * 10L + (d2 >> 4));
    else             value = 65535;
  }

  // Clamp. A device that acts on an unclamped value arriving over a
  // wire is a device that can be stopped by a typing error.
  if (value < MIN_INTERVAL_S) value = MIN_INTERVAL_S;
  if (value > MAX_INTERVAL_S) value = MAX_INTERVAL_S;
  timeInterval = value;
  TRACE(F("interval ")); TRACELN(timeInterval);

  turnaround();
  CasioSerial.write(CASIO_ACK);

  // The calculator's 50-byte END packet, counted rather than guessed.
  uint8_t junk[REQUEST_PACKET_LEN];
  if (read_block(junk, REQUEST_PACKET_LEN, 300) != REQUEST_PACKET_LEN) {
    stats.shortPackets++;
  }
  flush_line();
}

// ===================================================================
void setup() {
  pinMode(STATUS_LED, OUTPUT);
  digitalWrite(STATUS_LED, LOW);

#if CASIO_TRANSPORT_SOFT
  Serial.begin(115200);
  delay(300);
  CasioSerial.begin(9600);
  CasioSerial.listen();      // SoftwareSerial can only hear one port
#else
  CasioSerial.begin(9600);
#endif

#if DEBUG_TRACE
  TRACEPORT.println();
  TRACEPORT.println(F("Casio Arduino Uno R3 NSN datalogger"));
  TRACEPORT.println(F("A0, A1, A2 - all three sensors need a wire"));
  TRACEPORT.println(F("D8 <- Casio TX + 4.7k pull-up to +5V"));
  TRACEPORT.println(F("D9 -> Casio RX via 1N4148, BAR toward the Arduino"));
  TRACEPORT.println(F("SoftwareSerial 8N1 - this build needs one stop bit"));
  TRACEPORT.println(F("NSN: one value per Receive(, sign in the packet."));
  TRACEPORT.println(F("NO OFFSET. The Casio stores values as sent."));
  TRACEPORT.println(F("NSN has NO status field - faults appear here."));
  #if SIMULATE_SENSORS
  TRACEPORT.println(F("***********************************************"));
  TRACEPORT.println(F("*** SIMULATED DATA. THESE ARE NOT READINGS. ***"));
  TRACEPORT.println(F("*** The LED triple-blinks on every sample.  ***"));
  TRACEPORT.println(F("*** Set SIMULATE_SENSORS 0 for a real run.  ***"));
  TRACEPORT.println(F("***********************************************"));
  #endif
  TRACEPORT.println(F("Waiting for the calculator..."));
#endif

  // Even with no monitor, a board running on generated data says so:
  // a long triple-blink at power-up, then again on every sample.
#if SIMULATE_SENSORS
  for (uint8_t i = 0; i < 6; i++) {
    digitalWrite(STATUS_LED, HIGH); delay(120);
    digitalWrite(STATUS_LED, LOW);  delay(120);
  }
#endif
}

// ===================================================================
// MAIN LOOP
// Nothing happens until the calculator says something. The whole
// program is a reply to a question.
// ===================================================================
void loop() {
  if (!CasioSerial.available()) return;

  uint8_t inByte = (uint8_t)CasioSerial.read();
  if (inByte != CASIO_ATTENTION) {
    stats.badAttention++;
    stats.lastBadByte  = inByte;
    stats.lastBadStage = 3;
    // SAY WHAT ARRIVED. This path was silent until 17 August 2026, and
    // that silence made "the board just waits" mean two different
    // things at once: nothing arriving, or bytes arriving and being
    // discarded because they were not 0x15. Those need opposite
    // repairs. A discard is not a non-event and must not look like one.
    TRACE(F("not 0x15, got: ")); TRACEHEX(inByte); TRACELN(F(""));
    flush_line();
    return;
  }

  // The calculator has just finished transmitting and is turning its
  // port around to listen. Reply into that and it receives rubbish.
  turnaround();
  CasioSerial.write(DEVICE_PRESENT);

  // READ THE WHOLE 50-BYTE REQUEST, THEN CHECK IT.
  // A packet that is not complete is never acted upon, and having all
  // of it means the calculator's own checksum can be verified rather
  // than the packet's shape trusted. A read that has slipped by one
  // byte fails that checksum, which catches a desynchronisation when
  // it happens rather than several steps later.

  uint8_t rxBuf[REQUEST_PACKET_LEN];
  uint8_t got = read_block(rxBuf, REQUEST_PACKET_LEN, 2000);

  if (got != REQUEST_PACKET_LEN) {
    stats.shortPackets++;
    // WHAT ARRIVED MATTERS MORE THAN HOW MANY. A count alone cannot
    // tell a dead transmit line from a truncated packet, and those
    // need opposite repairs. Print the bytes.
    //
    //   got 1, byte 15  the calculator never heard our 0x13 and has
    //                   retried the attention byte. OUR TRANSMIT IS
    //                   NOT REACHING IT - check the diode orientation
    //                   first, bar toward the Arduino.
    //   got 1, byte 22  an explicit abort from the calculator.
    //   got 1, byte 3A  the request packet STARTED and was cut off -
    //                   a receive-side fault, not a transmit one.
    //                   Suspect the 64-byte SoftwareSerial buffer or
    //                   a marginal connection.
    //   got 0           nothing at all came back.
    TRACE(F("short ")); TRACE(got); TRACE(F(" bytes: "));
    for (uint8_t i = 0; i < got && i < 8; i++) TRACEHEX(rxBuf[i]);
    TRACELN(F(""));
    flush_line(); return;
  }
  if (rxBuf[0] != CASIO_PREAMBLE) { stats.badPreamble++; flush_line(); return; }
  if (rxBuf[REQUEST_PACKET_LEN - 1] != checksum_over(rxBuf, REQUEST_PACKET_LEN)) {
    stats.badChecksum++;   // counted, not rejected - watch it, then decide
  }
  stats.requestPackets++;

  uint8_t command = rxBuf[1];      // byte 0 is the ':' preamble
  uint8_t vname   = rxBuf[11];

  if      (command == CMD_RECEIVE) handle_receive(vname);
  else if (command == CMD_SEND)    handle_incoming();
}
