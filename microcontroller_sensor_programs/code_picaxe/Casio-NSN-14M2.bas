#REM
 Casio-NSN-14M2.bas
 (C) Michael Fenton, MRSNZ, 2026
 Licence: Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International 
 (CC BY-NC-SA 4.0)
 
 Two RECEIVE() windows (GAP 1 and GAP 2 in code below) discovered 2007/2008
 using Picaxe 18X connected to a Casio FX-9750G Plus. See the author's
 New Zealand Ministry of Education E-Learning Fellowship report and 
 conference paper; 

 1) Authentic Learning Using Mobile Sensor Technology (2008): https://doi.org/10.5281/zenodo.19302276
 2) RIGEL - Learning From Life, Kuala Lumpur (2009): https://doi.org/10.5281/zenodo.19334228

 https://mikefentonnz.github.io/projects/casio-calculator-data-logger-hack.html

 ================================================================= 
 Version 3.1; 30/07/2026
 (Production ready update of 2.0 rework 10/10/2025,
  original version 1.0 code for Picaxe 18X invented 2007)

  *** STATUS: PROOF OF CONCEPT - A FOUNDATION TO BUILD ON ***
 This is a reference implementation based on a validated method.
 It is deliberately minimal so that every line can be read and
 understood. It is NOT a finished classroom product and will not
 suit every use case. MODIFICATION IS EXPECTED - see the list of
 things a class would reasonably change, at the end of this header. 

 Author: Michael Fenton. Unbounded host wait and MFE both his discovery.

 Up to 3 sensors, read synchronously but delivered as consecutive 
 seperate RECEIVE() values. Works with Casio FX-9750G Plus &
 Casio FX-9750 GIII.
 
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

 =================================================================
 Features:
 - Robust and reliable communication - 0% packet loss
 - Casio calculator Auto power off (APO) disabled 
 - Non-blocking timing (improved timing precision)
 - First reading at time = 0 (synchronised to second boundary)
 - Subsequent readings every T seconds (2-300 second intervals)
 - Reads sensor 700 ms before send time (300 ms after the coarse
   wait ends). DS18B20 builds read 1000 ms before - see the switches.
 - Triple averaging on analogue sensors (noise reduction, ~5 ms)
 - Sensor range: 0 - 1023 (10 bit resolution)
 - Picaxe uses unsigned integers only
 - 16 MHz operation to send 9600 baud reliably NOTE: may affect some sensors!
 - Picaxe 08M2 and 14M2 socket compatible Casio communication pins
   (use of 08M2 C.0 and C.1, and 14M2 B.0 and B.1 is deliberate) 

 - NOTE: Use fast sensors (NTC thermistor, LDR, or i2c) NOT DS18B20!
 - DS18B20 takes up to 750ms per reading (too slow and operates at 4 MHz) 
   AND freezes the time clock affecting sensor read timing.
 - If you MUST use a DS18B20, time interval logging is limited to 5 seconds
   or longer.
 - NTC thermistors and LDRs - readadc10 (~1 ms) keeps all three reads 
   within a few milliseconds. That is what makes the data "synchronous". 
   Use 10k thermistor dividers and convert to degrees on the calculator.
 ===================================================================

 STUDENT / TEACHER WARNING!
 - NEVER use boiling water for temperature calibration (it is NOT needed)
 - NEVER connect mains electricity (240 V / 110 V) to the calculator,
   to this board, or to any sensor wiring. 
 - NEVER use mains-connected equipment near water.

 Picaxe can use 5V USB to PC for power with Casio FX-9760G Plus
 BUT
 Picaxe interface circuit MUST include the 1N4148 diode for connection
 to FX-9750 GIII (3V tolerant serial port).
 
 Modern (post 2020) Casio calculators are 3.3 V logic. 

 A SB-62 cross-over cable has male 2.5mm TRS plugs at both ends.

 HARDWARE connections(Picaxe 08M2): Wire colours are users choice

 - Pin B.0 -> to Casio RX [ring of 2.5mm TRS jack, BLUE wire] via 1N4148
 	      diode; bar to the picaxe  (HSEROUT)
 - Pin B.1 <- from Casio TX [tip of 2.5mm TRS jack, YELLOW wire] (SERIN,
              and also the hardware hserin pin - see the header)
 - Pin B.1 -> 4.7k pull-up resistor to V+ (3.3 V) - REQUIRED
 - Pin B.2 Sensor 1 (analogue, readadc10) - NTC thermistor or LDR
 - Pin B.3 Sensor 2 (analogue, readadc10) - NTC thermistor or LDR
 - Pin B.4 Sensor 3 (analogue, readadc10) - NTC thermistor or LDR OR
 			  DS18B20 temperature sensor
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

===============================================
*** THE CALCULATOR DISPLAY IS NOT A CLOCK ***
 RULE: trust the List data, not the screen. Every reading is captured
 but T x (I-1) IS AN ASSUMPTION, NOT A MEASUREMENT: it is the interval
 you ASKED FOR times the sample number, and nothing is timestamped. It
 is right only if this board honours the interval, and at short
 intervals it does not - see MIN_DELAY_INTERVAL. A slow board does not
 stop or drop a reading; it STRETCHES THE TIME AXIS, silently.
 
 To measure timing accuracy you need AN EXTERNAL CLOCK. The calculator 
 cannot report its own timing at all.

 WARNING: sertxd blocks while it transmits, serin stops the TIME variable  
 counting which is why hserin is used- it is non-blocking.
 DS18B20 ReadTemp also stops the TIME variable.

 HOW THE CALCULATOR SIDE WORKS (Casio BASIC, for reference):
   Receive(N)                 ' ask how many sensors (this file: 1)
   ?->T : Send(T)             ' user types interval 2-300 seconds
   For 1->I To 999
     Receive(A)               ' Triggers picaxe timing for T seconds
     Receive(B)               ' if 2 sensors
     Recieve(C)               ' if 3 sensors
     A->List 1[I]    
     similar for B,C
     refresh display
   Next
 
 Each reading I was taken at time (I-1)*T seconds. The AC key
 stops logging instantly; the PICAXE just times out back to its
 main loop, no harm done.

 =================================================================
 TIME INTERVAL NOTES:
 - `time` is a 16-bit word of seconds: it wraps at 65535 s (18.2 h). At
   300 s intervals keep a run under ~215 readings; at 10 s, the full 999
   readings fit comfortably (9990 s).
 - t=0 is defined by the FIRST reading, so logging cannot hang if
   the calculator program skips Send(T)
 - pause-duration at 16 MHz, PAUSE units are quarter-milliseconds: 
   pause 200 = 50 ms
 - all sensors are read together before the due time  
 =================================================================

 NOTE ON NEGATIVE VALUES: Negative values CAN be sent with care
 because the plain Casio value packet carries a sign in 
 bits 6 and 4 of the sign/info byte (see read_all_sensors and   
 send_value_packet). 

 *** WHAT TO CHANGE - this code is a starting point ***
 Things a class would reasonably modify, none of which break the
 method itself:
   - the number and type of sensors, and their calibration into
     real units (degrees, lux, percent) rather than raw 0-1023
   - the display layout on the calculator
   - the logging strategy: every reading, every Nth, only on change,
     or only when a threshold is crossed
   - data protection: the supplied Casio programs ask before
     overwriting stored lists, but you may want autosave, export, or
     a session name instead

 The parts NOT to change casually are the packet builders and the
 checksum: those are the protocol, and they are verified. 

; ===================================================================
;  DUAL-GENERATION BUILD - FX-9750G Plus AND FX-9750GIII
;  2026
; ===================================================================
;
;  1. EVERY PACKET IS BUILT FIRST, THEN SENT.
;
;  2. ONE BYTE AT A TIME, EVENLY SPACED.
;     Every byte goes through put_byte, so each is followed by the
;     same small gap. A block hserout sends them back to back with no
;     gap at all, which the G Plus refuses.
;
;  3. A 5 ms TURNAROUND BEFORE EVERY SEND.
;     The calculator needs a moment to stop transmitting and start
;     listening. Reply too fast and it mishears the first byte.
;     Found on a PICAXE in 2007; never written down until now.
;
;  COST: about 50 ms per packet, all of it inside a window where the
;  calculator is waiting anyway. Nothing measurable.
; ===================================================================
#ENDREM

#picaxe 14M2
#no_data
disableBOD

; ===================================================================
;  BUILD SWITCHES - DEFINE EXACTLY ONE SENSOR MODE
; ===================================================================
;  #DEFINE NO_DS18B20_FITTED  Three ADC channels for thermistor / LDR 
;                             triple-averaged. Interval floor 1 s.
;  #DEFINE USING_DS18B20      Channel 1 and channel 2 analogue,
;			      	CHANNEL 3 is a DS18B20. readtemp is 
;			      	750 ms and fixed on M2, so this forces
;			      	a 5 s interval logging floor.
;  #DEFINE SIMULATE_SENSORS   no sensors wired - synthetic staircases
;                             in the offset band, protocol testing only
; ===================================================================
#DEFINE SIMULATE_SENSORS

; ===================================================================
; PIN ASSIGNMENTS
; ===================================================================
symbol TO_CASIO_pin   = B.0   ; serial out to calculator (ring)
symbol FROM_CASIO_pin = B.1   ; serial in from calculator (tip)
symbol SENSOR1_PIN    = B.2   ; analogue sensor 1
symbol SENSOR2_PIN    = B.3   ; analogue sensor 2
symbol SENSOR3_PIN    = B.4   ; analogue sensor 3 / or DS18B20 temperature

; ===================================================================
; PROTOCOL CONSTANTS  (Casio legacy 3-pin serial, 9600 baud 8N2)
; ===================================================================
symbol CASIO_ATTENTION = $15  ; calculator: "are you there?"
symbol PICAXE_PRESENT  = $13  ; our reply: "yes, ready"
symbol CASIO_PREAMBLE  = $3A  ; ':' starts every packet
symbol CASIO_ACK       = $06  ; acknowledge
symbol CMD_RECEIVE     = "R"  ; ":REQ..." = calculator wants a value
symbol CMD_SEND        = "V"  ; ":VAL..." = calculator is sending one

symbol VNAME_N = "N"          ; sensor count request
symbol VNAME_A = "A"          ; the sensor1 reading
symbol VNAME_B = "B"          ; the sensor2 reading
symbol VNAME_C = "C"          ; the sensor3 reading
symbol VNAME_T = "T"          ; sampling interval (via Send(T))

; -------------------------------------------------------------------
;  TURNAROUND - wait before replying
; -------------------------------------------------------------------
;  The calculator does not switch from talking to listening instantly.
;  Answer too quickly and the first bits arrive while its port is
;  still turning round, so it mishears the byte and rejects the
;  exchange with $22.
;
;  20 quarter-ms = 5 ms at m16. The 2007 figure. Applied INSIDE every
;  send routine so a new one cannot forget it.
; -------------------------------------------------------------------
symbol TURNAROUND = 20

symbol MAX_DELAY_INTERVAL  = 300   ; 5 minutes = proven useful ceiling

; *******************************************************************
;  THE CALCULATOR PROGRAM MUST USE THE SAME FLOOR AS THIS ONE.
; *******************************************************************
;  You hold both ends of this system - the Casio BASIC program and
;  this file - so the floor exists in TWO places and they must agree.
;
;  IF THE CALCULATOR ALLOWS A SMALLER INTERVAL THAN THIS FILE, THE
;  CLAMP BELOW FIRES SILENTLY AND THE TIME AXIS IS WRONG BY THE RATIO:
;
;      asks 1 s, gets 5 s   ->  elapsed column wrong by 5x  (400%)
;      asks 2 s, gets 5 s   ->  wrong by 2.5x               (150%)
;
;  Nothing on the calculator can detect this. It computes its elapsed
;  column as T x (I-1) using the T IT ASKED FOR, not the T that
;  happened. A five-fold error that looks exactly like good data -
;  which is the one thing this project's design principle forbids.
;
;  SO: change `If T<n` in the Casio listing at the foot of this file AT
;  THE SAME TIME as MIN_DELAY_INTERVAL here. One setting, stored twice.
; *******************************************************************

; *******************************************************************
;  readtemp STOPS THE CLOCK TOO
; *******************************************************************
;  serin disables the timer interrupt while it listens. SO DOES
;  readtemp, for the same reason: 1-Wire is bit-banged and needs
;  microsecond timing, so interrupts cannot be left running.
;
;  A DS18B20 conversion is 750 ms. `time` STOPS FOR ALL OF IT.
;
;      loss per sample =  750 ms readtemp
;                      +   50 ms protocol
;                      =  800 ms
;
;  *** THE 5-SECOND FLOOR DOES NOT FIX THIS, AND WAS NEVER MEANT TO.
;  *** It exists so the 750 ms read FITS INSIDE the read window. That
;  *** is a scheduling problem. 
;
;      interval   time-axis error with a DS18B20
;         5 s          16 %
;        10 s           8 %
;        16 s           5 %
;        30 s         2.7 %
;       300 s         0.3 %
;
;  THE FLOOR IS DELIBERATELY LEFT AT 5 s. A DS18B20 at 5 s gives a
;  perfectly good CURVE SHAPE and a time axis 16 % long - the same
;  bargain as 1-second analogue sampling, and the guidance names the
;  cost rather than the firmware forbidding it. For a RATE from a
;  DS18B20 use 30 s or more.
;
;  *** AND THIS IS WHERE THE PICAXE PLATFORM ENDS. *** No arrangement
;  of PICAXE code can recover those 750 ms: the chip cannot run its
;  timer and bit-bang 1-Wire at once. An ESP8266 or ESP32 has hardware
;  timers that keep counting through a sensor read, which is the
;  reason to move platform - not raw speed.
; *******************************************************************

#IFDEF USING_DS18B20
symbol MIN_DELAY_INTERVAL  = 5     ; readtemp is 750 ms and the slow read
                                   ; window needs nextSendTime - 2, so the
                                   ; floor is 3 s arithmetically. 5 s gives
                                   ; margin. THE FLOOR MOVES WITH THE SENSOR
                                   ; CHOICE, so the constraint is enforced
                                   ; rather than remembered.
#ENDIF
#IFNDEF USING_DS18B20
symbol MIN_DELAY_INTERVAL  = 1     ; 4.8% time-axis
                                   ; error at 1 s on the 08M2, 5.0% on the
                                   ; 14M2. Needs SENSOR_READ_OFFSET at 300 ms.
#ENDIF

; -------------------------------------------------------------------
;  SENSOR_READ_OFFSET - how far into the read window the sensor is read
; -------------------------------------------------------------------
;  The coarse wait leaves us at the START of the window. This pause
;  then positions the read inside it. A LARGER value reads LATER, and
;  therefore fresher.
;
;      build      window   offset            read lands before the tick
;      analogue   1000 ms  1200 = 300 ms     700 ms
;      DS18B20    2000 ms  4000 = 1000 ms   1000 ms  (750 ms read then
;                                                     finishes 250 ms early)
;
;  A FIDELITY SETTING, NOT A SAFETY ONE: it decides how stale the sample
;  is relative to its timestamp. The lead is CONSTANT, so it shifts the
;  whole series uniformly - shape and relative timing are untouched.
; -------------------------------------------------------------------
#IFDEF USING_DS18B20
symbol SENSOR_READ_OFFSET  = 4000
#ENDIF
#IFNDEF USING_DS18B20
symbol SENSOR_READ_OFFSET  = 1200
#ENDIF
; ===================================================================
; RAM SCRATCH ADDRESSES (poke/peek)
; Addresses 0-27 are the b0-b27 variables, so scratch starts at 28.
; These hold the pieces of a value packet while it is being built.
; ===================================================================
symbol SP_intDigit = 28   ; the single integer digit I of I.DDDD x10^E
symbol SP_signInfo = 29   ; sign/magnitude flag byte (packet byte 14)
symbol SP_exponent = 30   ; exponent E (packet byte 15)
symbol SP_dec1     = 31   ; BCD decimal digits 1-2  (packet byte 7)
symbol SP_dec2     = 32   ; BCD decimal digits 3-4  (packet byte 8)
symbol SP_badbyte  = 39   ; DIAGNOSTIC: the byte that arrived where
                          ; an ACK was expected
symbol SP_stage    = 38   ; DIAGNOSTIC: how far the last transaction
                          ; got. Reported ONLY on failure.
symbol LED_PIN     = C.2  ; DIAGNOSTIC: shares SENSOR1_PIN. Sensor 1
                          ; is unavailable while diagnosing - an 08M2
                          ; has no free pin.
symbol SP_PKT      = 40   ; *** the sixteen-byte value packet
                          ; is BUILT here, at 40-55, and only then
                          ; transmitted. The 18X code of 2007 did the
                          ; same thing the only way PICAXE BASIC
                          ; allowed at the time - one serout with the
                          ; whole packet in it, every byte already
                          ; known before the first one left.
symbol SP_dec3     = 34   ; *** v10 *** BCD decimal digits 5-6
                          ; (packet byte 8). 

; ===================================================================
; VARIABLES  (b0-b27 / w0-w13; a word = two bytes)
; b19-b23 are shared scratch used inside subroutines only.
; ===================================================================
symbol flags        = b0      ; bit variables live in b0
symbol isNegative   = bit0    ; set while building a negative packet
symbol isLarge      = bit1    ; set when |value| >= 1

; --- ONE STORED SIGN PER SENSOR ------------------------------------
; isNegative above is a WORKING bit: it describes the packet being
; built RIGHT NOW, and it is loaded from one of these three
; immediately before each packet is sent.
;
; The sign must therefore be stored WITH the reading it belongs to,
; and selected at the moment that reading is sent. Three spare bits
; of b0 cost nothing.

symbol sensor1Neg   = bit3    ; sign of the STORED sensor1Value
symbol sensor2Neg   = bit4    ; sign of the STORED sensor2Value
symbol sensor3Neg   = bit5    ; sign of the STORED sensor3Value

symbol ds18b20Here  = bit6    ; 1 = a DS18B20 answered the 1-Wire roll
                              ; call at start-up. Set ONCE, in init.
                              ; See the DS18B20 detection note below.

symbol sensorCount  = b1      ; 1, 2 or 3 -> set in init below
symbol sensor1Value = w1      ; b2,b3   latest reading
symbol sensor2Value = w2      ; b4,b5   latest reading
symbol sensor3Value = w3      ; b6,b7   latest reading

symbol command      = b8      ; 'R' or 'V' from the packet header
symbol vname        = b9      ; which variable (N, A, T, ...)
symbol inByte       = b10     ; serial receive scratch
symbol checksum     = b11     ; packet checksum workspace

symbol currentValue = w6      ; b12,b13 value being sent/received
symbol nextSendTime = w7      ; b14,b15 when the next reading is due (s)
                              ;         0 = "first reading not yet taken"
symbol timeInterval = w8      ; b16,b17 sampling interval in seconds
symbol readTarget   = w12     ; b24,b25 WORD when to read the sensor (s)
                              ;         (a byte here overflows at 255 s!)

symbol tempWord     = w13     ; b26,b27 - the averaging accumulator.
                              ; b26/b27 are ALSO raw scratch inside
                              ; handle_incoming and send_mfe_value. Safe
                              ; only because every use here sets
                              ; tempWord = 0 first and nothing carries
                              ; across - the same discipline as rxWord
                              ; (w10) and pollGuard (w11).

; Subroutine scratch (do not use for long-lived data): b19-b23, b26, b27

symbol rxWord       = w10     ; b20,b21 - hserin target for the
symbol rxLow        = b20     ; attention poll. MUST be a word:
			      ; hserin leaves it UNCHANGED when no byte
                              ; arrived, and $FF is a legal data byte,
                              ; so only a word sentinel can tell the
                              ; two apart. 
                              ; b20/b21 are ALSO raw
                              ; scratch for the packet builder, which
                              ; is safe only because the poll writes
                              ; $FFFF before every single hserin. 

symbol pollOK       = bit2    ; poll_byte result: 1 = byte, 0 = timeout.
                              ; bit0/bit1 are the packet-sign flags.
symbol pollGuard    = w11     ; b22,b23 - the poll timeout counter.
                              ; w11 is ALSO scratch inside
                              ; decode_casio_value and normalise_value,
                              ; which is safe ONLY because poll_byte
                              ; zeroes it on entry every time and it is
                              ; dead outside the routine. 

symbol POLL_GUARD   = 625     ; ~5 s at m16. Raised from 2500 (~2 s)
                              ; for FX-9750G Plus support.
                              ;
                              ; 5 s is DELIBERATELY GENEROUS, not a
                              ; measured requirement. 

; ===================================================================
;  DETECTING A MISSING DS18B20 
; ===================================================================
;  readowsn fetches the 64-bit ROM code that every 1-Wire device
;  carries. Byte b6 is the FAMILY CODE, and $28 is a DS18B20. No
;  device on the bus, no family code. Presence and temperature become
;  two separate questions, which is what they always were.

symbol DS18B20_FAMILY = $28   ; 1-Wire family code for the DS18B20
symbol DS18B20_ABSENT = 999   ; sent when no sensor answered at start-up

symbol SENSOR_MAX    = 1023   ; upper limit of a legitimate reading

; --- v6: b18 is no longer free ---------------------------------------
symbol txByte        = b18    ; the single byte put_byte transmits.

; FREE for expansion: none. The next variable needed must come from
; the scratchpad (peek/poke), as the packet builder already does.

; ===================================================================
; INITIALISATION
; ===================================================================
init:
  setfreq m16                 ; 16 MHz: needed for clean 9600 baud
                              ; NOTE at m16: pause N waits N/4 ms,
                              ; and 'time' still counts REAL seconds
                              ;
                              ; hsersetup MUST come after this: the
                              ; B9600_16 constant assumes a 16 MHz
                              ; clock, so setting the frequency later
                              ; would silently change the baud rate.

  high TO_CASIO_pin           ; defined output-high before the UART
                              ; takes the pin. See the header - this
                              ; is the ONLY `high` on this pin now.
  hsersetup B9600_16, %00     ; %00 = true polarity, idle high.

  disabletime                 ; timer starts at the first reading

  ; clear the packet-building scratch RAM
  for b19 = SP_intDigit to SP_dec3
    poke b19, 0
  next b19

; --- DS18B20 PRESENCE - DEFAULT ONLY. THE REAL CHECK IS PER-SAMPLE ---
  ; Set to "absent" here so the variable has a defined value before the
  ; first reading. The roll call itself lives in read_all_sensors and
  ; runs EVERY sample - see the note there for why that is both safe
  ; and necessary. Do not add a second copy of it here.
  ds18b20Here = 0

  sensorCount   = 3
  timeInterval  = 10          ; default if Send(T) never arrives
  nextSendTime  = 0
  sensor1Value  = 0
  sensor2Value  = 0
  sensor3Value  = 0

  sensor1Neg    = 0
  sensor2Neg    = 0
  sensor3Neg    = 0

  pause 4000   ; pause 1 second to let picaxe settle

; ===================================================================
; MAIN LOOP - wait for the calculator to speak first
; The calculator opens every transaction with attention byte $15.
; We answer $13 within its ~0.5-1 s window or it shows "Com ERROR".
; ===================================================================
main_loop:
  ; Wait for the attention signal from the Casio.

  rxWord = $FFFF
  hserin rxWord
  if rxWord = $FFFF then main_loop           ; nothing yet
  if rxLow <> CASIO_ATTENTION then main_loop ; stale byte - discard
  
  inByte = CASIO_ATTENTION    ; reached only on a real $15, so the
                              ; test below is now always true. 

  if inByte = CASIO_ATTENTION then


    ; Send immediate response (Picaxe is present and ready)
    pause TURNAROUND          ; let the calculator turn round first
    poke SP_stage, 1          ; DIAG: attention byte seen
    hserout 0, (PICAXE_PRESENT)

    ; The calculator now sends a 50-byte packet. Both kinds put the
    ; command letter first and the variable name at byte 12:
    ;   ":REQ..." (Receive) -> command 'R'
    ;   ":VAL..." (Send)    -> command 'V'
    ; Read ':' + command + 9 bytes + vname = the part we need.

    serin [16000, ml_report], FROM_CASIO_pin, T9600_16, (":"), command, inByte, inByte, inByte, inByte, inByte, inByte, inByte, inByte, inByte, vname


    ; Let the remaining ~38 bytes of the 50 finish arriving

    pause 300                 ; 300/4 = 75 ms at 16 MHz

    ; Process command from Casio
    poke SP_stage, 2          ; DIAG: request header read

    if command = CMD_RECEIVE then
      gosub handle_receive    ; calculator wants a value (No. of sensors or sensor reading)
    elseif command = CMD_SEND then
      gosub handle_incoming   ; calculator sent a number (T; time delay, or remote control)
    else
      poke SP_stage, 9        ; DIAG: command was neither "R" nor "V"
    endif

    ; ---------------------------------------------------------------
    ; DIAG - REPORT ONLY ON FAILURE.
    ; A good transaction ends at stage 8 and NOTHING HAPPENS. Any
    ; other value means it broke, the calculator has already raised
    ; Com ERROR and stopped, so blocking here costs nothing.
    ; ---------------------------------------------------------------
    peek SP_stage, b26
    if b26 <> 8 then
      gosub blink_stage
    endif
    poke SP_stage, 0

  endif

ml_report:
  ; Reached if the request packet never arrives. Nothing to do but
  ; start listening again.
  goto main_loop

poll_byte:

poll_byte_drain:
  rxWord = $FFFF
  hserin rxWord
  if rxWord <> $FFFF then poll_byte_drain   ; until empty

poll_byte_nodrain:
  pollGuard = 0               ; MUST reset - w11 is scratch elsewhere

poll_byte_loop:
  rxWord = $FFFF              ; re-arm every pass: hserin leaves the
  hserin rxWord               ; variable UNCHANGED if nothing arrived
  if rxWord <> $FFFF then poll_byte_got
  pollGuard = pollGuard + 1
  if pollGuard < POLL_GUARD then poll_byte_loop
  pollOK = 0
  return

poll_byte_got:
  inByte = rxLow
  pollOK = 1
  return

handle_receive:
  poke SP_stage, 3            ; DIAG: command read as "R"


  ; Send acknowledgment
  pause TURNAROUND            ; let the calculator turn round first
  hserout 0, (CASIO_ACK)

  gosub poll_byte                   ; POLLED - drain needed here

  if pollOK = 0 then request_timeout
  if inByte <> CASIO_ACK then
    return                    ; calculator gave up (AC key?)
  endif


  poke SP_stage, 4            ; DIAG: calculator said go ahead

  gosub send_description
  poke SP_stage, 5            ; DIAG: description sent

  ; Wait for acknowledgment
  gosub poll_byte                   ; drain RESTORED - removing it
                                    ; made things worse, see header

  ; --- DIAG: the two ways this can fail are now different codes ---
  if pollOK = 0 then
    poke SP_stage, 5                ; NOTHING arrived in ~2 s
    goto request_timeout
  endif
  if inByte <> CASIO_ACK then
    poke SP_badbyte, inByte         ; a byte arrived, and it was not $06
    poke SP_stage, 11
    return
  endif


     ; == MICHAEL FENTON HOST-WAIT WINDOW DISCOVERY: The value window ==
     ; Pausing here during RECEIVE does NOT trigger COM ERROR on Casio
     ; This is the heart of Fenton's timed data logging discovery

  ; Determine which variable Casio requested
  ;
  ; *** EVERY BRANCH MUST SET isNegative AS WELL AS currentValue. ***

  poke SP_stage, 6            ; DIAG: DESCRIPTION ACCEPTED

  if vname = VNAME_N then
  ; Variable N: Send sensor count (how many sensors connected)
    currentValue = sensorCount          ; answer immediately
    isNegative   = 0                    ; a count is never negative

  elseif vname = VNAME_A then
    ; ================================================================
    ; First in sensor reading cluster - handle timing and read ALL sensors
    ; Sensors are read BEFORE interval completes (absorbed timing)
    ;
    ; THE A REQUEST IS THE TIMING ANCHOR. It is the only branch that
    ; calls wait_for_interval, so the calculator's program MUST ask
    ; for A first in every interval. Ask for B or C first and they
    ; return the PREVIOUS interval's readings while the interval
    ; clock has not advanced.
    ; ================================================================
  ; Variable A: Send sensor A reading	  
    gosub wait_for_interval             ; <-- the exploit in action
    currentValue = sensor1Value
    isNegative   = sensor1Neg

  elseif vname = VNAME_B then
  ; Sensor 2: use cached value from A's read (already fresh)
    currentValue = sensor2Value
    isNegative   = sensor2Neg

  elseif vname = VNAME_C then
  ; Sensor 3: use cached value from A's read (already fresh)
    currentValue = sensor3Value
    isNegative   = sensor3Neg

  else
    currentValue = 0                    ; unknown variable: send 0
    isNegative   = 0
  endif

  gosub send_value_packet
  poke SP_stage, 7            ; DIAG: value packet sent

  gosub poll_byte             ; drain RESTORED
  if pollOK = 1 then
    if inByte = CASIO_ACK then
      poke SP_stage, 8        ; DIAG: FULL SUCCESS
    else
      poke SP_stage, 10       ; DIAG: a byte came back, not $06
    endif
  endif

request_timeout:
  gosub send_end_packet
  return

wait_for_interval:
  ; ==== FIRST READING AT T=0 (Special case) ====
  ; The very first reading happens immediately at time zero
  ; No delay - just read sensors and set next target time 
 
  if nextSendTime = 0 then
    enabletime
    time = 0
    nextSendTime = timeInterval
    gosub read_all_sensors
    return
  endif

  ; ==== DETERMINE WHEN TO READ SENSORS ====
  ; Strategy: Read sensors before send time
  ; This ensures fresh data while absorbing protocol overhead

  ; coarse wait until 1 s before the due time

#IFDEF USING_DS18B20
  readTarget = nextSendTime - 2     ; SLOW WINDOW - a DS18B20 needs 750 ms
                                    ; and the normal window tops out at 700
#ENDIF
#IFNDEF USING_DS18B20
  readTarget = nextSendTime - 1     ; WORD maths - no 255 s overflow
#ENDIF
  do
    pause 50                        ; 12.5 ms at 16 MHz - light on the CPU
  loop while time < readTarget

  ; ==== SENSOR READ SLOT ====
  ; We are now at the start of the final second. Wait SENSOR_READ_OFFSET
  ; - 1200, which is 300 ms at 16 MHz - so the reading lands about
  ; 700 ms BEFORE the packet is due.
  ;
  ; The guard skips the wait if we are already running late.

  if time < nextSendTime then
    pause SENSOR_READ_OFFSET
  endif

  gosub read_all_sensors            ; microseconds apart, in-window

  ; ==== WAIT UNTIL EXACT SEND TIME ====
  ; If we read early, wait the remaining time until nextSendTime
  ; This ensures we send at EXACTLY the right moment

  do
    pause 1                         ; 0.25 ms -> land on the second
  loop until time >= nextSendTime

  nextSendTime = nextSendTime + timeInterval

  ; ==== OVERFLOW PROTECTION ====
  ; If something went very wrong and we're past the next target,
  ; reset to avoid getting stuck in the past
  if time > nextSendTime then
      nextSendTime = time + timeInterval
  endif

  return

read_all_sensors:
  ; -------------------------------------------------------------------
  ; ONE OF THE THREE BLOCKS BELOW IS COMPILED - choose with a #DEFINE at
  ; the top of the file. They are MUTUALLY EXCLUSIVE.
  ;
  ; A PICAXE byte or word variable cannot hold a negative.
  ; -------------------------------------------------------------------
  ; ALL THREE STORED SIGNS CLEARED ONCE, HERE, BEFORE ANY READING.
  ; Each read below sets its own bit if its own reading is negative.
  ; Clearing the working bit isNegative here would achieve nothing -
  ; it is reloaded per packet in handle_receive.
  sensor1Neg = 0
  sensor2Neg = 0
  sensor3Neg = 0

#IFDEF SIMULATE_SENSORS
  ; --- staircases, in the OFFSET band so the display is plausible ----
  sensor1Value = sensor1Value + 2
  sensor2Value = sensor2Value + 25
  sensor3Value = sensor3Value + 111
  if sensor1Value >SENSOR_MAX then
    sensor1Value = 0
  endif
  if sensor2Value > SENSOR_MAX then
    sensor2Value = 0
  endif
  if sensor3Value > SENSOR_MAX then
    sensor3Value = 0
  endif
#ENDIF

#IFDEF NO_DS18B20_FITTED
  gosub read_channel1
  if sensorCount >= 2 then
    gosub read_channel2	    
  endif
  if sensorCount >= 3 then
    gosub read_channel3
  endif
#ENDIF

#IFDEF USING_DS18B20
  ; --- Channel 3 a DS18B20 ---------------
  gosub read_channel1
  if sensorCount >= 2 then
   gosub read_channel2
  endif
  
  if sensorCount >= 3 then
    b6 = 0
    readowsn SENSOR3_PIN
    ds18b20Here = 0
    if b6 = DS18B20_FAMILY then
      ds18b20Here = 1
    endif

    if ds18b20Here = 0 then
      sensor3Value = DS18B20_ABSENT ; 999 - no sensor answered at
      sensor3Neg   = 0              ; start-up. Not a temperature.
    else
      readtemp SENSOR3_PIN, b19
      ; 0 IS NOW TRUSTED AS 0 DEGREES, because a sensor answered the
      ; roll call. That trust is what the roll call buys.
      if b19 > 127 then
        b19 = b19 - 128             ; negative: magnitude in b19
        sensor3Neg = 1              ; THIS sensor's sign, not the
                                    ; shared working bit. See the
                                    ; symbol table for why.
        sensor3Value = b19
      else
        sensor3Value = b19
      endif
    endif
  endif

#ENDIF

  return

read_channel1:
  tempWord = 0
  readadc10 SENSOR1_PIN, sensor1Value
  tempWord = tempWord + sensor1Value
  pause 5                           ; 1.25 ms settling at m16
  readadc10 SENSOR1_PIN, sensor1Value
  tempWord = tempWord + sensor1Value
  pause 5
  readadc10 SENSOR1_PIN, sensor1Value
  tempWord = tempWord + sensor1Value
  tempWord = tempWord / 3
  sensor1Value = tempWord
  return

read_channel2:
   ; if using a switch
   
   ; if SENSOR2_PIN = 0 then 
   ;	sensor2Value = 0
   ; else 
   ;	sensor2Value = 1
   ; Endif

  tempWord = 0
  readadc10 SENSOR2_PIN, sensor2Value
  tempWord = tempWord + sensor2Value
  pause 5
  readadc10 SENSOR2_PIN, sensor2Value
  tempWord = tempWord + sensor2Value
  pause 5
  readadc10 SENSOR2_PIN, sensor2Value
  tempWord = tempWord + sensor2Value
  tempWord = tempWord / 3
  sensor2Value = tempWord
  return

read_channel3:
  tempWord = 0
  readadc10 SENSOR3_PIN, sensor3Value
  tempWord = tempWord + sensor3Value
  pause 5
  readadc10 SENSOR3_PIN, sensor3Value
  tempWord = tempWord + sensor3Value
  pause 5
  readadc10 SENSOR3_PIN, sensor3Value
  tempWord = tempWord + sensor3Value
  tempWord = tempWord / 3
  sensor3Value = tempWord
  return
  
handle_incoming:
  ; Send acknowledgment 
  pause TURNAROUND            ; let the calculator turn round first
  hserout 0, (CASIO_ACK)

  ; Read the 15 bytes after ':'. The 7th BCD byte goes into b23

  ; Read preamble and header (5 bytes)
  serin FROM_CASIO_pin, T9600_16, (CASIO_PREAMBLE), inByte, inByte, inByte, inByte, b19, b20, b21, b22, inByte, inByte, inByte, b23, b26, b27, checksum

  poke SP_intDigit, b19
  poke SP_dec1, b20
  poke SP_dec2, b21
  poke SP_signInfo, b26
  poke SP_exponent, b27

  pause TURNAROUND            ; let the calculator turn round first
  hserout 0, (CASIO_ACK)

  poke SP_stage, 8            ; DIAG: Send(x) completed - counts as OK
  gosub decode_casio_value    ; -> currentValue (0..65535)

  if vname = VNAME_T then
    timeInterval = currentValue

    ; Smart minimum: sensor read time + ~200 ms protocol overhead,
    ; rounded up, + 1 s safety. With readadc10 this resolves to 2 s.
    b19 = MIN_DELAY_INTERVAL
    if timeInterval < b19 then
      timeInterval = b19
    endif
    if timeInterval > MAX_DELAY_INTERVAL then
      timeInterval = MAX_DELAY_INTERVAL
    endif

    nextSendTime = 0          ; next Receive(A) starts a fresh run
                              ; and defines t = 0

  endif
  return

decode_casio_value:
  peek SP_signInfo, b19
  b19 = b19 & $01             ; magnitude flag: 0 means |value| < 1
  if b19 = 0 then
    currentValue = 0
    return
  endif

  peek SP_exponent, b19
  peek SP_intDigit, b20

  if b19 = 0 then
    currentValue = b20                        ; I          (1 digit)

  elseif b19 = 1 then
    currentValue = b20 * 10                   ; I.D  x10   (2 digits)
    peek SP_dec1, b21
    b21 = b21 / 16
    currentValue = currentValue + b21

  elseif b19 = 2 then
    currentValue = b20 * 100                  ; I.DD x100  (3 digits)
    peek SP_dec1, b21
    b22 = b21 / 16
    ; PICAXE maths is strictly left-to-right ? no precedence!
    currentValue = b22 * 10 + currentValue
    b22 = b21 & $0F
    currentValue = currentValue + b22

  elseif b19 = 3 then
    currentValue = b20 * 1000                 ; IDDD       (4 digits)
    peek SP_dec1, b21
    b22 = b21 / 16
    currentValue = b22 * 100 + currentValue
    b22 = b21 & $0F
    currentValue = b22 * 10 + currentValue
    peek SP_dec2, b21
    b22 = b21 / 16
    currentValue = currentValue + b22

  else
    currentValue = b20 * 10000                ; IDDDD      (5 digits)
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

; ===================================================================
;  blink_stage - DIAGNOSTIC.  CALLED ONLY WHEN A TRANSACTION FAILED.
; ===================================================================
;    1   $15 seen, $13 sent, no request packet followed
;    2   request header read, but no handler ran
;    3   inside handle_receive, died waiting for the first ACK
;    4   calculator acknowledged our ACK
;    5   description sent, and NOTHING came back in ~2 s
;   11   description sent, a byte came back and it was NOT $06.
;        The byte is reported FIRST, as two groups of SHORT flashes
;        (100 ms), one second apart. EACH GROUP IS THE NIBBLE PLUS
;        ONE - subtract one from each:
;            1,7  = $06     4,11 = $3A     2,6  = $15
;        $06 here would mean the ACK arrived and was misread.
;        $3A or $15 means a stale packet byte - the link is out of
;        step rather than the packet refused.
;    6   *** DESCRIPTION ACCEPTED *** - died at the value packet
;    7   value packet sent, nothing came back within ~2 s
;    9   command letter was neither "R" nor "V"
;   10   a byte came back after the value packet and it was NOT $06
;
;  Stage 8 is success and never reaches here.
;  A ~2 s stall before the report is POLL_GUARD expiring - nothing
;  arrived. An instant report means a byte arrived that was not $06.
;
;  Blocks for up to 20 s, safe ONLY because the calculator has
;  already errored and stopped. NEVER call it on a good transaction.
; ===================================================================
blink_stage:
  peek SP_stage, b26
  if b26 = 0 then blink_done
  if b26 <> 11 then bs_stage
  low LED_PIN
  pause 4000
  peek SP_badbyte, b26
  b19 = b26 / 16
  b19 = b19 + 1
  gosub blink_fast
  pause 4000
  peek SP_badbyte, b26
  b19 = b26 & $0F
  b19 = b19 + 1
  gosub blink_fast
  pause 8000
  peek SP_stage, b26

bs_stage:

  low LED_PIN
  for b19 = 1 to b26
    high LED_PIN
    pause 4000                ; 1 s at m16
    low LED_PIN
    pause 4000
  next b19
  pause 8000                  ; 2 s gap

blink_done:
  low LED_PIN
  return

blink_fast:
  for b18 = 1 to b19
    high LED_PIN
    pause 400                 ; 100 ms
    low LED_PIN
    pause 1200                ; 300 ms
  next b18
  return

put_byte:
  hserout 0, (txByte)
  return

send_description:
  pause TURNAROUND            ; let the calculator turn round first
  checksum = 273 - vname ; Optimized: was $D0 - vname + 65

  ; bytes 0-10 : ':' V A L 00 V M 00 01 00 01
  for b19 = 0 to 10
    lookup b19, ($3A, $56, $41, $4C, $00, $56, $4D, $00, $01, $00, $01), txByte
    gosub put_byte
  next b19

  ; byte 11 : the variable name
  txByte = vname
  gosub put_byte

  ; bytes 12-18 : seven $FF
  txByte = $FF
  for b19 = 1 to 7
    gosub put_byte
  next b19

  ; bytes 19-28 : "VariableR" + $0A
  for b19 = 0 to 9
    lookup b19, ($56, $61, $72, $69, $61, $62, $6C, $65, $52, $0A), txByte
    gosub put_byte
  next b19

  ; bytes 29-48 : twenty $FF
  txByte = $FF
  for b19 = 1 to 20
    gosub put_byte
  next b19

  ; byte 49 : checksum
  txByte = checksum
  gosub put_byte
  return

send_value_packet:
  pause TURNAROUND            ; let the calculator turn round first

  if currentValue = 0 then
    gosub send_zero_packet    ; zero has its own special packet
    return
  endif

  ; copy magnitude into w10 (b20,b21) for the normaliser
  b20 = b12
  b21 = b13

  gosub normalise_value

  b19 = 0
  if isNegative = 1 then
    b19 = $50 ; Bits 6 & 4 for negative
  endif
  if isLarge = 1 then
    b19 = b19 + 1 ; Bit 0 for |value| >= 1
  endif
  poke SP_signInfo, b19

  ; Checksum FIRST - never while the packet is going out.
  gosub calculate_checksum
  gosub build_value_packet
  gosub emit_value_packet
  return

build_value_packet:
  b22 = SP_PKT

  for b19 = 0 to 4                  ; bytes 0-4 : ':' 00 01 00 01
    lookup b19, ($3A, $00, $01, $00, $01), b26
    poke b22, b26
    b22 = b22 + 1
  next b19

  peek SP_intDigit, b26             ; byte 5
  poke b22, b26
  b22 = b22 + 1

  peek SP_dec1, b26                 ; byte 6
  poke b22, b26
  b22 = b22 + 1

  peek SP_dec2, b26                 ; byte 7
  poke b22, b26
  b22 = b22 + 1

  peek SP_dec3, b26                 ; byte 8
  poke b22, b26
  b22 = b22 + 1

  b26 = 0                           ; bytes 9-12
  for b19 = 1 to 4
    poke b22, b26
    b22 = b22 + 1
  next b19

  peek SP_signInfo, b26             ; byte 13
  poke b22, b26
  b22 = b22 + 1

  peek SP_exponent, b26             ; byte 14
  poke b22, b26
  b22 = b22 + 1

  poke b22, checksum                ; byte 15
  return

emit_value_packet:
  for b19 = 0 to 15
    b22 = SP_PKT
    b22 = b22 + b19
    peek b22, txByte
    gosub put_byte
  next b19
  return

send_zero_packet:
  pause TURNAROUND          

  for b19 = 0 to 4
    lookup b19, ($3A, $00, $01, $00, $01), txByte
    gosub put_byte
  next b19

  txByte = $00
  for b19 = 1 to 10
    gosub put_byte
  next b19

  txByte = $FE
  gosub put_byte
  return

normalise_value:
  ; Clear decimal bytes in scratchpad
  poke SP_dec1, 0
  poke SP_dec2, 0
  poke SP_dec3, 0

  isLarge = 1                 ; everything we send here is >= 1

  ; w10 (b20,b21) contains the value to normalize
  if w10 < 10 then                      ; 1-9:  I x 10^0
    poke SP_intDigit, b20
    poke SP_exponent, 0

  elseif w10 < 100 then                 ; 10-99:  I.D x 10^1
    b19 = w10 / 10
    poke SP_intDigit, b19
    b19 = w10 // 10
    b19 = b19 * 16                      ; digit -> high nibble
    poke SP_dec1, b19
    poke SP_exponent, 1

  elseif w10 < 1000 then                ; 100-999:  I.DD x 10^2
    b19 = w10 / 100
    poke SP_intDigit, b19
    w11 = w10 // 100
    b19 = w11 / 10
    b19 = b19 * 16
    b20 = w11 // 10
    b19 = b19 + b20
    poke SP_dec1, b19
    poke SP_exponent, 2

  elseif w10 < 10000 then               ; 1000-9999:  I.DDD x 10^3
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

  else                                  ; 10000-65535:  I.DDDD x 10^4
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

  ; safety check: the integer digit must be 1-9
  peek SP_intDigit, b19
  if b19 = 0 then
    poke SP_intDigit, 1
  elseif b19 > 9 then
    poke SP_intDigit, 9
  endif
  return

calculate_checksum:
    ; Sum ALL 15 bytes: bytes 0-14
  checksum = $3A + $00 + $01 + $00 + $01 ; Bytes 0-4 (preamble + header)

  ; Byte 5: intDigit
  peek SP_intDigit, b19
  checksum = checksum + b19
  peek SP_dec1, b19
  checksum = checksum + b19
  peek SP_dec2, b19
  checksum = checksum + b19
  peek SP_dec3, b19
  checksum = checksum + b19

  ; the four remaining BCD bytes are $00 = 0 so nothing to add
  
  ; Byte 13: signInfo
  peek SP_signInfo, b19
  checksum = checksum + b19
  
  ; Byte 14: exponent  
  peek SP_exponent, b19
  checksum = checksum + b19

  ; take the ':' back out, flip the bits, add one - see the block above
  b19 = checksum - $3A
  b19 = 255 - b19             ; one's complement
  checksum = b19 + 1
  return

send_end_packet:
  pause TURNAROUND            ; let the calculator turn round first
  ; 50 bytes: ':' E N D, then 45 x $FF, then the constant $56.

  for b19 = 0 to 3
    lookup b19, ($3A, $45, $4E, $44), txByte
    gosub put_byte
  next b19

  ; 45 bytes of $FF padding. b19 is a BYTE, so 45 is safely in range.
  txByte = $FF
  for b19 = 1 to 45
    gosub put_byte
  next b19

  ; final byte: the END checksum, constant $56
  txByte = $56
  gosub put_byte
  return


#REM

==================
CASIO FX_9750 GIII CODE
Casio BASIC companion program - universal
works with Picaxe, Arduino, ESP32, ESP8266, BBC Micro:bit
NOTE: edit in calculator: T*(I-1)->List 1[I] is T times (I-1),
 and the * is a MULTIPLY SIGN - not the letter x, which the calculator
 would read as a variable named Tx...
and check for unwanted spaces, proper List command 
(not individual letters spelling the word LIST), etc 

"MEMCHECK"->List 1[0]
If Dim List 1>1
Then ClrText
Locate 1,1,"EXISTING DATA"
Locate 1,2,"Samples:"
Locate 10,2,Dim List 1
Locate 1,4,"Copy to PC first?"
Locate 1,6,"1=OVERWRITE"
Locate 1,7,"0=QUIT"
?->Z
If Z=0
Then Stop
IfEnd
IfEnd
ClrList 1
ClrList 2
ClrList 3
ClrList 4
0->I
0->T
ClrText
Locate 1,1,"CONNECTING..."
Receive(N)
Lbl 1
ClrText
Locate 1,2,"SENSORS FOUND"
Locate 16,2,N
Locate 1,4,"Select interval"
Locate 1,5,"1-300 seconds"
?->T
If T<1
Then Goto 1
IfEnd
If T>300
Then Goto 1
IfEnd
ClrText
Locate 1,1,"Reading "
Locate 9,1,N
Locate 11,1," sensor(s)"
Locate 1,2,"@ "
Locate 3,2,T
Locate 11,2,"AC=stop"
Locate 1,3,"Sample: "
Locate 1,6,"Elapsed(s):  "
Send(T)
Lbl 2:
Receive(A)
If N>1
Then Receive(B)
Ifend
If N>2
Then Receive(C)
Ifend
I+1->I
T*(I-1)->List 1[I]
A->List 2[I]
Locate 1,4,"A:        B:     "
Locate 4,4,A
Locate 9,3,I
Locate 13,6,List 1[I]
If N>1
Then B->List 3[I]
Locate 13,4,B
Ifend
If N>2
Then C->List 4[I]
Locate 1,5,"C:      "
Locate 4,5,C
Ifend
If I>997
Then ClrText
Locate 1,1,"LIST FULL"
Locate 1,2,""
Locate 1,3,I
Locate 4,3," samples recorded"
Locate 1,4,""
Locate 1,5,"Data in List(s)"
Locate 1,7,"Press AC"
Getkey
Stop
IfEnd
Goto 2

==================
==================
CASIO FX_9750G PLUS CODE

255->Dim List 1
255->Dim List 2
255->Dim List 3
255->Dim List 4
0->I
0->T
ClrText
Locate 1,1,"CONNECTING..."
Receive(N)
Lbl 1
ClrText
Locate 1,2,"SENSORS FOUND"
Locate 16,2,N
Locate 1,4,"Select interval"
Locate 1,5,"1-300 seconds"
?->T
If T<1
Then Goto 1
IfEnd
If T>300
Then Goto 1
IfEnd
ClrText
Locate 1,1,"Reading "
Locate 9,1,N
Locate 11,1," sensor(s)"
Locate 1,2,"@ "
Locate 3,2,T
Locate 11,2,"AC=stop"
Locate 1,3,"Sample: "
Locate 1,6,"Elapsed(s):  "
Send(T)
Lbl 2:
Receive(A)
If N>1
Then Receive(B)
Ifend
If N>2
Then Receive(C)
Ifend
I+1->I
T*(I-1)->List 1[I]
A->List 2[I]
Locate 1,4,"A:        B:     "
Locate 4,4,A
Locate 9,3,I
Locate 13,6,List 1[I]
If N>1
Then B->List 3[I]
Locate 13,4,B
Ifend
If N>2
Then C->List 4[I]
Locate 1,5,"C:      "
Locate 4,5,C
Ifend
If I>254
Then ClrText
Locate 1,1,"LIST FULL"
Locate 1,2,""
Locate 1,3,I
Locate 4,3," samples recorded"
Locate 1,4,""
Locate 1,5,"Data in List(s)"
Locate 1,7,"Press AC"
Getkey
Stop
IfEnd
Goto 2

#ENDREM