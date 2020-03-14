#include <avr/io.h>

#define PORT _SFR_IO_ADDR(PORTB)
#define DDR _SFR_IO_ADDR(DDRB)
#define PIN PINB0

.text
.globl sendbyte

sendbyte:
  ;cli
  push R5
  push R6
  push R7
  push R16
  push R17
  push R18
  push R19
  push R22
  push R23
  push R24
  push R25
  push R30
  push R31
  ////////////////////
  in R16, PORT
  in R17, PORT
  ori R16, (1<<PIN)                   ;Bitmask set bit
  andi R17, ~(1<<PIN)                 ;Bitmask clear bit
  out DDR, R16                        ;setzt das entsprechende bit als Output !!!! Überarbeiten
  movw R30, R24                       ;setzt den Z pointer aus den Übergebenen Variablen
  push R22
  or R22, R23                         ;check if both Registers are zero
  brne nzero
  pop R22
  pop R31
  pop R30
  pop R25
  pop R24
  pop R23
  pop R22
  pop R19
  pop R18
  pop R17
  pop R16
  pop R7
  pop R6
  pop R5
  ret
  
  nzero:
  pop R22
  ////////////////////

  loop_send:
    ldi R18, 8                        ;                                         1619ns     New Byte loop carry time
    ld R5, Z+                         ;                                         1682ns     ...... check if R5 is used in another way by the C Program
    loop_send_byte:                   ;                                                      
      cli                             ;                                                      _____
      out PORT, R16                   ;                                           63ns  ____/
      nop                             ; 16MHz one nop = 63ns                     126ns
      nop                             ;                                          189ns
      nop                             ;                                          252ns
      ;nop                            ;                                          315ns
      sbrs R5, 7                      ; Highbit defined skips if Bit is High                  ____
      out PORT, R17                   ;           (always 2 cycles till there)   441ns            \____________
      lsl R5                          ;                                          504ns       Low = 350ns +- 150ns
      sei
      nop                             ;                                          567ns
      nop                             ;                                          640ns
      nop                             ;                                          703ns
      nop                             ;                                          866ns       ___________
      out PORT, R17                   ;                                          949ns                  \_____
      nop                             ;                                         1012ns      High = 900ns +- 150ns
      ;sei                            ;
      nop                             ;                                         1075ns
      dec R18                         ;                                         1138ns
    brne loop_send_byte               ;           (2cycles jump back  )         1264ns
    subi R22, lo8(1)                  ; 16 bit support                          1327ns
    sbci R23, hi8(1)                  ;                                         1390ns
  brne loop_send                      ;                                         1556ns     New Byte reload

  ////////////////////
  pop R31
  pop R30
  pop R25
  pop R24
  pop R23
  pop R22
  pop R19
  pop R18
  pop R17
  pop R16
  pop R7
  pop R6
  pop R5
  ;
ret
