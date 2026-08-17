#REM
 Casio-HMI-example-08M2.bas
 (C) Michael Fenton, MRSNZ, 2026
 Licence: Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International 
 (CC BY-NC-SA 4.0)
 
 The calculator as a Human-Machine Interface (HMI), not a logger.

 Two RECEIVE() windows (GAP 1 and GAP 2 in code below) discovered 2007/2008
 using Picaxe 18X connected to a Casio FX-9750G Plus. See the author's
 New Zealand Ministry of Education E-Learning Fellowship report and 
 conference paper; 

 1) Authentic Learning Using Mobile Sensor Technology (2008): https://doi.org/10.5281/zenodo.19302276
 2) RIGEL - Learning From Life, Kuala Lumpur (2009): https://doi.org/10.5281/zenodo.19334228

 https://mikefentonnz.github.io/projects/casio-calculator-data-logger-hack.html

 ================================================================= 
 Version 1.0; 03/03/2026

 *** STATUS: PROOF OF CONCEPT - A FOUNDATION TO BUILD ON ***
 This is a reference implementation based on a validated method.
 It is deliberately minimal so that every line can be read and
 understood. It is NOT a finished classroom product and will not
 suit every use case. MODIFICATION IS EXPECTED - see the notes 
 below. 

 The NSN (normalised scientific notation) PICAXE implementation is
 validated in hardware.

 This is the third member of the family:
   Casio-NSN-08M2.bas - three sensors, normalised scientific notation
   Casio-MFE-14M2.bas - three sensors, MFE, data logging
   Casio-HMI-08M2.bas - Human machine interface - security PIN pad example (THIS)
   Casio-IMC-14M2.bas - three sensors + three actuators

 Author: Michael Fenton. Unbounded host wait and MFE both his discovery.

 *** THIS IS NOT A SECURITY DEVICE. IT IS A LESSON ABOUT ONE. ***
 Teaching notes, the brute-force arithmetic, and the fail-safe
 versus fail-secure distinction are in TEACHER-HMI-guide.md..
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
 ===================================================================
 HARDWARE connections(Picaxe 08M2): Wire colours are users choice

 - Pin C.0 -> to Casio RX [ring of 2.5mm TRS jack, BLUE wire] via 1N4148
 	      diode; bar to the picaxe  (HSEROUT)
 - Pin C.1 <- from Casio TX [tip of 2.5mm TRS jack, YELLOW wire] (SERIN,
              and also the hardware hserin pin - see the header)
 - Pin C.1 -> 4.7k pull-up resistor to V+ (3.3 V) - REQUIRED
 - Pin C.2 -> RED LED   - locked
 - Pin C.4 -> GREEN LED - unlocked
 - Pin C.3    unused
 - 0V      -> Casio GND [sleeve of 2.5mm TRS jack, BLACK wire]

 *** WHY A DIODE HERE AND NOT A SERIES RESISTOR ***
 The diode makes this an OPEN-DRAIN output: the PICAXE can only ever PULL
 THE LINE LOW. When it drives high the diode blocks, and the calculator
 raises the line with its own internal pull-up.  

  - The PICAXE supply no longer sets the calculator's high level, so the
    same wiring serves a 3.3 V FX-9750GIII and a 5 V FX-9750G Plus.
  - No contention is possible, so no series resistor is wanted. Do NOT fit
    a 1k as well - it shares the current path and pushes the LOW level back
    up, which is what breaks the link.
  - BAR (cathode) TOWARD THE PICAXE. Reversed, nothing works.

 1N4148 or 1N914. A Schottky is not required: the low sits near 0.6 V
 against a threshold near 0.82 V, and the drop falls as current falls.
 Verified on PICAXE, ESP8266, ESP32 and micro:bit V1/V2, 2026.

==============================================
  THE CALCULATOR IS THE WHOLE INTERFACE
 -----------------------------------------------------------------
 It already has a numeric keypad and a screen. This build uses
 both, and adds NO input hardware at all:

    the student types the PIN on the CALCULATOR
    the calculator sends it with Send(P)
    the PICAXE decides
    the calculator shows the result, and two LEDs echo it

 A build that uses a wired 4x4 membrane keypad is the wrong shape
 for this project: the whole argument here is that
 the device already in the school bag can do more, so bolting a
 second input device onto it argues against the premise. The
 keypad version also needed a 14M2, twelve ADC bands measured by
 hand before it would run at all, and it made every keypress lag
 about half a second because the calculator has to poll to find
 out what was typed.

 Typing on the calculator costs none of that. It also makes the
 security lesson SHARPER rather than weaker - see below.

 -----------------------------------------------------------------
  SAFETY
 -----------------------------------------------------------------
 - NEVER connect mains electricity (240 V / 110 V) to the
   calculator, to this board, or to any wiring.
 - NEVER use mains-connected equipment near water.
 - This is a MODEL DOOR on a bench. NEVER a door in an occupied
   building, and never a door a person could be behind.
 - Keep all input signals within 0 V to 3.3 V.

 *** WHY THE SOLENOID EXTENSION IS NOT IN THIS FILE ***
 A latching solenoid needs TWO pins - one coil to throw the bolt
 open, one to throw it closed - plus a driver transistor and a
 flyback diode on each. This part has one pin left. So:

      BASELINE  (this file)  08M2, two LEDs, no moving part
      EXTENSION              14M2, the same code plus SET and
                             RESET pins for the solenoid

 That is not a compromise, it is the tiering made honest: the low
 floor runs on the cheapest chip in the family, and the extension
 costs a bigger chip as well as a more expensive actuator. What
 the extension adds, and what it still cannot teach, is set out in
 TEACHER-HMI-guide.md.

 -----------------------------------------------------------------
  THE PROTOCOL SIDE
 -----------------------------------------------------------------
 Same transport as every other build here: hserout throughout,
 hserin for the attention wait and the single-byte ACK waits,
 serin for the two multi-byte packet reads.

   Receive(N)  - how many channels. Always 1. Keeps the standard
                 connect sequence working unchanged.

   Receive(S)  - the status word. ONE number, two fields, offset
                 by 1000:

                     S = 1000 + state*10 + tries

                     state  0 locked      tries 0 to 3
                            1 unlocked
                            2 wrong code
                            3 locked out

                 So S = 1012 means: unlocked, 2 attempts left.

   Send(P)     - the PIN the student just typed. The PICAXE
                 compares it and decides. See THE SECURITY LESSON.

 WHY OFFSET BY 1000? Normalised scientific notation sends
 1.DDD x 10^E, so leading zeros do not survive: a status of 0012
 would arrive as 12. Adding 1000 guarantees four digits and makes
 ANY value below 1000 a broken packet rather than a plausible
 status. 

 THE PIN NEEDS NO SUCH OFFSET, because it is a NUMBER and not a
 digit string. The calculator's ?->P prompt cannot accept a
 leading zero in the first place, so 0042 and 42 are the same PIN
 and there is nothing to preserve. Worth saying out loud to a
 class: the offset is not decoration, it is there for a specific
 failure that this particular value cannot have.

 THIS BUILD USES NO HOST-WAIT WINDOW, ON PURPOSE. The calculator
 tolerates a long pause at four points in its exchange, and the
 logging builds use gap 3 to wait out a sampling interval. A door
 must not keep a person waiting, so every request is answered
 immediately. The window is a PERMISSION, not an obligation -
 which is hard to show with a logger, where waiting is the point.

 -----------------------------------------------------------------
  THE SECURITY LESSON
 -----------------------------------------------------------------
 1. *** THE PIN TRAVELS THE CABLE IN CLEAR. ***
    9600 baud, unencrypted, on an exposed 3-pin jack. Give a group
    a second microcontroller and a serial monitor and they will
    read the code as it is typed. It takes one lesson and it is
    far more convincing than being told.

    THIS IS THE LESSON, NOT A DEFECT OF THE BUILD. Real systems
    send credentials over links, and whether that link can be
    listened to is the first question worth asking about any of
    them.

 2. WHERE THE SECRET LIVES IS STILL A DECISION. The correct PIN is
    stored in the PICAXE, not in the Casio BASIC program. Store it
    in the calculator instead and a student opens the door by
    LISTING THE PROGRAM - no wires, no monitor. Ask a class which
    is better before telling them, then let them defeat the weak
    one.

 3. A PIN TO BE CHECKED IS NOT AN UNLOCK COMMAND, and the
    difference is the whole architecture. Send(P) submits a
    candidate; the PICAXE decides. There is NO message that opens
    the door directly, and there must never be one - anyone able
    to write to the cable could then open it without knowing the
    PIN at all, and the whole mechanism would be decoration.

    Ask a class which commands a system should accept from a
    channel it cannot trust. Notice that "all of them" is the
    answer most real designs give by default.

 4. THE ARITHMETIC IS A MATHS LESSON. A 4-digit PIN is 10,000
    possibilities. At one attempt per second that is under three
    hours, about 1 h 23 min on average. Three attempts then a
    60-second lockout makes it roughly 58 hours. Have the class
    calculate both, then implement the lockout.

    Then point out what the table hides: none of those times
    matter to somebody who read the PIN off the cable in step 1.
    Rate-limiting guessing is worth doing only after the easier
    routes are closed.

 -----------------------------------------------------------------
  A FAULT MUST NEVER RESEMBLE A VALID UNLOCK
 -----------------------------------------------------------------
 - A dropped byte, a timed-out read or a half-received packet
   leaves the door as it was. Only check_pin can open it.
 - Loss of contact with the calculator LOCKS the door after
   WATCHDOG_S. It does not merely stop doing things.
 - The timers use `time`, which this project knows runs SLOW
   because serin stops the clock. The lockout therefore lasts
   LONGER than stated, which fails closed. The auto re-lock also
   lasts longer, which fails OPEN - so it is stated here rather
   than hidden.
 - See check_timers for a wrap bug found and fixed in this file,
   which failed in the dangerous direction.

 -----------------------------------------------------------------
  UNDOCUMENTED PICAXE BEHAVIOUR RELIED ON HERE
 -----------------------------------------------------------------
 - `hserin` on M2 parts is NON-BLOCKING and leaves its target
   UNCHANGED when nothing arrived. $FF is a legal data byte, so
   the target must be a WORD pre-loaded with $FFFF as a sentinel.
 - `hsersetup` disables `sertxd`.
 - `serout` will not work on C.0 unless the pin is driven HIGH
   immediately before the command. It powers up LOW. This file
   uses hserout, but init drives C.0 high anyway.
 - `sertxd` MUST NOT appear in this file. It shares the wire with
   the calculator and will produce a Com ERROR mid-transaction.
 - `pause` is calibrated for 4 MHz and does NOT compensate for
   setfreq. At m16 divide every literal by 4.

 =================================================================
  CASIO BASIC COMPANION PROGRAM
  Check for unwanted spaces, and use the proper Locate command
  rather than spelling it out letter by letter.
 =================================================================

 ClrText
 Locate 1,1,"CONNECTING..."
 Receive(N)
 Lbl 1
 Receive(S)
 S-1000->Z
 Int(Z/10)->A
 Z-A*10->T
 ClrText
 Locate 1,1,"MODEL DOOR LOCK"
 If A=0
 Then Locate 1,3,"LOCKED"
 IfEnd
 If A=1
 Then Locate 1,3,"** UNLOCKED **"
 IfEnd
 If A=2
 Then Locate 1,3,"WRONG CODE"
 IfEnd
 If A=3
 Then Locate 1,3,"LOCKED OUT"
 IfEnd
 Locate 1,5,"Tries left:"
 Locate 13,5,T
 Locate 1,7,"Enter PIN:"
 ?->P
 Send(P)
 Goto 1

#ENDREM

#picaxe 08M2
#no_data
disableBOD

' -----------------------------------------------------------------
'  PINS
' -----------------------------------------------------------------
symbol TO_CASIO_pin   = C.0   ; serial out to calculator (ring)
symbol FROM_CASIO_pin = C.1   ; serial in from calculator (tip)
symbol RED_LED        = C.2   ; locked
symbol GREEN_LED      = C.4   ; unlocked

' -----------------------------------------------------------------
'  PROTOCOL CONSTANTS
' -----------------------------------------------------------------
symbol CASIO_ATTENTION = $15  ; calculator: "are you there?"
symbol PICAXE_PRESENT  = $13  ; our reply: "yes, ready"
symbol CASIO_PREAMBLE  = $3A  ; ':' starts every packet
symbol CASIO_ACK       = $06  ; acknowledge
symbol CMD_RECEIVE     = "R"  ; calculator wants a value
symbol CMD_SEND        = "V"  ; calculator is sending one

symbol VNAME_N = "N"          ; channel count request
symbol VNAME_S = "S"          ; status word request
symbol VNAME_P = "P"          ; incoming PIN to check

symbol SP_intDigit = 28
symbol SP_signInfo = 29
symbol SP_exponent = 30
symbol SP_dec1     = 31
symbol SP_dec2     = 32

' -----------------------------------------------------------------
'  LOCK BEHAVIOUR - the numbers a class would change
' -----------------------------------------------------------------
symbol DEFAULT_PIN  = 1234    ; *** CHANGE THIS ***
symbol MAX_ATTEMPTS = 3       ; wrong tries before lockout
symbol LOCKOUT_S    = 60      ; seconds locked out
symbol WATCHDOG_S   = 10      ; no contact for this long -> lock
symbol RELOCK_S     = 10      ; door re-locks this long after opening

symbol ST_LOCKED   = 0
symbol ST_UNLOCKED = 1
symbol ST_WRONG    = 2
symbol ST_LOCKOUT  = 3
symbol STATUS_BASE = 1000     ; offset: see WHY OFFSET BY 1000

' -----------------------------------------------------------------
'  VARIABLES
'  b19 is scratch for the transport routines - never store state
'  in it. b26,b27 are clobbered by the incoming packet read.
' -----------------------------------------------------------------
symbol flags        = b0      ; bit variables live in b0
symbol isNegative   = bit0    ; set while building a negative packet
symbol isLarge      = bit1    ; set when |value| >= 1
symbol pollOK       = bit2    ; poll_byte result: 1 = byte, 0 = timeout

symbol lockState    = b1      ; ST_LOCKED / ST_UNLOCKED / ...
symbol storedPIN    = w1      ; b2,b3  the correct code
symbol attemptsLeft = b7      ; tries remaining before lockout

symbol command      = b8      ; 'R' or 'V' from the packet header
symbol vname        = b9      ; which variable (N, S, P)
symbol inByte       = b10     ; serial receive scratch
symbol checksum     = b11     ; packet checksum workspace
symbol currentValue = w6      ; b12,b13 value being sent/received

symbol timerStart   = w7      ; b14,b15 when the current timer began.
                              ;   START, not end - see check_timers

symbol rxWord       = w10     ; b20,b21 hserin target. MUST be a word:
symbol rxLow        = b20     ;   hserin leaves it UNCHANGED on no data
symbol pollGuard    = w11     ; b22,b23 poll_byte spin counter
symbol POLL_GUARD   = 2500    ; ~2 s at m16
symbol lastSeen     = w12     ; b24,b25 time of last calculator contact

' =================================================================
'  START
' =================================================================
init:
  setfreq m16
  high TO_CASIO_pin           ; C.0 powers up LOW - see header
  hsersetup B9600_16, %00     ; NOTE: this disables sertxd
  low RED_LED
  low GREEN_LED

  for b19 = SP_intDigit to SP_dec2
    poke b19, 0
  next b19

  flags        = 0
  storedPIN    = DEFAULT_PIN
  attemptsLeft = MAX_ATTEMPTS
  timerStart   = 0

  gosub do_lock               ; SAFE START: locked before anything else
  lastSeen = time

' =================================================================
'  MAIN LOOP
' =================================================================
main_loop:
  gosub check_timers
  gosub check_watchdog

  rxWord = $FFFF
  hserin rxWord
  if rxWord = $FFFF then main_loop
  if rxLow <> CASIO_ATTENTION then main_loop

  lastSeen = time
  hserout 0, (PICAXE_PRESENT)

  serin FROM_CASIO_pin, T9600_16, (":"), command, inByte, inByte, inByte, inByte, inByte, inByte, inByte, inByte, inByte, vname

  pause 200                   ; 50 ms at m16 - settling after serin

  if command = CMD_RECEIVE then
    gosub handle_receive
  elseif command = CMD_SEND then
    gosub handle_incoming
  endif

  goto main_loop

' =================================================================
'  THE INDICATORS - THE ONLY PLACE THE LEDs ARE DRIVEN
'
'  The state is shown in two places, here and on the calculator.
'  That is safe ONLY because both are rendered from lockState, and
'  the LEDs are touched nowhere else. Light an LED somewhere else
'  and the two displays can disagree - the failure this project
'  keeps catching.
'
'    red on,  green off = locked, wrong code, or locked out
'    red off, green on  = unlocked
'    both off           = no power (never commanded by this code)
'    both on            = never commanded either. If you see it,
'                         something is wrong.
' =================================================================
show_state:
  if lockState = ST_UNLOCKED then
    low RED_LED
    high GREEN_LED
  else
    low GREEN_LED
    high RED_LED
  endif
  return

' -----------------------------------------------------------------
'  THE TWO ACTUATOR ROUTINES
'  Everything that locks goes through do_lock, everything that
'  opens through do_unlock. Fitting a solenoid on a 14M2 means
'  editing these two routines and nothing else.
' -----------------------------------------------------------------
do_lock:
  lockState = ST_LOCKED
  gosub show_state
  return

do_unlock:
  lockState = ST_UNLOCKED
  gosub show_state
  return

' =================================================================
'  THE COMPARISON - the only place the door can be opened.
'  Reached ONLY from handle_incoming, and only for Send(P).
'  currentValue holds the PIN the student typed.
' =================================================================
check_pin:
  if lockState = ST_LOCKOUT then
    return                    ; refusing to listen until the timer expires
  endif

  if currentValue = storedPIN then
    attemptsLeft = MAX_ATTEMPTS
    timerStart   = time
    gosub do_unlock
    return
  endif

  lockState = ST_WRONG
  if attemptsLeft > 0 then
    attemptsLeft = attemptsLeft - 1
  endif
  if attemptsLeft = 0 then
    lockState  = ST_LOCKOUT
    timerStart = time
  endif
  gosub show_state
  return

' =================================================================
'  TIMERS
'
'  *** WHY THIS MEASURES ELAPSED TIME RATHER THAN COMPARING AGAINST
'      AN END TIME. ***
'  `time` is a WORD of seconds and wraps every 18 hours or so. An
'  earlier version stored `timerEnd = time + RELOCK_S` and waited
'  for `time > timerEnd`. Set that timer in the last minute before
'  a wrap and the addition overflows, so the comparison stays false
'  for the next eighteen hours - AND THE DOOR NEVER RE-LOCKS. The
'  lockout version of the same bug fails closed and is merely
'  annoying; the re-lock version fails OPEN, which is the one
'  direction this project's design principle forbids.
'
'  Subtracting two unsigned words wraps correctly, so
'  `time - timerStart` is the true elapsed count even across the
'  wrap. Store the START.
' =================================================================
check_timers:
  if lockState = ST_LOCKOUT then
    w11 = time - timerStart   ; pollGuard is free outside poll_byte
    if w11 >= LOCKOUT_S then
      attemptsLeft = MAX_ATTEMPTS
      gosub do_lock
    endif
    return
  endif

  if lockState = ST_UNLOCKED then
    w11 = time - timerStart
    if w11 >= RELOCK_S then
      gosub do_lock
    endif
  endif
  return

' -----------------------------------------------------------------
'  WATCHDOG - loss of contact LOCKS the door.
'  Unsigned subtraction wraps correctly, so no special case is
'  needed when `time` rolls over - see check_timers.
' -----------------------------------------------------------------
check_watchdog:
  w11 = time - lastSeen       ; pollGuard is free outside poll_byte
  if w11 > WATCHDOG_S then
    lastSeen = time
    if lockState <> ST_LOCKED then
      gosub do_lock
    endif
  endif
  return

' =================================================================
'  TRANSPORT
'  Identical to the logging builds. hserout throughout, hserin for
'  the attention wait and single-byte ACK waits, serin for the two
'  multi-byte packet reads.
' =================================================================
poll_byte:
poll_byte_drain:
  rxWord = $FFFF
  hserin rxWord
  if rxWord <> $FFFF then poll_byte_drain
  pollGuard = 0
poll_byte_loop:
  rxWord = $FFFF
  hserin rxWord
  if rxWord <> $FFFF then poll_byte_got
  pollGuard = pollGuard + 1
  if pollGuard < POLL_GUARD then poll_byte_loop
  pollOK = 0
  return
poll_byte_got:
  inByte = rxLow
  pollOK = 1
  return

' -----------------------------------------------------------------
'  THE CALCULATOR ASKS FOR A VALUE - answered immediately
' -----------------------------------------------------------------
handle_receive:
  hserout 0, (CASIO_ACK)

  gosub poll_byte
  if pollOK = 0 then request_timeout
  if inByte <> CASIO_ACK then
    return
  endif

  gosub send_description

  gosub poll_byte
  if pollOK = 0 then request_timeout
  if inByte <> CASIO_ACK then
    return
  endif

  if vname = VNAME_N then
    currentValue = 1                    ; one channel: the status word
  elseif vname = VNAME_S then
    gosub build_status
  else
    currentValue = 0
  endif
  isNegative = 0

  gosub send_value_packet
  gosub poll_byte

request_timeout:
  gosub send_end_packet
  return

' -----------------------------------------------------------------
'  THE STATUS WORD:  S = 1000 + state*10 + attempts
' -----------------------------------------------------------------
build_status:
  currentValue = lockState * 10
  currentValue = currentValue + attemptsLeft
  currentValue = currentValue + STATUS_BASE
  return

' -----------------------------------------------------------------
'  THE CALCULATOR SENDS A VALUE
'
'  *** ONLY Send(P) IS HONOURED, AND IT SUBMITS A PIN TO BE
'      CHECKED - IT DOES NOT OPEN ANYTHING. ***
'
'  There is no unlock message and there must never be one. The
'  PICAXE decides. See THE SECURITY LESSON in the header.
' -----------------------------------------------------------------
handle_incoming:
  hserout 0, (CASIO_ACK)
  serin FROM_CASIO_pin, T9600_16, (CASIO_PREAMBLE), inByte, inByte, inByte, inByte, b19, b20, b21, b22, inByte, inByte, inByte, b23, b26, b27, checksum
  poke SP_intDigit, b19
  poke SP_dec1, b20
  poke SP_dec2, b21
  poke SP_signInfo, b26
  poke SP_exponent, b27
  hserout 0, (CASIO_ACK)

  gosub decode_casio_value

  if vname <> VNAME_P then
    return
  endif
  gosub check_pin
  return

' -----------------------------------------------------------------
'  DECODE A NUMBER SENT BY THE CALCULATOR
' -----------------------------------------------------------------
decode_casio_value:
  peek SP_signInfo, b19
  b19 = b19 & $01
  if b19 = 0 then
    currentValue = 0
    return
  endif
  peek SP_exponent, b19
  peek SP_intDigit, b20
  if b19 = 0 then
    currentValue = b20
  elseif b19 = 1 then
    currentValue = b20 * 10
    peek SP_dec1, b21
    b21 = b21 / 16
    currentValue = currentValue + b21
  elseif b19 = 2 then
    currentValue = b20 * 100
    peek SP_dec1, b21
    b22 = b21 / 16
    currentValue = b22 * 10 + currentValue
    b22 = b21 & $0F
    currentValue = currentValue + b22
  elseif b19 = 3 then
    currentValue = b20 * 1000
    peek SP_dec1, b21
    b22 = b21 / 16
    currentValue = b22 * 100 + currentValue
    b22 = b21 & $0F
    currentValue = b22 * 10 + currentValue
    peek SP_dec2, b21
    b22 = b21 / 16
    currentValue = currentValue + b22
  else
    currentValue = b20 * 10000
    peek SP_dec1, b21
    b22 = b21 / 16
    currentValue = b22 * 1000 + currentValue
    b22 = b21 & $0F
    currentValue = b22 * 100 + currentValue
    peek SP_dec2, b21
    b22 = b21 / 16
    currentValue = b22 * 10 + currentValue
    b22 = b21 & $0F
    currentValue = currentValue + b22
  endif
  return

' -----------------------------------------------------------------
'  THE DESCRIPTION PACKET
' -----------------------------------------------------------------
send_description:
  checksum = 273 - vname
  hserout 0, (CASIO_PREAMBLE, $56, $41, $4C, $00, $56, $4D, $00, $01, $00, $01)
  hserout 0, (vname)
  hserout 0, ($FF, $FF, $FF, $FF, $FF, $FF, $FF)
  hserout 0, ($56, $61, $72, $69, $61, $62, $6C, $65, $52, $0A)
  hserout 0, ($FF, $FF, $FF, $FF, $FF, $FF, $FF, $FF, $FF, $FF, $FF, $FF, $FF, $FF, $FF, $FF, $FF, $FF, $FF, $FF)
  hserout 0, (checksum)
  return

' -----------------------------------------------------------------
'  THE VALUE PACKET
' -----------------------------------------------------------------
send_value_packet:
  if currentValue = 0 then
    gosub send_zero_packet
    return
  endif
  b20 = b12                   ; currentValue -> w10, which normalise reads
  b21 = b13
  gosub normalise_value
  b19 = 0
  if isNegative = 1 then
    b19 = $50
  endif
  if isLarge = 1 then
    b19 = b19 + 1
  endif
  poke SP_signInfo, b19
  peek SP_intDigit, b19
  hserout 0, (CASIO_PREAMBLE, $00, $01, $00, $01, b19)
  peek SP_dec1, b19
  peek SP_dec2, b20
  hserout 0, (b19, b20, $00, $00, $00, $00, $00)
  peek SP_signInfo, b19
  peek SP_exponent, b20
  hserout 0, (b19, b20)
  gosub calculate_checksum
  hserout 0, (checksum)
  return

send_zero_packet:
  hserout 0, (CASIO_PREAMBLE, $00, $01, $00, $01, $00, $00, $00, $00, $00, $00, $00, $00)
  hserout 0, ($00, $00, $FE)
  return

' -----------------------------------------------------------------
'  INTEGER -> NORMALISED SCIENTIFIC NOTATION
'  Reads w10, uses w11 and b19/b20 as scratch.
' -----------------------------------------------------------------
normalise_value:
  poke SP_dec1, 0
  poke SP_dec2, 0
  isLarge = 1
  if w10 < 10 then
    poke SP_intDigit, b20
    poke SP_exponent, 0
  elseif w10 < 100 then
    b19 = w10 / 10
    poke SP_intDigit, b19
    b19 = w10 // 10
    b19 = b19 * 16
    poke SP_dec1, b19
    poke SP_exponent, 1
  elseif w10 < 1000 then
    b19 = w10 / 100
    poke SP_intDigit, b19
    w11 = w10 // 100
    b19 = w11 / 10
    b19 = b19 * 16
    b20 = w11 // 10
    b19 = b19 + b20
    poke SP_dec1, b19
    poke SP_exponent, 2
  elseif w10 < 10000 then
    b19 = w10 / 1000
    poke SP_intDigit, b19
    w11 = w10 // 1000
    b19 = w11 / 100
    b19 = b19 * 16
    w11 = w11 // 100
    b20 = w11 / 10
    b19 = b19 + b20
    poke SP_dec1, b19
    b19 = w11 // 10
    b19 = b19 * 16
    poke SP_dec2, b19
    poke SP_exponent, 3
  else
    b19 = w10 / 10000
    poke SP_intDigit, b19
    w11 = w10 // 10000
    b19 = w11 / 1000
    b19 = b19 * 16
    w11 = w11 // 1000
    b20 = w11 / 100
    b19 = b19 + b20
    poke SP_dec1, b19
    w11 = w11 // 100
    b19 = w11 / 10
    b19 = b19 * 16
    b20 = w11 // 10
    b19 = b19 + b20
    poke SP_dec2, b19
    poke SP_exponent, 4
  endif
  peek SP_intDigit, b19
  if b19 = 0 then
    poke SP_intDigit, 1
  elseif b19 > 9 then
    poke SP_intDigit, 9
  endif
  return

calculate_checksum:
  checksum = $3A + $00 + $01 + $00 + $01
  peek SP_intDigit, b19
  checksum = checksum + b19
  peek SP_dec1, b19
  checksum = checksum + b19
  peek SP_dec2, b19
  checksum = checksum + b19
  peek SP_signInfo, b19
  checksum = checksum + b19
  peek SP_exponent, b19
  checksum = checksum + b19
  b19 = checksum - $3A
  b19 = 255 - b19             ; one's complement
  checksum = b19 + 1
  return

send_end_packet:
  hserout 0, (CASIO_PREAMBLE, "E", "N", "D")
  for b19 = 1 to 5
    hserout 0, ($FF, $FF, $FF, $FF, $FF, $FF, $FF, $FF, $FF)
  next b19
  hserout 0, ($56)
  return
