#REM
 Casio-NSN-multisensor-08M2.bas
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
 Version 3.0; 30/07/2026
 (Production ready update of 2.0 rework 10/10/2025,
  original version 1.0 code for Picaxe 18X invented 2007)

  *** STATUS: PROOF OF CONCEPT - A FOUNDATION TO BUILD ON ***
 This is a reference implementation based on a validated method.
 It is deliberately minimal so that every line can be read and
 understood. It is NOT a finished classroom product and will not
 suit every use case. MODIFICATION IS EXPECTED - see the list of
 things a class would reasonably change, at the end of this header. 

 The NSN (normalised scientific notation) PICAXE implementation is
 validated in hardware.

 Up to 3 sensora, read synchronously but delivered as consecutive 
 seperate RECEIVE() values.

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
 =================================================================
 STUDENT / TEACHER WARNING!
 - NEVER use boiling water for temperature calibration (it is NOT needed)
 - NEVER connect mains electricity (240 V / 110 V) to the calculator,
   to this board, or to any sensor wiring. 
 - NEVER use mains-connected equipment near water.

 IF USING BREADBOARD WITH 4-PIN SERIAL TO PC USB CABLE
    ****  REMOVE RED V+ TO USB 5V HOOK UP WIRE  ****
    **** OPERATE PICAXE ON 2 x AA BATTERY PACK  ****
    ****   OR YuRobot 6V to 3.3V POWER MODULE   ****
 
 Modern (post 2020) Casio calculators are 3.3 V logic. 
 Power the PICAXE at 3 - 3.3 V and keep the resistors below in place.

 =================================================================
 A SB-62 cross-over cable has male 2.5mm TRS plugs at both ends.
 
 SERIAL FORMAT: 9600 baud, 8N2. The calculator RECEIVES expecting
 2 stop bits; the PICAXE's bit-banged SEROUT plus its natural
 between-byte processing time satisfies this.
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
 HARDWARE connections(Picaxe 08M2): Wire colours are users choice

 - Pin C.0 -> to Casio RX [ring of 2.5mm TRS jack, BLUE wire] via 1N4148
 	      diode; bar to the picaxe  (HSEROUT)
 - Pin C.1 <- from Casio TX [tip of 2.5mm TRS jack, YELLOW wire] (SERIN,
              and also the hardware hserin pin - see the header)
 - Pin C.1 -> 4.7k pull-up resistor to V+ (3.3 V) - REQUIRED
 - Pin C.2 Sensor 1 (analogue, readadc10) - NTC thermistor or LDR
 - Pin C.3 Sensor 2 (digital IN) - switch - magnetic/touch/impact  
 - Pin C.4 Sensor 3 (analogue, readadc10) - NTC thermistor or LDR OR
 			  DS18B20 temperature sensor
 - 0V      -> Casio GND [sleeve of 2.5mm TRS jack, BLACK wire]

===============================================
*** THE CALCULATOR DISPLAY IS NOT A CLOCK ***
 RULE: trust the List data, not the screen. Every reading is captured
 but T x (I-1) IS AN ASSUMPTION, NOT A MEASUREMENT: it is the interval
 you ASKED FOR times the sample number, and nothing is timestamped. It
 is right only if this board honours the interval, and at short
 intervals it does not - see MIN_DELAY_INTERVAL. A slow board does not
 stop or drop a reading; it STRETCHES THE TIME AXIS, silently.
 
 To measure timing you need AN EXTERNAL CLOCK shows. The calculator 
 cannot report its own timing at all.

 WARNING: sertxd blocks while it transmits, serin stops the TIME variable  
 counting which is why hserin is used- it is non-blocking.
 DS18B20 ReadTemp also stops the TIME variable.

 HOW THE CALCULATOR SIDE WORKS (Casio BASIC, for reference):
   Receive(N)                 ' ask how many sensors (this file: 1)
   ?->T : Send(T)             ' user types interval 2-300 seconds
   For 1->I To 999
     Receive(A)               ' PICAXE pauses HERE for T seconds,
     A->List 1[I]             ' then sends the fresh reading
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

#ENDREM

#picaxe 08M2

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

#no_data
disableBOD

; ===================================================================
; PIN ASSIGNMENTS
; ===================================================================
symbol TO_CASIO_pin   = C.0   ; serial out to calculator (ring)
symbol FROM_CASIO_pin = C.1   ; serial in from calculator (tip)
symbol SENSOR1_PIN    = C.2   ; analogue sensor 1
symbol SENSOR2_PIN    = pinC.3; digital sensor 2
symbol SENSOR3_PIN    = C.4   ; analogue sensor 3 / or DS18B20 temperature sensor

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
;  HOW IT LOOKS ON THE BENCH: a 5-second interval really takes 5.8 s,
;  so samples land at 5.8, 11.6, 17.4, 23.2... Read to the nearest
;  second that is 6, 5, 6, 6, 6, 5 - which looks like jitter and is
;  not. 5.8 s simply cannot be read as a whole number of seconds.
;
;  *** THE 5-SECOND FLOOR DOES NOT FIX THIS, AND WAS NEVER MEANT TO.
;  *** It exists so the 750 ms read FITS INSIDE the read window. That
;  *** is a scheduling problem. The time axis is a different problem,
;  *** and the clock stops during the read either way.
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
                              ; dead outside the routine. Checked: no
                              ; w11 value is live across a poll_byte
                              ; call.

symbol POLL_GUARD   = 2500    ; ~2 s at m16, sized from the bench: a
                              ; 6-operation poll pass measured 965 us on
                              ; 8 August, so this 5-operation loop is
                              ; ~805 us. The exact figure does not
                              ; matter - it fires only on link failure
                              ; and need only exceed the calculator's
                              ; ~300 ms turnaround.

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

; FREE for expansion: b18

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
                              ; Proven by loopback.

  ; The UART leaves a glitch byte in the two-deep receive FIFO after
  ; hsersetup. It is NOT drained here, and that is deliberate: the
  ; attention poll in main_loop discards anything that is not $15, so
  ; the glitch byte is eaten by the same lines that do the real work.
  ; A separate drain step could throw the attention byte away. 

  disabletime                 ; timer starts at the first reading

  ; clear the packet-building scratch RAM
  for b19 = SP_intDigit to SP_dec2
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

; ===================================================================
; MAIN LOOP - wait for the calculator to speak first
; The calculator opens every transaction with attention byte $15.
; We answer $13 within its ~0.5-1 s window or it shows "Com ERROR".
; ===================================================================
main_loop:
  ; Wait for the attention signal from the Casio.
  ; -----------------------------------------------------------------
  ; THE ATTENTION POLL
  ;
  ; THREE LINES, AND EACH ONE IS LOAD-BEARING:
  ;   rxWord = $FFFF    hserin leaves the variable UNCHANGED if no byte
  ;                     arrived, so the sentinel must be re-armed every
  ;                     time. $FF is a legal data byte; $FFFF is not.
  ;   hserin rxWord     non-blocking. Returns instantly either way.
  ;   the two ifs       loop back on "nothing yet" AND on "something,
  ;                     but not $15". That second one replaces serin's
  ;                     (":") qualifier, and it is also what eats the
  ;                     stale packet bytes and the hsersetup glitch
  ;                     byte without a separate drain step that could
  ;                     swallow the attention byte. See the header.
  ;
  ; THE CLOCK RUNS THROUGH ALL OF THIS. That is the entire gain: serin
  ; disabled the timer interrupt and lost the calculator's ~300 ms
  ; turnaround every sample; a busy loop keeps perfect time.
  ; No timeout is needed because a poll is never disarmed.
  ; -----------------------------------------------------------------
  rxWord = $FFFF
  hserin rxWord
  if rxWord = $FFFF then main_loop           ; nothing yet
  if rxLow <> CASIO_ATTENTION then main_loop ; stale byte - discard
  
  inByte = CASIO_ATTENTION    ; reached only on a real $15, so the
                              ; test below is now always true. It is
                              ; kept so this file stays a one-change
                              ; diff against Casio-MFE-HSO-14M2.bas.


  if inByte = CASIO_ATTENTION then

    ; Send immediate response (Picaxe is present and ready)
    hserout 0, (PICAXE_PRESENT)

    ; The calculator now sends a 50-byte packet. Both kinds put the
    ; command letter first and the variable name at byte 12:
    ;   ":REQ..." (Receive) -> command 'R'
    ;   ":VAL..." (Send)    -> command 'V'
    ; Read ':' + command + 9 bytes + vname = the part we need.

    serin FROM_CASIO_pin, T9600_16, (":"), command, inByte, inByte, inByte, inByte, inByte, inByte, inByte, inByte, inByte, vname

    ; Let the remaining ~38 bytes of the 50 finish arriving before
    ; we answer (50 ms; a byte takes ~1.15 ms at 9600 baud 8N2).
    pause 200                 ; 200/4 = 50 ms at 16 MHz

    ; Process command from Casio
    if command = CMD_RECEIVE then
      gosub handle_receive    ; calculator wants a value (No. of sensors or sensor reading)
    elseif command = CMD_SEND then
      gosub handle_incoming   ; calculator sent a number (T; time delay, or remote control) 
    endif

  endif

  goto main_loop

; ===================================================================
;  poll_byte - ONE BYTE, WITHOUT STOPPING THE CLOCK
; ===================================================================
;  Replaces  serin [timeout, label], FROM_CASIO_pin, T9600_16, inByte
;  RETURNS   pollOK = 1 and inByte = the byte
;            pollOK = 0 if the guard expired
; ===================================================================
poll_byte:
  ; -----------------------------------------------------------------
  ; DRAIN FIRST.
  ;
  ; The UART receives EVERYTHING the calculator sends, in parallel
  ; with the serin packet reads. Those reads take 11 or 15 bytes of a
  ; FIFTY-byte packet, so on every transaction the two-deep FIFO is
  ; left holding stale packet bytes.
  ;
  ; Without this drain, poll_byte returns one of those stale bytes
  ; instead of the ACK. 
  ;
  ; WHY DRAINING IS SAFE HERE, WHEN IT WOULD BE A BUG IN main_loop.
  ; Every call to poll_byte happens IMMEDIATELY AFTER WE HAVE
  ; TRANSMITTED. The calculator's turnaround is about 300 ms and this
  ; drain takes microseconds, so nothing we want can have arrived yet:
  ; anything in the FIFO at this instant is old BY DEFINITION.
  ;
  ; In main_loop the opposite holds - the awaited byte can arrive at
  ; any moment - which is why THAT loop discards non-$15 bytes instead
  ; of draining. Same problem, two correct answers, and using either
  ; one in the other place breaks it.
  ; -----------------------------------------------------------------
poll_byte_drain:
  rxWord = $FFFF
  hserin rxWord
  if rxWord <> $FFFF then poll_byte_drain   ; until empty

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

; ===================================================================
; HANDLE Receive(x) - THE HEART OF THE LOGGER
;
; Normal protocol flow (all at 9600 baud):
;   us:   ACK                  "request received"
;   them: ACK
;   us:   50-byte description  "variable x is real, in use"
;   them: ACK
;         |   <===  FENTON'S TIMING WINDOW  ===
;         |   The pause goes RIGHT HERE, between the acknowledged
;         |   description and the value packet. The calculator sits
;         |   in Receive() and waits at least 300 s proven without 
;         |   COM error. This position is the useful discovery
;         |
;   us:   16-byte value packet
;   them: ACK
;   us:   50-byte END packet
; ===================================================================
handle_receive:
  ; Send acknowledgment
  hserout 0, (CASIO_ACK)
  
  ; Wait for Casio's acknowledgment (timeout after 2 seconds)
  gosub poll_byte                   ; POLLED - see poll_byte
  if pollOK = 0 then request_timeout
  if inByte <> CASIO_ACK then
    return                    ; calculator gave up (AC key?)
  endif

  ; Send description packet (tells Casio what variable we're sending)
  gosub send_description

  ; Wait for acknowledgment
  gosub poll_byte                   ; POLLED - see poll_byte
  if pollOK = 0 then request_timeout
  if inByte <> CASIO_ACK then
    return
  endif

     ; == MICHAEL FENTON HOST-WAIT WINDOW DISCOVERY: The value window ==
     ; Pausing here during RECEIVE does NOT trigger COM ERROR on Casio
     ; This is the heart of Fenton's timed data logging discovery

  ; Determine which variable Casio requested
  ;
  ; *** EVERY BRANCH MUST SET isNegative AS WELL AS currentValue. ***
  ; The two together are the reading - a magnitude with no sign is
  ; only half the number. isNegative is a WORKING bit reloaded here
  ; for each packet; the stored signs live in sensor1Neg/2Neg/3Neg.
  ; Omitting it does not fail, it inherits whatever the previous
  ; packet used, which is how sensor 3's minus sign reached sensors
  ; 1 and 2. If you add a variable D, set both or it will do it again.

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
  
  ; Wait for final acknowledgment (optional - continue anyway if timeout)
  gosub poll_byte   
  
request_timeout:
  gosub send_end_packet
  return

; ===================================================================
; WAIT FOR THE INTERVAL, THEN READ SENSOR - drift-free timing
;
; 'time' ticks in real seconds (even at 16 MHz). The next reading is
; always due at nextSendTime, and after each reading we add EXACTLY
; timeInterval. The schedule is arithmetic: protocol overhead cannot
; accumulate into drift.  
;
; The sensor is read 700 ms BEFORE the due time, so the data is fresh
; and the read time is absorbed inside the interval. If using slower
; sensors, increase that margin (and MIN_DELAY_INTERVAL) to suit.
;
; Limits: 'time' is a WORD variable, it wraps after 65535 s (~18 hours).
; 999 readings x 300 s = 83 hours would wrap; at 300 s intervals
; keep runs under ~215 readings, or reset between runs.
; ===================================================================
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
  ; A CONSTANT AND THE COMMENT THAT DESCRIBES IT MUST MOVE TOGETHER.
  ;
  ; Why not read right on the deadline: at short intervals the ADC work
  ; would butt up against the send, leaving nothing in hand if a read
  ; runs long. A dedicated slot removes that collision and still keeps
  ; the sample well inside its own interval.
  ; The guard skips the wait if we are already running late.

  if time < nextSendTime then
    pause SENSOR_READ_OFFSET
  endif

  ; ==== READ ALL SENSORS ====
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

; ===================================================================
; READ THE SENSORS (together - this is the "synchronous" in
; synchronous multi-sensor logging)
; ===================================================================
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
  sensor1Value = sensor1Value + 1
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

  ; readtemp gives whole degrees C, negatives as 128 + magnitude.
  ; A MISSING sensor leaves the line high through the 4.7k pull-up and
  ; readtemp returns $FF = 255, which would mean -127 C - not a
  ; plausible school measurement, so it is a safe sentinel.
  
  if sensorCount >= 3 then
    ; PRESENCE IS DECIDED BY THE ROLL CALL IN init, NOT BY THE READING.
    ; ASK EVERY SAMPLE, NOT ONCE AT SWITCH-ON.
    ; A sensor unplugged DURING a run, readtemp returns 0, and
    ; the logger reported a confident 0 degrees C - a fault wearing the
    ; costume of a measurement, and the exact thing this project refuses
    ; to ship. Now presence is re-established before every reading.
    ;
    ; *** readowsn WRITES INTO b6 TO b13 *** - sensor3Value (b6,b7),
    ; command (b8), vname (b9), inByte (b10), checksum (b11) and
    ; currentValue (b12,b13). IT IS SAFE HERE, and that was checked
    ; rather than assumed: by this point the calculator's request has
    ; been served as far as vname, the next serin reloads command,
    ; vname and inByte, the checksum is rebuilt from scratch when the
    ; packet is assembled, and sensor3Value is overwritten three lines
    ; below. Nothing that matters is read again before it is rewritten.
    ;
    ; IT IS NOT SAFE ANYWHERE INSIDE A TRANSACTION. Move this and you
    ; will destroy the variable name being served and the checksum
    ; being built, and the packet will go out corrupt with no error.
    ;
    ; Cost: a 1-Wire reset plus 64 bits, a few ms, and it stops the
    ; timer like readtemp does. Against the 750 ms already spent on the
    ; conversion, and the 5 s interval floor, this is noise.
    ; *** CLEAR b6 FIRST. THIS LINE IS THE WHOLE DETECTOR. ***
    ;
    ; From a cold start b6 is 0, readowsn finds nothing, b6 stays 0,
    ; and absence is detected. With a sensor already found, b6 holds
    ; $28; pull the wire and readowsn appears to leave it there, so
    ; the chip goes on believing a sensor is present and reports the
    ; 0 that readtemp hands back - a fault dressed as a measurement.
    ;
    ; ZEROING b6 MAKES A SILENT NO-OP READ AS ABSENCE, which is the
    ; only safe direction for the failure to fall.

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

; -------------------------------------------------------------------
;  Other channels: three reads averaged, then offset.
;  ~5 ms per channel - against a 325 ms read slot at a 1 s interval,
;  averaging is not a constraint.
; -------------------------------------------------------------------
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
   if SENSOR2_PIN = 0 then 
	sensor2Value = 0
   else 
	sensor2Value = 1
   Endif 
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
  
; ===================================================================
; HANDLE Send(x) ? the calculator is giving US a number (T)
;
; 16-byte value packet layout (see Technical Reference):
; Packet structure:
; Byte 0: Preamble (0x3A = ':')
; Bytes 1-4: Header (0x00 0x01 0x00 0x01)
; Byte 5: Integer digit (I)
; Bytes 6-12: Decimal digits (BCD packed)
; Byte 13: Sign info
; Byte 14: Exponent
; Byte 15: Checksum
; ===================================================================
handle_incoming:
  ; Send acknowledgment 
  hserout 0, (CASIO_ACK)

  ; Read the 15 bytes after ':'. The 7th BCD byte goes into b23

  ; Read preamble and header (5 bytes)
  serin FROM_CASIO_pin, T9600_16, (CASIO_PREAMBLE), inByte, inByte, inByte, inByte, b19, b20, b21, b22, inByte, inByte, inByte, b23, b26, b27, checksum

  poke SP_intDigit, b19
  poke SP_dec1, b20
  poke SP_dec2, b21
  poke SP_signInfo, b26
  poke SP_exponent, b27

  hserout 0, (CASIO_ACK)

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

; ===================================================================
; DECODE A CASIO NUMBER (integer 0..65535 is all a PICAXE can hold)
; The number arrives normalised: I.DDDDDDDDDDDDDD x 10^E
; For our use (T = 2..300) only exponents 0..4 matter.
; ===================================================================
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
; SEND THE 50-BYTE DESCRIPTION PACKET  ":VAL...Variable R..."
; Tells the calculator: "variable <vname> is a real number, in use."
; Checksum reduces to the constant expression 273 - vname (verified).
; ===================================================================
send_description:
  checksum = 273 - vname ; Optimized: was $D0 - vname + 65

  hserout 0, (CASIO_PREAMBLE, $56, $41, $4C, $00, $56, $4D, $00, $01, $00, $01)
  hserout 0, (vname)
  hserout 0, ($FF, $FF, $FF, $FF, $FF, $FF, $FF)
  hserout 0, ($56, $61, $72, $69, $61, $62, $6C, $65, $52, $0A)
  hserout 0, ($FF, $FF, $FF, $FF, $FF, $FF, $FF, $FF, $FF, $FF, $FF, $FF, $FF, $FF, $FF, $FF, $FF, $FF, $FF, $FF)
  hserout 0, (checksum)
  return

; ===================================================================
; SEND THE 16-BYTE VALUE PACKET
; Number format: I.DDDDDDDDDDDDDD x 10^E (normalised scientific
; notation - the same form students meet in Year 9-10 maths).
; ===================================================================
send_value_packet:
  if currentValue = 0 then
    gosub send_zero_packet    ; zero has its own special packet
    return
  endif

  ; copy magnitude into w10 (b20,b21) for the normaliser
  b20 = b12
  b21 = b13

  gosub normalise_value

  ; sign/info byte: bit0 = |value|>=1, bits 6+4 = negative
  b19 = 0
  if isNegative = 1 then
    b19 = $50 ; Bits 6 & 4 for negative
  endif
  if isLarge = 1 then
    b19 = b19 + 1 ; Bit 0 for |value| >= 1
  endif
  poke SP_signInfo, b19

  ; Send 16-byte packet (optimized: using scratchpad values)
  ; transmit: ':' 00 01 00 01, I, 7 BCD bytes, sign, exponent, sum
  peek SP_intDigit, b19
  hserout 0, (CASIO_PREAMBLE, $00, $01, $00, $01, b19)
  
  ; Send 7 decimal bytes (only first 2 have real data for our range)
  peek SP_dec1, b19
  peek SP_dec2, b20
  hserout 0, (b19, b20, $00, $00, $00, $00, $00)
  
  ; Send sign and exponent
  peek SP_signInfo, b19
  peek SP_exponent, b20
  hserout 0, (b19, b20)
  
  ; Calculate and send checksum
  gosub calculate_checksum
  hserout 0, (checksum)
  return

; ===================================================================
; SEND ZERO (0 x 10^0, magnitude flag clear, checksum constant $FE)
; ===================================================================
send_zero_packet:
  hserout 0, (CASIO_PREAMBLE, $00, $01, $00, $01, $00, $00, $00, $00, $00, $00, $00, $00)
  hserout 0, ($00, $00, $FE)
  return

; ===================================================================
; Limited Precision Normalization  
; NORMALISE w10 (b20,b21) INTO I.DDDD x 10^E FORM
; Fills SP_intDigit, SP_dec1, SP_dec2, SP_exponent.
; Max input 65535 -> 5 significant digits -> 2 BCD bytes sufficient.
; Uses w11 (b22,b23) for remainders and b19 for digit assembly 
; deliberately clear of the timing variables.
; ===================================================================
normalise_value:
  ; Clear decimal bytes in scratchpad
  poke SP_dec1, 0
  poke SP_dec2, 0

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

; ===================================================================
; PACKET CHECKSUM  -  the same rule for every packet in this protocol
;
; IN WORDS: add up every byte of the packet AFTER the ':' preamble
; and BEFORE the checksum itself. The checksum is whatever you must
; add to that total to bring it back to zero, working in single
; bytes. In eight bits 200 + 56 = 0, because 256 wraps round to
; nothing. That is called a TWO'S COMPLEMENT.
;
; It is the same rule for the 16-byte value packet and the 50-byte
; description and END packets. There is only one rule to learn.
;
; HOW THIS CODE DOES IT: the traditional form, used by every
; published description of the protocol, adds the ':' in at the
; start and takes it out again at the end:
;
;     total    = $3A + (every other byte)
;     b19      = total - $3A     ; takes the ':' back out
;     b19      = 255 - b19       ; flips the bits (one's complement)
;     checksum = b19 + 1         ; ...plus one = two's complement
;
; Adding $3A and then subtracting it does nothing at all, so the two
; forms give the same answer. The traditional shape is kept here so
; this code can be read side by side with the published references.
;
; Other languages write the bit flip as ~x. PICAXE BASIC has no ~
; operator, so it is written as 255 - x.
;
; WHY SOME CHECKSUMS ARE CONSTANTS: if every byte of a packet is
; fixed, its checksum is fixed too. That is why END is always $56,
; the zero packet always $FE, and the description packet always
; 273 - vname (the variable name being the one byte that changes).
;
;   *** A constant is only correct while nothing else in the packet
;   *** changes. Anything you build whose content varies MUST
;   *** calculate its checksum, not store it.
;
; COUNTING BYTES: this block counts from 1, like the technical
; reference - bytes 1 to 15 are summed and byte 16 is the checksum.
; The comments inside the routine below count from 0, like the code.
; Both are correct; they are simply different conventions.
;
; See: RIGEL Casio Protocol Technical Reference, Part II Chapter B.
; ===================================================================
calculate_checksum:
    ; Sum ALL 15 bytes: bytes 0-14
  checksum = $3A + $00 + $01 + $00 + $01 ; Bytes 0-4 (preamble + header)

  ; Byte 5: intDigit
  peek SP_intDigit, b19
  checksum = checksum + b19
  
  ; Bytes 6-12: decimal bytes (only dec1 and dec2 have real data)
  peek SP_dec1, b19
  checksum = checksum + b19
  peek SP_dec2, b19
  checksum = checksum + b19
  
  ; the five remaining BCD bytes are $00 = 0 so nothing to add
  
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

; ===================================================================
; SEND THE 50-BYTE END PACKET  ":END" + 45 x $FF + constant $56
; ===================================================================
send_end_packet:
  ; Send header: :END (0x3A 0x45 0x4E 0x44)
  hserout 0, (CASIO_PREAMBLE, "E", "N", "D")
  
  ; Send 45 bytes of 0xFF padding (9 bytes ? 5 iterations)
  for b19 = 1 to 5
    hserout 0, ($FF, $FF, $FF, $FF, $FF, $FF, $FF, $FF, $FF)
  next b19
  
  ; Send final 1 byte padding + checksum (0x56 - constant for END)
  hserout 0, ($56)
  return


#REM

; ===================================================================
; STUDENT NOTES - HOW THIS VERSION SENDS NEGATIVE READINGS
; ===================================================================
 
This file can send a reading of -3.7 to the calculator and have it
arrive as -3.7. Here is how to switch it on.

WHY A PICAXE NEEDS A TRICK AT ALL
A PICAXE word variable holds 0 to 65535. There is no minus sign
anywhere in the chip. Subtract past zero and the value wraps round
to 65535 instead of going negative. So the sign cannot be kept in
the number itself - it has to be carried separately.

WHERE THE SIGN LIVES IN THIS FILE
In a single bit per sensor - sensor1Neg, sensor2Neg and sensor3Neg
in the symbol list above. The reading itself stays in sensorNValue
as a plain positive magnitude. Two pieces of information, kept
apart:

      sensor1Value = 37  with  sensor1Neg = 1   means  -37
      sensor1Value = 37  with  sensor1Neg = 0   means  +37

This arrangement has a name - SIGN AND MAGNITUDE - and it is one of
the ways real computers store signed numbers.

HOW THE SIGN REACHES THE CALCULATOR
Find these lines in send_value_packet:

      b19 = 0
      if isNegative = 1 then
        b19 = $50 ; Bits 6 & 4 for negative
      endif

Byte 14 of the value packet is the sign/info byte. Setting bits 6
and 4 tells the calculator that the number is negative, and it
stores a genuinely negative value in its list. Nothing has to be
undone at the calculator end. No arithmetic is needed there at all.

The DS18B20 on channel 3 sets a negative sign whenever it reads
below zero, so this machinery is live rather than dormant. The two
analogue channels do not use it yet, because readadc10 can only
return 0 to 1023 and so never needs a sign. Give either of them a
reason to be negative and it starts working immediately.

THERE ARE FOUR SIGN BITS, AND THEY DO DIFFERENT JOBS
This is the part to read carefully, because it is where a real bug
lived until it was found by inspection.

      sensor1Neg, sensor2Neg, sensor3Neg   STORED signs. One per
                                           sensor, set when that
                                           sensor is read.
      isNegative                           the WORKING sign. It
                                           describes the packet
                                           being built right now.

All three sensors are read ONCE, at the start of an interval. The
calculator then asks for them one at a time - A, then B, then C -
so three packets are built from one set of readings. When there was
only ONE sign bit shared between them, sensor 3's minus sign was
still set when sensor 1's packet went out, and a perfectly ordinary
positive light reading arrived at the calculator as a negative
number. No error appeared. It would never show in a warm room and
would happen every time in an ice-water practical.

So: set the STORED bit when you read the sensor, and let
handle_receive copy it into isNegative when that sensor's turn
comes. If you add a fourth sensor, give it its own stored bit.

EXAMPLE: A NTC THERMISTOR THAT READS BELOW ZERO DEGREES
Suppose your conversion produces a temperature in tenths of a
degree, and it can fall below freezing. Work out the magnitude and
the sign separately, then load them into the two variables:

      readadc10 SENSOR1_PIN, tempWord
      ; ...your conversion turns temp into tenths of a degree...
      ; ...and you know from it whether the result was below zero...
      sensor1Value = tempWord    ; magnitude only, always positive
      sensor1Neg   = 1           ; ...or 0 for a reading above zero
      return

*** tempWord IS w13 IN THIS FILE. CHECK THE SYMBOL TABLE, DO NOT
TRUST THIS NOTE***

Note also that it is sensor1Neg that is set, NOT isNegative.
isNegative is loaded from the stored bits in handle_receive and
anything you write to it inside a read routine is discarded.

EXAMPLE: CENTRING A SENSOR ON ZERO
An LDR reads 0-1023 and is never negative. To make it read zero in
average light and swing both ways:

      readadc10 SENSOR1_PIN, tempWord
      if tempWord >= 512 then
        sensor1Value = tempWord - 512   ; brighter than average
        sensor1Neg   = 0
      else
        sensor1Value = 512 - tempWord   ; dimmer: magnitude of the gap
        sensor1Neg   = 1                ; ...and mark it negative
      endif
      return

The calculator now plots a graph that crosses zero, which is usually
far easier to read than one sitting up around 512.

CALIBRATION TIPS
- NEVER use boiling water for temperature calibration (it is NOT needed)
- NEVER connect mains electricity (240 V / 110 V) to the calculator,
   to this board, or to any sensor wiring. 
- NEVER use mains-connected equipment near water.

- Measure the real output range of your sensor before scaling it.
- Choose the centring point so the swing is balanced either side.
- Non-linear sensors - NTC thermistors especially - need a lookup
  table or a piecewise fit, not a single multiply.
  Alternative: use a Steinhart-Hart equation
- Averaging several readings to reduce noise is already done for
  you in this code.

==================
CASIO FX_9750 CODE
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
Locate 1,1,"Receive "
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
I+1->I
T*(I-1)->List 1[I]
A->List 2[I]
Locate 1,4,"A:               "
Locate 4,4,A
Locate 9,3,I
Locate 13,6,A
If N>1
Then Receive(B)
B->List 3[I]
Locate 11,4,"B:"
Locate 14,4,B
Ifend
If N>2
Then Receive(C)
C->List 4[I]
Locate 1,5,"C:      "
Locate 4,5,C
Ifend
If I>997
Then ClrText
Locate 1,1,"MAXIMUM LIMIT"
Locate 1,2,""
Locate 1,3,I
Locate 4,3," samples recorded"
Locate 1,4,""
Locate 1,5,"Data in Lists 1-4"
Locate 1,7,"Press AC"
Getkey
Stop
IfEnd
Goto 2

#ENDREM