#REM
 Casio-NSN-multisensor-08M2-bare.bas
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

 This is the first member of the family:
   Casio-NSN-08M2.bas - three sensors, normalised scientific notation (THIS)
   Casio-MFE-14M2.bas - three sensors, MFE, data logging
   Casio-HMI-08M2.bas - Human machine interface - security PIN pad example
   Casio-IMC-14M2.bas - three sensors + three actuators

 Author: Michael Fenton. Unbounded host wait and MFE both his discovery.

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
 
  THE CALCULATOR ACCEPTS ONE STOP BIT. IT DOES NOT REQUIRE TWO.

  Both settings were tested here and both work. That confirms
  Grindheim (2001), who reported the link is asymmetric - two stop
  bits FROM the calculator, one TO it.

  WHY TWO ALSO WORKS: an extra stop bit is only extra idle line. The
  receiver has already sampled the byte and is waiting for the next
  start bit, which simply arrives a fraction later.
 
  It is incorrect to say that one stop bit is rejected.
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

symbol TO_CASIO_pin   = C.0   ; serial out to calculator (ring)
symbol FROM_CASIO_pin = C.1   ; serial in from calculator (tip)
symbol SENSOR1_PIN    = C.2   ; analogue sensor 1
symbol SENSOR2_PIN    = pinC.3; digital sensor 2
symbol SENSOR3_PIN    = C.4   ; analogue sensor 3 / or DS18B20 temperature sensor

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

symbol MAX_DELAY_INTERVAL  = 300   

#IFDEF USING_DS18B20
symbol MIN_DELAY_INTERVAL  = 5     
#ENDIF
#IFNDEF USING_DS18B20
symbol MIN_DELAY_INTERVAL  = 1     
#ENDIF

#IFDEF USING_DS18B20
symbol SENSOR_READ_OFFSET  = 4000
#ENDIF
#IFNDEF USING_DS18B20
symbol SENSOR_READ_OFFSET  = 1200
#ENDIF

symbol SP_intDigit = 28   
symbol SP_signInfo = 29   
symbol SP_exponent = 30  
symbol SP_dec1     = 31   
symbol SP_dec2     = 32  

symbol flags        = b0      
symbol isNegative   = bit0    
symbol isLarge      = bit1    

symbol sensor1Neg   = bit3    
symbol sensor2Neg   = bit4    
symbol sensor3Neg   = bit5    
symbol ds18b20Here  = bit6    
symbol sensorCount  = b1      
symbol sensor1Value = w1      
symbol sensor2Value = w2      
symbol sensor3Value = w3      
symbol command      = b8     
symbol vname        = b9      
symbol inByte       = b10     
symbol checksum     = b11     
symbol currentValue = w6      
symbol nextSendTime = w7      
symbol timeInterval = w8      
symbol readTarget   = w12     
symbol tempWord     = w13     
symbol rxWord       = w10     
symbol rxLow        = b20     
symbol pollOK       = bit2    
symbol pollGuard    = w11     
symbol POLL_GUARD   = 2500    
symbol DS18B20_FAMILY = $28   
symbol DS18B20_ABSENT = 999   
symbol SENSOR_MAX    = 1023  

init:
  setfreq m16                 

  high TO_CASIO_pin           
  hsersetup B9600_16, %00     

  disabletime                 

  for b19 = SP_intDigit to SP_dec2
    poke b19, 0
  next b19

  ds18b20Here = 0

  sensorCount   = 3
  timeInterval  = 10      
  nextSendTime  = 0
  sensor1Value  = 0
  sensor2Value  = 0
  sensor3Value  = 0

  sensor1Neg    = 0
  sensor2Neg    = 0
  sensor3Neg    = 0

main_loop:
  rxWord = $FFFF
  hserin rxWord
  if rxWord = $FFFF then main_loop       
  if rxLow <> CASIO_ATTENTION then main_loop  
  
  inByte = CASIO_ATTENTION   

  if inByte = CASIO_ATTENTION then
    hserout 0, (PICAXE_PRESENT)
    serin FROM_CASIO_pin, T9600_16, (":"), command, inByte, inByte, inByte, inByte, inByte, inByte, inByte, inByte, inByte, vname
    pause 200              

    if command = CMD_RECEIVE then
      gosub handle_receive    
    elseif command = CMD_SEND then
      gosub handle_incoming  
    endif

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
  ; Send acknowledgment
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
    currentValue = sensorCount 
    isNegative   = 0    

  elseif vname = VNAME_A then	  
    gosub wait_for_interval     
    currentValue = sensor1Value
    isNegative   = sensor1Neg

  elseif vname = VNAME_B then
    currentValue = sensor2Value
    isNegative   = sensor2Neg

  elseif vname = VNAME_C then
    currentValue = sensor3Value
    isNegative   = sensor3Neg

  else
    currentValue = 0  
    isNegative   = 0
  endif

  gosub send_value_packet
  
  gosub poll_byte   
  
request_timeout:
  gosub send_end_packet
  return

wait_for_interval: 
  if nextSendTime = 0 then
    enabletime
    time = 0
    nextSendTime = timeInterval
    gosub read_all_sensors
    return
  endif

#IFDEF USING_DS18B20
  readTarget = nextSendTime - 2     
#ENDIF

#IFNDEF USING_DS18B20
  readTarget = nextSendTime - 1   
#ENDIF
  do
    pause 50                   
  loop while time < readTarget

  if time < nextSendTime then
    pause SENSOR_READ_OFFSET
  endif

  gosub read_all_sensors   

  do
    pause 1      
  loop until time >= nextSendTime

  nextSendTime = nextSendTime + timeInterval

  if time > nextSendTime then
      nextSendTime = time + timeInterval
  endif

  return

read_all_sensors:
  sensor1Neg = 0
  sensor2Neg = 0
  sensor3Neg = 0

#IFDEF SIMULATE_SENSORS
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
      sensor3Value = DS18B20_ABSENT 
      sensor3Neg   = 0     
    else
      readtemp SENSOR3_PIN, b19
      if b19 > 127 then
        b19 = b19 - 128      
        sensor3Neg = 1              
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
  pause 5            
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

  if vname = VNAME_T then
    timeInterval = currentValue

    b19 = MIN_DELAY_INTERVAL
    if timeInterval < b19 then
      timeInterval = b19
    endif
    if timeInterval > MAX_DELAY_INTERVAL then
      timeInterval = MAX_DELAY_INTERVAL
    endif

    nextSendTime = 0  

  endif
  return

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

send_description:
  checksum = 273 - vname ; Optimized: was $D0 - vname + 65

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
    b19 = $50 ; Bits 6 & 4 for negative
  endif
  if isLarge = 1 then
    b19 = b19 + 1 ; Bit 0 for |value| >= 1
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
  
  ; w10 (b20,b21) contains the value to normalize
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
  checksum = $3A + $00 + $01 + $00 + $01 ; Bytes 0-4 (preamble + header)

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