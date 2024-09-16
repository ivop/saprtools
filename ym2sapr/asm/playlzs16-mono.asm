; XXX2sapr lzss player
; adapted for XXX2sapr by Ivo van Poorten, based on dmsc's player.
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
; The player needs 256 ($100) bytes of buffer for each pokey register stored,
; for a full mono SAP-R file this is 2304 ($900) bytes.
;

.ifndef LOWMEM
LOWMEM = $2000
.endif

    icl 'cio.s'

ROWCRS = 84
SAVMSC = 88

SSKCTL = $0232
CONSOL = $d01f
RANDOM = $d20a
VCOUNT = $d40b
WSYNC  = $d40a
SKCTL = $d20f

    org $2000

    mva #1 752      ; cursor off
    mva #0 $02c6    ; black background
    mva #15 $02c5   ; white foreground
    mva #0 82       ; left margin
    prints 0, "}"   ; clear screen
.ifdef SID2SAPR
    prints 0, "SID2SAPR v1.10 (MONO)"
.else
    prints 0, "YM2SAPR v1.9 (MONO)"
.endif
    prints 0, "Copyright (C) 2023 by Ivo van Poorten"
    prints 0, "LZSS routines (C) 2020 by DMSC"
    prints 0, " "
.ifdef HZ60
    prints 0, "Replay rate: 60Hz"
.else
    .ifdef HZ100
        prints 0, "Replay rate: 100Hz"
    .else
        prints 0, "Replay rate: 50Hz"
    .endif
.endif
    prints 0, " "
    print 0, songname
    lda #11
    sta ROWCRS
    prints 0, "Time: 00:00"

    lda #0
    sta minutes
    sta seconds
.ifdef HZ60
    lda #60
.else
    .ifdef HZ100
        lda #100
    .else
        lda #50
    .endif
.endif
    sta frames
    inc SAVMSC+1
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

minutes .ds 1
seconds .ds 1
frames  .ds 1

POKEY = $D200

    org LOWMEM
buffers
    .ds 256 * 9

song_data
.ifdef SID2SAPR
        ins     'output.lz16'
.else
        ins     'left.lz16'
.endif
song_end


start

    mva #3 SSKCTL
    mva #3 SKCTL

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
    sta POKEY, x
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
    dec frames
    bne no_clock

    jsr do_clock

no_clock
.endp

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
    sta POKEY, x        ; Store to output and buffer
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
    .ifdef HZ100
        dta 86, 8, 86, 8, 86, 8
    .else
        dta 8, 8, 8, 8, 8, 8
    .endif
.endif
wait_table_len = * - wait_table

; ----------------------------------------------------------------------------

    .proc do_clock
.ifdef HZ60
    lda #60
.else
    .ifdef HZ100
        lda #100
    .else
        lda #50
    .endif
.endif
    sta frames

    sed
    lda seconds
    clc
    adc #1
    sta seconds
    cmp #$60
    bne no_minutes

    lda #0
    sta seconds
    lda minutes
    clc
    adc #1
    sta minutes

no_minutes
    cld

    ldy #190
    lda minutes
    lsr
    lsr
    lsr
    lsr
    ora #$10
    sta (SAVMSC),y
    lda minutes
    and #$0f
    ora #$10
    iny
    sta (SAVMSC),y
    lda seconds
    lsr
    lsr
    lsr
    lsr
    ora #$10
    iny
    iny
    sta (SAVMSC),y
    lda seconds
    and #$0f
    ora #$10
    iny
    sta (SAVMSC),y
    rts
    .endp

; ----------------------------------------------------------------------------
    run start

