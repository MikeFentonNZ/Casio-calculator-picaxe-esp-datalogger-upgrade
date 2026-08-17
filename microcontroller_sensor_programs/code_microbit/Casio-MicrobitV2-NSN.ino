/*
 ===========================================================
  CASIO FX-9750 <-> BBC micro:bit V2   NSN DATALOGGER
  Normalised scientific notation packets (NSN) DEMONSTRATION

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
 bench-validated in 2025 / 2026 with 0% packet failure.

  *** STATUS: PROOF OF CONCEPT - A FOUNDATION TO BUILD ON ***
 This is a reference implementation based on a validated method.
 It is deliberately minimal so that every line can be read and
 understood. It is NOT a finished classroom product and will not
 suit every use case. MODIFICATION IS EXPECTED. 

 ===================================================================
  THE SENSORS  -  AND WHICH NEED A WIRE
 ===================================================================
  sensor 1 = internal die temperature      INTERNAL - no pin used
  sensor 2 = analogue on edge pin P0       EXTERNAL - needs a sensor
  sensor 3 = internal accelerometer, X     INTERNAL - no pin used

  Two of the three are INSIDE THE BOARD, which is something the
  micro:bit can do and no other platform in this project can. A
  PICAXE, an ESP8266 or an ESP32 must have something wired to it
  before it can measure anything at all.

  So a first lesson can run with a cable and nothing else: set
  SENSOR_COUNT to 1 for die temperature alone, or leave P0
  unconnected and watch channel 2 sit at its centred value while the
  other two do real work. The external pin is the NEXT lesson, not
  the entry fee.

  P0, P1 and P2 are LED matrix columns, but nothing drives the matrix
  in the Arduino IDE unless a library is asked to, so they serve as
  analogue inputs here. This file starts no display driver.

 ===================================================================
  NSN, AND WHAT IT DOES NOT CARRY
 ===================================================================
  Each sensor is sent as its OWN value, in its own Receive(
  transaction, in normalised scientific notation. The calculator
  stores what arrives straight into a List with no processing:

      Receive(N)   how many sensors    -> N
      Send(T)      the interval        -> T
      Receive(A)   sensor 1            -> List 2
      Receive(B)   sensor 2            -> List 3   (only if N > 1)
      Receive(C)   sensor 3            -> List 4   (only if N > 2)

  SIGNED VALUES ARE NATIVE. Byte 14 of the value packet is the
  sign/info byte: bit 0 says the magnitude is >= 1, and bits 6 and 4
  together say NEGATIVE. So -12.5 travels as a negative number and
  arrives as one. NO OFFSET IS USED OR NEEDED ANYWHERE IN THIS BUILD,
  and no Casio program reading this logger has to subtract anything.

  Tilting the board drives the accelerometer from about -1000 to
  +1000 milli-g, which is a five-second test of the whole signed path
  and the quickest one in the project.

  *** WHAT NSN DOES NOT HAVE: A STATUS FIELD. ***
  There is nowhere in a plain value packet to put a fault code. This
  matters, and a worksheet should say so:

    - A missing accelerometer returns 0, and 0 is a legal reading.
    - A clamped sensor reports its clamped value and nothing else.

  The saturation mask below is still maintained, and DIAG_CHANNEL3
  can put any counter on channel 3 so the calculator displays it.
  That is the substitute, and it is a deliberate one: with three
  separate values there is no spare field, so a fault has to be shown
  by spending a CHANNEL rather than by hiding a code inside a number.

  THIS BOARD ALSO KEEPS ITS USB SERIAL MONITOR, which the V1 and the
  PICAXE do not. DEBUG_TRACE is the other place a fault will show.

 ===================================================================
  A QUESTION THIS BOARD SETTLED
 ===================================================================
  THE CALCULATOR ACCEPTS ONE STOP BIT. IT DOES NOT REQUIRE TWO.

  Both settings were tested here and both work. That confirms
  Grindheim (2001), who reported the link is asymmetric - two stop
  bits FROM the calculator, one TO it - and it finally disposes of a
  claim this project made for years without testing: that the
  calculator "insists on two; one is not accepted".

  That claim was withdrawn in August 2026 for lack of evidence. There
  is now evidence, and it is his.

  WHY TWO ALSO WORKS: an extra stop bit is only extra idle line. The
  receiver has already sampled the byte and is waiting for the next
  start bit, which simply arrives a fraction later.

 ===================================================================
  THE SECOND UART, AND WHY THIS FILE EXISTS IN THIS FORM
 ===================================================================
  It is widely stated that the micro:bit has one hardware UART. What
  is true is that the RUNTIME exposes one. CODAL, MakeCode and
  MicroPython enable UARTE0 for USB and offer no API for the second.
  The nRF52833 has UARTE0 AND UARTE1, each assignable to any GPIO,
  and from Arduino C++ the registers are simply memory addresses.

  So: UARTE0 stays locked to the USB serial monitor and is never
  touched. UARTE1 drives the calculator on P8 and P12.

  A capability that nothing exposes is not a capability absent. That
  is also how this project began - the calculator's unbounded host
  wait was in the hardware all along, undocumented, and everyone
  repeated that a mid-transaction pause would fail.

  WHAT THIS BOARD HAS THAT NO OTHER PLATFORM HERE DOES: EasyDMA. The
  UART can be told to receive a whole 50-byte packet into memory BY
  ITSELF. The PICAXE must sit inside a serin; the ESP must be told to
  read; this chip fills a buffer while the processor does something
  else. On the least powerful board of the five.

 ===================================================================
  HARDWARE - wire colours user choice
 ===================================================================
  - P8   -> to Casio RX   [RING of 2.5mm TRS, BLUE]  via 1N4148 diode,
            BAR (cathode) TOWARD THE MICRO:BIT
  - P12  <- from Casio TX [TIP of 2.5mm TRS, YELLOW]
  - P12  -> 4.7k pull-up to 3V   *** REQUIRED, NOT OPTIONAL ***
  - GND  -> Casio GND     [SLEEVE of 2.5mm TRS, BLACK]

  WHY A DIODE AND NOT A SERIES RESISTOR. The diode makes the output
  OPEN-DRAIN: this board can only ever pull the line LOW, and the
  calculator raises it with its own internal pull-up. So the board's
  supply no longer sets the calculator's high level, and one cable
  serves a 3.3 V FX-9750GIII and a 5 V FX-9750G Plus alike. Do NOT
  fit a series resistor as well - it shares the current path and
  pushes the LOW level back up, which is what breaks the link.

  WHY THE PULL-UP. When the calculator's port is not in use it goes
  HIGH IMPEDANCE, and the line floats to 0 V. Serial lines idle HIGH,
  so a board already listening reads that as a permanent break and
  logs junk until the calculator wakes its port. The pull-up supplies
  the idle state the calculator does not.

  P8 and P12 are ordinary GPIO, shared with nothing: not the LED
  matrix, not the buttons, not the internal I2C.

 ===================================================================
  WARNING
 ===================================================================
  NEVER connect mains electricity (240 V / 110 V) to the calculator,
  to this board, or to any sensor wiring. NEVER use mains-connected
  equipment near water.

  The micro:bit is a 3.3 V device. Keep sensor inputs within 0 V to
  3.3 V.

 ===================================================================
  THE DESIGN PRINCIPLE THIS FILE IS BUILT ON
 ===================================================================
  A FAULT MUST NEVER RESEMBLE A RESULT.

  Where a failure cannot be prevented it must be made visible. A
  logger that stops is a nuisance; a logger that carries on and
  quietly returns plausible numbers is worse than no logger at all.

  A student should be able to watch the phenomenon and FORGET what is
  doing the recording. An instrument can only be forgotten if, when
  it fails, this is apparent. Trust that has to be checked is not
  trust.

    clamp_to_range()   a reading beyond range is clamped, and every
                       clamp is recorded in saturatedMask.
    DIAG_CHANNEL3      spends a whole channel to show a counter,
                       because NSN has no spare field for one.
    centred ADC        an unconnected pin reads a clearly wrong
                       number rather than a plausible zero.
    the counters       every fault found in August 2026 was found by
                       arithmetic on them, not by reading code.

  READ THE NSN SECTION ABOVE. Without a status field, an absent
  sensor reads 0 and 0 is legal. That is the honest limitation of
  this build and it must be taught, not hidden.

 ===================================================================
  THE SWITCHES, IN ONE PLACE
 ===================================================================
   DEBUG_TRACE          1   protocol bytes to the USB monitor. This
                            board keeps its USB port, so unlike the
                            ESP8266 this costs nothing.
   STOP_BITS_TO_CASIO   1   1 or 2. See the note at UART_CONFIG.
   SINGLE_SENSOR        0   1 = report one sensor only. The simplest
                            first test.
   USE_ACCELEROMETER    1   1 = channel 3 is accelerometer X in
                            milli-g, which swings either side of zero
                            when the board is tilted.
   DIAG_CHANNEL3        0   0 = normal. 1-6 put a counter on channel 3
                            and turn the calculator into a diagnostic
                            display. 6 is the resynchronise counter -
                            the thing to watch when testing 1 s.
 ===================================================================
*/

#include <Arduino.h>

// ===================================================================
// PINS
// ===================================================================
#define CASIO_TX_PIN   8      // P8  -> Casio RX (blue, ring) via 1N4148, BAR to board
#define CASIO_RX_PIN   12     // P12 <- Casio TX (yellow, tip) + 4.7k
#define SENSOR2_PIN    A0     // edge P0 - free in Arduino, see header
#define SENSOR3_PIN    A1     // edge P1 - used only if the accelerometer is off

// ===================================================================
// SWITCHES
// ===================================================================
#define DEBUG_TRACE        1
#define SINGLE_SENSOR      0

// ===================================================================
// THE ACCELEROMETER ON CHANNEL 3
//
//   0 = channel 3 reads the analogue pin P1
//   1 = channel 3 reads accelerometer X, in milli-g
//
// WHY IT IS WORTH TURNING ON. Tilting the board drives the reading
// from about -1000 to +1000 milli-g, so the transmitted value swings
// either side of ZERO on demand. That is a five-second test of the
// entire signed path - the sign byte and the calculator's decode -
// and no other platform in this project has had one. Every negative
// value tested before this came from an unconnected pin or from
// waiting for cold weather.
//
// NO LIBRARY IS USED. The part is read directly over I2C, for the
// same reason the rest of this file avoids dependencies: a library
// that is missing, renamed or built for a different board revision
// is a failure a learner cannot diagnose.
//
// *** IF THE SENSOR IS NOT FOUND, CHANNEL 3 READS 0. ***
// NSN HAS NO STATUS FIELD in the value packet, so a missing sensor
// cannot be signalled in-band. The USB banner says so at startup and
// DIAG_CHANNEL3 = 1
// will report it on the calculator. A flat zero from a missing
// instrument is exactly the fault-resembling-a-result this project
// warns about, and NSN cannot signal it in-band. Say so in the
// worksheet.
// ===================================================================
#define USE_ACCELEROMETER  1

// ---- stop bits -----------------------------------------------------
// CONFIG bit 4: 0 = one stop bit, 1 = two.
//
// ANSWERED, 5 AUGUST 2026: BOTH WORK.
//
// One is kept as the default here because it is the minimum the
// calculator requires and it is now known to work. Two is equally
// valid and is what every other platform sends. Receiving is not
// affected: the calculator's second stop bit is simply idle line to a
// receiver expecting one.
#define STOP_BITS_TO_CASIO 1

#if DEBUG_TRACE
  #define TRACE(x)      Serial.print(x)
  #define TRACELN(x)    Serial.println(x)
  #define TRACEHEX(x)   do { if ((x) < 16) Serial.print('0'); \
                             Serial.print((x), HEX); Serial.print(' '); } while (0)
#else
  #define TRACE(x)
  #define TRACELN(x)
  #define TRACEHEX(x)
#endif

// ===================================================================
// PROTOCOL CONSTANTS  (identical to every other platform)
// ===================================================================
const uint8_t CASIO_ATTENTION    = 0x15;
const uint8_t DEVICE_PRESENT     = 0x13;
const uint8_t CASIO_ACK          = 0x06;
const uint8_t CASIO_PREAMBLE     = 0x3A;
const uint8_t CMD_RECEIVE        = 'R';
const uint8_t CMD_SEND           = 'V';
const uint8_t VNAME_N            = 'N';   // how many sensors?
const uint8_t VNAME_A            = 'A';   // sensor 1
const uint8_t VNAME_B            = 'B';   // sensor 2
const uint8_t VNAME_C            = 'C';   // sensor 3
const uint8_t VNAME_T            = 'T';   // sampling interval, via Send(

const uint8_t REQUEST_PACKET_LEN = 50;
const uint8_t VALUE_PACKET_LEN   = 16;

// The largest magnitude this build will transmit. Four significant
// digits is what the value packet carries in the form used here.
const int16_t NSN_MAX_VALUE = 9999;

// Byte 14 of the value packet, the sign/info byte.
//   bit 0        the magnitude is >= 1
//   bits 6 and 4 the value is NEGATIVE
// So 0x01 is a positive number and 0x51 a negative one. This is why
// no offset is needed: the packet carries the sign itself.
const uint8_t SIGN_POSITIVE = 0x01;
const uint8_t SIGN_NEGATIVE = 0x51;

#if SINGLE_SENSOR
  const uint8_t SENSOR_COUNT = 1;
#else
  const uint8_t SENSOR_COUNT = 3;
#endif

// ===================================================================
// nRF52 UARTE1 REGISTER VALUES
// ===================================================================
#define BAUD_9600    0x00275000UL
#if STOP_BITS_TO_CASIO == 2
  #define UART_CONFIG 0x00000010UL     // bit 4 set = two stop bits
#else
  #define UART_CONFIG 0x00000000UL
#endif

// The DMA pointer registers are cast with uintptr_t rather than
// uint32_t. On the nRF52833 the two are the same width, so the target
// code is unchanged - but it also lets this file be compiled on a PC
// against register stubs, which is how the packet tests run.
//
// *** EasyDMA CANNOT READ OR WRITE FLASH. Every buffer here is RAM. ***
// A string literal handed straight to TXD.PTR transmits nothing at
// all, with no error to say so. The easiest mistake to make with this
// peripheral, and the hardest to see.
static uint8_t txBuf[REQUEST_PACKET_LEN];
static uint8_t rxBuf[REQUEST_PACKET_LEN];
static uint8_t idleByte;
static bool    idleArmed = false;

// ===================================================================
// THE SHORTEST AND LONGEST INTERVAL THIS BOARD WILL ACCEPT
// ===================================================================
// MIN_INTERVAL_S WAS 2 UNTIL 7 AUGUST 2026. It is now 1.
//
// One Receive() transaction moves about 165 bytes. At 9600 baud that
// is ABOUT 180 ms, and it is 180 ms on every platform in this project
// - the wire does not care what is driving it. The device is not what
// decides whether a 1-second interval holds. THE CALCULATOR IS: its
// BASIC program must take each value in, write it to a list and
// refresh the display between one request and the next, and that has
// never been timed.
//
// *** NSN COSTS MORE TIME THAN A PACKED FRAME. *** Three sensors mean
// THREE complete Receive( transactions per sample, not one. Budget
// roughly three times the link time before choosing a short interval.
//
// IF THE CALCULATOR CANNOT KEEP UP, NOTHING CORRUPTS. This board sets
// the pace by holding the calculator inside the host-wait window. A
// slow calculator simply reaches its next Receive() late and this
// board, already waiting, answers at once. The samples sit further
// apart than asked, and the resynchronise branch fires - which is
// COUNTED, and can be put on channel 3 so the calculator reports it.
// ===================================================================
#define MIN_INTERVAL_S 1
#define MAX_INTERVAL_S 300

// ===================================================================
// LINK COUNTERS
//
// Every fault found on the ESP builds in August 2026 was found by
// ARITHMETIC ON THESE NUMBERS, not by reading code. If you add
// anything to this firmware, add a counter for it.
// ===================================================================
struct LinkStats {
  uint32_t valuePackets;
  uint32_t endPackets;
  uint32_t requestPackets;   // complete AND checksummed
  uint32_t shortPackets;     // fewer bytes than the packet length
  uint32_t badPreamble;      // byte 0 was not ':'
  uint32_t badChecksum;      // the calculator's own checksum failed
  uint32_t badAttention;     // a byte arrived that was not 0x15
  uint32_t timeouts;
  uint32_t flushed;
  uint32_t resyncs;          // calculator could not keep up
  uint8_t  lastBadByte;
  uint8_t  lastBadStage;     // 1 = ACK1, 2 = ACK2, 3 = attention
};
LinkStats stats = {};        // empty braces zero every member

// ===================================================================
// STATE
// ===================================================================
uint16_t timeInterval  = 10;      // seconds between readings (1-300)
uint32_t nextSendTime  = 0;
bool     firstReading  = true;

int16_t  physicalValue[3];
uint8_t  saturatedMask = 0;

// The sensor read is fast on this board - the temperature peripheral
// settles in well under a millisecond and analogRead is microseconds.
// There is no slow 1-Wire conversion to schedule around, so the
// read-ahead can be short. Compare the ESP builds, where a DS18B20
// forced a 244 ms budget.
const uint32_t SENSOR_READ_OFFSET_MS = 20;

// ===================================================================
// UARTE1  -  the calculator's port
// ===================================================================
void uarte1_begin() {
  uint32_t tx = g_ADigitalPinMap[CASIO_TX_PIN];
  uint32_t rx = g_ADigitalPinMap[CASIO_RX_PIN];

  pinMode(CASIO_TX_PIN, OUTPUT);
  digitalWrite(CASIO_TX_PIN, HIGH);      // a UART line rests HIGH
  pinMode(CASIO_RX_PIN, INPUT);

  NRF_UARTE1->ENABLE   = 0;
  NRF_UARTE1->PSEL.TXD = tx;
  NRF_UARTE1->PSEL.RXD = rx;
  NRF_UARTE1->PSEL.CTS = 0xFFFFFFFF;
  NRF_UARTE1->PSEL.RTS = 0xFFFFFFFF;
  NRF_UARTE1->CONFIG   = UART_CONFIG;
  NRF_UARTE1->BAUDRATE = BAUD_9600;
  NRF_UARTE1->ENABLE   = 8;
}

void uarte1_write(const uint8_t *data, uint8_t len) {
  if (len > sizeof(txBuf)) len = sizeof(txBuf);
  memcpy(txBuf, data, len);                 // into RAM - see above

  NRF_UARTE1->TXD.PTR       = (uintptr_t)txBuf;
  NRF_UARTE1->TXD.MAXCNT    = len;
  NRF_UARTE1->EVENTS_ENDTX  = 0;
  NRF_UARTE1->TASKS_STARTTX = 1;
  uint32_t t0 = millis();
  while (!NRF_UARTE1->EVENTS_ENDTX && (millis() - t0) < 300) { }
  NRF_UARTE1->EVENTS_ENDTX  = 0;
  NRF_UARTE1->TASKS_STOPTX  = 1;
}

void uarte1_write_byte(uint8_t b) { uarte1_write(&b, 1); }

// Receive EXACTLY len bytes, or as many as arrive before the timeout.
// Returns how many were actually received.
//
// THIS IS THE PART NO OTHER PLATFORM HERE CAN DO. The hardware fills
// the buffer on its own; the processor only waits. A 50-byte packet
// is one operation rather than fifty.
uint16_t uarte1_read(uint8_t *dst, uint16_t len, uint32_t timeoutMs) {
  NRF_UARTE1->RXD.PTR       = (uintptr_t)dst;
  NRF_UARTE1->RXD.MAXCNT    = len;
  NRF_UARTE1->EVENTS_ENDRX  = 0;
  NRF_UARTE1->EVENTS_RXTO   = 0;
  NRF_UARTE1->TASKS_STARTRX = 1;

  uint32_t t0 = millis();
  while (!NRF_UARTE1->EVENTS_ENDRX && (millis() - t0) < timeoutMs) { }

  NRF_UARTE1->TASKS_STOPRX = 1;
  uint32_t t1 = millis();
  while (!NRF_UARTE1->EVENTS_RXTO && (millis() - t1) < 10) { }

  uint16_t got = NRF_UARTE1->RXD.AMOUNT;
  NRF_UARTE1->EVENTS_ENDRX = 0;
  NRF_UARTE1->EVENTS_RXTO  = 0;
  idleArmed = false;
  return got;
}

bool uarte1_read_byte(uint8_t &b, uint32_t timeoutMs) {
  uint8_t one;
  if (uarte1_read(&one, 1, timeoutMs) == 1) { b = one; return true; }
  stats.timeouts++;
  return false;
}

// A single-byte receive left armed while nothing is happening, so the
// calculator's attention byte is caught the moment it arrives rather
// than only when we happen to look.
void arm_idle_listener() {
  if (idleArmed) return;
  NRF_UARTE1->RXD.PTR       = (uintptr_t)&idleByte;
  NRF_UARTE1->RXD.MAXCNT    = 1;
  NRF_UARTE1->EVENTS_ENDRX  = 0;
  NRF_UARTE1->TASKS_STARTRX = 1;
  idleArmed = true;
}

bool idle_byte_ready(uint8_t &b) {
  arm_idle_listener();
  if (!NRF_UARTE1->EVENTS_ENDRX) return false;
  NRF_UARTE1->EVENTS_ENDRX = 0;
  NRF_UARTE1->TASKS_STOPRX = 1;
  idleArmed = false;
  b = idleByte;
  return true;
}

// ===================================================================
// RESYNCHRONISE  -  clear the line after anything unexpected
//
// Every path that gives up on a transaction must leave the line
// EMPTY, or the bytes it walked away from are read one at a time
// afterwards and the board spends the next several transactions one
// packet behind. A single glitch becomes a cascade.
// ===================================================================
uint16_t flush_line() {
  uint16_t n = 0;
  uint8_t  junk;
  while (uarte1_read(&junk, 1, 15) == 1) {
    n++;
    if (n > 200) break;
  }
  stats.flushed += n;
  return n;
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

#if USE_ACCELEROMETER

// ===================================================================
// THE INTERNAL I2C BUS  -  found by scanning, 5 August 2026
//
// The micro:bit V2 has TWO I2C buses. The motion sensors sit on an
// INTERNAL one that is not brought out to the edge connector; P19 and
// P20 are a separate external bus. On V1 they were the same bus,
// which is why most guidance says P19/P20.
//
// Wire found nothing, because the Arduino core points it at the
// external bus. A bit-banged scan across candidate pin pairs found
// all four internal devices at once:
//
//     P0.16 = SDA, P0.08 = SCL
//       0x19  LSM303AGR accelerometer
//       0x1E  LSM303AGR magnetometer
//       0x70  interface chip
//       0x72  interface chip
//
// These pins have no Arduino pin number - they are not on the edge
// connector - so Wire cannot be pointed at them. The bus is driven
// directly instead.
//
// BIT-BANGING IS CORRECT HERE AND WAS WRONG FOR THE SERIAL LINK, and
// the difference is worth understanding rather than memorising. I2C
// has NO DEADLINE: the master drives the clock, so a pulse that
// arrives late is simply a slower bus and nothing is lost. A UART bit
// has a deadline every 104 microseconds and a late one corrupts the
// byte. The technique is neither good nor bad in itself - it depends
// entirely on whether anything is waiting.
//
// COST: about 0.6 ms per reading, inside a 20 ms sensor window.
// ===================================================================
const uint32_t I2C_SDA = 16;      // chip P0.16
const uint32_t I2C_SCL = 8;       // chip P0.08
const uint16_t I2C_HALF_US = 10;  // deliberately slow; this is not a data path

const uint8_t LSM303_ADDR      = 0x19;
const uint8_t LSM303_WHO_AM_I  = 0x0F;
const uint8_t LSM303_CTRL_REG1 = 0x20;
const uint8_t LSM303_CTRL_REG4 = 0x23;
const uint8_t LSM303_OUT_X_L   = 0x28;

bool accelPresent = false;

// ===================================================================
// OPEN-DRAIN PINS, CONFIGURED PROPERLY
//
// *** PIN_CNF MUST BE SET, AND THE FIRST VERSION OF THIS DID NOT. ***
//
// On the nRF52, PIN_CNF bit 1 DISCONNECTS the input buffer, and that
// is the state after reset. A pin left that way reads 0 from the IN
// register whatever the line is actually doing. Writing bytes still
// worked - that only needs the pin as an output - so the device
// acknowledged its address correctly and every bit read back came
// from a buffer that was not connected to anything.
//
// The symptom was WHO_AM_I = 0x00 on a bus where the scanner had
// already found the device. Again a zero that meant "not looking"
// rather than "the answer is zero".
//
//   DIR=1 output, INPUT=0 buffer CONNECTED (the fix), PULL=3 up,
//   DRIVE=6 S0D1: drives low, disconnects instead of driving high -
//   open-drain in hardware.
// ===================================================================
#define I2C_PIN_CNF  0x0000060DUL

static inline void i2c_pins_init() {
  NRF_P0->PIN_CNF[I2C_SDA] = I2C_PIN_CNF;
  NRF_P0->PIN_CNF[I2C_SCL] = I2C_PIN_CNF;
  NRF_P0->OUTSET = (1UL << I2C_SDA) | (1UL << I2C_SCL);   // both released
}

static inline void sda_release() { NRF_P0->OUTSET = (1UL << I2C_SDA); }
static inline void sda_low()     { NRF_P0->OUTCLR = (1UL << I2C_SDA); }
static inline void scl_release() { NRF_P0->OUTSET = (1UL << I2C_SCL); }
static inline void scl_low()     { NRF_P0->OUTCLR = (1UL << I2C_SCL); }
static inline uint8_t sda_read() { return (NRF_P0->IN >> I2C_SDA) & 1; }
static inline void    i2c_hold() { delayMicroseconds(I2C_HALF_US); }

static void i2c_start() { sda_release(); scl_release(); i2c_hold();
                          sda_low(); i2c_hold(); scl_low(); i2c_hold(); }
static void i2c_stop()  { sda_low(); i2c_hold(); scl_release(); i2c_hold();
                          sda_release(); i2c_hold(); }

static bool i2c_write(uint8_t b) {
  for (uint8_t i = 0; i < 8; i++) {
    if (b & 0x80) sda_release(); else sda_low();
    i2c_hold(); scl_release(); i2c_hold(); scl_low(); i2c_hold();
    b <<= 1;
  }
  sda_release(); i2c_hold(); scl_release(); i2c_hold();
  bool ack = (sda_read() == 0);
  scl_low(); i2c_hold();
  return ack;
}

static uint8_t i2c_read(bool ackIt) {
  uint8_t v = 0;
  sda_release();
  for (uint8_t i = 0; i < 8; i++) {
    i2c_hold(); scl_release(); i2c_hold();
    v = (uint8_t)((v << 1) | sda_read());
    scl_low();
  }
  if (ackIt) sda_low(); else sda_release();
  i2c_hold(); scl_release(); i2c_hold(); scl_low(); sda_release(); i2c_hold();
  return v;
}

// FALSE means nothing acknowledged, which is NOT the same as a device
// replying with zero. Keeping those apart is the same rule that runs
// through this whole file: two different failures must not share one
// value.
static bool accel_read_reg(uint8_t reg, uint8_t &value) {
  i2c_start();
  if (!i2c_write((uint8_t)(LSM303_ADDR << 1)))     { i2c_stop(); return false; }
  if (!i2c_write(reg))                              { i2c_stop(); return false; }
  i2c_start();
  if (!i2c_write((uint8_t)((LSM303_ADDR << 1) | 1))){ i2c_stop(); return false; }
  value = i2c_read(false);
  i2c_stop();
  return true;
}

static bool accel_write_reg(uint8_t reg, uint8_t value) {
  i2c_start();
  bool ok = i2c_write((uint8_t)(LSM303_ADDR << 1)) && i2c_write(reg) && i2c_write(value);
  i2c_stop();
  return ok;
}

bool accel_begin() {
  i2c_pins_init();
  delay(2);

  // A bus held low cannot work, and looks identical to an empty one
  // if you do not check. Say which it is.
  if (sda_read() == 0) {
    Serial.println(F("I2C: SDA is LOW at rest - the bus is held down."));
    return false;
  }

  uint8_t who = 0;
  if (!accel_read_reg(LSM303_WHO_AM_I, who)) {
    Serial.println(F("LSM303AGR: no acknowledge at 0x19 on the internal bus."));
    Serial.println(F("Run Microbit_i2c_finder.ino to locate it."));
    return false;
  }

  Serial.print(F("LSM303AGR WHO_AM_I = 0x"));
  if (who < 16) Serial.print('0');
  Serial.println(who, HEX);

  if (who != 0x33) {
    Serial.println(F("Answered, but not with 0x33 - a different part."));
    return false;
  }

  accel_write_reg(LSM303_CTRL_REG1, 0x57);   // 100 Hz, normal, XYZ on
  accel_write_reg(LSM303_CTRL_REG4, 0x00);   // +/- 2 g
  delay(10);
  return true;
}

// X axis in milli-g. Tilt the board and this swings about +/-1000,
// either side of ZERO on demand - the quickest test of the signed
// path in the whole project.
int16_t accel_x_milli_g() {
  uint8_t lo = 0, hi = 0;
  i2c_start();
  if (!i2c_write((uint8_t)(LSM303_ADDR << 1)))      { i2c_stop(); return 0; }
  if (!i2c_write((uint8_t)(LSM303_OUT_X_L | 0x80))) { i2c_stop(); return 0; }
  i2c_start();
  if (!i2c_write((uint8_t)((LSM303_ADDR << 1) | 1))){ i2c_stop(); return 0; }
  lo = i2c_read(true);
  hi = i2c_read(false);
  i2c_stop();

  int16_t raw    = (int16_t)((uint16_t)hi << 8 | lo);
  int16_t counts = raw >> 6;                       // left-justified, 10 bits
  return (int16_t)(((int32_t)counts * 39) / 10);   // 3.9 mg per count
}

#endif

// ===================================================================
// SENSORS                                        *** EDIT ME ***
//
// Return TENTHS of your unit, so one decimal place survives with no
// decimals in the packet: 23.4 degrees -> 234. Values may be
// NEGATIVE - NSN carries the sign, so there is nothing to add or
// subtract here.
// ===================================================================

// ---- Channel 1: the nRF52833's own temperature sensor -------------
//
// No pins, no wiring, no library. It reads in quarter-degree steps.
//
// *** IT IS NOT A THERMOMETER, AND A WORKSHEET MUST SAY SO. ***
// The sensor is on the processor die, not in the air. It reads
// several degrees above ambient because the chip warms itself, and
// its accuracy is roughly +/- 5 C. It is fine for proving the
// pipeline and for showing a TREND. It is a good illustration of the
// accuracy-versus-precision point: the quarter-degree steps are
// precision the instrument has not earned.
int16_t scale_to_physical_1() {
  NRF_TEMP->TASKS_START = 1;
  uint32_t t0 = millis();
  while (NRF_TEMP->EVENTS_DATARDY == 0 && (millis() - t0) < 10) { }
  NRF_TEMP->EVENTS_DATARDY = 0;
  int32_t quarters = (int32_t)NRF_TEMP->TEMP;
  NRF_TEMP->TASKS_STOP = 1;

  // quarter-degrees -> tenths of a degree:  x/4*10  =  x*5/2
  return (int16_t)((quarters * 5) / 2);
}

// ---- Channel 2: analogue on edge P0 -------------------------------
// THE ONE CHANNEL THAT NEEDS A WIRE. Centred like the ESP builds, so
// an unconnected pin reads a clearly wrong number rather than a
// plausible zero.
int16_t scale_to_physical_2() {
  int raw = analogRead(SENSOR2_PIN);       // 0 - 1023
  return (int16_t)(raw - 512);
}

// ===================================================================
// CHANNEL 3 AS A DIAGNOSTIC DISPLAY
// ===================================================================
// This board keeps its USB serial monitor, so it does not NEED the
// calculator as a diagnostic display the way the V1 and the PICAXE
// do. It is still worth having, for two reasons: a logging run may be
// nowhere near a laptop, and a counter nobody can read is no better
// than no counter.
//
// UNDER NSN IT MATTERS MORE. There is no status field in a value
// packet, so this channel is the only in-band way the board can tell
// the calculator about itself while it is running.
//
//   0 = normal. Channel 3 is the accelerometer or the ADC.
//   1 = accelerometer found (1) or not (0)
//   2 = short packets
//   3 = bad checksums
//   4 = unexpected bytes
//   5 = uptime in seconds, wrapping at 4095
//   6 = RESYNCHRONISED - the schedule fell a whole interval behind,
//       which is the ONE symptom of an interval shorter than the
//       calculator's own loop time. Watch this when testing 1 s.
//
// The value is masked to 12 bits so it always fits.
// ===================================================================
#define DIAG_CHANNEL3 0

int16_t scale_to_physical_3() {
#if   DIAG_CHANNEL3 == 1
  return (int16_t)(accelPresent ? 1 : 0);
#elif DIAG_CHANNEL3 == 2
  return (int16_t)(stats.shortPackets  & 0x0FFF);
#elif DIAG_CHANNEL3 == 3
  return (int16_t)(stats.badChecksum   & 0x0FFF);
#elif DIAG_CHANNEL3 == 4
  return (int16_t)(stats.badAttention  & 0x0FFF);
#elif DIAG_CHANNEL3 == 5
  return (int16_t)((millis() / 1000UL) & 0x0FFF);
#elif DIAG_CHANNEL3 == 6
  return (int16_t)(stats.resyncs       & 0x0FFF);   // interval too short
#elif USE_ACCELEROMETER
  // NSN cannot signal "not present" in-band. See the note at
  // USE_ACCELEROMETER: the startup banner and DIAG_CHANNEL3 = 1 are
  // the two places this is reported.
  return accelPresent ? accel_x_milli_g() : 0;
#else
  int raw = analogRead(SENSOR3_PIN);
  return (int16_t)(raw - 512);
#endif
}

// ===================================================================
// KEEP THE VALUE INSIDE WHAT THE PACKET CAN CARRY
//
// If a reading is out of range we do NOT silently pretend it was at
// the limit. We clamp it AND record that we did, so the fact is
// available to DIAG_CHANNEL3 and to anyone extending this build. A
// silent clamp is a lie the size of the error.
// ===================================================================
int16_t clamp_to_range(int32_t value, uint8_t channel) {
  if (value >  NSN_MAX_VALUE) { saturatedMask |= (1 << channel); return  NSN_MAX_VALUE; }
  if (value < -NSN_MAX_VALUE) { saturatedMask |= (1 << channel); return -NSN_MAX_VALUE; }
  return (int16_t)value;
}

void read_all_sensors() {
  saturatedMask = 0;
  physicalValue[0] = clamp_to_range(scale_to_physical_1(), 0);
  physicalValue[1] = clamp_to_range(scale_to_physical_2(), 1);
  physicalValue[2] = clamp_to_range(scale_to_physical_3(), 2);
}

// ===================================================================
// THE PACKETS  -  byte for byte as every other platform sends them
// ===================================================================

// ONE value, in normalised scientific notation, SIGNED.
//
// The magnitude is split into a leading digit and up to three more,
// packed two to a byte, with the exponent saying where the point
// belongs. The sign lives in byte 14 - see SIGN_POSITIVE and
// SIGN_NEGATIVE at the top of this file.
//
// Zero has its own packet with a fixed checksum, because a zero
// mantissa cannot be normalised.
void send_nsn_value(int16_t signedValue) {
  if (signedValue == 0) {
    const uint8_t zeroPacket[VALUE_PACKET_LEN] = {
      CASIO_PREAMBLE, 0x00, 0x01, 0x00, 0x01,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0xFE
    };
    uarte1_write(zeroPacket, VALUE_PACKET_LEN);
    stats.valuePackets++;
    TRACELN(F("value 0"));
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

  uint8_t packet[VALUE_PACKET_LEN];
  memset(packet, 0, sizeof packet);
  packet[0] = CASIO_PREAMBLE;
  packet[1] = 0x00; packet[2] = 0x01; packet[3] = 0x00; packet[4] = 0x01;
  packet[5] = intDigit;
  packet[6] = dec1;
  packet[7] = dec2;
  packet[13] = negative ? SIGN_NEGATIVE : SIGN_POSITIVE;
  packet[14] = exponent;
  packet[15] = calculate_checksum(packet);

  uarte1_write(packet, VALUE_PACKET_LEN);
  stats.valuePackets++;

  TRACE(F("value ")); TRACELN(signedValue);
}

// The checksum here is the constant 273 - vname, and that constant is
// only correct because every other byte never changes. Alter the
// padding and it silently becomes wrong.
void send_description(uint8_t vname) {
  uint8_t packet[REQUEST_PACKET_LEN];
  const uint8_t head[11] = { CASIO_PREAMBLE, 0x56, 0x41, 0x4C, 0x00,
                             0x56, 0x4D, 0x00, 0x01, 0x00, 0x01 };
  memcpy(packet, head, 11);
  packet[11] = vname;
  for (uint8_t i = 12; i <= 18; i++) packet[i] = 0xFF;
  const uint8_t tag[10] = { 0x56, 0x61, 0x72, 0x69, 0x61,
                            0x62, 0x6C, 0x65, 0x52, 0x0A };   // "VariableR\n"
  memcpy(packet + 19, tag, 10);
  for (uint8_t i = 29; i <= 48; i++) packet[i] = 0xFF;
  packet[49] = (uint8_t)(273 - vname);

  uarte1_write(packet, REQUEST_PACKET_LEN);
}

void send_end_packet() {
  uint8_t packet[REQUEST_PACKET_LEN];
  packet[0] = CASIO_PREAMBLE;
  packet[1] = 'E'; packet[2] = 'N'; packet[3] = 'D';
  for (uint8_t i = 4; i <= 48; i++) packet[i] = 0xFF;
  packet[49] = 0x56;
  uarte1_write(packet, REQUEST_PACKET_LEN);
  stats.endPackets++;
}

// ===================================================================
// WAIT FOR THE INTERVAL, THEN READ  -  drift-free scheduling
//
//        nextSendTime = nextSendTime + timeInterval;   <-- correct
//        nextSendTime = now + timeInterval;            <-- WRONG
//
// The second measures each interval from when the last one finished,
// so every scrap of delay is added to the next and kept forever. The
// first works from a schedule decided in advance: a late reading does
// not push the ones after it.
//
// Sensors are collected 20 ms before the due moment, so the reading
// and its timestamp describe the same instant.
// ===================================================================
void wait_for_interval() {
  if (firstReading) {
    firstReading = false;
    read_all_sensors();
    nextSendTime = millis() / 1000UL + timeInterval;
    return;
  }

  uint32_t dueMs   = nextSendTime * 1000UL;
  uint32_t readAt  = dueMs - SENSOR_READ_OFFSET_MS;

  while ((int32_t)(millis() - readAt) < 0) { delay(2); }
  read_all_sensors();
  while ((int32_t)(millis() - dueMs) < 0)  { delay(1); }

  nextSendTime += timeInterval;
  // Counted since 7 August 2026: this branch firing is the ONE symptom
  // of an interval shorter than the calculator's own loop time.
  if (nextSendTime < millis() / 1000UL) {
    nextSendTime = millis() / 1000UL + timeInterval;
    stats.resyncs++;
  }
}

// ===================================================================
// THE CALCULATOR WANTS A VALUE  -  Receive(
// ===================================================================
void handle_receive(uint8_t vname) {
  uint8_t b;
  uarte1_write_byte(CASIO_ACK);

  // A TIMEOUT and a WRONG BYTE are not the same thing:
  //   timeout    -> the calculator stopped listening. Close politely.
  //   wrong byte -> it abandoned the exchange. Say nothing further.
  if (!uarte1_read_byte(b, 2000)) { TRACELN(F("ACK1 timeout")); send_end_packet(); return; }
  if (b != CASIO_ACK) {
    stats.lastBadByte = b; stats.lastBadStage = 1;
    TRACE(F("ACK1 got ")); TRACEHEX(b); TRACELN(F(""));
    flush_line(); return;
  }

  // == HOST-WAIT WINDOW: THE DESCRIPTION WINDOW (GAP 2) ==
  // A pause here is tolerated. The value window is used instead,
  // because it sits immediately before the value packet and so the
  // reading is freshest when it is timestamped. That is a choice
  // about data quality, not a limitation of this window.

  send_description(vname);

  if (!uarte1_read_byte(b, 2000)) { TRACELN(F("ACK2 timeout")); send_end_packet(); return; }
  if (b != CASIO_ACK) {
    stats.lastBadByte = b; stats.lastBadStage = 2;
    TRACE(F("ACK2 got ")); TRACEHEX(b); TRACELN(F(""));
    flush_line(); return;
  }

  // == HOST-WAIT WINDOW: THE VALUE WINDOW (GAP 3) ==
  // The heart of it. The calculator is inside Receive() waiting for a
  // number and it will wait - for five minutes if asked - without the
  // COM ERROR every reference says should follow.
  //
  // 300 s is the SPECIFIED limit, not the observed one. Three hours
  // has been reached and NO CEILING HAS BEEN ESTABLISHED. Shorter
  // maxima recorded afterwards - around an hour - were long read as
  // the protocol behaving inconsistently, and were withdrawn on
  // 7 August 2026: the calculator's cells had never been changed
  // after the three-hour run, and a falling supply ends a session
  // with a Com ERROR and no warning at all. 300 s is a chosen margin.
  // A successful long pause still does not make the next one safe.
  //
  // *** THE INTERVAL WAIT BELONGS TO CHANNEL A ONLY. ***
  // A, B and C are three transactions within ONE sample. Waiting in
  // B or C as well would multiply the interval by the number of
  // sensors and the time axis would be silently wrong - the exact
  // failure this project refuses. A is the sample boundary; all three
  // sensors are read together inside wait_for_interval(), so B and C
  // return values taken at the same instant as A.

  if (vname == VNAME_N) {
    send_nsn_value((int16_t)SENSOR_COUNT);
  } else if (vname == VNAME_A) {
    wait_for_interval();
    send_nsn_value(physicalValue[0]);
  } else if (vname == VNAME_B) {
    send_nsn_value(physicalValue[1]);
  } else if (vname == VNAME_C) {
    send_nsn_value(physicalValue[2]);
  } else {
    send_nsn_value(0);
  }

  uarte1_read_byte(b, 1000);        // closing ACK is optional
  send_end_packet();
}

// ===================================================================
// THE CALCULATOR IS SENDING US A NUMBER  -  Send(T)
// ===================================================================
void handle_incoming() {
  uarte1_write_byte(CASIO_ACK);

  uint8_t packet[VALUE_PACKET_LEN];
  if (uarte1_read(packet, VALUE_PACKET_LEN, 2000) != VALUE_PACKET_LEN) {
    stats.shortPackets++; flush_line(); return;
  }
  if (packet[0] != CASIO_PREAMBLE) { stats.badPreamble++; flush_line(); return; }
  if (packet[VALUE_PACKET_LEN - 1] != checksum_over(packet, VALUE_PACKET_LEN)) {
    stats.badChecksum++;
    TRACELN(F("interval packet checksum failed"));
  }

  // Decode I.DDD x 10^E. The check that is easy to miss is the first
  // one: bit 0 of the sign/info byte is CLEAR when the magnitude is
  // below 1, and then the whole-number part is zero however big the
  // digits look. Without it, 0.5 arrives as 5.
  uint16_t value = 0;
  uint8_t I = packet[5], d1 = packet[6], d2 = packet[7];
  uint8_t signInfo = packet[13], E = packet[14];
  if (signInfo & 0x01) {
    if      (E == 0) value = I;
    else if (E == 1) value = I * 10  + (d1 >> 4);
    else if (E == 2) value = I * 100 + (d1 >> 4) * 10 + (d1 & 0x0F);
    else if (E == 3) value = I * 1000 + (d1 >> 4) * 100 + (d1 & 0x0F) * 10 + (d2 >> 4);
    else             value = 65535;
  }

  // Clamp. A device that acts on an unclamped value arriving over a
  // wire is a device that can be stopped by a typing error.
  if (value < MIN_INTERVAL_S) value = MIN_INTERVAL_S;
  if (value > MAX_INTERVAL_S) value = MAX_INTERVAL_S;
  timeInterval = value;
  TRACE(F("interval set to ")); TRACELN(timeInterval);

  uarte1_write_byte(CASIO_ACK);

  // The calculator's 50-byte END packet, counted rather than guessed.
  uint8_t junk[REQUEST_PACKET_LEN];
  if (uarte1_read(junk, REQUEST_PACKET_LEN, 300) != REQUEST_PACKET_LEN) {
    stats.shortPackets++;
  }
  flush_line();
}

// ===================================================================
void setup() {
  Serial.begin(115200);            // USB - UARTE0, never touched
  delay(2000);

  analogReadResolution(10);        // 0 - 1023, like the PICAXE
  uarte1_begin();

#if USE_ACCELEROMETER
  accelPresent = accel_begin();
  if (!accelPresent) {
    Serial.println(F("ACCELEROMETER NOT FOUND - channel 3 will read 0."));
    Serial.println(F("NSN HAS NO STATUS FIELD, so the calculator CANNOT be"));
    Serial.println(F("told in-band. Set DIAG_CHANNEL3 = 1 to report it, or"));
    Serial.println(F("treat a flat zero on channel 3 as suspect."));
  }
#endif

  Serial.println();
  Serial.println(F("Casio micro:bit V2 NSN datalogger"));
  Serial.print  (F("UARTE1 ENABLE = ")); Serial.println(NRF_UARTE1->ENABLE);
  Serial.print  (F("TX  P8 -> chip P0.")); Serial.println(g_ADigitalPinMap[CASIO_TX_PIN]);
  Serial.print  (F("RX P12 <- chip P0.")); Serial.println(g_ADigitalPinMap[CASIO_RX_PIN]);
  Serial.print  (F("stop bits to calculator: ")); Serial.println(STOP_BITS_TO_CASIO);
  Serial.print  (F("sensors reported: ")); Serial.println(SENSOR_COUNT);
  Serial.println(F("channels 1 and 3 are INTERNAL; channel 2 needs a sensor on P0."));
  Serial.println(F("NSN: one value per Receive(, sign carried in the packet."));
  Serial.println(F("NO OFFSET is applied. The Casio program stores values as sent."));
  Serial.println();
}

// ===================================================================
// MAIN LOOP
// Nothing happens until the calculator says something. The whole
// program is a reply to a question.
// ===================================================================
void loop() {
  uint8_t inByte;
  if (!idle_byte_ready(inByte)) { delay(1); return; }

  if (inByte != CASIO_ATTENTION) {
    stats.badAttention++;
    stats.lastBadByte  = inByte;
    stats.lastBadStage = 3;
    flush_line();                  // do not leave the rest for next time
    return;
  }

  uarte1_write_byte(DEVICE_PRESENT);
  TRACELN(F("\nATT 15 -> sent 13"));

  // ===============================================================
  // READ THE WHOLE 50-BYTE REQUEST PACKET, THEN CHECK IT
  //
  // The packet is 50 bytes, so read 50 bytes. A packet that is not
  // complete is never acted upon, and having all of it means the
  // calculator's own checksum can be verified instead of the packet's
  // shape being trusted. A read that has slipped by one byte fails
  // that checksum, which catches a desynchronisation when it happens
  // rather than several steps later as a byte where an ACK belonged.
  //
  // On this board the hardware does the reading. One operation.
  // ===============================================================
  uint16_t got = uarte1_read(rxBuf, REQUEST_PACKET_LEN, 2000);

  if (got != REQUEST_PACKET_LEN) {
    stats.shortPackets++;
    TRACE(F("short packet ")); TRACELN(got);
    flush_line(); return;
  }
  if (rxBuf[0] != CASIO_PREAMBLE) { stats.badPreamble++; flush_line(); return; }
  if (rxBuf[REQUEST_PACKET_LEN - 1] != checksum_over(rxBuf, REQUEST_PACKET_LEN)) {
    stats.badChecksum++;
    TRACELN(F("request checksum failed"));
    // NOT rejected. The rule is verified against packets this board
    // SENDS, never against one the calculator sent. Counting it first
    // establishes whether the rule holds on real traffic; rejecting
    // on an unverified check could refuse every transaction and kill
    // the logger outright. Watch this counter, then decide.
  }
  stats.requestPackets++;

  uint8_t command = rxBuf[1];      // byte 0 is the ':' preamble
  uint8_t vname   = rxBuf[11];

  TRACE(F("REQ cmd=")); TRACE((char)command);
  TRACE(F(" vname="));  TRACELN((char)vname);

  if      (command == CMD_RECEIVE) handle_receive(vname);
  else if (command == CMD_SEND)    handle_incoming();
}
