# Casio FX-9750 to ESP32 to Bluetooth droid — DroidX build

**Michael Fenton MRSNZ. 31 August 2026.**
Licence: Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International
(CC BY-NC-SA 4.0).

**STATUS: RUNS ON HARDWARE.** Verified August 2026 on an FX-9750GIII
and an FX-9750G Plus, one interface circuit, one calculator program.

---

## What this is

A Casio FX-9750 graphing calculator drives a Hasbro Smart R2-D2 over Bluetooth
through an ESP32, and reads three quantities back from it. The calculator needs
no modification and no firmware change.

**And the same board serves a controller to a phone.** A phone will not find the
droid in its Bluetooth list. It will find the ESP32 in its WiFi list. Join the
access point, open a browser, and **DroidX** appears: a block sequencer served
from the chip itself. No app, no store, no account, no internet anywhere in the
chain.

```
[ Phone / tablet browser ]  --WiFi WebSocket-->  [ ESP32 ]  --serial-->  [ Casio screen ]
      DroidX block UI                               |
                                                    +--Bluetooth LE-->  [ droid ]
```

The Casio serial side is the same protocol every other build in this repository
uses — `Send(` and `Receive(` at 9600 baud, 8N2, over the four-component
interface. It is described in full in `serial_protocol_technical-manual/`.

## What the calculator sends and reads

**Commands go out two-step:** send the parameter number, then send its value.
The calculator can only send a bare number, so the label goes first and is
remembered — which is also the shape of the droid's own protocol, an opcode
followed by a payload.

| # | function | accepted |
|---|---|---|
| 1 | head position | 0 left, 1 centre, 2 right |
| 2 | drive motor | 0 stop, 1 forward, 2 backward |
| 3 | cam position | 0 to 8 |
| 4 | lamp red | 0 to 100 % |
| 5 | lamp blue | 0 to 100 % |
| 6 | sound | 0 to 174 |
| 7 | lamp sequence | 0 to 264 |
| 8 | motion sequence | 0 to 477 |
| 9 | stop everything | value ignored |

**Power-down is deliberately absent.** A typing error should not end the lesson.

**Telemetry comes back as three ordinary signed numbers, one per `Receive(`**,
in normalised scientific notation. 

| | quantity | units | when unavailable |
|---|---|---|---|
| `Receive(A)` | infrared proximity, the reflected signal | counts | `-999` |
| `Receive(B)` | app-mode hold age | tenths of a second | `-999` |
| `Receive(C)` | head position | 0, 1, 2, or 3 in transit | `-999` |

`Receive(N)` returns 3 and is called once, outside the loop, so it costs nothing
per cycle.

### A fault must never resemble a result

A dropped Bluetooth link — a flat battery, a droid carried out of range — is
invisible from the calculator: the machine would go on accepting keypresses and
go on being acknowledged by a board with nothing on the other end. This build
has no status variable, so the values carry the news themselves.

| what you see | what it means |
|---|---|
| `-999` in A and C together | the droid is not connected, or has never reported |
| `-999` in B | no app-mode hold has ever been sent |
| B climbing | the hold is being starved; past about 5 s the droid takes over |
| C reading 3 | the head is between positions, not at one |

**A value that cannot be a real reading is proof of a fault.**

Range checking happens on the calculator, which holds the maximum for each
parameter and refuses to send anything outside it — and again at the board,
which cannot assume the program at the other end is the one printed in the
sketch header.

## Two control surfaces at once

The calculator and the browser both drive the droid, at the same time, and
neither knows the other is there.

It is part of the architecture design: `loop()` serves
only the calculator, and everything that blocks lives in a FreeRTOS task, so a
WebSocket command and a `Send(` from the keypad are just two callers of the same
`droid_write()`. **The remedy this project exists to demonstrate is what makes a
second control surface free.**

While a sequence runs, the calculator mirrors each step on its own screen.

---

## Build settings — read this first

**Tools ▸ Partition Scheme ▸ "Minimal SPIFFS (1.9MB APP…)"**

The default partition allows the sketch 1.31 MB. NimBLE, WiFi, an async web
server and the DroidX page together fill about **95 percent** of it — a build
with no room to add anything. Minimal SPIFFS allows 1.97 MB and the same binary
drops to roughly two thirds. **Nothing in the code changes. It is one menu.**

If you are tempted to save space by deleting diagnostics instead, measure first:

| what | flash |
|---|---|
| every diagnostic string in the sketch, all 143 of them | **3.9 kB** |
| the DroidX page, gzipped | 12.6 kB |
| the DroidX page, uncompressed | 40.6 kB |
| the partition change | **~650 kB** |

Stripping the instrumentation buys under 4 kB and costs the ability to reproduce
every measurement below. The intuition that diagnostics fill a chip is, here,
wrong by two orders of magnitude.

## Requirements

- **ESP32 development board.** Developed on an ESP-WROOM-32 devkit; board type
  *ESP32 Dev Module*.
- **NimBLE-Arduino 2.x**, developed against 2.5.1. The 1.x API **will not
  compile**: `setScanCallbacks`, the `const` on `onResult` and the `reason`
  argument on `onDisconnect` are all 2.x forms.
- **ESP Async WebServer** by ESP32Async, and **Async TCP** by ESP32Async.
  *They must be that pair.* The older me-no-dev AsyncTCP will not build against
  the ESP32Async server, and the failure is a wall of template errors rather
  than anything that names the cause.
- A calculator: FX-9750G Plus or FX-9750GIII.
- The four interface components and the SB-62 wiring. See
  `serial_protocol_technical-manual/` — the manual PDF and
  `casio-universal-interface.png`.

## The page

`droidx_page.h` holds `DroidX.html` **gzipped** — 40,613 bytes of HTML in
12,602 bytes of flash — served with `Content-Encoding: gzip`, which every
browser that can join a WiFi network understands.

**To change the page:** edit `DroidX.html`, then run `make_page.py`. 
`droidx_page.h` is generated and must never be edited by hand. The script
 is deterministic, so a rebuild that changes nothing shows as no change.

The page is self-contained by requirement. Served from an access point with no
internet behind it, it fetches nothing: no fonts, no CDN, no icon set, no
block-editor library. It also runs standalone in any browser with the controller
absent, saying so plainly in its toolbar, so a class can build and check
sequences without a droid each.

**Every sequence ends with a stop** — finished, aborted, or the last browser
walked away. The drive motor latches on, and a sequence that ended without one
would leave the droid driving.

---

## The app-mode hold, and what it is for

The reverse-engineered protocol work sends `0x50 0x8D` every 2.0 s and calls it
a keepalive. **It is not - nothing dies.** Nothing disconnects when it stops.

It is an **app-mode hold**: a standing claim that an application is driving,
which the droid honours by suspending its own behaviour for as long as the claim
keeps arriving.

| hold period | what the droid does | dropped connections |
|---|---|---|
| 1500 ms — the default | quiet, obeys every command | 0 |
| **5000 ms** | **the limit — quiet for minutes on end** | **0** |
| **5100 ms** | **takes over every cycle: one lamp flash, or the first note, then it stops** | **0** |
| 6000 ms | about a second of self-driving — a whole whistle | 0 |

**The droid's timeout is 5.0 s.** The edge is a step rather than a fuzzy region,
because the droid's timer is *reset by each packet* — there is no drift between
the two clocks to smear it. Either every interval overruns by the same amount or
none does.

**The hold also cancels a routine already under way.** At 5100 ms the droid
begins a routine and is cut off mid-flash when the next hold lands. So what you
hear is the *overshoot*: roughly `period − 5.0 s` of self-driving. A stutter at
5100, a whole whistle at 6000.

> **The hold does not buy control. It buys sole occupancy.**

| | with the hold | without it |
|---|---|---|
| commands accepted | yes | **yes** |
| telemetry reported | yes | **yes** |
| link maintained | yes | **yes** |
| droid acts on its own | no | **yes** |

The command channel and the app-mode claim are independent. Losing the claim
does not take the droid away from you; it stops the droid being *yours alone*.
Ordinary traffic will not stand in for the hold — an input-state request every
3000 ms, well inside the window, does not satisfy it.

**For a builder: anything at or below 5000 ms is safe**, and there is nothing to
gain by going far below. This build sends one every 1500 ms, from a task that
does not touch the Casio transaction state.

## The bench console

Two settings are adjustable at runtime over the serial monitor, at 115200 baud.
They are teaching controls now rather than unknowns, and they are how every
measurement here is reproduced.

| command | what it does |
|---|---|
| `p <ms>` | telemetry pause — the host wait. `0` by default; set it to 1200 to re-create the wait and show the finding |
| `k <ms>` | app-mode hold period. The tolerance test |
| `m 0\|1` | `1` suppresses the hold while keeping the input poll running |
| `s` | counters &nbsp;&nbsp; `z` zero them |
| `r` | settings &nbsp;&nbsp; `?` help |

**It runs in its own task.** `loop()` is blocked inside the host wait for much
of every cycle, so a console serviced from there would be unresponsive exactly
when you were trying to diagnose something. Same lesson as the hold, third time.

`HOLD_IN_LOOP` at the top of the sketch selects the fault instead of the remedy,
for demonstration. Set it to 1, set the pause to 3000, and watch value B climb.

## Measured, not assumed

- **The web stack costs nothing detectable.** WiFi and Bluetooth share one radio
  in an ESP32, so this was measured: 2685 ms per cycle with the access point
  alone, 2682 ms with the access point, the web server, the WebSocket and two
  background tasks all running. Same floor, same span.
- **The telemetry pause was doing nothing** and now defaults to 0, taking the
  cycle from 2736 ms to 1539 ms. The step-down and its error counters are in
  `FINDINGS.md`, finding 10.
- **The infrared consistency check held over roughly 571 frames** with zero
  failures, checked in firmware on every frame.

> **Bench note.** The droid switches itself off after about ten minutes, sooner
> as the batteries fall, and it watches its own supply well enough to say so on
> its debug channel. **That registers as a link drop without being one.** Check
> the droid is still awake before believing a drop.

---

## Warnings

**Never connect mains electricity** to the calculator, to the board, or to any
wiring attached to either. The ESP32 is not 5 V tolerant; keep every signal
between 0 V and 3.3 V.

**The drive motor runs until it is stopped.** Keep the droid on the floor, clear
of stairs and drops. Parameter 9 stops everything, and every DroidX sequence
ends with a stop — but if the board loses contact with both the calculator and
the browser mid-run, nothing outside the firmware will stop it for you.

**Power-down is deliberately absent** from the command set. A typing error
should not end the lesson.

## The droid side is someone else's work

UUIDs, opcodes and index ranges are taken from
[elitistphoenix/r2d2-robot-apps](https://github.com/elitistphoenix/r2d2-robot-apps),
which reverse engineered them from the decompiled `Assembly-CSharp.dll` of the
original manufacturer's Android app. Nothing about the toy protocol is claimed
as this project's work, and no manufacturer code or assets are redistributed.

Not affiliated with or endorsed by Hasbro, Disney or Lucasfilm.

## References

Fenton, M. (2008). *Authentic Learning Using Mobile Sensor Technology.* New
Zealand Ministry of Education E-Learning Fellowship report.
https://doi.org/10.5281/zenodo.19302276

Fenton, M. (2009). *RIGEL: Learning From Life.* Kuala Lumpur.
https://doi.org/10.5281/zenodo.19334228

Fenton, M. (2026). *Casio Graphing Calculator Serial Interface: Priority
Disclosure.* Zenodo. https://doi.org/10.5281/zenodo.19303911

Fenton, M. (2026). *RIGEL Casio Serial Protocol and Wiring Technical Manual: Microcontroller Data Logging and Measurement and Control with FX-9750 and FX-9860 Graphing Calculators.* Zenodo. https://doi.org/10.5281/zenodo.22095227
