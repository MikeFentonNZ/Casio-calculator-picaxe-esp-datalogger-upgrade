/* ===========================================================
  CASIO FX-9750 <-> ESP32 DATALOGGER AND WEBSERVER 192.168.4.1
  Normalised scientific notation packets (NSN) DEMONSTRATION
  and diagnostic webserver DEMONSTRATION

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
 For example,  up to 3 sensors, read synchronously but delivered 
 as consecutive seperate RECEIVE() values.

 The webserver includes lots of diagnostic data; instead show a chart
 of sensor readings once this code is trusted based on 0% errors.

 Bench testing and classroom use in 2008, updated coding and testing
 in 2025 & 2026, all returned 0% packet failure and 0% COM error across  
 sampling intervals from 1 second to 5 minutes, with data logging up to 3 
 hours. Estimated packets transmitted in total 100,000+ in 600+ logging 
 sessions. 
 =================================================================
 THE DESIGN PRINCIPLE THIS FILE IS BUILT ON
 =================================================================
 A FAULT MUST NEVER RESEMBLE A RESULT.

 Where a failure cannot be prevented, it must be made visible. A
 logger that stops is a nuisance. A logger that carries on and
 quietly returns plausible numbers is worse than no logger at all,
 because the experiment continues and the error is found - if ever -
 long after the apparatus has been put away.

 The point of the whole project is that a student should be able to
 watch the phenomenon and FORGET what is doing the recording. An
 instrument can only be forgotten if, when it fails, this is
 apparent. Trust that has to be checked is not trust.

 A NOTE ON ACCURACY
  -----------------------------------------------------------------
  This code uses analogRead(), which returns a raw 0-4095 count.
  The ESP32's converter is noticeably NON-LINEAR near 0 V and near
  3.3 V, so raw counts are not proportional to voltage at the ends
  of the range.

  analogReadMilliVolts() applies calibration data programmed into
  each chip at the factory and returns millivolts directly. It is
  the better choice for real measurement work, and converting this
  code to use it is a worthwhile exercise:

      int mv = analogReadMilliVolts(SENSOR2_PIN);   // 0 - 3300 mV

  Raw analogRead() is kept here because it makes the arithmetic in
  scale_to_physical() visible to a beginner, which is the point of
  a proof of concept.
 =================================================================
 STUDENT / TEACHER WARNING!
 - NEVER use boiling water for temperature calibration (it is NOT needed)
 - NEVER connect mains electricity (240 V / 110 V) to the calculator,
   to this board, or to any sensor wiring. 
 - NEVER use mains-connected equipment near water.
 
 Modern (post 2020) Casio calculators are 3.3 V logic. 
 Power the PICAXE at 3 - 3.3 V and keep the resistors below in place.

 =================================================================
 A SB-62 cross-over cable has male 2.5mm TRS plugs at both ends.
 
  THE CALCULATOR ACCEPTS ONE STOP BIT. IT DOES NOT REQUIRE TWO.

  Both settings were tested here and both work. That confirms
  Grindheim (2001), who reported the link is asymmetric - two stop
  bits FROM the calculator, one TO it.

  WHY TWO ALSO WORKS: an extra stop bit is only extra idle line. The
  receiver has already sampled the byte and is waiting for the next
  start bit, which simply arrives a fraction later.
 
  It is incorrect to say that one stop bit is rejected.

 Checksum independently derived and confirmed by Grindheim (2001). Grindheim 
 documents exactly one timeout - 0.5 to 1 second for the device to answer the 
 opening 0x15 message from the Casio. This strict timeout at the handshake was 
 reasonably, and wrongly, assumed to be time-critical by the wider community. His
 work was also done from a PERSONAL COMPUTER, which accounts for most of what his
 work does not cover. A PC has a deep buffer, an operating system, and no reason
 to want a pause mid-transaction. A microcontroller inverts all of that.

 The Casio host-wait-windows (GAP 1-4) were not discovered by something that failed
 but by methodically driving the encoding deliberately to its extremes. 
 A similar methodology has been applied to all derivative works by the author in
 the porting to other microcontroller platforms for data logging applications.
 
 Detailed documentation now provides all steps and methods for Casio serial 
 communication with a microcontoller for data logging, remote control, industrial 
 measurement and control (IMC) simulation, and as a Human Machine Interface. 
 
 Limits and rules for coding and data transmission come with exhaustive evidence 
 filling in all details of what was previously unknown and/or undocumented.
 =================================================================

  This program demonstrates the two complementary uses of the
  nsn packets:

  1. RECEIVE direction (Casio requests data from the ESP32):
     - Casio RECEIVE(A) requests Sensor 1 reading (thermistor)
     - Casio RECEIVE(B) requests Sensor 2 reading (LDR)
     - Casio RECEIVE(C) requests Sensor 3 reading (ultrasonic)

  2. SEND direction (Casio pushes values to the ESP32):
     - Casio SEND(N) requests sensor count (returns 3)

 ===================================================================
  HARDWARE - Wire colours users choice
  -----------------------------------------------------------------
  - GPIO 16 (RX) <- from Casio TX  [TIP of 2.5mm TRS, YELLOW wire]
  - GPIO 16      -> 4.7k pull-up resistor to 3.3 V  *** REQUIRED ***
  - GPIO 17 (TX) -> to Casio RX  [RING of 2.5mm TRS, BLUE wire]
                    via via 1N4148 diode, bar towards ESP32
  - GND          -> Casio GND    [SLEEVE of 2.5mm TRS, BLACK wire]

  - GPIO 4       -> DS18B20 data, with 4.7k pull-up to 3.3 V
                    (sensor 1 - the one that can read below zero)
  - GPIO 35      -> Sensor 2, analogue
  - GPIO 36      -> Sensor 3, analogue
  - GPIO 2       -> on-board LED (status)
 =================================================================*/

#include <OneWire.h>
#include <DallasTemperature.h>

// ===================================================================
// PIN ASSIGNMENTS
// ===================================================================
#define CASIO_RX_PIN   16     // from Casio TX  (yellow, tip)
#define CASIO_TX_PIN   17     // to   Casio RX  (blue, ring) via 1N4148, BAR to board
#define ONEWIRE_PIN     4     // DS18B20 data line  (sensor 1)
#define SENSOR2_PIN    35     // analogue, ADC1_CH7
#define SENSOR3_PIN    36     // analogue, ADC1_CH0
#define LED_PIN         2     // on-board LED

// ===================================================================
// THE RADIO
//
//   0 = OFF. The configuration every reliability figure for this
//       board was earned with. Leave it here unless you are
//       deliberately testing the radio.
//
//   1 = ON, as an ACCESS POINT. The board makes its own network so
//       phones can watch the same readings the calculator is
//       recording.
//
// WHY AN ACCESS POINT AND NOT A CLIENT. A group of students will
// often have one calculator between three or four of them. The
// others follow the readings on a phone they already carry. An
// access point needs no school network, no credentials typed by a
// student, no IT department, and works on a field trip with no
// coverage at all.
//
// ADC1 ONLY. Sensor channels 2 and 3 are on GPIO 35 and 36, which
// are ADC1. ADC2 STOPS WORKING ENTIRELY while WiFi is active. That
// choice was made when this file was written and must not be undone.
// ===================================================================
#define WIFI_ENABLED 1

#define AP_SSID        "CASIO-ESP32-1"   // one per unit
#define AP_PASSWORD    ""     // 8 characters minimum, or
                              // "" for an open network
#define AP_CHANNEL     1
#define AP_MAX_CLIENTS 4

// ===================================================================
// THE STATUS PAGE
//
//   0 = radio only, no server.
//   1 = serve a status page at http://192.168.4.1/
//
// Kept separate from WIFI_ENABLED on purpose. If a byte ever goes
// missing, turning the server off without turning the radio off
// tells you which of the two was responsible.
//
// READ-ONLY by design: it observes and cannot configure, calibrate
// or control, so no phone can spoil another student's experiment.
//
// WHEN IT IS SERVED. Only while the board is waiting - idle between
// sessions, or during the long part of a sampling interval. NEVER
// between starting the sensor conversions and transmitting, and
// never in the gap between transactions, which contains the
// attention-byte handshake and has no host-wait window at all.
//
// THIS IS THE SINGLE-CORE ARCHITECTURE, PORTED UNCHANGED FROM THE
// ESP8266 WHERE IT IS PROVEN. This chip could run the server as a
// task on core 0 and remove the constraint entirely - and would then
// have two cores touching the sample history at once.
// ===================================================================
#define WEB_SERVER_ENABLED 1

// Turns a #define into a string for the page: STR(999) -> "999".
#define STR_(x) #x
#define STR(x)  STR_(x)

// ---- what the three channels are called on the page ---------------
// EDIT THESE to match what you have actually connected.
#define CH1_NAME  "Temperature"
#define CH1_UNIT  "&deg;C"
#define CH2_NAME  "Analogue GPIO35"
#define CH2_UNIT  "counts"

// Column headings for the CSV. Plain ASCII, no spaces, unit in the
// name - a spreadsheet has no room for a second row to explain
// itself.
#define CH1_CSV  "temperature_C"
#define CH2_CSV  "gpio35_counts"
#if DIAG_HEAP_ON_CH3
  #define CH3_CSV  "free_heap_x100B"
#else
  #define CH3_CSV  "gpio36_counts"
#endif

// The radio headers are included HERE, below the switches, because
// they are only wanted when WIFI_ENABLED is set - and a #if that
// appears above its own #define is simply read as zero.
#if WIFI_ENABLED
  #include <WiFi.h>
  #if WEB_SERVER_ENABLED
    #include <WebServer.h>
  #endif
#endif

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
// PROTOCOL CONSTANTS  (identical to the PICAXE version)
// ===================================================================
const uint8_t CASIO_ATTENTION = 0x15;   // calculator: "are you there?"
const uint8_t ESP32_PRESENT   = 0x13;   // our reply:  "yes, ready"
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
// It was called REQUEST_CHECKSUM_STRICT until 7 August 2026, which
// named one of the two and was quietly wrong about the other. If you
// have notes or an older sketch using the old name, this is it.
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

const uint8_t CMD_RECEIVE     = 'R';    // ":REQ..." calculator wants a value
const uint8_t CMD_SEND        = 'V';    // ":VAL..." calculator is sending one

const uint8_t VNAME_N         = 'N';    // how many sensors?
const uint8_t VNAME_A         = 'A';    // sensor 1
const uint8_t VNAME_B         = 'B';    // sensor 2
const uint8_t VNAME_C         = 'C';    // sensor 3
const uint8_t VNAME_T         = 'T';    // sampling interval, via Send(

// Byte 14 of the value packet, the sign/info byte.
//   bit 0        the magnitude is >= 1
//   bits 6 and 4 the value is NEGATIVE
const uint8_t SIGN_POSITIVE   = 0x01;
const uint8_t SIGN_NEGATIVE   = 0x51;

// The largest magnitude the value packet carries in the form used here.
constexpr int16_t NSN_MAX_VALUE = 9999;

const uint8_t  SENSOR_COUNT = 3;        // A, B and C - the released NSN-LOGR asks for all three

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
// program.
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

// ===================================================================
// THE TIMING BUDGET  -  why a slow sensor must not be read late
//
// A reading is due on a particular second and the packet must leave
// on that second. Anything the board must do BEFORE it can transmit
// has to fit in the time set aside beforehand, or the packet leaves
// late - every sample, by the same amount, invisibly.
//
// The DS18B20 is the problem child. Read it in the ordinary blocking
// way at 600 ms before the due moment, with the library's default
// 12-bit setting, and the 750 ms conversion returns 150 ms AFTER the
// due moment. The wait-until-due loop then falls straight through and
// the logger runs permanently late. This build previously did that.
//
// THE FIX IS TO SEPARATE STARTING A CONVERSION FROM COLLECTING IT.
//
//   due - offset   request a conversion, and do NOT wait for it
//   due -  50 ms   collect the result, and read the fast sensors
//   due            transmit
//
// The conversion now runs during time that was being spent waiting
// anyway, and the readings belong to a moment 50 ms before the
// timestamp rather than 600 ms before it - more accurate as well as
// more punctual.
//
// A GENERAL RULE WORTH TAKING AWAY. Prefer fast sensors where timing
// reliability matters. The DS18B20 is kept here because it is the
// clearest teaching example of a signed, sub-zero, real-world
// quantity. A build that cared only about timing would use the
// analogue and I2C channels and leave 1-Wire alone.
// ===================================================================
const uint32_t SENSOR_READ_OFFSET_MS = DS18B20_CONVERSION_MS + 150;
const uint32_t FINAL_READ_MS         = 50;

// ===================================================================
// GLOBAL STATE
// ===================================================================
HardwareSerial    CasioSerial(1);          // UART1 - a real hardware UART
OneWire           oneWire(ONEWIRE_PIN);
DallasTemperature ds18b20(&oneWire);



// ===================================================================
// LINK COUNTERS
//
// Cheap to keep, and the only evidence that survives a fault. Every
// bug found on the ESP8266 in August 2026 was found by ARITHMETIC ON
// THESE NUMBERS, not by reading code: "bytes discarded" should always
// be an exact multiple of 38, and the run that came to 38 x 43 + 26
// was the whole diagnosis. Not one fault was found by inspection, and
// not one would have been prevented by defensive code.
//
// If you add anything to this firmware, add a counter for it.
// ===================================================================
// ===================================================================
// THE SHORTEST AND LONGEST INTERVAL THIS BOARD WILL ACCEPT
// ===================================================================
// The calculator sends the interval it wants. These clamp it, because
// a device that acts on an unclamped number arriving over a wire is a
// device that can be stopped by a typing error.
//
// MIN_INTERVAL_S WAS 2 UNTIL 7 AUGUST 2026. It is now 1.
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
// BASIC program has to take the value in, write it to a list and
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
  uint32_t timeoutFinal;   // ...on the closing ACK, which is optional
  uint32_t abortedAck1;    // wrong byte where the first ACK belongs
  uint32_t abortedAck2;    // wrong byte where the second ACK belongs
  uint32_t resyncs;        // schedule fell a whole interval behind -
                           // the calculator could not keep up
  uint32_t turnLast;       // the calculator's own loop time, ms
  uint32_t turnMin;        // the fastest seen - the best estimate
  uint32_t turnMax;        // the slowest; may include human delay
  uint32_t turnCount;      // how many turnarounds have been timed
  uint32_t badAttention;   // a byte arrived that was not 0x15
  uint32_t requestPackets; // complete, checksummed request packets
  uint32_t shortPackets;   // fewer than 50 bytes arrived
  uint32_t badPreamble;    // byte 0 was not ':'
  uint32_t badChecksum;    // the calculator's own checksum failed
  uint32_t extraBytes;     // bytes beyond the 38 - a different fault
  uint32_t flushed;        // bytes cleared after something unexpected
  uint32_t csvDownloads;   // CSV files served to a GET
  uint32_t csvOtherReq;    // HEAD and anything else asking for it
  uint8_t  lastSat;        // most recent saturation mask
  uint8_t  saturatedNow;   // current saturation mask
  uint8_t  lastBadByte;    // WHAT arrived - the whole question
  uint8_t  lastBadStage;   // 1 = ACK1, 2 = ACK2, 3 = attention
};

// Empty braces zero EVERY member, however many there are. A list of
// literal zeros has to be recounted each time a counter is added,
// and miscounting it is a compile error at best and a silently
// wrong initial value at worst.
LinkStats stats = {};

// ===================================================================
// HOW LONG DOES THE CALCULATOR ITSELF TAKE?
// ===================================================================
// Added 7 August 2026. This measures something no part of this
// project could measure before: THE CASIO'S OWN LOOP TIME. Not the
// link, not this board - the calculator finishing Receive(, storing
// the value it received, writing a list element, refreshing the
// display, and coming back to ask again.
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
// them, with any transport conditioning already undone. That
// conditioning lives in exactly one function instead of being
// repeated wherever data is displayed. It is a transport detail and
// has no business appearing on a screen.
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
  int16_t  v[3];     // PHYSICAL values, signed, as the sensors read them
  uint8_t  sat;      // saturation mask: bit 0,1,2 set if that channel clamped
};

Sample   history[HISTORY_SIZE];
uint16_t historyCount = 0;     // filled slots, stops at HISTORY_SIZE
uint16_t historyHead  = 0;     // where the next sample goes

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

uint32_t lastExchangeMs = 0;   // when the calculator last spoke to us

#if WIFI_ENABLED && WEB_SERVER_ENABLED
WebServer server(80);
#endif

uint16_t timeInterval = 10;    // seconds between readings (2 - 300)
uint32_t nextSendTime = 0;     // when the next reading is DUE, in seconds
bool     firstReading = true;  // the very first reading happens at once

int16_t  physicalValue[3];     // the sensor value
uint8_t  saturatedMask = 0;    // bit 0,1,2 set if that channel was clamped
// NSN has NO status field in the value packet, so a clamp cannot be
// reported to the calculator in-band. It is reported on the WEB PAGE
// and in the CSV instead - see handle_status_page() and Sample.sat.

// Forward declarations. The Arduino IDE generates these for you, but
// stating them plainly means this file also compiles as ordinary C++
// (which is how the packet tests are run on a PC).
int16_t  scale_to_physical_1();
int16_t  scale_to_physical_2();
int16_t  scale_to_physical_3();

uint8_t  calculate_checksum(const uint8_t *packet);
uint8_t  checksum_over(const uint8_t *packet, uint8_t len);
void     read_all_sensors();
void     history_add();
const Sample& history_at(uint16_t i);
void     note_sample_request();
void     handle_status_page();
void     handle_csv_download();
void     start_slow_conversions();
void     send_nsn_value(int16_t signedValue);
int16_t  clamp_to_range(int32_t value, uint8_t channel);
void     send_description(uint8_t vname);
void     send_end_packet();
uint16_t denormalise_value(uint8_t intDigit, uint8_t dec1, uint8_t dec2,
                           uint8_t signInfo, uint8_t exponent);

// ===================================================================
// SECONDS SINCE POWER-UP
//
// The PICAXE has a 'time' variable that counts real seconds and wraps
// at 65535 (about 18 hours). millis() wraps at about 49.7 days, so
// this version has far more headroom - but it is not infinite, and a
// logger left running for seven weeks would misbehave. Worth knowing
// rather than discovering.
// ===================================================================
uint32_t nowSeconds() {
  return millis() / 1000UL;
}

// ===================================================================
// STAGE 2 of 4:  RAW READING  ->  REAL QUANTITY        *** EDIT ME ***
//
// This is where your sensors are defined. Everything above and below
// is protocol machinery that does not care what the numbers mean.
//
// Return the value in TENTHS of your unit, so one decimal place
// survives without needing any decimals in the packet:
//     23.4 degrees  ->  return 234
//     -3.7 degrees  ->  return -37
//
// The value may be NEGATIVE. That is the point of this build.
// ===================================================================

// ---- Sensor 1: DS18B20 digital thermometer ------------------------
// Chosen deliberately as the demonstrator, because it returns a
// SIGNED temperature natively. The negative number is real, not
// manufactured by subtracting a midpoint from a positive reading.
//
// Range -55 to +125 C, so -550 to +1250 in tenths 
//
// TO TEST THE NEGATIVE PATH: ice with salt stirred through it
// reaches about -10 C. That reads -100,
// genuinely below the zero point, so the sign is actually exercised
// rather than assumed. A domestic freezer at -18 C gives 3200.
//
// FOOTNOTE: the PICAXE version CANNOT do this. Its readtemp command
// is unusable at the 16 MHz clock the 9600 baud link requires, so a
// DS18B20 is off the menu there. This is a concrete example of what
// the extra capability buys.
int16_t scale_to_physical_1() {
  // NO requestTemperatures() HERE. The conversion was started by
  // start_slow_conversions() one read-offset ago, so the answer is
  // already waiting and this returns at once. See THE TIMING BUDGET.
  float celsius = ds18b20.getTempCByIndex(0);

  // The library returns -127 for a missing or faulty sensor.
  if (celsius <= -100.0f) {
    return 0;                      // treat as zero; the flag will report it
  }
  return (int16_t)lroundf(celsius * 10.0f);   // tenths of a degree
}

// ---- Sensor 2: analogue --------------------------------------------
// Placeholder: raw ADC count, centred so it can swing either way.
// Replace this with your own conversion.
int16_t scale_to_physical_2() {
  int raw = analogRead(SENSOR2_PIN);    // 0 - 4095
  return (int16_t)(raw - 2048);         // -2048 to +2047
}

// ---- Sensor 3: analogue --------------------------------------------
int16_t scale_to_physical_3() {
  int raw = analogRead(SENSOR3_PIN);    // 0 - 4095
  return (int16_t)(raw - 2048);
}


// ===================================================================
// READ ALL THREE SENSORS
//
// All three are read together, one after another with no waiting in
// between, so the three readings in a packet belong to the same
// instant. That is what makes comparing them meaningful.
// ===================================================================
// ===================================================================
// START THE SLOW SENSORS CONVERTING
//
// Called SENSOR_READ_OFFSET_MS before a reading is due, and returns
// immediately: the DS18B20 gets on with its conversion while the
// board waits for the due second, which it was going to do anyway.
// Add any other slow sensor's "begin measurement" call here.
// ===================================================================
void start_slow_conversions() {
  ds18b20.requestTemperatures();      // non-blocking; see setup()
}

// KEEP THE VALUE INSIDE WHAT THE PACKET CAN CARRY
//
// A reading beyond range is clamped AND the clamp is recorded, so the
// web page and the CSV can report it. A silent clamp is a lie the
// size of the error.
int16_t clamp_to_range(int32_t value, uint8_t channel) {
  if (value >  NSN_MAX_VALUE) { saturatedMask |= (1 << channel); return  NSN_MAX_VALUE; }
  if (value < -NSN_MAX_VALUE) { saturatedMask |= (1 << channel); return -NSN_MAX_VALUE; }
  return (int16_t)value;
}

void read_all_sensors() {
  saturatedMask = 0;

  // All three are read together, one after another with no waiting in
  // between, so the three values a sample carries belong to the same
  // instant even though they travel in three separate transactions.
  physicalValue[0] = clamp_to_range(scale_to_physical_1(), 0);
  physicalValue[1] = clamp_to_range(scale_to_physical_2(), 1);
  physicalValue[2] = clamp_to_range(scale_to_physical_3(), 2);

  stats.lastSat = saturatedMask;
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
// THE STATUS PAGE
// ===================================================================
#if WIFI_ENABLED && WEB_SERVER_ENABLED


// ESP32 has no ESP.getResetReason(). The reason is still available,
// as an enum, and it is worth showing for the same reason it was on
// the ESP8266: a board that restarts mid-session looks exactly like
// a board that hung, and every counter begins again from zero.
static const char* reset_reason_text() {
  switch (esp_reset_reason()) {
    case ESP_RST_POWERON:  return "Power on";
    case ESP_RST_EXT:      return "External reset";
    case ESP_RST_SW:       return "Software restart";
    case ESP_RST_PANIC:    return "CRASH - exception";
    case ESP_RST_INT_WDT:  return "CRASH - interrupt watchdog";
    case ESP_RST_TASK_WDT: return "CRASH - task watchdog";
    case ESP_RST_WDT:      return "CRASH - other watchdog";
    case ESP_RST_BROWNOUT: return "BROWNOUT - check the supply";
    case ESP_RST_DEEPSLEEP:return "Woke from deep sleep";
    default:               return "Unknown";
  }
}

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
//   4 Aug, stop() present, page truncated, phone refreshing hard
//        -> 241 samples, no error
//   4 Aug, stop() removed, page correct, NOBODY touching the phone
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
// has to hold the sample history and the WiFi buffers.
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
  // run look like a two-minute one. The board knows why it restarted,
  // so it says so.
  server.sendContent(F("<h2>Board</h2><table>"));
  row("Last restart", reset_reason_text(), "");
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

  server.sendContent(F("time_s," CH1_CSV "," CH2_CSV "," CH3_CSV ",clamped\r\n"));

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
// CHECKSUM  -  identical formula to the PICAXE
//
//     sum every byte from 0 to 14
//     subtract 0x3A
//     take the one's complement
//     add 1
//
// All of it in 8 bits, so it wraps round at 256 and that is intended.
// ===================================================================
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
// SEND A NORMALISED SCIENTIFIC NOTATION (NSN) NUMBER
//
// An ordinary number in scientific notation, the same form students
// meet in Year 9-10 maths: I.DDDD x 10^E. Only the sensor count and
// the occasional zero travel this way.
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
    CasioSerial.write(zeroPacket, 16);
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

  CasioSerial.write(packet, 16);
  stats.valuePackets++;
}

// ===================================================================
// SEND THE DESCRIPTION PACKET  -  50 bytes
//
// This tells the calculator WHICH variable is about to arrive. Every
// byte matters: the checksum is the constant 273 - vname, and that
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

  CasioSerial.write(packet, 50);
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
  CasioSerial.write(packet, 50);
  stats.endPackets++;
}

// ===================================================================
// RESYNCHRONISE  -  clear the line after anything unexpected
//
// Every path that gives up on a transaction must leave the line
// EMPTY. If it does not, the bytes it walked away from are read one
// at a time by the next pass of loop(), and the board spends the
// following transactions one packet behind: the byte where an ACK
// belongs turns out to be byte 5 of a request header (0x56). A single
// glitch becomes a cascade, and the Com ERROR arrives minutes later
// with nothing to connect it to.
//
// Silence is the right signal HERE, unlike the drain below, because
// we do not know how many bytes are left - that something unexpected
// happened is the whole point.
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
// The first version works from a schedule that was decided in
// advance. If one reading is late, the next is still due at its
// original time, and the lateness is absorbed rather than banked.
// This is why the PICAXE version holds +/-1 second over hours.
//
// SENSOR READ TIMING: slow conversions are STARTED one read-offset
// before the due moment and COLLECTED 50 ms before it, so the packet
// still leaves on the second and the readings are as fresh as they
// can be. See THE TIMING BUDGET above.
// ===================================================================
void wait_for_interval() {
  // The very first reading happens immediately, defining t = 0.
  // Nothing has been scheduled yet, so the conversion is started and
  // waited out here - the one place the delay is spent openly.
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
  // THE ONLY PLACE THE WEB SERVER RUNS DURING A SESSION. From
  // start_slow_conversions() onwards the board is committed to
  // transmitting on the second, and an HTTP request is not welcome.
  uint32_t convStartMs = dueMs - SENSOR_READ_OFFSET_MS;
  while ((int32_t)(millis() - convStartMs) < 0) {
    serve_web();
    delay(5);
  }
  start_slow_conversions();

  // --- 2. hold while they convert, then collect -------------------
  uint32_t readAtMs = dueMs - FINAL_READ_MS;
  while ((int32_t)(millis() - readAtMs) < 0) {
    delay(5);
  }

  read_all_sensors();

  // --- 3. hold the last few ms so the packet leaves on the second -
  while ((int32_t)(millis() - dueMs) < 0) {
    delay(1);
  }

  // Arithmetic schedule - see the note above.
  nextSendTime = nextSendTime + timeInterval;

  // If we have somehow fallen a whole interval behind, resynchronise
  // rather than sprinting to catch up.
  //
  // COUNTED SINCE 7 AUGUST 2026. This branch used to fire silently,
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

  CasioSerial.write(CASIO_ACK);

  // A TIMEOUT and a WRONG BYTE are not the same thing, and the PICAXE
  // treats them differently. Match it:
  //   timeout    -> the calculator has stopped listening. Close the
  //                 transaction politely with an END packet.
  //   wrong byte -> the calculator has abandoned the exchange (AC key,
  //                 or it sent 0x22 to abort). Send nothing further.
  if (!waitForByte(b, 2000)) { stats.timeoutAck1++;
                               TRACELN(F("ACK1 TIMEOUT")); send_end_packet(); return; }
  if (b != CASIO_ACK)        { stats.abortedAck1++; stats.lastBadByte = b; stats.lastBadStage = 1;
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
  if (b != CASIO_ACK)        { stats.abortedAck2++; stats.lastBadByte = b; stats.lastBadStage = 2;
                               TRACE(F("ACK2 got ")); TRACEHEX(b); TRACELN(F("(expected 06)"));
                               flush_line(); return; }
  TRACELN(F("ACK2 ok"));

  // == FENTON 2025 HOST-WAIT WINDOW: THE VALUE WINDOW (GAP 3) ==
  // THIS is the heart of the discovery. The calculator is sitting
  // inside Receive() waiting for a number, and it will wait - for
  // five minutes if we ask it to - without raising the COM ERROR
  // that every reference says should happen. That patience is what
  // turns a calculator into a datalogger.

  // *** THE INTERVAL WAIT BELONGS TO CHANNEL A ONLY. ***
  // A, B and C are three transactions within ONE sample. Waiting in B
  // or C as well would multiply the interval by the number of sensors
  // and stretch the time axis silently. All three sensors are read
  // together inside wait_for_interval(), so B and C return values
  // taken at the same instant as A.
  if (vname == VNAME_N) {
    send_nsn_value((int16_t)SENSOR_COUNT);
  } else if (vname == VNAME_A) {
    note_sample_request();
    wait_for_interval();          // <-- the long pause lives in here
    send_nsn_value(physicalValue[0]);
    sessionSamples++;
  } else if (vname == VNAME_B) {
    send_nsn_value(physicalValue[1]);
  } else if (vname == VNAME_C) {
    send_nsn_value(physicalValue[2]);
  } else {
    send_nsn_value(0);          // unknown variable: zero
  }

  // The closing ACK is OPTIONAL - the PICAXE does not require it
  // either. Counted separately so a timeout total of 1 never has to
  // be interpreted without knowing which wait produced it.
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
// Used to set the sampling interval from the calculator, so the
// student never has to re-flash the board to change it.
// ===================================================================
void handle_incoming() {
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
  //      drain that failed in August 2026, with a larger cushion.
  //
  // THE THIRD ONE MAY EXPLAIN A MYSTERY. Stray bytes appeared at the
  // top of loop() during August testing - 0xFF once, 0x56 another
  // time - and were never accounted for. An END packet is 45 bytes
  // of 0xFF ending in the checksum 0x56. If this delay ever came up
  // short, the bytes left behind would be exactly those two.
  // Unproven, but it fits, and the cause is removed either way.

  // THE CALCULATOR ACCEPTS ONE STOP BIT. IT DOES NOT REQUIRE TWO.
  // Both settings were tested here and both work. That confirms
  // Grindheim (2001), who reported the link is asymmetric - two stop
  // bits FROM the calculator, one TO it.
  // WHY TWO ALSO WORKS: an extra stop bit is only extra idle line. The
  // receiver has already sampled the byte and is waiting for the next
  // start bit, which simply arrives a fraction later.
 
  // It is incorrect to say that one stop bit is rejected.
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
  Serial.begin(115200);
  delay(300);

  CasioSerial.begin(9600, SERIAL_8N2, CASIO_RX_PIN, CASIO_TX_PIN);

  analogReadResolution(12);            // 0 - 4095

#if WIFI_ENABLED
  // Access point. See the note at WIFI_ENABLED, and MEASURE the heap
  // cost on this chip rather than inheriting the ESP8266's figure.
  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID,
              (sizeof(AP_PASSWORD) > 1) ? AP_PASSWORD : nullptr,
              AP_CHANNEL, false, AP_MAX_CLIENTS);

  // No modem sleep. Sleep saves power and pays for it in latency
  // spikes, and latency is what a serial link cannot afford. The
  // board is on a USB lead, not a battery.
  WiFi.setSleep(false);

  #if WEB_SERVER_ENABLED
    server.on("/", handle_status_page);
    server.on("/data.csv", handle_csv_download);
    server.onNotFound(handle_status_page);
    server.begin();
  #endif
#endif
  analogSetAttenuation(ADC_11db);      // full 0 - 3.3 V span

  ds18b20.begin();
  ds18b20.setResolution(DS18B20_RESOLUTION);
  // Do not block inside requestTemperatures(). The conversion runs
  // while we are waiting for the due second anyway.
  ds18b20.setWaitForConversion(false);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  Serial.println();
  Serial.println(F("Casio ESP32 NSN datalogger - ready"));
}

// ===================================================================
// MAIN LOOP
//
// Nothing happens until the calculator says something. The whole
// program is a reply to a question.
// ===================================================================
void loop() {
  if (!CasioSerial.available()) {
    // *** THE WEB SERVER IS NOT ALLOWED HERE DURING A SESSION. ***
    // This gap contains the attention-byte handshake, where the
    // calculator allows 0.5 to 1 second (Grindheim 2001) and the
    // October 2025 search found a pause fails. Serve a file here and
    // the attention byte goes unanswered for as long as it takes.
    // Only serve when the calculator has been quiet for longer than
    // two sampling intervals - which means no session is running.
    uint32_t quietMs = (uint32_t)timeInterval * 2000UL + 4000UL;
    if (millis() - lastExchangeMs > quietMs) serve_web();
    delay(1);
    return;
  }

  uint8_t inByte = CasioSerial.read();
  if (inByte != CASIO_ATTENTION) {
    stats.badAttention++;
    stats.lastBadByte  = inByte;
    stats.lastBadStage = 3;
    flush_line();                 // do not leave the rest for next time
    return;
  }

  note_turnaround();        // the gap since we last spoke IS the calculator
  lastExchangeMs = millis();

  digitalWrite(LED_PIN, HIGH);

  CasioSerial.write(ESP32_PRESENT);
  TRACE(F("\nATT 15  -> sent 13\n"));

  // The calculator now sends a 50-byte packet. Both kinds put the
  // command letter first and the variable name at byte 11:
  //     ":REQ..."  -> command 'R', it wants a value from us
  //     ":VAL..."  -> command 'V', it is giving us a value
  //
  // NOTE THE POSITIONS. Byte 0 is the ':' itself, so the command is
  // byte 1 and the variable name is byte 11. Getting this wrong by
  // one is an easy mistake and produces a logger that handshakes
  // perfectly and then answers the wrong question.
  // ===============================================================
  // READ THE WHOLE 50-BYTE REQUEST PACKET, THEN CHECK IT
  //
  // This replaced a two-stage "read 12 bytes, then discard 38" on
  // 5 August 2026. The idea came from the 2025 micro:bit draft,
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
  //   the August 2026 fault - almost never checksums, so this
  //   catches the condition rather than inferring it later from a
  //   byte that should have been an ACK.
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
    TRACE(F("SHORT PACKET ")); TRACELN(got);
    flush_line();
    digitalWrite(LED_PIN, LOW);
    return;
  }
  if (packet[0] != CASIO_PREAMBLE) {
    stats.badPreamble++;
    flush_line();
    digitalWrite(LED_PIN, LOW);
    return;
  }
  if (packet[REQUEST_PACKET_LEN - 1] !=
      checksum_over(packet, REQUEST_PACKET_LEN)) {
    stats.badChecksum++;
    TRACELN(F("REQUEST CHECKSUM FAILED"));
#if RX_CHECKSUM_STRICT
    flush_line();
    digitalWrite(LED_PIN, LOW);
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

  digitalWrite(LED_PIN, LOW);
  txDoneMs = millis();     // the calculator's own work starts here
}
