/*
 ===================================================================
  CASIO-ESP32-DROIDX
  A Casio FX-9750 graphing calculator and a phone, both driving a
  Bluetooth Hasbro Smart R2D2 droid through one ESP32.
  https://mikefentonnz.github.io/projects/casio-calculator-data-logger-hack.html

  (C) Michael Fenton, MRSNZ, 2026.
  Licence: Creative Commons Attribution-NonCommercial-ShareAlike 4.0
  International (CC BY-NC-SA 4.0)

  Version 1.0, 20 August 2026.
  *** RUNS ON HARDWARE. Verified on an
      FX-9750GIII and an FX-9750G Plus. ***

  ANCESTRY. The two Receive( host-wait windows this build depends on
  were found in 2007-2008 with a PICAXE 18X and an FX-9750G Plus:
    Authentic Learning Using Mobile Sensor Technology (2008)
      https://doi.org/10.5281/zenodo.19302276
    RIGEL - Learning From Life, Kuala Lumpur (2009)
      https://doi.org/10.5281/zenodo.19334228

 ===================================================================
  REQUIRED BUILD SETTINGS  -  READ THIS FIRST
 ===================================================================
  *** Tools > Partition Scheme > "Minimal SPIFFS (1.9MB APP...)" ***

  THE DEFAULT PARTITION IS TOO SMALL. It allows the sketch 1.31 MB,
  and NimBLE, WiFi, an asynchronous web server and the DroidX page
  together fill about 95 percent of it - a build with no room left to
  add anything. Minimal SPIFFS allows 1.97 MB and the same binary
  drops to roughly 62%. Nothing in the code changes. It is one
  menu, and it is worth two hundred times more than deleting every
  diagnostic message in this file.

  Board:     ESP32 Dev Module. Developed on an ESP-WROOM-32 devkit.

  Libraries: NimBLE-Arduino 2.x. Developed against 2.5.1. THE 1.x API
               WILL NOT COMPILE: setScanCallbacks, the const on
               onResult and the reason argument on onDisconnect are
               all 2.x forms.
             ESP Async WebServer, by ESP32Async.
             Async TCP, by ESP32Async. *** IT MUST BE THE ESP32Async
               FORK, PAIRED WITH THE ABOVE. *** The older me-no-dev
               AsyncTCP will not build against it.

 ===================================================================
  WHAT IT DOES
 ===================================================================
  A phone will not find the droid in its Bluetooth list. It WILL find
  this board in its WiFi list. Join the access point, open a browser,
  and DroidX appears: a block sequencer served from the chip itself.
  No app, no store, no account, no internet anywhere in the chain.

  Meanwhile the calculator goes on working.

  *** TWO CONTROL SURFACES AT ONCE, AND NEITHER KNOWS THE OTHER IS
      THERE. *** That was not designed for. It falls out of the
  architecture: loop() serves only the calculator, and everything
  that blocks lives in a task, so a WebSocket command and a Send(
  from the keypad are just two callers of the same droid_write().
  The remedy this file exists to demonstrate is what makes a second
  control surface free.

 ===================================================================
  THE WEB STACK COSTS NOTHING MEASURABLE
 ===================================================================
  WiFi and Bluetooth share ONE RADIO on an ESP32, so this was
  measured rather than assumed. Cycle time read off value B:

    access point only                    2685 ms    min B 0
    AP + server + WebSocket + 2 tasks    2682 ms    min B 0

  Three milliseconds apart, same floor, same span.

  *** THE LOW END IS THE EVIDENCE, NOT THE AVERAGE. *** A value below
  the length of the host wait can only be produced by the app-mode
  hold firing DURING it. Re-run that probe if you change the radio
  settings: if B stops reaching low values, or its peak climbs,
  coexistence is costing you.

 ===================================================================
  THE SERVER IS ASYNCHRONOUS, AND THAT IS NOT A PREFERENCE
 ===================================================================
  The Arduino core's built-in WebServer is synchronous and must be
  serviced from loop(). loop() is EXACTLY where the host wait blocks,
  so the page would be unreachable for as long as the calculator was
  thinking.

  That is the same fault this file exists to close, one layer up:
  anything that must respond while the calculator is thinking cannot
  live in loop(). The async server is the web-layer form of the same
  remedy, and the sequence runner below is a task for the same
  reason.

 ===================================================================
  THE DROIDX PAGE IS STORED GZIPPED
 ===================================================================
  droidx_page.h holds DroidX.html compressed - 40.6 kB of HTML in
  12.6 kB of flash - and it is served with Content-Encoding: gzip,
  which every browser that can join a WiFi network understands.

  *** TO CHANGE THE PAGE: edit DroidX.html, then run make_page.py
      beside it. *** Both live in the DroidX folder of the development
  tree, and beside this sketch in the published repository; the script
  handles either. droidx_page.h is generated and must never be edited
  by hand.

 ===================================================================
  WHAT THIS IS
 ===================================================================
  A Casio FX-9750 graphing calculator drives a Bluetooth toy droid
  through an ESP32, and reads three quantities back from it. The
  calculator needs no modification and no firmware change. The
  interface circuit is the same four components used by every other
  platform in this project.

  ONE VALUE PER TRANSACTION. Three Receive( calls per cycle, each
  carrying one ordinary signed number in normalised scientific
  notation. Nothing is packed and nothing is combined.

    Receive(N)   how many quantities        -> 3   (once, outside the loop)
    Receive(A)   IR proximity               reflected signal, counts
    Receive(B)   app-mode hold age         tenths of a second
    Receive(C)   head position              0, 1, 2, or 3 in transit

  Any of A, B or C reads -999 when the value is not available. -99.9
  of anything here is not a plausible reading, so a fault cannot be
  mistaken for a measurement.

 ===================================================================
  WHY THIS FILE EXISTS
 ===================================================================
  NOT to demonstrate outbound control. That was established in 2008.
  Casio-IMC-14M2.bas already switches three actuators from the
  keypad, and nothing here claims otherwise.

  It exists because the droid is the FIRST endpoint this interface
  has met that has a timebase of its own. Every previous endpoint -
  heater, fan, vent, LED, relay - was entirely indifferent to how long
  the board sat inside a host-wait window.

  WHAT THE PERIODIC PACKET ACTUALLY DOES - MEASURED August 2026.
  The reverse-engineered work calls 0x50 0x8D a keepalive, and the
  reference controller sends it every 2.0 s. IT IS NOT A KEEPALIVE.
  Nothing disconnects when it stops. It is an APP-MODE HOLD: it tells
  the toy that an application is driving, and the toy suspends its own
  behaviour for as long as the holds keep arriving. BISECTED
  August 2026: the toy's own timeout is 5.0 s. At 5000 ms it stays
  quiet for minutes. At 5100 ms it lapses every cycle. Throughout:
  LINK DROPS 0, IR telemetry still arriving, calculator commands still
  obeyed.

  AND THE HOLD RECLAIMS MID-ROUTINE. At 5100 ms the toy begins a
  routine - one LED flash, or the first note of a whistle - and STOPS
  when the next hold arrives. So a hold does not merely prevent the
  toy from starting; it cancels what the toy has already begun. That
  is why 6000 ms sounded like a whole whistle and 5100 ms sounds like
  a stutter: what you hear is the OVERSHOOT, roughly (period - 5.0 s)
  of self-driving before the hold takes it back.

  The endpoint has a timebase of its own; an unbounded host wait starves
  whatever is serviced from the same loop; what is lost when it
  starves is control of the toy's behaviour, not the connection. A
  fault that leaves the link up and the readings flowing is HARDER to
  see than one that disconnects, which is the whole reason value B
  exists.

    THE LIMITATION  an unbounded host wait starves any endpoint whose
                    periodic packet is serviced from the same loop.
    THE FIX         move that packet to an independent task. The
                    calculator's transaction time then stops being a
                    constraint at all.

  BOTH ARE SELECTABLE HERE. See HOLD_IN_LOOP below.

  *** VALUE B IS THE INSTRUMENT FOR THIS. *** It is the age of the
  last app-mode hold at the moment its packet is built. Under the remedy
  it stays small however long the calculator takes. Under the fault
  it cannot fall below the length of the host wait, because nothing
  can have fired during it.

  B IS COMPUTED FRESH AT ITS OWN TRANSMISSION, not captured with the
  others. A and C describe the droid and belong to one instant; B
  describes the board, and taking it fresh means every cycle gives an
  independent observation rather than a repeat of A's.

 ===================================================================
  A FAULT MUST NEVER RESEMBLE A RESULT
 ===================================================================
  A dropped Bluetooth link is INVISIBLE from the calculator. The
  machine goes on accepting keypresses and goes on being acknowledged
  by a board with nothing on the other end.

  This build has no status field, so the values carry the news
  themselves:

    -999 in A and C     the droid is not connected, or has never
                        reported. All of them go together, which is
                        unmistakable on the screen.
    -999 in B           no app-mode hold has ever been sent.
    B climbing          the hold is being starved. Past about 5 s the
                        toy stops obeying and starts performing, with
                        the link still up and A and C still true.
    C reading 3         the head is between positions, not at one.

  A value that cannot be a real reading is proof of a fault. That is
  the same principle as the unused bands elsewhere in this project.

 ===================================================================
  RANGE CHECKING BELONGS ON THE CALCULATOR
 ===================================================================
  The student types the value, so the calculator checks it. See the
  companion program below: it holds the maximum for each parameter
  and refuses to send anything outside it.

  THE BOARD STILL CHECKS. Both. A device that acts on an unclamped
  number arriving over a wire is a device that can be driven into the
  stairs by a typing error, and the board cannot assume the program
  at the other end is the one printed here. But with the calculator
  doing the user-facing check, a refusal at the board becomes a
  should-never-happen rather than routine traffic, and it is reported
  on the serial monitor rather than costing a transaction.

 ===================================================================
  THE TWO-STEP COMMAND MENU
 ===================================================================
  The parameter is chosen first and remembered, then the value is
  sent. The calculator can only send a bare number, so the label goes
  first. That is the right shape here because the toy's own protocol
  is OPCODE THEN PAYLOAD. The menu IS the opcode.

    1  head position     0 left, 1 centre, 2 right
    2  drive motor       0 stop, 1 forward, 2 backward
    3  cam position      0 to 8
    4  LED red           0 to 100 percent
    5  LED blue          0 to 100 percent
    6  sound             0 to 174
    7  LED sequence      0 to 264
    8  motion sequence   0 to 477
    9  stop everything   0

  *** POWER-DOWN IS DELIBERATELY ABSENT. *** A typing error should
  not end the lesson.

 ===================================================================
  HARDWARE - the interface is UNCHANGED. No droid-specific parts.
 ===================================================================
  - GPIO 16 (RX) <- from Casio TX  [TIP of 2.5mm TRS, YELLOW]
                    via 10k IN SERIES, 4.7k pull-up to 3V3, and a
                    1N5711 small-signal Schottky from the pin to 3V3,
                    BAND toward 3V3.
  - GPIO 17 (TX) -> to Casio RX    [RING of 2.5mm TRS, BLUE]
                    via a 1N4148, BAR (cathode) TOWARD THE ESP32.
  - GND          -> Casio GND      [SLEEVE, BLACK]

  THE 10k IS IN SERIES ONLY. Nothing goes from the pin to 0 V: that
  makes a divider, and a divider drops a GIII's 2.75 V mark to about
  1.8 V. See INTERFACE-universal-wiring.md.

  THE DIODE, NOT A SERIES RESISTOR, on the transmit side. It makes
  the output open-drain so the board can only pull the line low and
  the calculator raises it with its own pull-up. That is what lets
  one cable serve a 3.3 V GIII and a 5 V G Plus alike.

  THE PULL-UP IS NOT OPTIONAL. Between transfers the calculator's
  port is high impedance and the line falls to 0 V. Serial idles
  HIGH, so a board already listening reads a permanent break and
  frames junk until the calculator wakes its port.

 ===================================================================
  CASIO BASIC COMPANION PROGRAMS
 ===================================================================
  --- Program 1: droid telemetry ---
   ClrText
   Receive(N)
   Lbl 1
   Receive(A)
   Receive(B)
   Receive(C)
   ClrText
   Locate 1,1,"IR:"
   Locate 6,1,A
   Locate 1,2,"HOLD AGE:"
   Locate 12,2,B
   Locate 1,3,"HEAD:"
   Locate 7,3,C
   Locate 1,5,"-999 = NO DROID"
   Goto 1

   *** ALL Receive( CALLS BEFORE ANY DISPLAY WORK. ***
   On an FX-9750G Plus a program that draws between consecutive
   Receive( calls will fail, because the next Receive( begins while
   the machine is still drawing. Read all three, then draw.

  --- Program 2: two-step remote control, WITH RANGE CHECKING ---
   ClrText
   Locate 1,1,"1HEAD 2DRIVE 3CAM"
   Locate 1,2,"4RED 5BLUE 6SOUND"
   Locate 1,3,"7LEDSQ 8MOTSQ 9STOP"
   Locate 1,4,"Select 1-9?"
   ?->P
   If P<1 Or P>9
   Then "BAD PARAMETER"
   Stop
   IfEnd
   0->M
   If P=1 Or P=2
   Then 2->M
   IfEnd
   If P=3
   Then 8->M
   IfEnd
   If P=4 Or P=5
   Then 100->M
   IfEnd
   If P=6
   Then 174->M
   IfEnd
   If P=7
   Then 264->M
   IfEnd
   If P=8
   Then 477->M
   IfEnd
   Locate 1,5,"Value 0 to"
   Locate 12,5,M
   ?->V
   If V<0 Or V>M
   Then "OUT OF RANGE"
   Stop
   IfEnd
   Send(P)
   Send(V)
   Locate 1,6,"Sent"

   THE CHECK IS THE LESSON. The student is told the range before
   typing, and a value outside it never leaves the calculator.

 ===================================================================
  WARNING
 ===================================================================
  NEVER connect mains electricity to the calculator, to this board,
  or to any wiring attached to either. The ESP32 is NOT 5 V tolerant.

  *** THE DRIVE MOTOR RUNS UNTIL IT IS STOPPED. *** Keep the droid on
  the floor and clear of stairs. Parameter 9 stops everything.

 ===================================================================
  THE DROID SIDE IS SOMEONE ELSE'S WORK
 ===================================================================
  UUIDs, opcodes and index ranges are taken from
  github.com/elitistphoenix/r2d2-robot-apps, which reverse engineered
  them from the decompiled Assembly-CSharp.dll of the original
  manufacturer's Android app. Nothing about the toy protocol is
  claimed as this project's work. Not affiliated with or endorsed by
  Hasbro, Disney or Lucasfilm.

  REQUIRES NimBLE-Arduino 2.x (developed against 2.5.1). The 1.x API
  will NOT compile.
 ===================================================================
*/

#include <Arduino.h>
#include <NimBLEDevice.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "droidx_page.h"

// ===================================================================
// THE SWITCH THIS FILE IS ABOUT
// ===================================================================
//   0 = THE FIX. The app-mode hold on an independent FreeRTOS
//       task. It fires whether or not the board is blocked in a
//       host wait.
//   1 = THE FAULT.  The hold serviced from loop(). Raise
//       the telemetry pause (console: p 3000), watch B climb.
//
// SET THIS TO 1 ONLY TO DEMONSTRATE THE FAILURE. Leave it at 0.
// ===================================================================
#define HOLD_IN_LOOP 0

// ===================================================================
// SET ENABLE_WIFI_AP to 1 = ON
// ===================================================================
#define ENABLE_WIFI_AP 1
const char AP_SSID[]     = "DroidX";
const char AP_PASSWORD[] = "droidx1234";   // WPA2 needs 8 characters

// ===================================================================
// THE WIRE FORMAT - line-based, and deliberately not JSON
//
//   in:   RUN\n<hex>|<ms>|<mirror>\n...      a whole program
//         NOW <hex>|<mirror>                  one command
//         HALT                                stop a run
//   out:  STEP <i>            as each step begins
//         DONE                the run finished
//         HALT <why>          it stopped early
//         TEL <ir> <ka> <head> <link>
//
// Hand-parsing nested JSON on a part already running a Bluetooth
// stack, a web server and a calculator link is fragile, and a JSON
// library bought to read four fields is a dependency bought for
// nothing. It is also easier for a learner to read on the wire.
// ===================================================================
#define MAX_STEPS      64
#define MAX_TX_BYTES    8
#define MAX_MIRROR     24

struct SeqStep {
  uint8_t  tx[MAX_TX_BYTES];
  uint8_t  txLen;              // 0 = a Wait: pause and transmit nothing
  uint32_t ms;
  char     mir[MAX_MIRROR];
};
SeqStep  g_seq[MAX_STEPS];
volatile uint16_t g_seqCount = 0;
volatile bool     g_seqRun   = false;
volatile bool     g_seqAbort = false;

AsyncWebServer server(80);
AsyncWebSocket  ws("/ws");
TaskHandle_t    g_runnerTask = nullptr;
TaskHandle_t    g_telTask    = nullptr;

// ===================================================================
// PINS - Casio only. Nothing else is attached.
// ===================================================================
#define CASIO_RX_PIN    16
#define CASIO_TX_PIN    17
#define BUILTIN_LED      2

#define DEBUG_TRACE 1
#if DEBUG_TRACE
  #define TRACE(x)    Serial.print(x)
  #define TRACELN(x)  Serial.println(x)
  #define TRACEHEX(x) do { if ((x) < 16) Serial.print('0'); \
                           Serial.print((x), HEX); } while (0)
#else
  #define TRACE(x)
  #define TRACELN(x)
  #define TRACEHEX(x)
#endif

// ===================================================================
// CASIO PROTOCOL CONSTANTS (identical to every other platform)
// ===================================================================
const uint8_t CASIO_ATTENTION = 0x15;
const uint8_t DEVICE_PRESENT  = 0x13;
const uint8_t CASIO_ACK       = 0x06;
const uint8_t CASIO_PREAMBLE  = 0x3A;
const uint8_t CMD_RECEIVE     = 'R';
const uint8_t CMD_SEND        = 'V';

const uint8_t VNAME_N = 'N';   // how many quantities?
const uint8_t VNAME_A = 'A';   // IR proximity
const uint8_t VNAME_B = 'B';   // app-mode hold age, tenths of a second
const uint8_t VNAME_C = 'C';   // head position

const uint8_t REQUEST_PACKET_LEN = 50;
const uint8_t VALUE_PACKET_LEN   = 16;

// Byte 13 of the value packet, the sign/info byte.
//   bit 0        the magnitude is >= 1
//   bits 6 and 4 the value is NEGATIVE
const uint8_t SIGN_POSITIVE = 0x01;
const uint8_t SIGN_NEGATIVE = 0x51;
const uint8_t SIGN_NEG_MASK = 0x50;

constexpr int16_t NSN_MAX_VALUE = 9999;
const uint8_t QUANTITY_COUNT = 3;

// The value a quantity carries when it is NOT AVAILABLE. -99.9 of
// anything in this build is not a plausible reading.
constexpr int16_t NOT_AVAILABLE = -999;

// ===================================================================
// DROID BLE PROTOCOL - from elitistphoenix/r2d2-robot-apps
// ===================================================================
const char DROID_SERVICE_UUID[] = "DAB91435-B5A1-E29C-B041-BCD562613BE4";
const char DROID_WRITE_UUID[]   = "DAB91383-B5A1-E29C-B041-BCD562613BE4";
const char DROID_NOTIFY_UUID[]  = "DAB91382-B5A1-E29C-B041-BCD562613BE4";
const char DROID_NAME_1[]       = "Kipps";
const char DROID_NAME_2[]       = "2ndHeroD";

// app -> toy
const uint8_t OP_PLAY_AUDIO   = 0x10;
const uint8_t OP_MOTOR1_CAM   = 0x12;
const uint8_t OP_HEAD_GOTO    = 0x13;
const uint8_t OP_MOTOR2_RUN   = 0x14;
const uint8_t OP_LED_DUTY     = 0x15;
const uint8_t OP_PLAY_SEQ     = 0x17;
const uint8_t OP_STOP_SEQ     = 0x18;
const uint8_t OP_REQ_INPUT    = 0x20;

// toy -> app
const uint8_t RX_IR_DISTANCE  = 0x16;
const uint8_t RX_INPUT_STATE  = 0x20;

const uint8_t APP_MODE_HOLD[2]    = { 0x50, 0x8D };
// POWER_DOWN would be { 0x50, 0x91 }. Deliberately not used.

const uint8_t SEQ_LED   = 0;
const uint8_t SEQ_MOTOR = 1;

const int16_t MAX_AUDIO_INDEX   = 174;
const int16_t MAX_LEDSEQ_INDEX  = 264;
const int16_t MAX_MOTSEQ_INDEX  = 477;
const int16_t MAX_CAM_POSITION  = 8;

// The head is a two-bit reported field. Its codes are NOT the
// commanded codes: established on hardware by commanding each
// position and watching the head move.
//
//   reported   physical              commanded
//      0       RIGHT                 2
//      1       LEFT                  0
//      2       CENTRE                1
//      3       IN TRANSIT, both contacts made
//
// Left and right here are as seen by someone facing the droid, which
// is also what the commanded codes mean. The mapping was derived from
// the reported STATE CODES rather than from which way the head was
// seen to move, so it is unaffected by how the directions are named.
//
// The table translates back into the commanded convention so that a
// student who types 1 sees 1. A display disagreeing with the keypad
// would be a coordinate system masquerading as a fault.
//   0 LEFT   1 CENTRE   2 RIGHT   3 in transit
const uint8_t HEAD_REPORT_TO_COMMAND[4] = { 2, 0, 1, 3 };

// ===================================================================
// TIMING
// The reference controller sends 0x50 0x8D every 2.0 s and calls it a
// keepalive. MEASURED August 2026: it is not one. Nothing
// disconnects when it stops. It is an APP-MODE HOLD, and what lapses
// is the toy's willingness to stay out of the way.
//
//   1500 ms  default. The toy sits quietly and does as it is told.
//   5000 ms  THE LIMIT. Quiet for minutes on end.
//   5100 ms  LAPSES EVERY CYCLE, but only just: one LED flash or the
//            first note, cut off when the next hold arrives.
//   6000 ms  a full second of self-driving - a whole whistle.
//
// The toy's own timeout is 5.0 s and the edge is a STEP, not a band:
// once the period exceeds it, EVERY hold is late by the same amount,
// so the toy lapses every cycle rather than sometimes.
//
// What you hear is the OVERSHOOT, about (period - 5.0 s) of
// self-driving, because a hold CANCELS a routine already in progress
// rather than merely preventing the next one.
//
// LINK DROPS 0 and commands still obeyed at every setting tested.
//
// So this period sets BEHAVIOUR, not survival. There is no deadline
// to miss, which is exactly why the failure needed an instrument.
// ===================================================================
// *** RUNTIME SETTABLE. See the serial console below. *** It stays a
// variable because it is the demonstration: the console stretches it
// live and the class watches the toy start driving itself.
volatile uint32_t g_holdPeriodMs = 1500;
const uint32_t SCAN_SECONDS        = 5;
const uint32_t RECONNECT_PAUSE_MS  = 2000;
const uint32_t CONNECT_TIMEOUT_MS  = 10000;
const uint8_t  INPUT_POLL_EVERY    = 2;   // poll input every N holds

// ===================================================================
// THE TELEMETRY PAUSE - held inside the value window of A ONLY
//
// *** MEASURED August 2026. IT DEFAULTS TO ZERO, AND THAT IS THE
//     RESULT OF AN EXPERIMENT RATHER THAN AN ASSUMPTION. ***
//
// It was originally 1200 ms, put there expecting an FX-9750G Plus to
// need cover for its drawing time. Stepped down on a G Plus through
// 1200, 800, 500, 300, 150, 0:
//
//     cycle = 1539 + 0.998 x pause   milliseconds
//     residuals within +/-2 ms at every setting
//     zero short packets, bad preambles, bad checksums, bad attention
//     bytes, timeouts or flushed bytes AT ANY SETTING INCLUDING ZERO
//     140 consecutive steps, no outlier
//
// THE PAUSE WAS INSURANCE AGAINST A FAILURE THIS PROGRAM CANNOT HAVE.
// Finding 10 is that a G Plus fails when a Receive( begins while the
// machine is still drawing, and it is resolved by reading every value
// BEFORE displaying anything. The companion program already does that.
// The fit is linear right down to zero with no threshold anywhere,
// which is what you see when something was never doing any work.
//
// Removing it takes the cycle from 2736 ms to 1539 ms, a 44% saving.
// 1539 ms is the irreducible cycle on that machine for three
// transactions plus its own interpretation and redraw.
//
// *** BUT IT IS NOW THE TEACHING CONTROL. *** At zero there is no host
// wait, so the finding is not demonstrable: "value B below the length
// of the wait" has nothing to be below. Set it to 1200 or more to
// re-create the wait and show the mechanism. On the WEB build the
// console does this live: p 1200.
//
// *** TO DEMONSTRATE THE FAULT: HOLD_IN_LOOP 1 AND a pause of
//     3000. Value B climbs past 30 and the droid drops. ***
//
// A program that DOES interleave display work between Receive( calls
// will still need a pause. This result is about this program's
// structure, not about the machine.
// ===================================================================
volatile uint32_t g_telemetryPauseMs = 0;   // console: p <ms>

// ===================================================================
// STATE
// ===================================================================
// Captured together, inside A's window, so they describe one instant.
int16_t snapIR   = NOT_AVAILABLE;
int16_t snapHead = NOT_AVAILABLE;

// Two-step Send( menu. 0 = waiting for a parameter number.
uint8_t g_pending_param = 0;

uint8_t g_led_red_pct  = 0;
uint8_t g_led_blue_pct = 0;

// Telemetry as last reported BY THE DROID. Written in the NimBLE host
// task, read in loop(). Aligned 16-bit stores are atomic on this
// part, so volatile is sufficient and the notify path takes no lock.
volatile int16_t g_ir_proximity  = NOT_AVAILABLE;
volatile int16_t g_head_position = NOT_AVAILABLE;
volatile bool    g_irEverSeen    = false;
volatile bool    g_headEverSeen  = false;

volatile uint32_t g_lastHoldMs   = 0;
volatile bool     g_holdEverSent = false;
volatile bool     g_linkUp            = false;
volatile bool     g_suppressHold = false;
// Set when the toy announces it has chosen its own mode. Cleared when
// the next app-mode hold takes it back. NOT wired to the calculator's
// three values: those stay TRUE while this is set, because they are.
// The droid whistling is the loudest signal in the room and does not
// need the telemetry to lie on its behalf.
volatile bool     g_toyChoseOwnMode   = false;

TaskHandle_t g_linkTask = nullptr;
NimBLEClient* g_client  = nullptr;
NimBLERemoteCharacteristic* g_writeChr  = nullptr;
NimBLERemoteCharacteristic* g_notifyChr = nullptr;

// 4-bit grey code -> major cam position, from the reference. 0 is
// CAM_INVALID and is a REAL state: the cam is between detents.
// Kept for the serial monitor; the cam is not sent to the calculator
// in this build.
const uint8_t GREYCODE_TO_CAM[16] =
  { 1, 0, 0, 2, 0, 4, 3, 0, 0, 8, 7, 0, 5, 0, 0, 6 };

// ===================================================================
// COUNTERS
// Every fault found in this project in August 2026 was found by
// arithmetic on numbers like these. If you add anything, add a
// counter.
// ===================================================================
struct LinkStats {
  uint32_t valuePackets;
  uint32_t endPackets;
  uint32_t requestPackets;
  uint32_t shortPackets;
  uint32_t badPreamble;
  uint32_t badChecksum;
  uint32_t badAttention;
  uint32_t timeouts;
  uint32_t flushed;
  uint32_t commandsApplied;
  uint32_t commandsRejected;
  uint32_t holdsSent;
  uint32_t linkDrops;
  uint32_t notifications;
  uint32_t connectAttempts;
  uint32_t irCheckFail;
  uint32_t modeChanges;        // app-mode hold lapsed, toy went freelance
  uint32_t casioCycles;        // completed Receive(A) transactions
  int      lastDropReason;
  uint32_t lastDropHoldMs;  // the period in force when it dropped
  uint8_t  lastBadByte;
  uint8_t  lastBadStage;   // 1 = ACK1, 2 = ACK2, 3 = attention
};
LinkStats stats = {};

HardwareSerial CasioSerial(2);

// ===================================================================
// FORWARD DECLARATIONS
// ===================================================================
void     send_nsn_value(int16_t signedValue);
int16_t  decode_signed_value(const uint8_t *packet);
void     send_description(uint8_t vname);
void     send_end_packet();
uint8_t  calculate_checksum(const uint8_t *packet);
uint8_t  checksum_over(const uint8_t *packet, uint8_t len);
uint16_t flush_line();
bool     waitForByte(uint8_t &b, uint32_t timeoutMs);
void     handle_receive(uint8_t vname);
void     handle_incoming();
void     applyRemoteValue(int16_t value);
bool     droid_write(const uint8_t *data, size_t len, const char *what);
void     droid_service_hold();
static bool droid_scan_and_connect();
int16_t  hold_age_now();

// ===================================================================
// THE TURNAROUND DELAY - a 2007 lesson, relearned twice
//
// The calculator does not switch from transmitting to listening
// instantly. Reply into that turnaround and the first bits land while
// its port is still changing direction: it mishears the byte and
// answers 0x22.
//
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
// SENDING A PACKET - one bulk write, the UART sets the spacing
//
// The FX-9750G Plus needs about one bit period of idle BETWEEN bytes.
// 8N2 supplies it in the framing, so nothing is added here.
//
// *** IF YOU CHANGE THE FRAMING TO 8N1, THIS ROUTINE MUST PACE. ***
// One bit period at 9600 baud is 104 us. Use 250 us.
// ===================================================================
static void casio_send_packet(const uint8_t *buf, size_t len) {
  turnaround();
  CasioSerial.write(buf, len);
}

// ===================================================================
// ===================================================================
//  THE RADIO
//
//  Everything about the Bluetooth link lives in this block and in the
//  link task. Nothing above or below it knows the radio exists.
//
//  *** THE LINK TASK OWNS THE LINK. *** Scanning, connecting,
//  reconnecting, the app-mode hold and the input poll are ALL in one
//  FreeRTOS task. loop() does none of it. Putting every blocking
//  operation in one place is what makes the remedy true rather than
//  merely intended.
//
//  WHY A TASK AND NOT esp_timer: an esp_timer callback runs on the
//  shared system timer task, on a small stack, and MUST NOT BLOCK. A
//  NimBLE write takes the host mutex and can block; a reconnection
//  blocks for seconds. Either one in a timer callback stalls every
//  other timer in the system, and that fault would present as
//  something else entirely.
// ===================================================================
// ===================================================================

class DroidClientCallbacks : public NimBLEClientCallbacks {
  void onConnect(NimBLEClient *c) override {
    (void)c; g_linkUp = true; TRACELN(F("[BLE] connected"));
  }
  void onDisconnect(NimBLEClient *c, int reason) override {
    (void)c;
    g_linkUp = false; g_writeChr = nullptr; g_notifyChr = nullptr;
    stats.linkDrops++;
    stats.lastDropReason    = reason;
    stats.lastDropHoldMs = g_holdPeriodMs;
    // SAY SO, AND SAY UNDER WHAT CONDITIONS. A bisection is only as
    // good as its record of what was in force when it failed.
    //   0x13  the toy terminated. An application watchdog, and the
    //         threshold is a real property of its firmware.
    //   0x08  BLE supervision timeout. The link layer gave up and the
    //         toy never decided anything. A DIFFERENT FINDING.
    TRACE(F("[BLE] *** DISCONNECTED, reason 0x")); Serial.print(reason, HEX);
    TRACE(F(" at hold period ")); TRACE(g_holdPeriodMs);
    TRACE(F(" ms, suppressed=")); TRACE(g_suppressHold ? 1 : 0);
    TRACELN(F(" ***"));
  }
  void onConnectFail(NimBLEClient *c, int reason) override {
    (void)c; g_linkUp = false;
    TRACE(F("[BLE] connect failed, reason ")); TRACELN(reason);
  }
};
static DroidClientCallbacks g_clientCB;

// ===================================================================
// THE NOTIFY PATH
//
// Runs in the NimBLE HOST TASK. It parses and stores and does NOTHING
// ELSE. No waiting, no writing. IR packets arrive as a flood while
// the toy is awake; input state arrives only when asked for.
// ===================================================================
static void notifyCB(NimBLERemoteCharacteristic *chr,
                     uint8_t *data, size_t len, bool isNotify) {
  (void)chr; (void)isNotify;
  if (len < 1) return;
  stats.notifications++;

  switch (data[0]) {

    case RX_IR_DISTANCE:
      // ambient at [3],[4]; lit at [5],[6]; both little-endian.
      //
      // BYTES 1-2 CARRY A THIRD VALUE THE REFERENCE PARSER DISCARDS.
      // Established on hardware across 17 frames from four captures
      // and three connections:
      //
      //     bytes 1-2 + (lit - ambient) == 4095, exactly, every time.
      //
      // It is the toy's own computed proximity, inverted against
      // 12-bit full scale. It carries no new information, but it is a
      // FREE PER-FRAME CONSISTENCY CHECK on someone else's packet,
      // and it is used as one below.
      if (len >= 7) {
        int32_t ambient = (int32_t)data[3] | ((int32_t)data[4] << 8);
        int32_t lit     = (int32_t)data[5] | ((int32_t)data[6] << 8);
        int32_t own     = (int32_t)data[1] | ((int32_t)data[2] << 8);
        int32_t prox    = lit - ambient;      // the REFLECTED signal, SIGNED

        // The toy's own answer must agree with ours. If it does not,
        // this frame is malformed or the parse has drifted, and it is
        // knowable NOW rather than days later.
        if (own + prox != 4095) { stats.irCheckFail++; break; }

        // *** NO CLAMP AT ZERO. *** With nothing in front of the
        // droid the difference sits within a few counts and crosses
        // zero on noise. Clamping negatives away would turn "nothing
        // is there, and I am still measuring" into a flat 0, which is
        // also what a broken parse returns. The two must not look
        // alike, and the value packet carries the sign.
        if (prox >  NSN_MAX_VALUE) prox =  NSN_MAX_VALUE;
        if (prox < -NSN_MAX_VALUE) prox = -NSN_MAX_VALUE;
        g_ir_proximity = (int16_t)prox;
        g_irEverSeen   = true;
      }
      break;

    case RX_INPUT_STATE:
      if (len >= 3) {
        uint16_t raw  = (uint16_t)data[1] | ((uint16_t)data[2] << 8);
        uint8_t  head = (raw >> 7) & 0x03;
        g_head_position = (int16_t)HEAD_REPORT_TO_COMMAND[head];
        g_headEverSeen  = true;
        // The cam is decoded for the serial monitor only. It is not
        // one of the three values, and it is NOT combined with the
        // head into a single number.
        TRACE(F("[IN] head ")); TRACE(g_head_position);
        TRACE(F("  cam "));     TRACE(GREYCODE_TO_CAM[(raw >> 3) & 0x0F]);
        TRACE(F("  button "));  TRACELN(raw & 0x01);
      }
      break;

    // ===============================================================
    // THE TOY NARRATES ITSELF, IN PLAIN ASCII
    // Established on hardware August 2026
    // ===============================================================
    // Opcodes 0x60, 0x61 and 0x62 carry NUL-terminated text: debug,
    // log and error. The reference lists them and prints them only in
    // verbose mode, as noise. They are not noise. Observed:
    //
    //   "LVD Sample"       the toy sampling its own battery
    //   "NewModeIdx:10"    THE TOY HAS CHOSEN A MODE FOR ITSELF
    //   "MODE SELECTED"    and committed to it
    //   "Queue overflow!"  an ERROR from its own firmware
    //   "Mic NOT Clear"    its microphone gate
    //
    // *** THIS IS WHERE THE APP-MODE LAPSE ANNOUNCES ITSELF. *** Not
    // on 0x50, where the reference's toy-to-app enum would lead you to
    // look, and where I expected it.
    case 0x60: case 0x61: case 0x62: {
      char txt[48]; size_t n = 0;
      for (size_t i = 1; i < len && n < sizeof txt - 1; i++) {
        if (data[i] == 0) break;
        txt[n++] = (char)data[i];
      }
      txt[n] = 0;
      const char *kind = (data[0] == 0x62) ? "ERROR"
                       : (data[0] == 0x61) ? "log" : "debug";
      Serial.print(F("[droid ")); Serial.print(kind);
      Serial.print(F("] ")); Serial.println(txt);

      // 0x50 0x8D is NOT a keepalive. It is an APP-MODE HOLD: it tells
      // the toy an app is still driving. Let it lapse and the toy
      // picks a mode for itself and runs its own idle routines, WHILE
      // STAYING CONNECTED. Bisected August: the toy's timeout is
      // 5.0 s. Quiet at 5000, lapses every cycle at 5100.
      //
      // THE HOLD DOES NOT BUY CONTROL. IT BUYS SOLE OCCUPANCY.
      // Verified August under a permanent lapse: commands are still
      // accepted, telemetry still arrives, the link stays up. The
      // command channel and the app-mode claim are independent, so
      // there is no symptom to find - only a droid doing things
      // nobody asked for.
      if (strstr(txt, "NewModeIdx")) {
        stats.modeChanges++;
        g_toyChoseOwnMode = true;
        Serial.println(F("  *** APP-MODE HOLD LAPSED. The toy is driving itself. ***"));
        Serial.println(F("  *** Commands still work; it is talking over you.     ***"));
      }
      break;
    }

    // Anything still unaccounted for. Safe to log unthrottled: this
    // arm excludes the IR flood and the input-state replies.
    default:
      Serial.print(F("[UNHANDLED] op 0x")); Serial.print(data[0], HEX);
      Serial.print(F(" len ")); Serial.print((int)len); Serial.print(F("  "));
      for (size_t i = 0; i < len; i++) {
        if (data[i] < 16) Serial.print('0');
        Serial.print(data[i], HEX); Serial.print(' ');
      }
      Serial.println();
      break;
  }
}

// ===================================================================
// WRITE ONE COMMAND TO THE TOY
//
// WITHOUT RESPONSE. The reference controller uses response=False on
// every write and NimBLE's writeValue defaults to the same. Do not
// "improve" this to a write with response.
//
// A write into a dead link RETURNS FALSE AND IS NOT BUFFERED. A
// buffered command executed on reconnection is the apparatus doing
// something plausible at the wrong moment.
// ===================================================================
bool droid_write(const uint8_t *data, size_t len, const char *what) {
  if (!g_linkUp || g_writeChr == nullptr) {
    TRACE(F("[BLE] link down, DISCARDED: ")); TRACELN(what);
    return false;
  }
  bool ok = g_writeChr->writeValue(data, len, false);
  TRACE(F("[BLE] "));
  for (size_t i = 0; i < len; i++) { TRACEHEX(data[i]); TRACE(F(" ")); }
  TRACE(ok ? F(" <- ") : F(" FAILED <- "));
  TRACELN(what);
  return ok;
}

void droid_service_hold() {
  if (!g_linkUp || g_writeChr == nullptr) return;
  if (g_writeChr->writeValue(APP_MODE_HOLD, sizeof APP_MODE_HOLD, false)) {
    g_lastHoldMs   = millis();
    g_holdEverSent = true;
    stats.holdsSent++;
    // The hold takes the toy back. If it had gone freelance, it has
    // now been reclaimed, which is why a too-long hold OSCILLATES
    // rather than failing once: lapse, whistle, reclaim, repeat.
    g_toyChoseOwnMode = false;
  }
}

// ===================================================================
// SCAN AND CONNECT - BLOCKS. Only ever called from the link task.
// ===================================================================
static bool droid_scan_and_connect() {
  NimBLEScan *scan = NimBLEDevice::getScan();
  scan->setActiveScan(true);
  scan->setInterval(100);
  scan->setWindow(80);

  TRACELN(F("[BLE] scanning ..."));
  NimBLEScanResults results = scan->getResults(SCAN_SECONDS * 1000, false);

  const NimBLEAdvertisedDevice *found = nullptr;
  for (int i = 0; i < results.getCount(); i++) {
    const NimBLEAdvertisedDevice *d = results.getDevice((uint32_t)i);
    std::string name = d->getName();
    // The reference matches on a PREFIX, not equality. Keep it that
    // way: the advertised name has been seen with a suffix.
    if (name.rfind(DROID_NAME_1, 0) == 0 || name.rfind(DROID_NAME_2, 0) == 0) {
      found = d;
      TRACE(F("[BLE] found ")); TRACELN(name.c_str());
      break;
    }
  }
  if (found == nullptr) { scan->clearResults(); TRACELN(F("[BLE] none in range")); return false; }

  if (g_client == nullptr) {
    g_client = NimBLEDevice::createClient();
    g_client->setClientCallbacks(&g_clientCB, false);   // static, do not delete
  }
  g_client->setConnectTimeout(CONNECT_TIMEOUT_MS);

  bool connected = g_client->connect(found);
  scan->clearResults();                 // AFTER connect: found points into it
  if (!connected) return false;

  NimBLERemoteService *svc = g_client->getService(DROID_SERVICE_UUID);
  if (svc == nullptr) {
    TRACELN(F("[BLE] service not found - wrong device?"));
    g_client->disconnect(); return false;
  }
  g_writeChr  = svc->getCharacteristic(DROID_WRITE_UUID);
  g_notifyChr = svc->getCharacteristic(DROID_NOTIFY_UUID);
  if (g_writeChr == nullptr || g_notifyChr == nullptr) {
    TRACELN(F("[BLE] characteristic missing"));
    g_client->disconnect(); return false;
  }
  if (!g_notifyChr->subscribe(true, notifyCB)) {
    TRACELN(F("[BLE] subscribe failed"));
    g_client->disconnect(); return false;
  }
  TRACELN(F("[BLE] ready"));
  return true;
}

// ===================================================================
// THE LINK TASK - *** THIS IS THE REMEDY ***
//
// It runs whether or not loop() is blocked inside a host wait, which
// is exactly what an endpoint with its own timebase requires and what
// a heartbeat in loop() cannot give it. It also owns RECONNECTION,
// because that blocks for seconds and loop() may be sitting in the
// value window when the link goes.
// ===================================================================
static void link_task(void *arg) {
  (void)arg;
  uint8_t pollDivider = 0;
  for (;;) {
    if (!g_linkUp) {
      stats.connectAttempts++;
      if (!droid_scan_and_connect()) vTaskDelay(pdMS_TO_TICKS(RECONNECT_PAUSE_MS));
      continue;
    }
#if HOLD_IN_LOOP == 0
    // g_suppressHold stops the hold WITHOUT stopping the input poll
    // below. That is the test for whether 0x50 0x8D is special or
    // whether ANY app-to-toy traffic satisfies the toy that an
    // application is present. Under m 1 the ONLY outbound traffic is
    // the 0x20 poll, and its interval is INPUT_POLL_EVERY * the hold
    // period - so k is the knob that moves the poll past the lapse
    // window while the hold stays off.
    if (!g_suppressHold) droid_service_hold();
#endif
    // Input state is POLLED. IR distance arrives unsolicited.
    if (++pollDivider >= INPUT_POLL_EVERY) {
      pollDivider = 0;
      const uint8_t req[1] = { OP_REQ_INPUT };
      droid_write(req, sizeof req, "request input state");
    }
    vTaskDelay(pdMS_TO_TICKS(g_holdPeriodMs));
  }
}

// ===================================================================
//  END OF THE RADIO BLOCK
// ===================================================================

// ===================================================================
// ===================================================================
//  THE WEB LAYER
//
//  Nothing below this block knows the web layer exists.
// ===================================================================
// ===================================================================

static uint8_t hexNibble(char c) {
  if (c >= '0' && c <= '9') return c - '0';
  if (c >= 'A' && c <= 'F') return c - 'A' + 10;
  if (c >= 'a' && c <= 'f') return c - 'a' + 10;
  return 0xFF;
}

// Parse "AABBCC" into bytes. Returns -1 on anything malformed, and a
// malformed step is DISCARDED rather than half-executed.
static int parseHex(const String &h, uint8_t *out, uint8_t maxLen) {
  if (h.length() % 2) return -1;
  int n = h.length() / 2;
  if (n > maxLen) return -1;
  for (int i = 0; i < n; i++) {
    uint8_t hi = hexNibble(h[i*2]), lo = hexNibble(h[i*2+1]);
    if (hi == 0xFF || lo == 0xFF) return -1;
    out[i] = (hi << 4) | lo;
  }
  return n;
}

// ===================================================================
// THE SEQUENCE RUNNER - its own task, for the same reason the
// app-mode hold is. It sleeps between steps, and a task may sleep where a
// callback may not.
//
// *** IT ALWAYS ENDS WITH A STOP. *** However the run ends - finished,
// aborted, or the last browser walked away - the motor is stopped.
// The drive motor latches on, and a sequence that ended without one
// would leave the droid driving.
// ===================================================================
static void runner_task(void *arg) {
  (void)arg;
  const uint8_t STOP_CMD[2] = { OP_STOP_SEQ, 0x3F };
  for (;;) {
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    g_seqAbort = false;
    const char *why = nullptr;

    for (uint16_t i = 0; i < g_seqCount && !g_seqAbort; i++) {
      // A run with nobody watching is a droid moving in an empty
      // room. Treat a vanished browser as a reason to stop.
      if (ws.count() == 0) { why = "no client"; break; }

      ws.textAll(String("STEP ") + i);
      if (g_seq[i].txLen) droid_write(g_seq[i].tx, g_seq[i].txLen, g_seq[i].mir);
      else                TRACE(F("[seq] wait ")), TRACELN(g_seq[i].ms);

      // Sleep in slices so an abort is acted on promptly rather than
      // at the end of a three-second block.
      uint32_t left = g_seq[i].ms;
      while (left && !g_seqAbort) {
        uint32_t slice = left > 100 ? 100 : left;
        vTaskDelay(pdMS_TO_TICKS(slice));
        left -= slice;
      }
    }

    droid_write(STOP_CMD, sizeof STOP_CMD, "STOP (end of run)");
    if (g_seqAbort || why) ws.textAll(String("HALT ") + (why ? why : "aborted"));
    else                   ws.textAll("DONE");
    g_seqRun = false;
  }
}

// ===================================================================
// TELEMETRY TO THE BROWSER
//
// The SAME THREE QUANTITIES the calculator displays, sampled at this
// task's own moment rather than the calculator's. The two can differ
// by up to one calculator cycle, and neither is wrong: they are two
// views of one droid taken at different instants. The phone is
// mirroring the calculator's DISPLAY, not its packets.
//
// It also means the page is useful with no calculator attached.
// ===================================================================
static void telemetry_task(void *arg) {
  (void)arg;
  for (;;) {
    if (ws.count()) {
      int16_t ir   = (g_linkUp && g_irEverSeen)   ? g_ir_proximity  : NOT_AVAILABLE;
      int16_t head = (g_linkUp && g_headEverSeen) ? g_head_position : NOT_AVAILABLE;
      ws.textAll(String("TEL ") + ir + " " + hold_age_now() + " "
                 + head + " " + (g_linkUp ? 1 : 0)
                 + " " + (g_toyChoseOwnMode ? 1 : 0));
    }
    vTaskDelay(pdMS_TO_TICKS(500));
  }
}

// ===================================================================
// WEBSOCKET MESSAGES
// ===================================================================
static void handleWsText(const String &msg) {

  if (msg.startsWith("HALT")) {
    g_seqAbort = true;
    TRACELN(F("[ws] halt"));
    return;
  }

  // NOW <hex>|<mirror>   a single command from Live Control
  if (msg.startsWith("NOW ")) {
    int bar = msg.indexOf('|');
    String h = (bar < 0) ? msg.substring(4) : msg.substring(4, bar);
    uint8_t tx[MAX_TX_BYTES];
    int n = parseHex(h, tx, MAX_TX_BYTES);
    if (n <= 0) { ws.textAll("HALT bad command"); return; }
    droid_write(tx, n, bar < 0 ? "now" : msg.substring(bar + 1).c_str());
    return;
  }

  // RUN\n<hex>|<ms>|<mirror>\n...
  if (msg.startsWith("RUN")) {
    if (g_seqRun) { ws.textAll("HALT already running"); return; }
    g_seqCount = 0;
    int pos = msg.indexOf('\n');
    while (pos >= 0 && g_seqCount < MAX_STEPS) {
      int end = msg.indexOf('\n', pos + 1);
      String line = (end < 0) ? msg.substring(pos + 1) : msg.substring(pos + 1, end);
      line.trim();
      pos = end;
      if (!line.length()) continue;

      int b1 = line.indexOf('|');
      int b2 = (b1 < 0) ? -1 : line.indexOf('|', b1 + 1);
      if (b1 < 0 || b2 < 0) { ws.textAll("HALT malformed step"); g_seqCount = 0; return; }

      SeqStep &st = g_seq[g_seqCount];
      String h = line.substring(0, b1);
      if (h.length() == 0) {
        st.txLen = 0;                       // a Wait
      } else {
        int n = parseHex(h, st.tx, MAX_TX_BYTES);
        if (n <= 0) { ws.textAll("HALT bad bytes"); g_seqCount = 0; return; }
        st.txLen = (uint8_t)n;
      }
      // A step may not sleep forever. 30 s is generous and finite.
      long ms = line.substring(b1 + 1, b2).toInt();
      if (ms < 0) ms = 0;
      if (ms > 30000) ms = 30000;
      st.ms = (uint32_t)ms;
      strncpy(st.mir, line.substring(b2 + 1).c_str(), MAX_MIRROR - 1);
      st.mir[MAX_MIRROR - 1] = 0;
      g_seqCount++;
    }
    if (!g_seqCount) { ws.textAll("HALT empty"); return; }
    TRACE(F("[ws] run, ")); TRACE(g_seqCount); TRACELN(F(" steps"));
    g_seqRun = true;
    if (g_runnerTask) xTaskNotifyGive(g_runnerTask);
    return;
  }
}

static void onWsEvent(AsyncWebSocket *s, AsyncWebSocketClient *c,
                      AwsEventType type, void *arg, uint8_t *data, size_t len) {
  (void)s;
  if (type == WS_EVT_CONNECT) {
    TRACE(F("[ws] client ")); TRACELN(c->id());
  } else if (type == WS_EVT_DISCONNECT) {
    TRACELN(F("[ws] client gone"));
    // The runner checks ws.count() itself and will stop the motor.
  } else if (type == WS_EVT_DATA) {
    AwsFrameInfo *info = (AwsFrameInfo *)arg;
    if (info->final && info->index == 0 && info->len == len &&
        info->opcode == WS_TEXT) {
      String msg;
      msg.reserve(len + 1);
      for (size_t i = 0; i < len; i++) msg += (char)data[i];
      handleWsText(msg);
    }
  }
}

// The page lives in flash gzipped. The browser does the
// decompressing, which is why 40.6 kB of HTML costs 12.6 kB here.
//
// If your ESPAsyncWebServer has dropped beginResponse_P, use
// beginResponse with the same four arguments - on an ESP32, PROGMEM
// is a no-op and flash is memory-mapped, so both read the array the
// same way.
static void send_droidx(AsyncWebServerRequest *req) {
  AsyncWebServerResponse *res =
      req->beginResponse_P(200, "text/html",
                           DROIDX_PAGE_GZ, DROIDX_PAGE_GZ_LEN);
  res->addHeader("Content-Encoding", "gzip");
  req->send(res);
}

static void start_web() {
  ws.onEvent(onWsEvent);
  server.addHandler(&ws);

  server.on("/", HTTP_GET, send_droidx);
  // A phone joining an access point probes several well-known URLs to
  // decide whether it has internet. Answering them all with the page
  // means the captive-portal notification opens DroidX directly.
  server.onNotFound(send_droidx);
  server.begin();
  Serial.println(F("[web] server up on port 80, WebSocket at /ws"));
}

// ===================================================================
//  END OF THE WEB LAYER
// ===================================================================

// ===================================================================
// ===================================================================
//  THE BENCH CONSOLE
//
//  Two figures in this project were BORROWED rather than known: the
//  toy's tolerance for a missing 0x50 0x8D (2.0 s, taken from
//  somebody else's working code) and the telemetry pause an
//  FX-9750G Plus actually needs (1200 ms, chosen when this build made
//  ONE transaction per cycle and not three).
//
//  BOTH WERE MEASURED HERE August 2026. The pause was doing
//  nothing and is now 0. The 2.0 s figure was not a tolerance at all:
//  0x50 0x8D is an app-mode hold, the toy's timeout for it is 5.0 s
//  exactly (bisected August: quiet at 5000, lapsing at 5100), and
//  when it lapses the toy takes over WITHOUT dropping the link. 
//  The console stays because both settings are now
//  teaching controls, and because the counters that say whether a
//  setting is working are printable here.
//
//  *** IT RUNS IN ITS OWN TASK. *** loop() is blocked inside the host
//  wait for most of every cycle, so a console serviced from there
//  would be unresponsive exactly when you were trying to diagnose
//  something. Same lesson as the app-mode hold, third time.
// ===================================================================
// ===================================================================

static void print_settings() {
  Serial.println();
  Serial.println(F("--- settings ---"));
  Serial.print(F("  app-mode hold      ")); Serial.print(g_holdPeriodMs);
  Serial.println(F(" ms   (0x50 0x8D. NOT a keepalive.)"));
  Serial.print(F("  hold packets       "));
  Serial.println(g_suppressHold ? F("SUPPRESSED (input poll still running)")
                                     : F("running"));
  Serial.print(F("  input poll every   ")); Serial.print(INPUT_POLL_EVERY);
  Serial.print(F(" holds = ")); 
  Serial.print(INPUT_POLL_EVERY * g_holdPeriodMs); Serial.println(F(" ms"));
  Serial.print(F("  telemetry pause    ")); Serial.print(g_telemetryPauseMs);
  Serial.println(F(" ms"));
  Serial.print(F("  droid link         "));
  Serial.println(g_linkUp ? F("up") : F("DOWN"));
}

static void print_stats() {
  Serial.println();
  Serial.println(F("--- counters ---"));
  Serial.print(F("  casio cycles       ")); Serial.println(stats.casioCycles);
  Serial.print(F("  value packets      ")); Serial.println(stats.valuePackets);
  // Every value is followed by an end packet, so these two must
  // agree. A gap means a transaction was abandoned mid-reply.
  Serial.print(F("  end packets        ")); Serial.println(stats.endPackets);
  Serial.print(F("  request packets    ")); Serial.println(stats.requestPackets);
  Serial.println(F("  -- the ones that matter when a G Plus misbehaves --"));
  Serial.print(F("  short packets      ")); Serial.println(stats.shortPackets);
  Serial.print(F("  bad preamble       ")); Serial.println(stats.badPreamble);
  Serial.print(F("  bad checksum       ")); Serial.println(stats.badChecksum);
  Serial.print(F("  bad attention      ")); Serial.println(stats.badAttention);
  Serial.print(F("  timeouts           ")); Serial.println(stats.timeouts);
  Serial.print(F("  bytes flushed      ")); Serial.println(stats.flushed);
  // WHERE a transaction stopped, not just that one did. Without this
  // an in-flight request has to be reconstructed by arithmetic.
  if (stats.lastBadStage) {
    Serial.print(F("  last bad stage     ")); Serial.print(stats.lastBadStage);
    Serial.print(F("  ("));
    switch (stats.lastBadStage) {
      case 1:  Serial.print(F("first ACK"));  break;
      case 2:  Serial.print(F("second ACK")); break;
      case 3:  Serial.print(F("attention"));  break;
      default: Serial.print(F("?"));
    }
    Serial.print(F("), byte 0x")); Serial.println(stats.lastBadByte, HEX);
  }
  Serial.println(F("  -- droid --"));
  Serial.print(F("  hold packets sent  ")); Serial.println(stats.holdsSent);
  Serial.print(F("  notifications      ")); Serial.println(stats.notifications);
  Serial.print(F("  IR check failures  ")); Serial.println(stats.irCheckFail);
  Serial.print(F("  APP-MODE LAPSES    ")); Serial.println(stats.modeChanges);
  if (stats.modeChanges) {
    Serial.println(F("    the hold lapsed and the toy chose its own mode."));
    Serial.println(F("    toy timeout is 5.0 s. Quiet 5000, lapses 5100."));
    Serial.println(F("    a CLIMBING count = starved and reclaimed each cycle."));
    Serial.println(F("    stuck at 1        = app mode gone and staying gone."));
  }
  Serial.print(F("  connect attempts   ")); Serial.println(stats.connectAttempts);
  Serial.print(F("  LINK DROPS         ")); Serial.println(stats.linkDrops);
  if (stats.linkDrops) {
    Serial.print(F("    last reason      0x")); Serial.println(stats.lastDropReason, HEX);
    Serial.print(F("    at hold period   ")); Serial.print(stats.lastDropHoldMs);
    Serial.println(F(" ms"));
    Serial.println(F("    0x13 = the TOY hung up, an application watchdog"));
    Serial.println(F("    0x08 = BLE supervision timeout, a different finding"));
    Serial.println(F("    AFTER ~10 MIN THIS IS THE TOY'S OWN AUTO POWER-OFF,"));
    Serial.println(F("    sooner on low batteries. NOT a finding. Check the"));
    Serial.println(F("    droid is still switched on before believing a drop."));
  }
  Serial.print(F("  commands applied   ")); Serial.println(stats.commandsApplied);
  Serial.print(F("  commands rejected  ")); Serial.println(stats.commandsRejected);
}

static void print_help() {
  Serial.println();
  Serial.println(F("--- bench console ---"));
  Serial.println(F("  p <ms>   telemetry pause. THE STEP-DOWN TEST."));
  Serial.println(F("           1200 800 500 300 150 0 - watch the counters,"));
  Serial.println(F("           not the screen: a marginal pause shows up as"));
  Serial.println(F("           short packets long before a Com ERROR does."));
  Serial.println(F("  k <ms>   app-mode hold period. THE TOLERANCE TEST,"));
  Serial.println(F("           already run: 5.0 s is the toy's timeout."));
  Serial.println(F("           Watch APP-MODE LAPSES, not LINK DROPS. The"));
  Serial.println(F("           link does not drop. The toy takes over."));
  Serial.println(F("  m 0|1    1 suppresses the hold but KEEPS the input"));
  Serial.println(F("           poll. ANSWERED Aug: the toy lapses anyway."));
  Serial.println(F("           A 0x20 poll every 3 s does NOT stand in for"));
  Serial.println(F("           0x50 0x8D. A minute with no hold at all cost"));
  Serial.println(F("           0 link drops and 0 reconnects."));
  Serial.println(F("  s        counters      z   zero the counters"));
  Serial.println(F("  r        settings      ?   this help"));
  Serial.println(F("  ZERO THE COUNTERS AT THE START OF EACH STEP, or you are"));
  Serial.println(F("  reading the whole session's history at every one."));
}

static void console_exec(String cmd) {
  cmd.trim();
  if (!cmd.length()) return;
  char c = cmd[0];
  long arg = (cmd.length() > 1) ? cmd.substring(1).toInt() : -1;

  switch (c) {
    case 'p':
      if (arg < 0 || arg > 30000) { Serial.println(F("p 0..30000")); break; }
      g_telemetryPauseMs = (uint32_t)arg;
      Serial.print(F("telemetry pause -> ")); Serial.print(arg); Serial.println(F(" ms"));
      Serial.println(F("(counters not zeroed - send z if this is a new step)"));
      break;
    case 'k':
      if (arg < 100 || arg > 60000) { Serial.println(F("k 100..60000")); break; }
      g_holdPeriodMs = (uint32_t)arg;
      Serial.print(F("app-mode hold period -> ")); Serial.print(arg); Serial.println(F(" ms"));
      break;
    case 'm':
      g_suppressHold = (arg == 1);
      Serial.println(g_suppressHold
        ? F("app-mode hold SUPPRESSED. Input poll still running.")
        : F("app-mode hold running."));
      break;
    case 's': print_stats(); break;
    case 'r': print_settings(); break;
    case 'z':
      memset((void *)&stats, 0, sizeof stats);
      Serial.println(F("counters zeroed"));
      break;
    case '?': case 'h': print_help(); break;
    default:  Serial.println(F("? for help"));
  }
}

static void console_task(void *arg) {
  (void)arg;
  String line;
  for (;;) {
    while (Serial.available()) {
      char ch = (char)Serial.read();
      if (ch == '\n' || ch == '\r') { console_exec(line); line = ""; }
      else if (line.length() < 32)  { line += ch; }
    }
    vTaskDelay(pdMS_TO_TICKS(50));
  }
}

// ===================================================================
//  END OF THE BENCH CONSOLE
// ===================================================================

// ===================================================================
// THE THREE VALUES
// ===================================================================

// Value B. Computed FRESH at its own transmission, not captured with
// A and C. A and C describe the droid and belong to one instant; this
// describes the board, and taking it fresh means every cycle is an
// independent observation of the finding rather than a repeat of A's.
int16_t hold_age_now() {
  if (!g_holdEverSent) return NOT_AVAILABLE;
  uint32_t age = millis() - g_lastHoldMs;
  int32_t tenths = (int32_t)(age / 100UL);
  if (tenths > NSN_MAX_VALUE) tenths = NSN_MAX_VALUE;
  return (int16_t)tenths;
}

// Values A and C, captured TOGETHER at the end of A's host wait so
// that they describe the same instant. The transactions are
// staggered; the measurement is not.
void snapshot_droid() {
  snapIR   = (g_linkUp && g_irEverSeen)   ? g_ir_proximity  : NOT_AVAILABLE;
  snapHead = (g_linkUp && g_headEverSeen) ? g_head_position : NOT_AVAILABLE;
}

// ===================================================================
// APPLY ONE VALUE FROM THE CALCULATOR
//
// *** EVERY VALUE IS RANGE-CHECKED HERE AS WELL AS ON THE
//     CALCULATOR. *** The board cannot assume the program at the
//     other end is the one printed in this header. A device that acts
//     on an unclamped number arriving over a wire can be driven into
//     the stairs by a typing error.
//
// A refusal here is a SHOULD-NEVER-HAPPEN, because the calculator
// checks first. It is counted and reported on the serial monitor, and
// it does not cost a transaction.
// ===================================================================
void applyRemoteValue(int16_t value) {
  uint8_t  param = g_pending_param;
  uint8_t  buf[4];
  size_t   len = 0;
  char     what[48];
  bool     ok  = true;

  switch (param) {
    case 1:  // head position. 0 = left, 1 = centre, 2 = right,
             // as seen by someone facing the droid. Measured.
      if (value < 0 || value > 2) { ok = false; break; }
      buf[0] = OP_HEAD_GOTO; buf[1] = (uint8_t)value; len = 2;
      snprintf(what, sizeof what, "head %d", (int)value);
      break;

    case 2:  // drive motor. 0 stop, 1 forward, 2 backward.
      if (value < 0 || value > 2) { ok = false; break; }
      buf[0] = OP_MOTOR2_RUN; buf[1] = (uint8_t)value; len = 2;
      snprintf(what, sizeof what, "motor %d", (int)value);
      break;

    case 3:  // cam. NOTE THE LITERAL 2 IN BYTE 1 - it is in the
             // reference implementation and is not a length.
      if (value < 0 || value > MAX_CAM_POSITION) { ok = false; break; }
      buf[0] = OP_MOTOR1_CAM; buf[1] = 2; buf[2] = (uint8_t)value; len = 3;
      snprintf(what, sizeof what, "cam %d", (int)value);
      break;

    case 4:  // LED red percent
    case 5:  // LED blue percent
      if (value < 0 || value > 100) { ok = false; break; }
      if (param == 4) g_led_red_pct = (uint8_t)value;
      else            g_led_blue_pct = (uint8_t)value;
      // LED_DUTY carries BOTH channels every time, so the other one
      // has to be re-sent or it would be zeroed by this write.
      buf[0] = OP_LED_DUTY;
      buf[1] = (uint8_t)((255 * (int)g_led_red_pct)  / 100);
      buf[2] = (uint8_t)((255 * (int)g_led_blue_pct) / 100);
      len = 3;
      snprintf(what, sizeof what, "led r%d b%d",
               (int)g_led_red_pct, (int)g_led_blue_pct);
      break;

    case 6:  // sound, 16-bit little-endian index
      if (value < 0 || value > MAX_AUDIO_INDEX) { ok = false; break; }
      buf[0] = OP_PLAY_AUDIO;
      buf[1] = (uint8_t)(value & 0xFF); buf[2] = (uint8_t)((value >> 8) & 0xFF);
      len = 3;
      snprintf(what, sizeof what, "sound %d", (int)value);
      break;

    case 7:  // LED sequence
      if (value < 0 || value > MAX_LEDSEQ_INDEX) { ok = false; break; }
      buf[0] = OP_PLAY_SEQ; buf[1] = SEQ_LED;
      buf[2] = (uint8_t)(value & 0xFF); buf[3] = (uint8_t)((value >> 8) & 0xFF);
      len = 4;
      snprintf(what, sizeof what, "ledseq %d", (int)value);
      break;

    case 8:  // motion sequence
      if (value < 0 || value > MAX_MOTSEQ_INDEX) { ok = false; break; }
      buf[0] = OP_PLAY_SEQ; buf[1] = SEQ_MOTOR;
      buf[2] = (uint8_t)(value & 0xFF); buf[3] = (uint8_t)((value >> 8) & 0xFF);
      len = 4;
      snprintf(what, sizeof what, "motseq %d", (int)value);
      break;

    case 9:  // stop everything. The value is ignored ON PURPOSE: a
             // stop that can be mistyped into a no-op is not a stop.
      buf[0] = OP_STOP_SEQ; buf[1] = 0x3F; len = 2;
      snprintf(what, sizeof what, "STOP");
      break;

    default:
      ok = false;
      snprintf(what, sizeof what, "unknown parameter %d", (int)param);
      break;
  }

  if (!ok) {
    stats.commandsRejected++;
    TRACE(F("[REJECTED - the calculator should have caught this] param "));
    TRACE(param); TRACE(F(" value ")); TRACELN(value);
    g_pending_param = 0;
    return;
  }
  if (droid_write(buf, len, what)) stats.commandsApplied++;
  else                             stats.commandsRejected++;
  g_pending_param = 0;
}

// ===================================================================
// CHECKSUM - one rule, any packet length
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
// ===================================================================
bool waitForByte(uint8_t &b, uint32_t timeoutMs) {
  uint32_t t0 = millis();
  while ((millis() - t0) < timeoutMs) {
    if (CasioSerial.available()) { b = (uint8_t)CasioSerial.read(); return true; }
    yield();
  }
  stats.timeouts++;
  return false;
}

// Every path that abandons a transaction must leave the line EMPTY,
// or the bytes it walked away from are read one at a time afterwards
// and the board spends the next transactions one packet behind.
uint16_t flush_line() {
  uint16_t n = 0;
  uint32_t idle = millis();
  while ((millis() - idle) < 15 && n < 200) {
    if (CasioSerial.available()) { CasioSerial.read(); n++; idle = millis(); }
    yield();
  }
  stats.flushed += n;
  return n;
}

// ===================================================================
// SEND ONE VALUE - normalised scientific notation, SIGNED
//
// The magnitude is split into a leading digit and up to three more,
// packed two to a byte, with the exponent saying where the point
// belongs. The sign lives in byte 13.
//
// Zero has its own packet with a fixed checksum, because zero has no
// normalised form. THIS MATTERS HERE: head position 0 and a hold age
// of 0 are both ordinary readings, and both occur constantly.
// ===================================================================
void send_nsn_value(int16_t signedValue) {
  if (signedValue == 0) {
    const uint8_t zeroPacket[VALUE_PACKET_LEN] = {
      CASIO_PREAMBLE, 0x00, 0x01, 0x00, 0x01,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0xFE
    };
    casio_send_packet(zeroPacket, VALUE_PACKET_LEN);
    stats.valuePackets++;
    return;
  }

  bool     negative = (signedValue < 0);
  uint16_t value    = (uint16_t)(negative ? -(int32_t)signedValue : signedValue);
  uint8_t  intDigit = 0, dec1 = 0, dec2 = 0, exponent = 0;

  if (value < 10) {
    intDigit = (uint8_t)value; exponent = 0;
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

  casio_send_packet(packet, VALUE_PACKET_LEN);
  stats.valuePackets++;
}

// ===================================================================
// DECODE ONE VALUE ARRIVING FROM THE CALCULATOR - SIGNED
//
// *** THE SIGN TEST IS THE POINT OF THIS FUNCTION. ***
// bit 0 is set whenever the magnitude is >= 1, and it is set for a
// NEGATIVE number too, because 0x51 & 0x01 is 1. Testing bit 0 alone
// decodes -20 as +20. On a logger that never mattered - the only
// thing received was the interval. HERE IT DRIVES A MACHINE.
// ===================================================================
int16_t decode_signed_value(const uint8_t *packet) {
  uint8_t intDigit = packet[5];
  uint8_t d1       = packet[6];
  uint8_t d2       = packet[7];
  uint8_t signInfo = packet[13];
  uint8_t E        = packet[14];

  int32_t value = 0;
  if (signInfo & 0x01) {                     // magnitude >= 1
    if      (E == 0) value = intDigit;
    else if (E == 1) value = intDigit * 10   + (d1 >> 4);
    else if (E == 2) value = intDigit * 100  + (d1 >> 4) * 10 + (d1 & 0x0F);
    else if (E == 3) value = intDigit * 1000 + (d1 >> 4) * 100
                                             + (d1 & 0x0F) * 10 + (d2 >> 4);
    else             value = 0;
  }
  // A magnitude below 1 arrives with bit 0 CLEAR and decodes as zero,
  // which is correct for whole-number commands.
  if ((signInfo & SIGN_NEG_MASK) == SIGN_NEG_MASK) value = -value;
  if (value >  NSN_MAX_VALUE) value =  NSN_MAX_VALUE;
  if (value < -NSN_MAX_VALUE) value = -NSN_MAX_VALUE;
  return (int16_t)value;
}

// ===================================================================
// THE DESCRIPTION PACKET - 50 bytes
// The checksum is the constant 273 - vname, and that is only correct
// because every other byte never changes. Alter the padding and it
// silently becomes wrong.
// ===================================================================
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
  casio_send_packet(packet, REQUEST_PACKET_LEN);
}

void send_end_packet() {
  uint8_t packet[REQUEST_PACKET_LEN];
  packet[0] = CASIO_PREAMBLE;
  packet[1] = 'E'; packet[2] = 'N'; packet[3] = 'D';
  for (uint8_t i = 4; i <= 48; i++) packet[i] = 0xFF;
  packet[49] = 0x56;
  casio_send_packet(packet, REQUEST_PACKET_LEN);
  stats.endPackets++;
}

// ===================================================================
// THE CALCULATOR WANTS A VALUE - Receive(
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
  send_description(vname);

  if (!waitForByte(b, 2000)) { send_end_packet(); return; }
  if (b != CASIO_ACK) {
    stats.lastBadByte = b; stats.lastBadStage = 2; flush_line(); return;
  }

  // == FENTON 2025 HOST-WAIT WINDOW: THE VALUE WINDOW (GAP 3) ==
  // THIS is the heart of the discovery. The calculator is sitting
  // inside Receive() waiting for a number, and it will wait - for
  // five minutes if we ask it to - without raising the COM ERROR
  // that every reference says should happen. That patience is what
  // turns a calculator into a datalogger.

  // Under the remedy the link task fires right through it. Under
  // the fault nothing fires, because loop() is not running.
  if (vname == VNAME_N) {
    send_nsn_value((int16_t)QUANTITY_COUNT);

  } else if (vname == VNAME_A) {
    // THE PAUSE BELONGS TO CHANNEL A ALONE. Waiting in B or C as well
    // would multiply the cycle time by three for no gain.
    uint32_t t0 = millis();
    while ((millis() - t0) < g_telemetryPauseMs) {
      // Under the FAULT this loop is where the app-mode hold starves,
      // because loop() is not running. Servicing it here would hide
      // the very thing the file exists to show, so it is NOT
      // serviced here. That omission is deliberate.
      delay(2);
    }
    snapshot_droid();          // A and C together, AFTER the wait
    stats.casioCycles++;
    send_nsn_value(snapIR);

  } else if (vname == VNAME_B) {
    send_nsn_value(hold_age_now());   // fresh, not snapshotted

  } else if (vname == VNAME_C) {
    send_nsn_value(snapHead);              // same instant as A

  } else {
    send_nsn_value(0);
  }

  waitForByte(b, 1000);        // the closing ACK is optional
  send_end_packet();
}

// ===================================================================
// THE CALCULATOR IS SENDING US A NUMBER - Send(
// Step one selects the parameter, step two carries the value.
// ===================================================================
void handle_incoming() {
  turnaround();
  CasioSerial.write(CASIO_ACK);

  uint8_t packet[VALUE_PACKET_LEN];
  uint8_t got = 0;
  uint32_t t0 = millis();
  while (got < VALUE_PACKET_LEN && (millis() - t0) < 2000) {
    if (CasioSerial.available()) { packet[got++] = (uint8_t)CasioSerial.read(); t0 = millis(); }
    yield();
  }
  if (got != VALUE_PACKET_LEN) { stats.shortPackets++; flush_line(); return; }
  if (packet[0] != CASIO_PREAMBLE) { stats.badPreamble++; flush_line(); return; }
  if (packet[VALUE_PACKET_LEN - 1] != checksum_over(packet, VALUE_PACKET_LEN)) {
    stats.badChecksum++;   // counted, not acted on - the rule is not yet
  }                        // verified against traffic the calculator sends
  stats.requestPackets++;

  int16_t value = decode_signed_value(packet);

  if (g_pending_param == 0) {
    if (value >= 1 && value <= 9) {
      g_pending_param = (uint8_t)value;
      TRACE(F("[Parameter ")); TRACE(value); TRACELN(F(" selected]"));
    } else {
      stats.commandsRejected++;
      TRACE(F("[Invalid parameter ")); TRACE(value); TRACELN(F(" - ignored]"));
    }
  } else {
    applyRemoteValue(value);
  }

  turnaround();
  CasioSerial.write(CASIO_ACK);

  // The calculator's 50-byte END packet, counted rather than guessed.
  uint8_t junk[REQUEST_PACKET_LEN];
  uint8_t n = 0;
  t0 = millis();
  while (n < REQUEST_PACKET_LEN && (millis() - t0) < 300) {
    if (CasioSerial.available()) { junk[n++] = (uint8_t)CasioSerial.read(); t0 = millis(); }
    yield();
  }
  if (n != REQUEST_PACKET_LEN) stats.shortPackets++;
  flush_line();
}

// ===================================================================
void setup() {
  Serial.begin(115200);
  delay(1500);

  CasioSerial.begin(9600, SERIAL_8N2, CASIO_RX_PIN, CASIO_TX_PIN);
  pinMode(BUILTIN_LED, OUTPUT);
  digitalWrite(BUILTIN_LED, LOW);

#if ENABLE_WIFI_AP
  // AP ONLY. No station mode, no scanning for other networks: both
  // would add radio activity that has nothing to do with the question
  // being asked here.
  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID, AP_PASSWORD);
  Serial.print(F("[WiFi] AP \"")); Serial.print(AP_SSID);
  Serial.print(F("\" at ")); Serial.println(WiFi.softAPIP());
  Serial.println(F("[WiFi] *** RADIO SHARED WITH BLUETOOTH FROM HERE ***"));
#endif

  NimBLEDevice::init("");
  // NO setPower CALL. In NimBLE 2.x setPower() takes dBm, while the
  // ESP_PWR_LVL_* constants are ENUM INDICES for the older
  // setPowerLevel(). Passing one to the other compiles and silently
  // asks for the wrong power. The default is ample for a toy in the
  // same room.

  // THE LINK TASK RUNS UNDER BOTH SETTINGS OF THE SWITCH. Under the
  // fault it still owns scanning, connecting and the input poll; only
  // the app-mode hold moves to loop(). That isolates ONE variable, which
  // is what makes the comparison worth anything.
  if (xTaskCreate(link_task, "droid_link", 4096, nullptr, 5, &g_linkTask) != pdPASS) {
    Serial.println(F("*** xTaskCreate link FAILED ***"));
  }
#if ENABLE_WIFI_AP
  if (xTaskCreate(runner_task, "seq_runner", 4096, nullptr, 4, &g_runnerTask) != pdPASS) {
    Serial.println(F("*** xTaskCreate runner FAILED ***"));
  }
  if (xTaskCreate(telemetry_task, "telemetry", 3072, nullptr, 2, &g_telTask) != pdPASS) {
    Serial.println(F("*** xTaskCreate telemetry FAILED ***"));
  }
  start_web();
#endif
  xTaskCreate(console_task, "console", 4096, nullptr, 1, nullptr);

  Serial.println();
  Serial.println(F("CASIO-ESP32-DROIDX   calculator + phone -> droid"));
#if ENABLE_WIFI_AP
  Serial.print(F("Join WiFi \"")); Serial.print(AP_SSID);
  Serial.println(F("\" then open http://192.168.4.1/"));
#else
  Serial.println(F("WiFi AP is OFF. Calculator only, no DroidX."));
#endif
  Serial.println(F("Verified on hardware August 2026."));
  Serial.println(F("Partition Scheme MUST be Minimal SPIFFS (1.9MB APP)."));
  Serial.println(F("A = IR proximity   B = hold age (0.1 s)   C = head"));
  Serial.println(F("-999 in any value = not available. Nothing is packed."));
  Serial.println(F("Head 0 = left, 1 = centre, 2 = right. 3 = in transit."));
#if HOLD_IN_LOOP
  Serial.println(F("HOLD_IN_LOOP = 1  ->  THE FAULT is selected."));
#else
  Serial.println(F("HOLD_IN_LOOP = 0  ->  THE REMEDY is selected."));
#endif
  print_help();
  Serial.println();
  Serial.println(F("Waiting for the calculator..."));
}

// ===================================================================
// MAIN LOOP
//
// Nothing happens until the calculator says something - EXCEPT, under
// the fault, the app-mode hold, which is exactly the problem this
// file exists to show. Under the remedy loop() has nothing to do with
// the hold at all, and that is the point.
// ===================================================================
void loop() {

#if HOLD_IN_LOOP
  // ---------------- THE FAULT ----------------
  // This LOOKS correct and is the obvious place to put it. It is
  // starved the moment the board blocks inside a host wait, and past
  // about 5 s the toy stops waiting and starts running its own
  // routines while the calculator carries on being served. Nothing
  // disconnects, nothing reports an error, and value B sits high -
  // which is the only place the failure is visible at all.
  static uint32_t lastKa = 0;
  if (millis() - lastKa >= g_holdPeriodMs) {
    lastKa = millis();
    droid_service_hold();
  }
#endif

  if (!CasioSerial.available()) { delay(1); return; }

  uint8_t inByte = (uint8_t)CasioSerial.read();
  if (inByte != CASIO_ATTENTION) {
    stats.badAttention++;
    stats.lastBadByte  = inByte;
    stats.lastBadStage = 3;
    // Say so. Silence here is indistinguishable from receiving
    // nothing at all, and those are different faults.
    TRACE(F("NOT ATTENTION: ")); TRACEHEX(inByte); TRACELN(F(""));
    flush_line();
    return;
  }

  digitalWrite(BUILTIN_LED, HIGH);
  turnaround();
  CasioSerial.write(DEVICE_PRESENT);

  // READ THE WHOLE 50-BYTE REQUEST, THEN CHECK IT. A packet that is
  // not complete is never acted upon, and having all of it means the
  // calculator's own checksum can be verified rather than the
  // packet's shape trusted.
  uint8_t rxBuf[REQUEST_PACKET_LEN];
  uint8_t got = 0;
  uint32_t t0 = millis();
  while (got < REQUEST_PACKET_LEN && (millis() - t0) < 2000) {
    if (CasioSerial.available()) { rxBuf[got++] = (uint8_t)CasioSerial.read(); t0 = millis(); }
    yield();
  }

  if (got != REQUEST_PACKET_LEN)  { stats.shortPackets++; flush_line(); goto done; }
  if (rxBuf[0] != CASIO_PREAMBLE) { stats.badPreamble++;  flush_line(); goto done; }
  if (rxBuf[REQUEST_PACKET_LEN - 1] != checksum_over(rxBuf, REQUEST_PACKET_LEN)) {
    stats.badChecksum++;   // counted, not rejected - watch it, then decide
  }
  stats.requestPackets++;

  {
    uint8_t command = rxBuf[1];    // byte 0 is the ':' preamble
    uint8_t vname   = rxBuf[11];
    if      (command == CMD_RECEIVE) handle_receive(vname);
    else if (command == CMD_SEND)    handle_incoming();
  }

done:
  digitalWrite(BUILTIN_LED, LOW);
}
