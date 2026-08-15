# How accurate is the time axis?

### A note for teachers, with a worked example for students

Michael Fenton MRSNZ — 8 August 2026, figures updated 9 August 2026

---

## The short version

**The calculator does not measure time. It assumes it.**

Nothing in the recorded data is timestamped. The calculator builds its elapsed
time column by arithmetic — reading 1 is at 0 seconds, reading 2 is at *T*
seconds, reading 47 is at *T* × 46 — where *T* is the interval you asked for.

If the microcontroller runs slightly slow, **nothing fails**. No error appears,
no reading is dropped, no gap opens in the data. The recorded time axis is
simply stretched, uniformly, and everything looks perfectly normal.

Worth saying plainly to students: *this is an error that does not look like an
error.*

---

## What it means for an investigation

| this survives | this does not |
|---|---|
| the **shape** of a curve | the **rate** of change |
| where a peak or trough falls in the sequence | how long the peak took to arrive |
| whether one thing happens before another | half-lives and time constants |
| comparing two runs at the same interval | gradients in units per second |

---

## The numbers

Timed against a stopwatch. **Every figure below is measured**, across four
builds — two chips (08M2, 14M2) and two encodings (one sensor, three sensors).

| interval | time-axis error | measured over |
|---|---|---|
| **1 second** | **4.4 – 5.0 %** | up to 997 intervals |
| **2 seconds** | **2.5 – 3.1 %** | 100 intervals |
| 5 seconds | ~1 % | interpolated |
| **300 seconds** | **0.12 %** | 4 intervals, one build |

The spread at each interval is the difference between chips and between the
one-sensor and three-sensor builds — the single-sensor version is a few
milliseconds quicker per reading, as you would expect.

**A full list has been run.** 998 readings at a 1-second interval, 17 minutes
21 seconds, no failure and no dropped sample. The Casio holds 999, so that is
the whole capacity of the instrument in one go. **Total drift across the full
list: 44 seconds.**

**How much better this is than it was.** The original `serout` build lost
410 ms per reading; the current builds lose **44–50 ms**. At a 2-second interval
that is a time-axis error of 2.5–3.1 % where it used to be 20.5 %, and at
1 second **4.4–5.0 %** where it used to be 41 %.

**What is measured and what is not.** All four builds are measured at 1 second,
both chips at 2 seconds, and **one** build at 300 seconds — a second build runs
correctly there but was not timed precisely, so its figure is not quoted. Only
the 5-second row is interpolated. **If a result depends on the time axis at an interval you have not
checked, time a run against a wall clock** — it takes four minutes and the
method is at the end of this note.

**The short end is the strong end**, which is worth knowing because it is the
opposite of the usual worry. The per-reading loss is *smaller* at a 1-second
interval than at 2 seconds on **both** chips — 47.7 against 50.9 ms on the
08M2, and 50 against 62 ms on the 14M2. The fastest sampling rate is not the
one under most strain, because the cost is mostly fixed per reading and only
the small remainder grows with the waiting.

---

## Choosing an interval

| you want | use | why |
|---|---|---|
| **a fast-changing process, or a first taste of data logging** | **1 second** | under 5 % — fine for shape, see below |
| the shape of something | 2 seconds or longer | 2.5–3.1 % |
| a rate, gradient or time constant | 10 seconds or longer | comfortably under 1.5 % |
| a published or theory-compared result | 30 seconds or longer | well under 1 % |

### One-second sampling on a PICAXE: available, with a caveat

**It works.** The logger keeps up at 1 Hz — every reading collected, no
dropped samples. Watch the calculator's display against a ticking clock and
it advances with the ticks.

**And the time axis is 4.4 – 5.0 % long**, depending on chip and build. Over a
three-minute run that is about 8 seconds of accumulated stretch; over a full
998-reading list, 44 seconds. So:

| for a 1 Hz PICAXE run | verdict |
|---|---|
| the shape of a decay, a rise, a peak | **fine** — shape is unaffected |
| when something happened relative to something else | **fine** |
| how many seconds a process took | ±5 % |
| a half-life or decay constant | use a longer interval, or a faster board |

**A worked case.** Recording the brightness of a glow-stick at 1-second
intervals for three minutes is a genuinely useful investigation. 180 readings:
the calculator reports 179 seconds, the run really takes 187.5, and the decay
curve comes out with exactly the right shape while students watch it fall away
in real time. If the class then wants the decay *constant*, they either repeat
it at a longer interval — where the error drops below 1.5 % — or move to a
board built for it.

**This is a platform-choice message, not a warning.** The least expensive and
least capable microcontroller should not be expected to serve the highest
precision work. Learners pick the platform that matches the precision the
question needs, and knowing *why* is part of what the exercise teaches.

**Where to go for precise 1 Hz timing:** an ESP8266 or ESP32, both verified
exact against a wall clock at 1 Hz over 259 and 234 readings.

**The micro:bit has not been tested at 1 second** and should not be assumed
equivalent; it has only ever been run at 2 seconds.

---

## Temperature sensors: the one place the PICAXE runs out

A **DS18B20** is a digital temperature sensor that needs no calibration, which
makes it attractive for a class. On a PICAXE it costs more than it looks.

**Reading one takes 750 milliseconds, and the chip's clock stops for all of
it.** The PICAXE cannot run its timer and talk to a 1-Wire sensor at the same
time — the sensor's timing is measured in microseconds and has to be produced
by the processor itself.

So each reading costs about **0.8 seconds** of lost time:

| interval | time-axis error with a DS18B20 |
|---|---|
| 5 seconds | **16 %** |
| 10 seconds | 8 % |
| 16 seconds | 5 % |
| 30 seconds | 2.7 % |
| 5 minutes | 0.3 % |

**What you will see on the bench.** Set 5 seconds and the gaps between readings
look like 6, 5, 6, 6, 6, 5. That is not the logger misbehaving. Each interval
really takes 5.8 seconds, and 5.8 cannot be read as a whole number of seconds.

**The 5-second minimum is still allowed, on purpose.** At 5 seconds a DS18B20
gives a perfectly good *shape* — a cooling curve, a warming curve, the moment
something changes — with a time axis 16 % long. That is the same bargain as
1-second sampling with analogue sensors: the capability is offered and the cost
is named. **For a rate or a time constant from a DS18B20, use 30 seconds or
more.**

**And this is where the PICAXE platform ends.** Not because it is slow, but
because no arrangement of PICAXE code can recover those 750 milliseconds. An
ESP8266 or ESP32 has hardware timers that keep counting while a sensor is read,
so a DS18B20 costs it nothing in time. **That, rather than raw speed, is the
reason to change board** — and it is a good thing for students to see: a limit
that belongs to the architecture rather than to the programmer.

**A thermistor or LDR does not have this problem at all.** They read in about a
millisecond, so the figures in the table above this section apply.

### If the temperature channel reads 4096 (MFE) or 999 (NSN)

**No sensor answered.** The logger asks the 1-Wire bus for the sensor's identity
code **before every single reading**, and reports that value when nothing
replies. Reconnect the signal wire and normal readings resume on the very next
sample — there is no need to switch anything off and on.

On an MFE build the flag beside it will read **89**, which means "system fault".
A healthy run shows flag **10**.

**Why the reading is not simply reported as zero.** A disconnected sensor makes
the PICAXE read 0 °C — a perfectly good temperature, and the one a class
produces deliberately when calibrating in ice water. So the logger decides
whether a sensor exists by asking a *different question* from "what is the
temperature", and a reading of 0 can therefore be trusted to mean zero degrees.

**Why 4096 and not something rounder.** The largest reading the encoding can
carry is 4095. 4096 is the smallest number it cannot possibly produce, so it
announces itself as a fault to anyone who knows the range.

---

## Choosing a platform

| platform | 1-second sampling | notes |
|---|---|---|
| **PICAXE 08M2 / 14M2** | **works, 4.4–5.0 % time axis** | lowest cost, single chip, no board. Full 998-reading list proven. **Not with a DS18B20** — see the section above. |
| **ESP8266 / ESP32** | **verified exact** | needs a board and USB. **The only option if you want a DS18B20 and an accurate time axis.** |
| **micro:bit** | **not yet tested** | only ever run at 2 seconds |

The PICAXE is not the weakest option, it is the *cheapest entry point*: one
chip, no board, and by far the most thoroughly documented troubleshooting notes
of any platform here. It now reaches every sampling interval the calculator
supports, from 1 second to 300, and the only thing that varies is how much
precision the time axis carries.

---

## A worked example for students

> A class records cooling water every 2 seconds for 500 readings on a
> PICAXE 14M2.
>
> **The calculator says:** the last reading was taken at 2 × 499 = **998
> seconds** (16 min 38 s).
>
> **In reality:** each reading costs about 0.062 s of lost time, so the run
> actually took 499 × 2.062 = **1029 seconds** (17 min 09 s).
>
> **The time axis is stretched by 31 seconds — about 3.1 %.**
>
> **Questions**
> 1. The class calculates a cooling constant from the gradient. Is their answer
>    too large or too small, and by roughly what percentage?
> 2. The *shape* of their cooling curve is correct. Explain why the shape
>    survives an error that changes every single time value.
> 3. Another group runs the same experiment at 30-second intervals. Why is
>    their percentage error so much smaller, even though the number of readings
>    is the same?
> 4. Suggest a way to check whether the recorded time axis is accurate, using
>    only a clock on the wall.
> 5. Last year this same experiment would have had a 20.5 % time error instead
>    of 3.1 %. Does that change which of your conclusions you would trust?

*Answers: 1. Too large by about 3 %, because the true interval is longer than
the calculator assumes, so the real gradient is shallower. 2. Every time value
is stretched by the same factor, so the relative positions of all points are
unchanged — only the scale of the axis is wrong. 3. The loss happens once per
**reading**, not once per second, so a longer interval spreads the same fixed
cost over more elapsed time. 4. Note the wall-clock time when the first reading
appears and when the last one does, and compare with the elapsed time the
calculator reports. 5. Conclusions about shape and ordering were always safe;
conclusions about rates were not.*

---

## A companion guide, for a different use of the same equipment

This guide is about the calculator **recording** measurements. The same cable,
the same microcontroller and the same protocol will also let the calculator act
as a **control panel** — showing the state of something and letting a person
operate it.

`TEACHER-HMI-guide.md` covers that: a model door lock with a keypad, indicator
LEDs and an optional latching solenoid, built as a security *lesson* rather than
a security device. It carries the terminology (HMI, and why HID is the wrong
word), a worked brute-force calculation, the fail-safe versus fail-secure
distinction with the part most people get backwards, and a compare-and-contrast
of what the cheap build and the expensive build each do and do not teach.

**Neither of those builds has been run on hardware.** The figures in *this*
guide are measured; the ones in that guide are design intentions.

---

## Why this guide gives you numbers rather than reassurance

Every limit in this document was found by deliberately pushing the equipment
until it misbehaved, and then measuring where the edge was. **The boundaries are
known, and they are written down as rules you can follow** — which is a
different and better thing from being told the equipment is reliable.

A logger that says "it works" leaves you to discover the exceptions during a
lesson. A logger that says "the time axis is 3.1 % long at a 2-second interval,
and here is why" lets you decide in advance whether that matters for what your
class is measuring. Most of the time it will not. When it does, you will know
before the apparatus is put away rather than afterwards.

## How to check the time axis yourself

Four minutes, no equipment beyond a stopwatch.

1. Set a 2-second interval and start the logger.
2. Start the stopwatch when reading **1** appears.
3. Stop it when reading **101** appears — that is **100 intervals**, not 99.
4. `error % = (elapsed − 200) ÷ 200 × 100`

**Step 3 is the one to be careful about.** Stopping on reading 100 gives 99
intervals, and at long intervals that single miscount is larger than the effect
being measured. Use the same convention every time.

---

## An extension for students: add a live graph, then find out what it cost

**The shipped programs display text, and that is deliberate — every figure in
this guide was measured with the text display.** Changing what the calculator
draws changes how long it takes per reading, so the shipped version stays as it
is and the graph is offered as a task rather than a feature.

It is a good task because the graph is not really the point. **Measuring what
the graph costs is the point**, and a student who does it re-derives the central
finding of this guide for themselves instead of being told.

### The challenge, in three parts

**Part 1 — make it work.** Add a user-selectable live graph to the logging
program. Every reading should appear on screen as it arrives.

**Part 2 — measure what it cost.** Use the stopwatch method above, twice: once
with the text display, once with the graph. The difference is the plotting cost
per reading. Convert it to a time-axis error at the interval you used.

**Part 3 — the one that teaches the most.** Write a second version that redraws
the *whole* graph each time a reading arrives, instead of adding one point.
Measure it at 20 samples, at 100, and at 300.

> **What Part 3 should reveal:** the redraw version gets slower as the run goes
> on. The per-reading cost is no longer constant — it grows.
>
> **Then ask why that is worse than simply being slow.** A constant cost
> stretches every time value by the same factor, so the *shape* of the curve
> survives even though the axis is wrong — that is the argument made near the
> start of this guide. A growing cost stretches later readings more than earlier
> ones, and **the shape itself distorts.** A slow logger that is uniformly slow
> is usable for shape; one that slows down as it runs is not.
>
> This is the difference between an error you can reason about and one you
> cannot, and it is worth more than the graph.

### A starting point

Not a solution — a scaffold, with the interesting decisions left in. **Check
every command against the manual;** this has not been run.

```
ClrText
Locate 1,1,"Display mode"
Locate 1,3,"0 = text"
Locate 1,4,"1 = live graph"
?->G

If G=1
Then Locate 1,6,"Y min"
?->L
Locate 1,7,"Y max"
?->H
Locate 1,8,"Samples to plot"
?->M
ViewWindow 0,T*M,T*10,L,H,(H-L)/10
ClrGraph
IfEnd

Send(T)
Lbl 2
Receive(A)
I+1->I
T*(I-1)->List 1[I]
A->List 2[I]

If G=1
Then PlotOn List 1[I],List 2[I]
Text 1,1,"A="
Text 1,25,A
Else
Locate 1,3,"Sample:"
Locate 9,3,I
Locate 1,4,"A:"
Locate 4,4,A
IfEnd
Goto 2
```

**Three things a student will hit, and each is worth hitting.**

**`Text` and `Locate` write to different screens.** `Locate` writes on the text
screen, `Text` on the graph screen. Mix them and the calculator switches screens
every sample — it flickers, and it costs time. Each branch above stays on one
screen for that reason.

**The window has to be set before any data arrives.** You do not know the range
in advance, which is why the scaffold asks. The tempting fix — rescale
automatically when a point falls outside — requires redrawing everything, and
that is Part 3's problem arriving by the back door. Worth letting a class
propose it and then work out why it is expensive.

**A point outside the window is simply not drawn.** No error, no warning, no
gap in the recorded data — the reading is still in the list. Another instance of
a fault that does not look like one.

### Why this cannot break the link

Worth reassuring a class about, because it looks risky and is not. **The
calculator initiates every transaction**, and between them the microcontroller
simply waits for the next request. That wait has no deadline. However long the
calculator spends drawing, nothing times out and no reading is lost.

**What suffers is the time axis, never the data.** Every value that arrives is
recorded correctly. Only the assumed times attached to them stretch — which is
this guide's subject from beginning to end.

---

## How these figures were obtained

Four successive builds of the logger were timed against each other on one rig
and one set of batteries, each change measured against the previous one. The
per-reading loss fell from 410 ms to 51 ms (08M2) and 62 ms (14M2).

**Three mathematical models of the drift were proposed during this work and all
three were disproved by the next measurement.** The figures above are
measurements. Where a single conservative number is wanted, use **0.07 seconds
lost per reading**.

