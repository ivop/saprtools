; sid2sapr lzss player
; adapted for sid2sapr by Ivo van Poorten, based on dmsc's player.
; added 3 channel softsynth for the lower octaves
;
; LZSS Compressed SAP player for 16 match bits
; --------------------------------------------
;
; (c) 2020 DMSC
; Code under MIT license, see LICENSE file.
;
; This player uses:
;  Match length: 8 bits  (1 to 256)
;  Match offset: 8 bits  (1 to 256)
;  Min length: 2
;  Total match bits: 16 bits
;
; Compress using:
;  lzss -b 16 -o 8 -m 1 input.rsap test.lz16
;
; Assemble this file with MADS assembler, the compressed song is expected in
; the `test.lz16` file at assembly time.
;
; The plater needs 256 bytes of buffer for each pokey register stored, for a
; full SAP file this is 2304 bytes.
;

    icl 'cio.s'

AUDF1  = $d200
AUDC1  = $d201
AUDF2  = $d202
AUDC2  = $d203
AUDF3  = $d204
AUDC3  = $d205
AUDF4  = $d206
AUDC4  = $d207
AUDCTL = $d208
STIMER = $d209

POKMSK = $10
IRQEN  = $d20e
IRQST  = $d20e

ACK1    = %110
ACK2    = %101
ACK4    = %011
DIS1    = ACK1
DIS2    = ACK2
DIS4    = ACK4
ENA1    = %001
ENA2    = %010
ENA4    = %100
ENAALL  = %111

VIMIRQ  EQU $0216   ;2-byte immediate IRQ vector

SSKCTL = $0232
CONSOL = $d01f
RANDOM = $d20a
VCOUNT = $d40b
WSYNC  = $d40a
SKCTL  = $d20f

    org $2000

    mva #1 752      ; cursor off
    mva #0 $02c6    ; black background
    mva #15 $02c5   ; white foreground
    mva #0 82       ; left margin
    prints 0, " "
    prints 0, "SID2SAPR v1.3 (MONO) [SOFTBASS]"
    prints 0, "Copyright (C) 2023 by Ivo van Poorten"
    prints 0, "LZSS routines (C) 2020 by DMSC"
    prints 0, " "
.ifdef HZ60
    prints 0, "Replay rate: 60Hz"
.else
    prints 0, "Replay rate: 50Hz"
.endif
    prints 0, " "
    print 0, songname
    rts

songname
    ins 'songname.txt'
    .by eol

    ini $2000

; --------------------------------------------

    org $80

chn_copy    .ds     9
chn_pos     .ds     9
bptr        .ds     2
cur_pos     .ds     1
chn_bits    .ds     1

bit_data    .byte   1

.proc get_byte
    lda song_data+1
    inc song_ptr
    bne skip
    inc song_ptr+1
skip
    rts
.endp
song_ptr = get_byte + 1


POKEY = $D200

    org $2000
buffers
    .ds 256 * 9

song_data
        ins     'output.lz16'
song_end


start
    mva #3 SSKCTL
    mva #3 SKCTL

    lda #0
    sta POKMSK
    sta IRQEN

; use OS vector. if more speed is needed (game/demo) you can disable the ROM
; and takeover the CPU IRQ vector directly.

    mwa #IRQ VIMIRQ

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Song Initialization - this runs in the first tick:
;
.proc init_song

    ; Init all channels:
    ldx #8
    ldy #0
clear
    ; Read just init value and store into buffer and POKEY
    jsr get_byte
    sta SHADOW, x
    sty chn_copy, x
cbuf
    sta buffers + 255
    inc cbuf + 2
    dex
    bpl clear

    ; Initialize buffer pointer:
    sty bptr
    sty cur_pos
.endp

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Wait for next frame
;
.proc wait_frame
    lda #0
    sta $d01a

wait_counter=*+1
    ldx #wait_table_len-1

delay
    lda VCOUNT
    cmp wait_table,x
    bne delay

    dec wait_counter
    bpl no_loop

    lda #wait_table_len-1
    sta wait_counter

no_loop
    lda CONSOL
    cmp #6              ; START key
    bne dont_show

    lda #7
    sta $d01a

dont_show
.endp

    jsr copy_shadow_registers

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Play one frame of the song
;
.proc play_frame
    lda #>buffers
    sta bptr+1

    lda song_data
    sta chn_bits
    ldx #8

    ; Loop through all "channels", one for each POKEY register
chn_loop:
    lsr chn_bits
    bcs skip_chn       ; C=1 : skip this channel

    lda chn_copy, x    ; Get status of this stream
    bne do_copy_byte   ; If > 0 we are copying bytes

    ; We are decoding a new match/literal
    lsr bit_data       ; Get next bit
    bne got_bit
    jsr get_byte       ; Not enough bits, refill!
    ror                ; Extract a new bit and add a 1 at the high bit (from C set above)
    sta bit_data       ;
got_bit:
    jsr get_byte       ; Always read a byte, it could mean "match size/offset" or "literal byte"
    bcs store          ; Bit = 1 is "literal", bit = 0 is "match"

    sta chn_pos, x     ; Store in "copy pos"

    jsr get_byte
    sta chn_copy, x    ; Store in "copy length"

                        ; And start copying first byte
do_copy_byte:
    dec chn_copy, x     ; Decrease match length, increase match position
    inc chn_pos, x
    ldy chn_pos, x

    ; Now, read old data, jump to data store
    lda (bptr), y

store:
    ldy cur_pos
    sta SHADOW, x        ; Store to output and buffer
    sta (bptr), y

skip_chn:
    ; Increment channel buffer pointer
    inc bptr+1

    dex
    bpl chn_loop        ; Next channel

    inc cur_pos
.endp

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Check for ending of song and jump to the next frame
;
.proc check_end_song
    lda song_ptr + 1
    cmp #>song_end
    bne wait_frame
    lda song_ptr
    cmp #<song_end
    bne wait_frame
.endp

end_loop
    ldx #8
    lda #0
    sta POKMSK
    sta IRQEN
silence
    sta POKEY,x
    dex
    bpl silence

    jmp *

; read in reverse!
; HZ60: NTSC is actually 50 scanline shorter, which is 25 steps of VCOUNT
; here we use a stepsize of 26 because 6*26 = 156 (which is a full PAL frame)
; it's close enough ;)

wait_table
.ifdef HZ60
    dta 0, 26, 52, 78, 104, 130
.else
    dta 8, 8, 8, 8, 8, 8
.endif
wait_table_len = * - wait_table

; shadow pokey registers

SAUDF1  = SHADOW+0
SAUDC1  = SHADOW+1
SAUDF2  = SHADOW+2
SAUDC2  = SHADOW+3
SAUDF3  = SHADOW+4
SAUDC3  = SHADOW+5
SAUDF4  = SHADOW+6
SAUDC4  = SHADOW+7
SAUDCTL = SHADOW+8

SHADOW
:9  dta 0

; ----------------------------------------------------------------------------
; SOFTBASS

timer .macro num

    .def timr:num = *
    lda #ACK:num
    sta IRQEN
    lda POKMSK
    sta IRQEN
val:num = *+1
    lda wave:num
    sta AUDC:num
    dec val:num
    bpl nope:num
    .def oct:num = *+1
    lda #3
    sta val:num
nope:num
    pla
    rti
.endm

IRQ
    pha

    lda IRQST
    lsr
    bcc timr1
    lsr
    bcc timr2

    timer 4
    timer 1
    timer 2

; ----------------------------------------

; play four bytes -> one octave lower       audc = $3x
; play eight bytes -> two octaves lower     audc = $7x

; if (round(divider) > 255)
;   if (round(divider/2) <= 255)    ; 128-255
;       audc = $3x
;       audf = round(divider/2)
;   if (round(divider/4) <= 255)    ; again 128-255
;       audc = $7x
;       audf = round(divider/4)
;   else
;       audc = $7x
;       audf = $ff

; set volume:
; set wave+0,+1,+6,+7 to $3x or $7x from sap-r file (both are volume only)
;
; 4x lsr, store oct1, oct2 or oct4
; sets oct1, oct2 or oct4 to either 3 or 7

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Copy shadow registers to real Pokey and enable or
; disable timer interrupts
;
.proc copy_shadow_registers
    mva SAUDCTL AUDCTL
    mva SAUDC3  AUDC3
    mva SAUDF3  AUDF3

handle_channel .macro num
    lda SAUDC:num
    tay
    and #$10            ; volume only
    beq normal

    tya
    sta 0+wave:num
    sta 1+wave:num
    sta 6+wave:num
    sta 7+wave:num

    lsr
    lsr
    lsr
    lsr
    sta oct:num

    lda POKMSK
    ora #ENA:num
    sta POKMSK
    sta IRQEN
    bne done

normal
    lda POKMSK
    and #DIS:num
    sta POKMSK
    sta IRQEN
    tya
    sta AUDC:num
done
    mva SAUDF:num AUDF:num
    .endm

    handle_channel 1
    handle_channel 2
    handle_channel 4

    rts
.endp

; page aligned, as the waves are played in reverse order and end is detected
; with the sign. count from 7 to 0, after which -1 sets the sign bit
; the wave is phase shifted so the whole wave or just the first half can
; both be played. full wave is two octaves down, half wave is one octave down

    .align $100
wave1
    dta $1c, $1c, $10, $10, $10, $10, $1c, $1c

    .align $100
wave2
    dta $1c, $1c, $10, $10, $10, $10, $1c, $1c

    .align $100
wave4
    dta $1c, $1c, $10, $10, $10, $10, $1c, $1c


; ----------------------------------------------------------------------------

    run start

