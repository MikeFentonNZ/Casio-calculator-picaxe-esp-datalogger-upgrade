# The calculator as a control panel

### A note for teachers, with a security lesson built in

Michael Fenton MRSNZ — March 2026

---

## The short version

Everything else in this project uses the calculator to **record** measurements.
This use turns it around: the calculator becomes the panel a person operates,
and the microcontroller does something in the world as a result.

That has a name, and using the right one matters more than it looks.

> **The companion guide is `TEACHER-timing-guide.md`**, which covers the
> recording use — how accurate the time axis is, which interval to choose, and
> where each platform runs out. **Its figures are measured on hardware. The ones
> here are not:** the door-lock build is written and has never been run.

---

## What to call it

**A Human-Machine Interface — HMI.** In industry an HMI is an operator's means
of monitoring and controlling machinery. It ranges from a panel of buttons and
lamps to a graphics terminal running dedicated software. A graphing calculator
showing a status line and taking a keypress sits comfortably inside that
definition.

### The three things the calculator does in this project


| what it is doing | terminology|
|---|---|
| screen and store while a run is in progress | **display and datastore** |
| retrieving a stored record afterwards for analysis | **analysis terminal** |
| showing readings and switching a process | **HMI - Human machine interface** |

---

## The lock, and why it is a good lesson

The build these notes accompany is a model door. **No input hardware at all** —
the student types the PIN on the calculator, which already has a numeric keypad
and a screen. Two LEDs echo the state and a PICAXE decides. An optional latching
solenoid turns it from a panel into a mechanism.

**It is not a security device and must never be presented as one.** 

### Four steps a class can perform

**1. The PIN travels the cable in clear — and that is the lesson, not a defect.**
The student types the code on the calculator, and the calculator sends it to the
microcontroller over an unencrypted 9600-baud link on an exposed 3-pin jack.
Give a group a second microcontroller and a serial monitor and they will read
the code as it is typed. It takes one lesson and it is far more convincing than
being told.

Real systems send credentials over links all the time. **Whether that link can
be listened to is the first question worth asking about any of them**, and most
people never ask it.

**2. Where the secret lives is still a decision, and a separate one.** The
*correct* PIN is stored in the microcontroller, not in the calculator's program.
Store it in the calculator instead and a student opens the door by **listing the
program** — no wires, no monitor, no cleverness. Ask a class which arrangement
is better *before* telling them, then let them defeat the weak one.

The rule underneath: **the secret belongs with the thing being protected**, not
with the thing a user is holding.

**3. A PIN to be checked is not an unlock command.** The calculator submits a
candidate; the microcontroller decides. There is **no message that opens the
door directly**, and there must never be one — anyone able to *write* to the
cable could then open it without knowing the PIN at all, and every part of the
PIN mechanism would be decoration.

**Ask a class which commands a system should accept from a channel it cannot
trust.** Notice that "all of them" is the answer most real designs give by
default. This is the most transferable idea in the whole build.

**4. A four-digit PIN is 10,000 possibilities, and that is a number, not a
feeling.**

| attack | time |
|---|---|
| one attempt per second, no lockout | 10,000 s ≈ **2 h 47 min** worst case, ~1 h 23 min average |
| three attempts, then a 60-second lockout | ≈ **58 hours** worst case, ~29 hours average |
| six-digit PIN, one per second, no lockout | ≈ **11.6 days** |

Have the class calculate the first row, implement the lockout, then calculate
the second. The defence is now motivated by their own arithmetic rather than by
instruction, which is the difference between a rule and an understanding.

**Then point out what the table hides.** None of those times matter to the group
who read the PIN off the cable in step 1. Rate-limiting guesses is worth doing
only *after* the easier routes are closed — and spending effort on the lock
while the window is open is a real and very common engineering mistake.

---

## Fail states — get the vocabulary right before teaching it

**Fail-safe** means the lock *releases* when power is removed — power is applied
to keep it locked. 
**Fail-secure** means it *stays locked* when power is removed — power is applied
to release it.

**Both terms describe entry, not exit, and this is the part most people get
backwards.** A fail-secure lock does not trap anybody: escape is provided
mechanically, by the lever or push-bar on the inside, which works whether or not
there is any electricity in the building. It is the *fail-safe* lock that
carries the extra duty, because a door that unlocks itself in a power cut has to
be tied into the fire alarm system deliberately rather than by accident.

So the choice is a security question with a safety constraint attached, not a
straight safety question:

| | releases on power loss | typical use |
|---|---|---|
| **fail-safe** | yes | internal escape-route doors, where the door must open in an outage |
| **fail-secure** | no | main entrances, stockrooms, server rooms — where you do not want the door falling open, and the inside lever handles egress |

**Everything here is a model door on a bench.** Never a door in an occupied
building, and never a door anyone could be behind. That is not a compromise for
cost — it is so the class can argue about the distinction with nothing
depending on their answer.

### The design principle, at higher stakes

Every source file in this project carries one rule:

> **A fault must never resemble a result.**

On a logger, a fault that resembles a result is bad data. On a door it becomes:

> **A fault must never resemble a valid unlock.**

A dropped byte, a timed-out read, a half-received packet, a brown-out when a
coil fires — every one of these must leave the door closed. Not because closed
is safer in general (see the table above — it is not, universally) but because
*an unlock must be the result of a decision, never of a failure*. This is the
same rule the whole project runs on, with the consequences visible.

---

## The two builds

The lock is built twice, in this order, and the order is the lesson.

### Baseline — two LEDs

Red for locked, green for unlocked. Two LEDs, two resistors, a few cents, and
nothing that moves. Every class can build this one, which is the point of it.

**What it teaches**

**An indicator is a claim, not a fact.** The green LED shows what the
microcontroller *believes*. That sounds obvious said aloud and is very easily
forgotten in front of a working panel — most people read a green light as
evidence about the world rather than as a report from a program.

**Dark is not safe.** Cut the power and both LEDs go out. Not red. *Nothing*.
An observer cannot distinguish "locked" from "switched off" by looking at an
unlit red LED. Absence of an alarm is not evidence of safety — the same error
sits behind an untested smoke detector and an unmonitored server.

**One source of truth, two views.** The state now appears in two places, on the
LEDs and on the calculator. That is safe *only* because both are drawn from a
single variable in a single routine. The moment a second piece of code lights an
LED on its own initiative, the two displays can disagree — and that is the exact
failure this project has caught in its own files more than once.

**What it cannot teach**

Nothing physical moves, so nothing physical can go wrong. **The indicator can
never be caught lying**, which means the first lesson above has to be *asserted*
rather than demonstrated. A student is entitled to be unconvinced.

And it shows no fail state. Both LEDs going dark is honest, but it is passive —
the system is not *deciding* anything, it has simply stopped.

### Extension — a latching solenoid

A latching, or bistable, solenoid takes a *pulse* to change state and then holds
its position with no current at all. Two coils, two driver transistors, two
flyback diodes and its own supply. It is markedly more expensive than everything
else in the build put together, which is why it is an extension for one shared
rig rather than a per-student component.

**The LEDs stay.** The solenoid does not replace them. Watching the indicator
and the mechanism disagree is the whole reason for building it.

**What it teaches**

**State can be held without power.** A plain solenoid burns hundreds of
milliamps for as long as the door stays open; this one draws nothing between
pulses. A real engineering trade-off with a real number attached, and a class
can measure both.

**It is a flip-flop you can hold in your hand.** Two stable states, changed by a
pulse, retained indefinitely. The same idea as a memory cell, at a scale that
clicks audibly.

**Real driver electronics.** A microcontroller pin cannot switch a coil.
Discovering *why* — current, inductance, and the spike a collapsing field
produces — is a genuine electronics lesson that the LED version cannot motivate.

**And the one that matters: the mechanism remembers, and the controller
forgets.** Power-cycle the rig while the bolt is open. The solenoid holds
position, because that is what latching means. The microcontroller boots knowing
nothing, assumes locked, and lights the red LED.

> **Red light. Open bolt. The indicator is now lying** — and a student produced
> it deliberately, by pulling a plug.

That single demonstration converts the baseline's central lesson from something
claimed into something seen. It is also, precisely, the fault this project's
design principle forbids, which makes it worth producing on purpose in a lesson
rather than discovering by accident in service.

**The two honest fixes**, neither free: drive the bolt to a known position at
power-up, which is blind and wastes a pulse but restores agreement; or fit a
position sensor and *read* where the bolt actually is. The second is closed-loop
control, and it is the answer a real system gives. The code takes the first.

**What it cannot teach**

**It has no fail state either.** Latching means it holds wherever it was when
the power died. It cannot demonstrate fail-safe against fail-secure, and that is
the exact price paid for the zero holding current.

---

## Compare and contrast — what each build does and does not show

| | LEDs only | LEDs + latching solenoid |
|---|---|---|
| cost | cents | more than the rest of the build combined |
| every student can build one | **yes** | no — one shared rig |
| an indicator is a claim, not a fact | asserted | **demonstrated on demand** |
| dark is not safe | **yes** | yes |
| one source of truth, two views | **yes** | yes, and now three |
| state held with no power | no | **yes, and measurable** |
| bistability, a flip-flop you can hear | no | **yes** |
| driver electronics, inductance, flyback | no | **yes** |
| controller and mechanism can disagree | impossible | **yes — the reason to build it** |
| closed-loop control motivated | no | **yes** |
| fail-safe versus fail-secure | **no** | **no** |
| an actual security device | no | no |

**Read the last three rows together, because they are the honest part.**

Neither build demonstrates a fail state. Both LEDs go dark; the latching
solenoid holds. **If you want that lesson you need something with a spring
return** — a plain solenoid, or a small relay, which is an electromagnet pulling
an armature against a spring and costs almost nothing. Cut its power and it goes
to the same place every time. That is a third demonstration, cheaper than the
second, and it teaches something neither of the others can.

And neither build is a lock. The PIN is four digits, the enclosure is cardboard,
and the whole apparatus sits on a bench. What is being modelled is not security
but *reasoning about* security: where a secret should live, which commands a
system should accept from a channel it cannot trust, and how to tell the
difference between what a machine reports and what is true.

**The order is deliberate.** Build the LEDs and the lesson about indicators is a
claim the teacher makes. Add the solenoid and the same lesson becomes something
the class produces at will, with a plug. Neither half is as good alone as the
sequence is together — and the expensive half is only worth its price *because*
the cheap half came first.

---

## A worked example for students

> A class builds the model lock with a 4-digit PIN and no lockout. A second
> group is asked to open it without being told the code.
>
> **Questions**
> 1. Working at one attempt per second, how long would trying every code take?
>    What is the *average* time, and why is it not the same number?
> 2. The class adds a 60-second lockout after three wrong attempts. Recalculate.
>    By what factor did the defence improve?
> 3. The second group connects a serial monitor to the cable. They cannot read
>    the PIN. What *can* they learn, and why might that be useful to them?
> 4. A third group proposes sending an "unlock" command from the calculator so
>    a teacher can open the door remotely. Explain what this would cost, and
>    suggest a version of the feature that does not cost it.
> 5. The lock is commanded closed when contact with the calculator is lost.
>    Give one situation where this is the right behaviour and one where it is
>    dangerous.
> 7. With the latching solenoid fitted, a student switches the power off while
>    the bolt is open, then switches it back on. Describe what the red LED
>    shows and what the bolt is doing. Which one is telling the truth?
> 6. The lock re-closes after 10 seconds using the PICAXE's `time` clock, which
>    is known to run slow. Does the door stay open for more or less than 10 real
>    seconds, and is that error in the safe direction?

*Answers: 
1. 10,000 s ≈ 2 h 47 min worst case; the average is half that, because
on average the correct code is found halfway through the search. 
2. About 58
hours, roughly a twentyfold improvement. 
3. The number of digits typed, in real
time, and the moment a code was accepted — so they learn when someone is at the
door, how long they took, and whether they succeeded. That is traffic analysis.
4. It would let anyone able to write to the cable open the door without knowing
the PIN, making the whole PIN mechanism pointless. A safe version keeps the
asymmetry: allow a remote **lock**, never a remote unlock. 
5. Right for a
cabinet or store room; dangerous for any door a person might need to escape
through. 
6. Longer than 10 real seconds, because a slow clock takes longer to
count to 10. For a door that is the unsafe direction — it stays open longer than
intended — which is why the header says so plainly rather than hiding it.
7. The red LED shows locked, because the microcontroller assumes locked on boot;
the bolt is still open, because a latching solenoid holds position without
power. The bolt is telling the truth. The LED is reporting a belief, and the
belief is wrong.*

---

## Why this belongs in a data-logging project

Because it is the same protocol, the same cable and the same calculator, doing
something that is not logging at all — which shows learners that what they have
learned transfers.

It also demonstrates one thing the loggers cannot. The calculator tolerates
long pauses at four points in its exchange, and the logging builds use one of
them to wait out a sampling interval. **A door must not keep a person waiting,
so this build uses none of them.** The window is a *permission*, not an
obligation. That is difficult to show with a logger, where waiting is the whole
purpose.

---

## The build these notes accompany

`Casio-HMI-08M2.bas` — **written 12 August 2026, never run.**

- **PICAXE 08M2, the cheapest part in the family.** Two pins for the calculator,
  two for the LEDs, one spare.
- **No input hardware at all.** The calculator's own keypad takes the PIN and
  sends it with `Send(P)`. An earlier draft wired a separate 4×3 keypad through
  a resistor ladder; it needed a 14M2, twelve ADC bands measured by hand before
  it would run at all, and it made every keypress lag about half a second
  because the calculator has to poll to learn what was typed. It also argued
  against the premise — the whole point is that the device already in the bag
  can do more.
- **Named for its role, not its encoding**, matching `Casio-IMC-14M2.bas`.
- **NSN encoding.** One value per exchange is enough, and it keeps the code and
  the cognitive load smaller than the other encoding method would.
- **The correct PIN lives in the PICAXE**, never in the calculator's program.
- **Two LEDs driven from one routine**, `show_state`, and nowhere else.
- **The watchdog and the power-up both COMMAND the locked state.** Neither
  relies on an actuator falling to a safe position, because neither of these
  actuators has one.

**The solenoid extension needs a 14M2, and that is a hardware fact rather than a
preference.** A latching solenoid takes two pins — one coil to throw the bolt
open, one to throw it closed — and the 08M2 has one pin left after the
calculator and the LEDs. So the low floor runs on the cheapest chip in the
family, and the extension costs a bigger chip as well as a more expensive
actuator.

### Two things to expect

**Normalised scientific notation loses leading zeros**, which is why the status
word is offset by 1000 before it is sent. `1.DDD × 10^E` cannot carry `0012`; it
arrives as `12`. Adding 1000 guarantees four digits and makes any received value
below 1000 a broken packet rather than a plausible status. It is the same trick
as the +5000 offset that carries sign on the sensor channels, and worth showing
a class as a general technique rather than a special case.

*The PIN needs no such treatment, and it is worth saying why rather than just
doing it.* A PIN here is a **number**, not a digit string — the calculator's
`?→P` prompt cannot accept a leading zero in the first place, so `0042` and `42`
are the same code and there is nothing to preserve. The offset is not
decoration; it is there for a specific failure that this particular value cannot
have.

**One bug is already recorded in the file, and it failed in the dangerous
direction.** The auto re-lock originally stored an *end* time — `time + 10` —
and waited for the clock to pass it. `time` is a 16-bit count of seconds and
wraps roughly every 18 hours; set that timer just before a wrap and the addition
overflows, so the comparison never becomes true **and the door never re-locks**.
It now stores the *start* and measures elapsed time by unsigned subtraction,
which wraps correctly. Worth showing a class: the same bug in the lockout timer
would merely have kept someone waiting, and nobody would have called it serious.
