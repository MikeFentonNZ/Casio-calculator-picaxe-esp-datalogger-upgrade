#REM
 Casio-Wait-Window-proof-08M2.bas
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
 
 Coded for PICAXE 08M2. Responds ONLY to Receive(N), the sensor count request.
 No sensors, no interval wait, no Send( handling. Smallest complete
 Casio transaction. Host-wait window probe. Not a logger.
 All four gaps are marked below, numbered 1-4 by position.

 Proven in Picaxe 18X in 2007/2008 and in Picaxe 14M2, ESP8266, ESP32,
 BBC Micro:bit V1, and BBC Micro:bit V2 in 2025. Should work with Casio FX-9860.

 All platforms successfully coded to demonstrate fully functional bidirectional  
 serial communication at 9600 baud using the 3-pin 2.5 mm port for multisensor
 time-interval data logging using the Casio FX-9750GIII. 
 
 Standard SB-62 cables and homemade DIY crossover cables provide the connection 
 between the Casio calculator and an external microcontroller. 

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
 
 Author website:  
 https://mikefentonnz.github.io/projects/casio-calculator-data-logger-hack.html

 ===================================================================
 HARDWARE connections(Picaxe 08M2): Wire colours are users choice

 - Pin C.0 -> to Casio RX [ring of 2.5mm TRS jack, BLUE wire] via 1N4148
 	      diode; bar to the picaxe  (HSEROUT)
 - Pin C.1 <- from Casio TX [tip of 2.5mm TRS jack, YELLOW wire] (SERIN,
              and also the hardware hserin pin - see the header)
 - Pin C.1 -> 4.7k pull-up resistor to V+ (3.3 V) - REQUIRED
 - Pin C.1 -> 10k IN SERIES from the TIP, and a 1N5711 Schottky from
            C.1 to V+, BAND toward V+
            *** REQUIRED IF V+ IS 3.3 V AND YOU USE AN FX-9750G PLUS ***
 - Pin C.2 -> RED LED   - locked
 - Pin C.4 -> GREEN LED - unlocked
 - Pin C.3    unused
 - 0V      -> Casio GND [sleeve of 2.5mm TRS jack, BLACK wire]


  *** THE RECEIVE NETWORK - SETTLED 2026 ***
  Use the universal interface. Four parts, one circuit, and it serves
  both calculator generations on a 3.3 V board and on a 5 V board:

      Casio TIP --+-- 4.7k --- 3.3 V     (the BOARD's own supply)
                  |
                  +-- 10k ---+--- GPIO 16
                             |
                             +--|<|--- 3.3 V   1N5711, BAND to 3.3 V

 THE RECEIVE-SIDE CLAMP - separate from the 1N4148 above.
 An FX-9750G Plus holds its transmit line at 4.75 V, measured. Run
 this chip at 3.3 V and that lands above its own supply, so the 10k
 limits the current to about 110 uA and the 1N5711 - 0.3 V forward -
 conducts before the PIC's internal protection diode at 0.6 V and
 takes the current first. Run the chip at 5 V (a PC USB port will do
 it) and nothing exceeds the supply, so the Schottky never conducts
 and costs nothing. An FX-9750GIII holds its line at 2.75 V and never
 needs the clamp on either supply.
 THE 10k IS IN SERIES ONLY. Nothing goes from C.1 to 0V: that makes a
 divider, which drops a GIII's 2.75 V to about 1.8 V and breaks it.
 Small-signal Schottky only - BAT85 or BAT43 will do, a 1N5817 will
 not.  

 *** WHY A DIODE HERE AND NOT A SERIES RESISTOR ***
 The diode makes this an OPEN-DRAIN output: the PICAXE can only ever PULL
 THE LINE LOW. When it drives high the diode blocks, and the calculator
 raises the line with its own internal pull-up. Measured 15 Aug 2026.

  - The PICAXE supply no longer sets the calculator's high level, so the
    same wiring serves a 3.3 V FX-9750GIII and a 5 V FX-9750G Plus.
  - No contention is possible, so no series resistor is wanted. Do NOT fit
    a 1k as well - it shares the current path and pushes the LOW level back
    up, which is what breaks the link.
  - BAR (cathode) TOWARD THE PICAXE. Reversed, nothing works.

 1N4148 or 1N914 ON THE TRANSMIT LINE. A Schottky is not required in
 THAT position: the low sits near 0.6 V against a threshold near
 0.82 V, and the drop falls as current falls. This says nothing
 about the receive line, which is a different problem - see the
 1N5711 note above.
 Verified on PICAXE with an FX-9750G Plus and an FX-9750GIII, 2026.

===============================================
#ENDREM

#picaxe 08M2
#no_data
disableBOD

symbol TO_CASIO_pin   = C.0
symbol FROM_CASIO_pin = C.1
symbol LED_pin = c.4

symbol CASIO_ATTENTION = $15
symbol PICAXE_PRESENT  = $13
symbol CASIO_PREAMBLE  = $3A
symbol CASIO_ACK       = $06
symbol CMD_RECEIVE     = "R"

symbol SP_intDigit = 28
symbol SP_signInfo = 29
symbol SP_exponent = 30
symbol SP_dec1     = 31
symbol SP_dec2     = 32

symbol flags        = b0
symbol isNegative   = bit0
symbol isLarge      = bit1
symbol pollOK       = bit2

symbol sensorCount  = b1
symbol command      = b8
symbol vname        = b9
symbol inByte       = b10
symbol checksum     = b11
symbol currentValue = w6
symbol rxWord       = w10
symbol rxLow        = b20
symbol pollGuard    = w11
symbol POLL_GUARD   = 2500

init:
  setfreq m16
  high TO_CASIO_pin
  hsersetup B9600_16, %00
  disabletime

  for b19 = SP_intDigit to SP_dec2
    poke b19, 0
  next b19

  flags       = 0
  sensorCount = 1

main_loop:
  rxWord = $FFFF
  hserin rxWord
  if rxWord = $FFFF then main_loop
  if rxLow <> CASIO_ATTENTION then main_loop
  inByte = CASIO_ATTENTION

  hserout 0, (PICAXE_PRESENT)

  serin FROM_CASIO_pin, T9600_16, (":"), command, inByte, inByte, inByte, inByte, inByte, inByte, inByte, inByte, inByte, vname

  pause 200

  if command = CMD_RECEIVE then
    gosub handle_receive
  endif

  goto main_loop

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

handle_receive:
  ;
  ; GAP 1 - host-wait window. Can pause hserout here indefinitely.
  ; Real, but not offered as a usable window: the whole description
  ; exchange still follows, so a reading taken here is already stale
  ; by the time the value packet goes out. The shipped builds put a
  ; fixed settling guard at this position, not a tunable window.
  ;
  For rxLOW = 1 to 20    ; pause 5 minutes
    toggle LED_pin	  
    pause 60000
  Next rxLow
  LOW LED_pin
  ;
  
  hserout 0, (CASIO_ACK)

  gosub poll_byte
  if pollOK = 0 then request_timeout
  if inByte <> CASIO_ACK then
    return
  endif

  ;
  ; GAP 2 - THE DESCRIPTION WINDOW. Can pause hserout here
  ; indefinitely. This is the window found in 2007 and used in the
  ; 2008 classroom trials to wait for a student to press a key.
  ;
  
  gosub send_description

  gosub poll_byte
  if pollOK = 0 then request_timeout
  if inByte <> CASIO_ACK then
    return
  endif

  currentValue = sensorCount
  isNegative   = 0

  ;
  ; GAP 3 - THE VALUE WINDOW. Can pause hserout here indefinitely.
  ; Found October 2025. This is where wait_for_interval sits in the
  ; full logging builds, because it is the last gate before the data
  ; goes out: a reading taken here is as fresh as it can be when the
  ; calculator timestamps it.
  ;

  gosub send_value_packet

  gosub poll_byte

request_timeout:

  ;
  ; GAP 4 - host-wait window. Real, and rejected for two reasons.
  ; The value has already been sent, so a pause here buys nothing;
  ; and request_timeout is the join point of BOTH paths - success
  ; falls into it, and all three poll failures jump to it - so a
  ; pause here would lengthen every failed transaction as well.
  ;

  gosub send_end_packet
  return

send_description:
  checksum = 273 - vname
  hserout 0, (CASIO_PREAMBLE, $56, $41, $4C, $00, $56, $4D, $00, $01, $00, $01)
  hserout 0, (vname)
  hserout 0, ($FF, $FF, $FF, $FF, $FF, $FF, $FF)
  hserout 0, ($56, $61, $72, $69, $61, $62, $6C, $65, $52, $0A)
  hserout 0, ($FF, $FF, $FF, $FF, $FF, $FF, $FF, $FF, $FF, $FF, $FF, $FF, $FF, $FF, $FF, $FF, $FF, $FF, $FF, $FF)
  hserout 0, (checksum)
  return

send_value_packet:
  if currentValue = 0 then
    gosub send_zero_packet
    return
  endif
  b20 = b12
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
  b19 = 255 - b19
  checksum = b19 + 1
  return

send_end_packet:
  hserout 0, (CASIO_PREAMBLE, "E", "N", "D")
  for b19 = 1 to 5
    hserout 0, ($FF, $FF, $FF, $FF, $FF, $FF, $FF, $FF, $FF)
  next b19
  hserout 0, ($56)
  return
