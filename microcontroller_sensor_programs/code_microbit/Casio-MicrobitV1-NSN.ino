/*
 ===================================================================
  CASIO FX-9750 <-> BBC micro:bit V1   NSN DATALOGGER
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

 ===================================================================
  THE SENSORS  -  THREE IMPLEMENTED, FIVE POSSIBLE
 ===================================================================
  sensor 1 = internal die temperature      INTERNAL - no pin used
  sensor 2 = button A state                INTERNAL - no pin used
  sensor 3 = internal accelerometer, X     INTERNAL - no pin used

  *** ALL THREE SENSORS ARE INSIDE THE BOARD. NO EDGE PIN IS USED
      BY ANY OF THEM. ***

  Two of the three are INSIDE THE BOARD, which is something the
  micro:bit can do and no other platform in this project can. A
  PICAXE, an ESP8266 or an ESP32 must have something wired to it
  before it can measure anything at all.
  
  This board logs three real quantities out
  of the box, with a cable and nothing else.

  Tilting the board drives the accelerometer from about -1000 to
  +1000 milli-g, which is a five-second test of the whole signed path
  and the quickest one in the project.


  P0, P1 and P2 stay FREE for external sensors the learner builds.
. This file starts no display driver. 

  FIVE IS THE MAXIMUM this board will carry, not three:
      sensor 4 = external ADC  A0 = edge pin P0
      sensor 5 = external ADC  A1 = edge pin P1
  The scaling functions for both are present below and the Casio
  program's N value decides how many are asked for. THIS RELEASE
  IMPLEMENTS THREE. To go further, the master BASIC program must also
  request D and E, which the released NSN-LOGR does not.

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
  THE USB SERIAL MONITOR IS LOST, AND THAT IS MANAGEABLE
 ===================================================================
  Serial IS UART0 on this core. Pointing UART0 at the calculator
  takes the monitor away. Two things make it acceptable:

  1. THE STARTUP BANNER IS PRINTED FIRST. Serial.begin, print
     everything worth knowing - board identity, I2C scan,
     accelerometer detection, resolved pin numbers - then Serial.end
     and repoint. It appears once at power-up, which is when it
     matters.

  2. AFTER THAT THE CALCULATOR IS THE DIAGNOSTIC DISPLAY, exactly as
     the PICAXE and ESP8266 used it. DIAG_CHANNEL3 puts a chosen
     counter on channel 3. A logger that reports its own health
     through the instrument it is attached to needs no laptop.

 ===================================================================
  TWO THINGS THIS OLD V1 BOARD DOES BETTER
 ===================================================================
  EXPLICIT UART ERROR REPORTING. The nRF51's ERRORSRC register flags
  overrun, framing, parity and break. The nRF52's UARTE hides these
  behind DMA. This board can COUNT a corrupted byte.

  TWO POSSIBLE ACCELEROMETERS, DETECTED RATHER THAN ASSUMED. V1.3
  carries an MMA8653 at 0x1D; V1.5 carries an LSM303AGR at 0x19. The
  firmware probes for both. Assuming either would produce a logger
  that works for half its users and silently reports nonsense for the
  rest.

 ===================================================================
  HARDWARE  -  identical to the V2 - colours user choice
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
  equipment near water. Keep sensor inputs within 0 V to 3.3 V.

 ===================================================================
  THE SWITCHES
 ===================================================================
   DEBUG_BANNER     1   startup report over USB before the port moves
   SINGLE_SENSOR    0   1 = channel 1 only. The simplest first test.
   USE_ACCELEROMETER 1  channel 3 from the accelerometer
   DIAG_CHANNEL3    0   0 = normal. 1-7 put a counter on channel 3,
                        turning the calculator into the diagnostic
                        display. See the table at the define.
 ===================================================================
*/

#include <Arduino.h>

// ===================================================================
// PINS  (edge connector numbers; chip pins resolved at run time)
// ===================================================================
#define CASIO_TX_PIN   8
#define CASIO_RX_PIN   12
#define SENSOR4_PIN    A0        // edge P0 - FREE, not used by this release
#define SENSOR5_PIN    A1        // edge P1 - FREE, not used by this release
#define I2C_SCL_EDGE   19
#define I2C_SDA_EDGE   20

// V1 Pin Mapping
const int buttonA = 5;
const int buttonB = 11;

// ===================================================================
// SWITCHES
// ===================================================================
#define DEBUG_BANNER       1
#define SINGLE_SENSOR      0
#define USE_ACCELEROMETER  1

// ===================================================================
// DIAGNOSTICS ON CHANNEL 3  -  the calculator as the instrument panel
//
// With no USB monitor after startup, this is how the board reports on
// itself. Each setting replaces the channel 3 sensor with a counter,
// which the calculator then logs and graphs like any other quantity.
//
//   0  normal - channel 3 is a sensor
//   1  which accelerometer was found: 0 none, 1 MMA8653, 2 LSM303AGR
//   2  short packets      - should stay 0
//   3  bad checksums      - should stay 0
//   4  unexpected bytes   - should stay 0
//   5  UART errors        - overrun/framing/parity. Should stay 0
//   6  uptime in seconds, wrapping at 4095
//   7  resyncs            - interval shorter than the calculator's loop
//
// The PICAXE and ESP8266 both used this trick. It is not a
// workaround: a quantity that ought to be constant, logged and
// plotted, is exactly the shape of a diagnostic - and it is the same
// statistics lesson as a cooling curve.
//
// UNDER NSN IT MATTERS MORE. There is no status field in a value
// packet, so this channel is the only way the board can tell you
// about itself while it is running.
// ===================================================================
#define DIAG_CHANNEL3      0

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
  // 3 = the internal sensors, using no edge pins at all.
  // 5 is this board's maximum and needs a master BASIC program that
  // also requests D and E. The released NSN-LOGR requests A, B, C.
  const uint8_t SENSOR_COUNT = 3;
#endif

#define BAUD_9600 0x00275000UL

// The nRF51 GPIO block is a single port. The nRF52 has two; code
// copied from the V2 build that mentions NRF_P1 will not compile here.
#ifndef NRF_GPIO
  #define NRF_GPIO NRF_P0
#endif

// ===================================================================
// THE SHORTEST AND LONGEST INTERVAL THIS BOARD WILL ACCEPT
// ===================================================================
//
// One Receive() transaction moves about 165 bytes. At 9600 baud that
// is ABOUT 180 ms, and it is 180 ms on every platform in this project
// - the wire does not care what is driving it. The device is not what
// decides whether a 1-second interval holds.
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
// Every fault found on the ESP builds was found by
// ARITHMETIC ON THESE NUMBERS, not by reading code. If you add
// anything to this firmware, add a counter for it.
// ===================================================================
struct LinkStats {
  uint32_t valuePackets;
  uint32_t endPackets;
  uint32_t requestPackets;
  uint32_t shortPackets;
  uint32_t badPreamble;
  uint32_t badChecksum;
  uint32_t badAttention;
  uint32_t uartErrors;      // nRF51 only: overrun, framing, parity
  uint32_t timeouts;
  uint32_t flushed;
  uint32_t resyncs;         // calculator could not keep up
  uint8_t  lastBadByte;
  uint8_t  lastBadStage;
};
LinkStats stats = {};

uint16_t timeInterval  = 10;
uint32_t nextSendTime  = 0;
bool     firstReading  = true;

// Five slots because the board can carry five. Three are filled by
// this release; slots 3 and 4 stay at zero unless SENSOR_COUNT is
// raised and a master program is written that asks for them.
int16_t  physicalValue[5];
uint8_t  saturatedMask   = 0;

const uint32_t SENSOR_READ_OFFSET_MS = 20;

// ===================================================================
// UART0  -  the ONLY one, pointed at the calculator
//
// No EasyDMA on this chip. Bytes arrive one at a time into a single
// register, so every read is polled. At 9600 baud a byte takes
// 1.15 ms, which is about eighteen thousand instructions on a 16 MHz
// Cortex-M0 - ample, provided the loop is not interrupted.
//
// ERRORSRC is checked after every read. It reports overrun, framing
// and parity faults explicitly, which the nRF52's DMA hides. A byte
// that arrived corrupted is COUNTED here rather than silently
// accepted, on the oldest board in the project.
// ===================================================================
void uart_begin() {
  uint32_t tx = g_ADigitalPinMap[CASIO_TX_PIN];
  uint32_t rx = g_ADigitalPinMap[CASIO_RX_PIN];

  pinMode(CASIO_TX_PIN, OUTPUT);
  digitalWrite(CASIO_TX_PIN, HIGH);      // a UART line rests HIGH
  pinMode(CASIO_RX_PIN, INPUT);

  NRF_UART0->ENABLE      = 0;
  NRF_UART0->PSELTXD     = tx;
  NRF_UART0->PSELRXD     = rx;
  NRF_UART0->PSELCTS     = 0xFFFFFFFF;
  NRF_UART0->PSELRTS     = 0xFFFFFFFF;
  NRF_UART0->CONFIG      = 0;            // no parity, no flow control
  NRF_UART0->BAUDRATE    = BAUD_9600;    // 8N1 - the only option here
  NRF_UART0->ENABLE      = 4;            // 4 = UART enabled on nRF51

  NRF_UART0->EVENTS_RXDRDY = 0;
  NRF_UART0->EVENTS_TXDRDY = 0;
  NRF_UART0->ERRORSRC      = 0xFFFFFFFF; // clear by writing ones
  NRF_UART0->TASKS_STARTRX = 1;          // listen continuously
  NRF_UART0->TASKS_STARTTX = 1;
}

static void uart_check_errors() {
  uint32_t e = NRF_UART0->ERRORSRC;
  if (e) { stats.uartErrors++; NRF_UART0->ERRORSRC = e; }
}

void uart_write_byte(uint8_t b) {
  NRF_UART0->EVENTS_TXDRDY = 0;
  NRF_UART0->TXD = b;
  uint32_t t0 = millis();
  while (!NRF_UART0->EVENTS_TXDRDY && (millis() - t0) < 50) { }
  NRF_UART0->EVENTS_TXDRDY = 0;
}

void uart_write(const uint8_t *data, uint8_t len) {
  for (uint8_t i = 0; i < len; i++) uart_write_byte(data[i]);
}

bool uart_read_byte(uint8_t &b, uint32_t timeoutMs) {
  uint32_t t0 = millis();
  while ((millis() - t0) < timeoutMs) {
    if (NRF_UART0->EVENTS_RXDRDY) {
      NRF_UART0->EVENTS_RXDRDY = 0;
      b = (uint8_t)NRF_UART0->RXD;
      uart_check_errors();
      return true;
    }
  }
  stats.timeouts++;
  return false;
}

// Reads exactly len bytes, or as many as arrive before the timeout.
uint16_t uart_read(uint8_t *dst, uint16_t len, uint32_t timeoutMs) {
  uint16_t got = 0;
  uint32_t t0 = millis();
  while (got < len && (millis() - t0) < timeoutMs) {
    if (NRF_UART0->EVENTS_RXDRDY) {
      NRF_UART0->EVENTS_RXDRDY = 0;
      dst[got++] = (uint8_t)NRF_UART0->RXD;
      uart_check_errors();
      t0 = millis();                 // restart on progress
    }
  }
  return got;
}

bool uart_byte_waiting() { return NRF_UART0->EVENTS_RXDRDY != 0; }

// Every path that abandons a transaction must leave the line EMPTY,
// or the bytes it walked away from are read one at a time afterwards
// and the board spends the next transactions one packet behind.
uint16_t flush_line() {
  uint16_t n = 0;
  uint8_t junk;
  while (uart_read_byte(junk, 15) && n < 200) n++;
  stats.flushed += n;
  return n;
}

// ===================================================================
// CHECKSUM  -  one rule, any packet length
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
// I2C  -  bit-banged on the P19/P20 bus
//
// On V1 the sensors share the edge connector bus, unlike the V2 where
// they sit on an internal one. Bit-banging is correct here for the
// same reason it was correct there and wrong for the serial link:
// I2C has no deadline, because the master drives the clock.
//
// PIN_CNF must be set. The input buffer is DISCONNECTED after reset,
// and a pin left that way reads 0 from IN whatever the line is doing.
// That cost an hour on the V2. Writes still work, so a device
// acknowledges its address and every read comes back zero.
//   DIR=1, INPUT connected, PULL=up, DRIVE=S0D1 (open drain)
// ===================================================================
#if USE_ACCELEROMETER
#define I2C_PIN_CNF 0x0000060DUL
const uint16_t I2C_HALF_US = 10;

static uint32_t I2C_SDA, I2C_SCL;

// Both parts the V1 has been built with, and how to know which.
const uint8_t MMA8653_ADDR   = 0x1D, MMA8653_WHO = 0x0D, MMA8653_ID = 0x5A;
const uint8_t LSM303_ADDR    = 0x19, LSM303_WHO  = 0x0F, LSM303_ID  = 0x33;

uint8_t accelKind = 0;          // 0 none, 1 MMA8653, 2 LSM303AGR
uint8_t accelAddr = 0;

static inline void sda_release() { NRF_GPIO->OUTSET = (1UL << I2C_SDA); }
static inline void sda_low()     { NRF_GPIO->OUTCLR = (1UL << I2C_SDA); }
static inline void scl_release() { NRF_GPIO->OUTSET = (1UL << I2C_SCL); }
static inline void scl_low()     { NRF_GPIO->OUTCLR = (1UL << I2C_SCL); }
static inline uint8_t sda_read() { return (NRF_GPIO->IN >> I2C_SDA) & 1; }
static inline void    i2c_hold() { delayMicroseconds(I2C_HALF_US); }

static void i2c_pins_init() {
  I2C_SDA = g_ADigitalPinMap[I2C_SDA_EDGE];
  I2C_SCL = g_ADigitalPinMap[I2C_SCL_EDGE];
  NRF_GPIO->PIN_CNF[I2C_SDA] = I2C_PIN_CNF;
  NRF_GPIO->PIN_CNF[I2C_SCL] = I2C_PIN_CNF;
  NRF_GPIO->OUTSET = (1UL << I2C_SDA) | (1UL << I2C_SCL);
}

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

// FALSE means nothing acknowledged. That is NOT the same as a device
// replying with zero, and the two must never share a value.
static bool i2c_read_reg(uint8_t addr, uint8_t reg, uint8_t &value) {
  i2c_start();
  if (!i2c_write((uint8_t)(addr << 1)))       { i2c_stop(); return false; }
  if (!i2c_write(reg))                         { i2c_stop(); return false; }
  i2c_start();
  if (!i2c_write((uint8_t)((addr << 1) | 1)))  { i2c_stop(); return false; }
  value = i2c_read(false);
  i2c_stop();
  return true;
}

static bool i2c_write_reg(uint8_t addr, uint8_t reg, uint8_t value) {
  i2c_start();
  bool ok = i2c_write((uint8_t)(addr << 1)) && i2c_write(reg) && i2c_write(value);
  i2c_stop();
  return ok;
}

// Probe for both parts rather than assuming. V1.3 and V1.5 differ, and
// a build that assumes one works for half its users and reports
// nonsense for the rest.
bool accel_begin() {
  i2c_pins_init();
  delay(2);

  uint8_t id = 0;
  if (i2c_read_reg(LSM303_ADDR, LSM303_WHO, id) && id == LSM303_ID) {
    accelKind = 2; accelAddr = LSM303_ADDR;
    i2c_write_reg(accelAddr, 0x20, 0x57);      // 100 Hz, normal, XYZ
    i2c_write_reg(accelAddr, 0x23, 0x00);      // +/- 2 g
    delay(10);
    return true;
  }
  if (i2c_read_reg(MMA8653_ADDR, MMA8653_WHO, id) && id == MMA8653_ID) {
    accelKind = 1; accelAddr = MMA8653_ADDR;
    i2c_write_reg(accelAddr, 0x2A, 0x00);      // standby to configure
    i2c_write_reg(accelAddr, 0x0E, 0x00);      // +/- 2 g
    i2c_write_reg(accelAddr, 0x2A, 0x01);      // active
    delay(10);
    return true;
  }
  accelKind = 0;
  return false;
}

// X axis in milli-g. Both parts present X as a left-justified 10-bit
// value in two registers; only the base address differs.
int16_t accel_x_milli_g() {
  if (!accelKind) return 0;
  const uint8_t base = (accelKind == 2) ? 0x28 : 0x01;   // LSM : MMA
  uint8_t lo = 0, hi = 0;

  i2c_start();
  if (!i2c_write((uint8_t)(accelAddr << 1)))              { i2c_stop(); return 0; }
  if (!i2c_write((uint8_t)(base | ((accelKind == 2) ? 0x80 : 0)))) { i2c_stop(); return 0; }
  i2c_start();
  if (!i2c_write((uint8_t)((accelAddr << 1) | 1)))        { i2c_stop(); return 0; }
  if (accelKind == 2) { lo = i2c_read(true); hi = i2c_read(false); }
  else                { hi = i2c_read(true); lo = i2c_read(false); }  // MMA is big-endian
  i2c_stop();

  int16_t raw    = (int16_t)((uint16_t)hi << 8 | lo);
  int16_t counts = raw >> 6;
  return (int16_t)(((int32_t)counts * 39) / 10);          // 3.9 mg per count
}
#endif  // USE_ACCELEROMETER

// ===================================================================
// SENSORS                                        *** EDIT ME ***
// Return TENTHS of your unit. Values may be NEGATIVE - NSN carries
// the sign, so there is nothing to add or subtract here.
// ===================================================================

// SENSOR 1 - the nRF51's own temperature sensor. No pins, no library,
// quarter degree steps.
//
// *** IT IS NOT A THERMOMETER, AND A WORKSHEET MUST SAY SO. ***
// The sensor is on the processor die. It reads several degrees above
// ambient because the chip warms itself. Fine for proving the
// pipeline and showing a trend; the quarter-degree steps are
// precision the instrument has not earned.
int16_t scale_to_physical_1() {
  NRF_TEMP->TASKS_START = 1;
  uint32_t t0 = millis();
  while (NRF_TEMP->EVENTS_DATARDY == 0 && (millis() - t0) < 10) { }
  NRF_TEMP->EVENTS_DATARDY = 0;
  int32_t quarters = (int32_t)NRF_TEMP->TEMP;
  NRF_TEMP->TASKS_STOP = 1;
  return (int16_t)((quarters * 5) / 2);       // quarters -> tenths
}

// SENSOR 2 - button A. 1 = pressed, 0 = not pressed. No pin used:
// the button is on the board.
int16_t scale_to_physical_2() {
  return (int16_t)(!digitalRead(buttonA));
}

// SENSOR 3 - the on-board accelerometer, X axis, in milli-g. SIGNED,
// and routinely negative, which is exactly what NSN handles natively.
int16_t scale_to_physical_3() {
#if DIAG_CHANNEL3 == 1
  return (int16_t)accelKind;                         // 0 / 1 / 2
#elif DIAG_CHANNEL3 == 2
  return (int16_t)(stats.shortPackets & 0x0FFF);
#elif DIAG_CHANNEL3 == 3
  return (int16_t)(stats.badChecksum & 0x0FFF);
#elif DIAG_CHANNEL3 == 4
  return (int16_t)(stats.badAttention & 0x0FFF);
#elif DIAG_CHANNEL3 == 5
  return (int16_t)(stats.uartErrors & 0x0FFF);
#elif DIAG_CHANNEL3 == 6
  return (int16_t)((millis() / 1000UL) & 0x0FFF);
#elif DIAG_CHANNEL3 == 7
  return (int16_t)(stats.resyncs & 0x0FFF);          // interval too short
#elif USE_ACCELEROMETER
  return accelKind ? accel_x_milli_g() : 0;
#else
  return 0;
#endif
}

// SENSORS 4 AND 5 - EXTERNAL, on edge pins P0 and P1. NOT USED BY
// THIS RELEASE. They are left here complete so that a learner who
// builds a sensor has somewhere obvious to attach it: raise
// SENSOR_COUNT and write a master BASIC program that also requests
// D and E.
int16_t scale_to_physical_4() {
  return (int16_t)(analogRead(SENSOR4_PIN) - 512);
}

int16_t scale_to_physical_5() {
  return (int16_t)(analogRead(SENSOR5_PIN) - 512);
}

// ===================================================================
// CLAMP
// A reading beyond what the packet can carry is clamped, and the
// clamp is recorded.
// saturatedMask exists for DIAG_CHANNEL3 and for anyone extending
// this build.
// ===================================================================
int16_t clamp_to_range(int32_t value, uint8_t channel) {
  if (value >  NSN_MAX_VALUE) { saturatedMask |= (1 << channel); return  NSN_MAX_VALUE; }
  if (value < -NSN_MAX_VALUE) { saturatedMask |= (1 << channel); return -NSN_MAX_VALUE; }
  return (int16_t)value;
}

void read_all_sensors() {
  saturatedMask = 0;
  physicalValue[0] = clamp_to_range(scale_to_physical_1(), 0);  // internal die temperature
  physicalValue[1] = clamp_to_range(scale_to_physical_2(), 1);  // button A
  physicalValue[2] = clamp_to_range(scale_to_physical_3(), 2);  // accelerometer, or a diagnostic
  physicalValue[3] = 0;   // external P0 - not used by this release
  physicalValue[4] = 0;   // external P1 - not used by this release
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
    uart_write(zeroPacket, VALUE_PACKET_LEN);
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

  uart_write(packet, VALUE_PACKET_LEN);
  stats.valuePackets++;
}

void send_description(uint8_t vname) {
  uint8_t packet[REQUEST_PACKET_LEN];
  const uint8_t head[11] = { CASIO_PREAMBLE, 0x56, 0x41, 0x4C, 0x00,
                             0x56, 0x4D, 0x00, 0x01, 0x00, 0x01 };
  memcpy(packet, head, 11);
  packet[11] = vname;
  for (uint8_t i = 12; i <= 18; i++) packet[i] = 0xFF;
  const uint8_t tag[10] = { 0x56,0x61,0x72,0x69,0x61,0x62,0x6C,0x65,0x52,0x0A };
  memcpy(packet + 19, tag, 10);
  for (uint8_t i = 29; i <= 48; i++) packet[i] = 0xFF;
  packet[49] = (uint8_t)(273 - vname);
  uart_write(packet, REQUEST_PACKET_LEN);
}

void send_end_packet() {
  uint8_t packet[REQUEST_PACKET_LEN];
  packet[0] = CASIO_PREAMBLE;
  packet[1] = 'E'; packet[2] = 'N'; packet[3] = 'D';
  for (uint8_t i = 4; i <= 48; i++) packet[i] = 0xFF;
  packet[49] = 0x56;
  uart_write(packet, REQUEST_PACKET_LEN);
  stats.endPackets++;
}

// ===================================================================
// DRIFT-FREE SCHEDULING
//     nextSendTime = nextSendTime + timeInterval;   <-- correct
//     nextSendTime = now + timeInterval;            <-- WRONG
// The second banks every scrap of delay forever. The first works from
// a schedule decided in advance, so a late reading does not push the
// ones after it.
// ===================================================================
void wait_for_interval() {
  if (firstReading) {
    firstReading = false;
    read_all_sensors();
    nextSendTime = millis() / 1000UL + timeInterval;
    return;
  }
  uint32_t dueMs  = nextSendTime * 1000UL;
  uint32_t readAt = dueMs - SENSOR_READ_OFFSET_MS;
  while ((int32_t)(millis() - readAt) < 0) { delay(2); }
  read_all_sensors();
  while ((int32_t)(millis() - dueMs)  < 0) { delay(1); }
  nextSendTime += timeInterval;
  // Counted since 7 August 2026: this branch firing is the ONE symptom
  // of an interval shorter than the calculator's own loop time.
  if (nextSendTime < millis() / 1000UL) {
    nextSendTime = millis() / 1000UL + timeInterval;
    stats.resyncs++;
  }
}

// ===================================================================
void handle_receive(uint8_t vname) {
  uint8_t b;
  uart_write_byte(CASIO_ACK);

  if (!uart_read_byte(b, 2000)) { send_end_packet(); return; }
  if (b != CASIO_ACK) { stats.lastBadByte = b; stats.lastBadStage = 1;
                        flush_line(); return; }

  send_description(vname);

  if (!uart_read_byte(b, 2000)) { send_end_packet(); return; }
  if (b != CASIO_ACK) { stats.lastBadByte = b; stats.lastBadStage = 2;
                        flush_line(); return; }

  // == HOST-WAIT WINDOW: THE VALUE WINDOW (GAP 3) ==
  // The heart of it. The calculator is inside Receive() waiting for a
  // number and it will wait - for five minutes if asked - without the
  // COM ERROR every reference says should follow.
  //
  // 300 s is the SPECIFIED limit, not the observed one. Three hours
  // has been reached and NO CEILING HAS BEEN ESTABLISHED. 
  //
  // *** THE INTERVAL WAIT BELONGS TO CHANNEL A ONLY. ***
  // A, B and C are three transactions within ONE sample. Waiting in
  // B or C as well would multiply the interval by the number of
  // sensors and the time axis would be silently wrong - the exact
  // failure this project refuses. A is the sample boundary; the
  // sensors are all read together inside wait_for_interval(), so B
  // and C return values already taken at the same instant as A.

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

  uart_read_byte(b, 1000);          // closing ACK is optional
  send_end_packet();
}

// ===================================================================
// THE CALCULATOR IS SENDING US A NUMBER  -  Send(T)
// ===================================================================
void handle_incoming() {
  uart_write_byte(CASIO_ACK);

  uint8_t packet[VALUE_PACKET_LEN];
  if (uart_read(packet, VALUE_PACKET_LEN, 2000) != VALUE_PACKET_LEN) {
    stats.shortPackets++; flush_line(); return;
  }
  if (packet[0] != CASIO_PREAMBLE) { stats.badPreamble++; flush_line(); return; }
  if (packet[VALUE_PACKET_LEN - 1] != checksum_over(packet, VALUE_PACKET_LEN)) {
    stats.badChecksum++;     // counted, not acted on - the rule is not
  }                          // yet verified against calculator traffic

  uint16_t value = 0;
  uint8_t I = packet[5], d1 = packet[6], d2 = packet[7];
  uint8_t signInfo = packet[13], E = packet[14];
  if (signInfo & 0x01) {
    if      (E == 0) value = I;
    else if (E == 1) value = I * 10   + (d1 >> 4);
    else if (E == 2) value = I * 100  + (d1 >> 4) * 10 + (d1 & 0x0F);
    else if (E == 3) value = I * 1000 + (d1 >> 4) * 100 + (d1 & 0x0F) * 10 + (d2 >> 4);
    else             value = 65535;
  }
  if (value < MIN_INTERVAL_S) value = MIN_INTERVAL_S;
  if (value > MAX_INTERVAL_S) value = MAX_INTERVAL_S;
  timeInterval = value;

  uart_write_byte(CASIO_ACK);

  uint8_t junk[REQUEST_PACKET_LEN];
  if (uart_read(junk, REQUEST_PACKET_LEN, 300) != REQUEST_PACKET_LEN) stats.shortPackets++;
  flush_line();
}

// ===================================================================
void setup() {
  analogReadResolution(10);

  pinMode(buttonA, INPUT_PULLUP);
  pinMode(buttonB, INPUT_PULLUP);

#if DEBUG_BANNER
  // Printed BEFORE the port moves. Everything worth knowing appears
  // once, at power-up, which is when it matters. After Serial.end()
  // the calculator is the only display this board has.
  Serial.begin(115200);
  delay(2500);
  Serial.println();
  Serial.println(F("Casio micro:bit V1 NSN datalogger - PROTOTYPE"));
  Serial.print  (F("TX  edge P8  -> chip P0.")); Serial.println(g_ADigitalPinMap[CASIO_TX_PIN]);
  Serial.print  (F("RX  edge P12 <- chip P0.")); Serial.println(g_ADigitalPinMap[CASIO_RX_PIN]);
  Serial.print  (F("SCL edge P19 -> chip P0.")); Serial.println(g_ADigitalPinMap[I2C_SCL_EDGE]);
  Serial.print  (F("SDA edge P20 -> chip P0.")); Serial.println(g_ADigitalPinMap[I2C_SDA_EDGE]);
#endif

#if USE_ACCELEROMETER
  accel_begin();
  #if DEBUG_BANNER
    Serial.print(F("accelerometer: "));
    if (accelKind == 2)      Serial.println(F("LSM303AGR at 0x19 (V1.5 board)"));
    else if (accelKind == 1) Serial.println(F("MMA8653 at 0x1D (V1.3 board)"));
    else                     Serial.println(F("NONE FOUND - channel 3 will read 0"));
  #endif
#endif

#if DEBUG_BANNER
  Serial.print  (F("sensors reported: ")); Serial.println(SENSOR_COUNT);
  Serial.println(F("all three are INTERNAL - no edge pin is used by any sensor"));
  Serial.print  (F("channel 3 diagnostic mode: ")); Serial.println(DIAG_CHANNEL3);
  Serial.println(F("8N1 to the calculator - this chip cannot send two stop bits."));
  Serial.println(F("NSN: one value per Receive(, sign carried in the packet."));
  Serial.println(F("NO OFFSET is applied. The Casio program stores values as sent."));
  Serial.println(F("Releasing UART0 to the calculator now. USB serial ends here."));
  Serial.flush();
  delay(50);
  Serial.end();
#endif

  uart_begin();          // UART0 now belongs to the calculator
}

// ===================================================================
// MAIN LOOP
// Nothing happens until the calculator says something. The whole
// program is a reply to a question.
// ===================================================================
void loop() {
  unsigned long now = millis();

  if (!uart_byte_waiting()) { delay(1); return; }

  uint8_t inByte;
  if (!uart_read_byte(inByte, 10)) return;

  if (inByte != CASIO_ATTENTION) {
    stats.badAttention++;
    stats.lastBadByte  = inByte;
    stats.lastBadStage = 3;
    flush_line();
    return;
  }

  uart_write_byte(DEVICE_PRESENT);

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

  uint8_t rxBuf[REQUEST_PACKET_LEN];
  uint16_t got = uart_read(rxBuf, REQUEST_PACKET_LEN, 2000);

  if (got != REQUEST_PACKET_LEN)   { stats.shortPackets++; flush_line(); return; }
  if (rxBuf[0] != CASIO_PREAMBLE)  { stats.badPreamble++;  flush_line(); return; }
  if (rxBuf[REQUEST_PACKET_LEN - 1] != checksum_over(rxBuf, REQUEST_PACKET_LEN)) {
    stats.badChecksum++;   // counted, not rejected - see the ESP builds
  }
  stats.requestPackets++;

  uint8_t command = rxBuf[1];      // byte 0 is the ':' preamble
  uint8_t vname   = rxBuf[11];

  if      (command == CMD_RECEIVE) handle_receive(vname);
  else if (command == CMD_SEND)    handle_incoming();

}
