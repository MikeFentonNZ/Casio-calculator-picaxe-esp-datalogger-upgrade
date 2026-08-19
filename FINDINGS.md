# Findings

What is original in this project, what the evidence is, and what is deliberately withheld.

This is the short public version. A formal, timestamped priority record exists separately:

> Fenton, M. (2026). \*Casio Graphing Calculator Serial Interface: Priority Disclosure of Timing Discoveries, Encoding Invention, and Operational Modes (FX-9750 and FX-9860 Series).\* Zenodo. https://doi.org/10.5281/zenodo.19303911

Throughout, **discovery** means a previously undocumented property of an existing system. **Invention** means a method or application that did not previously exist.

\---

## 1\. Host-wait windows in the `RECEIVE()` protocol — discovery

The Casio serial protocol has positions in its `RECEIVE()` exchange where the calculator waits for the attached device **without any deadline**. It raises no COM ERROR however long the wait.

Community documentation (Grindheim, 2001) describes a timeout of roughly one second. That figure is real, but it applies to one place only — the reply to the attention byte. Everywhere else in the transaction, the calculator simply waits.

**Four such positions exist.** They were found by inserting a pause at every point in the device-side flow, line by line, and recording what happened — including every position where a pause *fails*, which is half the result. Two of the four are useful:

* **Gap 2, the description window** — usable, and the one used for manual triggering.
* **Gap 3, the value window** — the one used for interval logging. It sits immediately before the value packet, so a reading taken there is as fresh as it can be when it is sent.

Gaps 1 and 4 are real and deliberately unused: gap 1 is too early to give a fresh reading, and gap 4 comes after the value has already gone.

**On dates, stated as precisely as the record allows.** One or more of the early positions — gap 1 or gap 2, and probably both — were found in 2007/2008. The handwritten notes of that period do not distinguish which, and there was no opportunity to confirm it. **They were not used at the time**: the classroom work of 2008 needed only bidirectional serial communication for data logging and remote control, and where a student triggered a reading, the trigger was on the calculator rather than in the device. **The exhaustive search, the complete enumeration of four positions, and the value window are October 2025.**

**Terminology.** The priority disclosure uses the earlier name *READY-gated timing tolerance*, and designates two windows W1 and W2. The name was changed to **host-wait window** in August 2026, before peer-reviewed publication, and the numbering was extended to all four positions found. The findings in the disclosure are unchanged.

**A figure in the disclosure has since been withdrawn.** It records a validated maximum pause of 50 minutes. Later work found that sessions ending after roughly an hour were caused by **falling calculator battery voltage**, not by any protocol limit — a weak supply ends a session with a COM ERROR and no warning. Those maxima were withdrawn on 7 August 2026. The current position is narrower and better evidenced:

|||
|-|-|
|**Specified working range**|1 to 300 seconds|
|**Demonstrated**|a pause of three hours, held successfully|
|**Ceiling**|none established|

300 seconds is a chosen margin, not an observed limit. A successful long pause does not make the next one safe: fit fresh cells before any long unattended run.

\---

## 2\. Autonomous interval logging — invention

Holding the calculator inside a host-wait window while the device counts out a sampling interval makes the **device** keep time. The calculator simply receives.

That turns a graphing calculator into a datalogger with no specialised hardware — no EA-200, no CLAB, no E-CON application — and the data lands in the calculator's own lists, where its graphing and statistics tools are already waiting.

**One calculator program serves every microcontroller.** It never learns which device is attached.

|platform|interval logging|handshake and value packet|
|-|-|-|
|PICAXE 08M2, 14M2, 18X|validated|validated|
|ESP8266|validated|validated|
|ESP32|validated|validated|
|BBC micro:bit V1, V2|validated|validated|
|Arduino Uno R3 (5 V)|**validated**|validated|

Every build published here was compiled and run to a full interval logging session against an **FX-9750GIII** in 2026. The earlier 2007–2008 work ran on an **FX-9750G Plus**, which is where the two-generation claim comes from.

*Not tested on the FX-9860 series or the FX-CG50.* The FX-9750GIII and FX-9860GIII share one Casio firmware image, so the 9860GIII is expected to behave identically — but expected is not tested, and it is not claimed here.

**Two platform notes worth knowing before you choose a board.** A BBC micro:bit needs **no sensors at all** to start — two of its three channels are inside the board — so a first lesson runs with a cable and nothing else. An Arduino Uno is at the other end: all three channels need a wire, but it is the board most likely to already be in a school cupboard.

**The Arduino Uno needs two components, and both are mandatory.** A 5 V Uno against a 3.3 V GIII works only because of them. The **4.7 kΩ pull-up to +5 V** lifts the calculator's 2.75 V mark to about 3.9 V, clear of the Uno's 3.0 V input threshold — bare, the Uno cannot read a GIII at all. The **1N4148 diode**, bar toward the Arduino, keeps the Uno's 5 V output off a 3.3 V input that is not 5 V tolerant. On this pairing the diode is protection for the calculator, not convenience for the cable.

\---

## 3\. Electrical characterisation of the SB-62 port — discovery

The measurements are taken in 2026 and are documented nowhere by Casio. One of the two conclusions is from 2008.

**Both lines are open-drain with internal pull-ups.** The transmit side follows from direct measurement and appears in no prior source. The receive side was demonstrated in 2026 by replacing a series resistor with a diode that can only *sink* current — the link still transmitted, so the calculator raises its own receive line. That behaviour, though, had already been recorded thirteen years earlier in this project's own first-edition manual of July 2008, which notes that the Casio *"has an active serial in line that sources current"* and fits the diode *"to pull the normally high Casio Rx line low"*. **The 2008 document asserts it and builds a working circuit on it; the 2026 work measures it.** Both are on the record: https://doi.org/10.5281/zenodo.22004530

|calculator|transmit line, held static in a host-wait window|port not in use|
|-|-|-|
|FX-9750GIII|2.75 V|0 V — high impedance, about 120 kΩ|
|FX-9750G Plus|4.75 V|not yet measured|

Two consequences that matter to anyone building against this port:

* **A pull-up on the receive side is not optional.** Serial lines idle high; the calculator's port does not define an idle state when it is not in use. Without a pull-up, a listening device sees a continuous break condition.
* **A device need only sink current.** That makes the interface voltage-agnostic, and it is why one cable serves two calculator generations thirteen years apart.

**A superseded figure, recorded because it was published.** Earlier material from this project stated the calculator "drives its transmit line weakly, about 1.4 V at idle". **Withdrawn.** 1.4 V was a multimeter averaging a line switching between 0 V and 2.75 V. The line is weakly *driven* — high source impedance — but its voltage is correct.

\---

## 4\. Mantissa Frame Encoding — invention, method withheld

MFE encodes several synchronous sensor readings into a **single** `RECEIVE()` transmission. The calculator receives one value and recovers every reading from it using only arithmetic available in Casio BASIC on all supported models, including the oldest.

All sensors are read together inside the host-wait window immediately before that one transmission, so every reading in a frame belongs to the same instant — the condition required for valid multi-variable investigation.

**The same decoder program, unchanged, decodes transmissions from every platform.**

> \*\*The encoding algorithm, the field structure and the decoder are withheld pending peer-reviewed publication.\*\* They are not in this repository and are not in the priority disclosure. What is published here is the \*\*NSN\*\* method — one value per `RECEIVE()`, which needs no encoding at all.

\---

## 5\. A corruption mode in the calculator's own decoder — discovery, details withheld

There is a previously undocumented condition under which the FX-9750GIII's own number handling **silently loses or corrupts part of a received value**. It is deterministic and reproducible, not intermittent.

**This is a property of the calculator, not of anything in this project.** It constrains *anything* transmitted through that decoder near the limit of its precision, whether or not the person doing it has ever heard of this work.

> \*\*The condition, the position affected, and the design rule derived from it are withheld\*\* pending peer-reviewed publication — for the same reason as the encoding itself. Stating them precisely would disclose structure of the withheld method. Both appear in the paper.

What can be said without enabling:

* It was established from **44 observations across six hardware sweeps**, with every value in service measured rather than inferred.
* **The mechanism is not established and is not claimed.** What is established is the condition and the rule that avoids it.
* A simple design rule makes the failure **unformable**, and that rule is applied in every build this project ships.
* **It does not affect NSN**, the method published in this repository. NSN carries one ordinary four-digit value per transaction.

*Not tested on the FX-9860 series or FX-CG50. It is a decoder property, so it may differ between models.*



\---

## 6\. What the calculator can be — modes and applications

Status is stated per row. All of these are built.

|mode|what it does|status|
|-|-|-|
|**Display and datastore**|receives, displays, stores, graphs|classroom-validated, 2008 report|
|**Analysis terminal**|the calculator's own regression and statistics tools, on data it captured|classroom-validated|
|**Remote control / HMI**|the calculator's keypad sends values to the device, which acts on them|demonstrated 2008 (robot); model door lock tested 2026|
|**Closed-loop measurement and control**|the two combined — observe, decide, act, observe the effect|implemented, not classroom-trialled|



### The field survey logger — a claimed invention, not yet built

Every mode above is driven by a clock or by a request. This one is driven by **a person deciding that now is the moment worth recording.**
This mode wsa used in 2008 for manual trigering manual sensor readings.

This work extends on that. The device supplies what a device is good at — the sensor readings, and elapsed time it has counted itself. The **user supplies the category**, typed on the calculator's own keypad: `1` = scoria, `2` = sand, `3` = basalt; or vegetation type, land-use class, surface condition. One row of the record is a human judgement that no sensor can make, sitting alongside readings that no human can make.

That combination — **automatic sensor acquisition, device-maintained elapsed time, and user-entered categorical observation, triggered by the observer rather than by an interval** — is claimed as an original invention in the priority disclosure.

**Status: the components are proven, the combination is not built.**

|component|status|
|-|-|
|device-maintained elapsed time and sensor acquisition|validated on six platforms|
|category entry from the calculator keypad via `Send(`|proven — 2008 robot control, and the 2026 door lock|
|**position and true clock time from GPS**|proven on ESP32 in a companion project of the author's — a u-blox NEO-8M on a spare hardware serial port, using the standard `TinyGPS++` library. Straightforward, not speculative.|
|**the whole combined into a survey logger**|**described and claimed; not yet built as a Casio application**|

**On timestamps, and on what the released code is for.** The published code carries no clock — the calculator computes elapsed time as `interval × (reading number − 1)`. **That is not a gap to be fixed.** What is published is a proof of concept: the simplest thing that demonstrates the method and can be read end to end. It needs no modification to do what it was built for.

Where a true clock time is wanted, there are two straightforward routes, both set out in `teaching\_resources/TEACHER-timing-guide.md`. An ESP board can join a network at startup, take **one** internet time check, set its clock and revert to serving its own access point — with the true time recorded in **its own List**, alongside rather than instead of elapsed time. Or a **GPS module supplies time and position together with no network at all**, which is the better answer for anything outdoors and is what a field survey would use.

**Why it is worth building.** A field survey is the case where the calculator being *in the student's bag already* stops being a cost argument and becomes a practical one. A logger that goes up a riverbank, records substrate at each station, and comes back with the data in Lists ready to graph is a different instrument from one never permitted to leave the classroom.

\---

## Prior work

Naming the strongest thing found is what makes a prior-art statement worth reading.

* **Erik Grindheim (2001)** — published the CFX-9950G communications protocol. Written from a personal computer, which explains what it does not contain: one timeout of 0.5 to 1 second, and no host-wait window.
* **Tom Lynn (1999)** — earlier community documentation of the interface.
* **Michael Fenton (2004–2008)** — adapted Grindheim's work after correspondence with Andrew Hornblow, and published PICAXE-to-Casio example code and protocol notes on the Revolution Education forum.
* **Anobium (2011–2012)** and **nsg21 (2018)** — both worked from this project's published manual and code, both credited it, and neither claimed precedence. On 30 December 2011 Anobium wrote: *"most roads led to the Nexusresearch website. I have the Picaxe code from Micheal Fenton. The code does work… all of the Picaxe searching I did provided nothing except for Micheals good work."*

  Anobium's logger uses a fixed delay before the END packet as its sampling interval. The delay excludes the transaction time, so the interval drifts, and with more than one sensor the error compounds across successive `RECEIVE()` requests — the condition that synchronous multi-sensor encoding exists to remove. 
* **MiniExperimenter (shabaz123, 2020)** — the most technically sophisticated independent prior work identified. It uses the EA-200 hardware protocol at a different baud rate, does not address interval timing, and does not encode multiple sensors within a single transmission.

The **diode used as an open-drain level translator** in the interface cable is standard practice in mixed-voltage interfacing and is **not** claimed as novel. What is claimed is its application to the Casio SB-62 interface, arrived at independently in 2007 without reference to that literature, and — on the evidence examined — the earliest known use of the configuration on this interface.

Casio's EA-100 and EA-200 data acquisition units are discontinued. The Casio New Zealand office confirmed in 2026 that the last unit was sold in this market in 2008.

\---

## Citing this work

> Fenton, M. (2026). \*Casio Graphing Calculator Serial Interface: Priority Disclosure.\* Zenodo. https://doi.org/10.5281/zenodo.19303911

Related publications:

* Fenton, M. (2026). *Casio Protocol and Wiring Guide: Serial Communication, Wiring and Packet Structure for Microcontroller Data Logging and Control (Second Edition).* Zenodo. https://doi.org/10.5281/zenodo.22003135 — the technical manual for this repository: the wiring, the `RECEIVE()` sequence, every byte of every packet, and the classroom activities. **It is also in this repository**, identically, in [`serial_protocol_technical-manual/`](serial_protocol_technical-manual/RIGEL-Casio-Serial-Protocol-Technical-Manual.pdf) and [`teaching_resources/`](teaching_resources/RIGEL-Casio-Serial-Protocol-Technical-Manual.pdf). Cite the DOI, not the filename.
* Fenton, M. (2008). *Connecting the PICAXE 08M and PICAXE 18X to the Casio 9750G Plus graphics calculator* and *RIGEL / CASIO Data logger manual.* Zenodo. https://doi.org/10.5281/zenodo.22004530 — the first edition, deposited unaltered with its original 2008 file properties. The interface circuit it describes is unchanged in the second edition.
* Fenton, M. (2008). *Authentic Learning Using Mobile Sensor Technology.* New Zealand Ministry of Education E-Learning Fellowship report. Zenodo. https://doi.org/10.5281/zenodo.19302276
* Fenton, M. (2009). *RIGEL — Learning From Life: Communities of Learning via a Connected Curriculum.* Microsoft Partners in Learning Regional Innovative Teachers Conference, Kuala Lumpur. Zenodo. https://doi.org/10.5281/zenodo.19334228

\---

*Michael Fenton MRSNZ — New Plymouth, New Zealand. Licensed CC BY-NC-SA 4.0; see* [*LICENSE*](LICENSE)*.*

