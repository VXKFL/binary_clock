#include <avr/io.h>

#define PORT _SFR_IO_ADDR(PORTB)
#define DDR _SFR_IO_ADDR(DDRB)
#define PIN PINB0

.text
.globl sendbyte

; NOTES: 
; R18–R27, R30, R31
;   These GPRs are call clobbered. An ordinary function may use them without 
;   restoring the contents. Interrupt service routines (ISRs) must save and 
;   restore each register they use. 
; https://gcc.gnu.org/wiki/avr-gcc#Fixed_Registers

sendbyte:
  ;cli
  ////////////////////
  in R26, PORT
  in R27, PORT
  ori R26, (1<<PIN)                   ;Bitmask set bit
  andi R27, ~(1<<PIN)                 ;Bitmask clear bit
  out DDR, R26                        ;setzt das entsprechende bit als Output !!!! Überarbeiten
  movw R30, R24                       ;setzt den Z pointer aus den Übergebenen Variablen
  
  mov R20, R22
  or R20, R23                         ;check if both Registers are zero
  breq return

  ////////////////////

  loop_send:
    ldi R18, 8                        ;                                         1619ns     New Byte loop carry time
    ld R19, Z+                        ;                                         1682ns     ...... check if R19 is used in another way by the C Program
    loop_send_byte:                   ;                                                      
      cli                             ;                                                      _____
      out PORT, R26                   ;                                           63ns  ____/
      nop                             ; 16MHz one nop = 63ns                     126ns
      nop                             ;                                          189ns
      nop                             ;                                          252ns
      ;nop                            ;                                          315ns
      sbrs R19, 7                     ; Highbit defined skips if Bit is High                  ____
      out PORT, R27                   ;           (always 2 cycles till there)   441ns            \____________
      lsl R19                         ;                                          504ns       Low = 350ns +- 150ns
      sei
      nop                             ;                                          567ns
      nop                             ;                                          640ns
      nop                             ;                                          703ns
      nop                             ;                                          866ns       ___________
      out PORT, R27                   ;                                          949ns                  \_____
      nop                             ;                                         1012ns      High = 900ns +- 150ns
      ;sei                            ;
      nop                             ;                                         1075ns
      dec R18                         ;                                         1138ns
    brne loop_send_byte               ;           (2cycles jump back  )         1264ns
    subi R22, lo8(1)                  ; 16 bit support                          1327ns
    sbci R23, hi8(1)                  ;                                         1390ns
  brne loop_send                      ;                                         1556ns     New Byte reload

  ////////////////////
return: ret
