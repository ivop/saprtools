; ym2sapr lzss player
; Play left and right LZSS stream on stereo Pokey
; by Ivo van Poorten, based on dmsc's player.
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
    prints 0, " "
    prints 0, "YM2SAPR v1.7"
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

.proc detect_2nd_pokey
    lda 20
delay
    cmp 20
    beq delay

    mva #0 SSKCTL
    mva #0 SKCTL
    mva #0 SKCTL+$10        ; make sure a potential 2nd pokey is cleared

    lda 20
delay2
    cmp 20
    beq delay2

    ; Restart SKCTL. This starts all the poly counters

    mva #3 SSKCTL
    mva #3 SKCTL

    lda 20
delay3
    cmp 20
    beq delay3

    ; Except when there's a seconds pokey!! Its counters are not restarted.
    ; Its RANDOM should not change.

    lda RANDOM+$10
    cmp RANDOM+$10
    beq detected_stereo         ; so equal means there's a 2nd pokey

detected_mono
    prints 0, "No STEREO Pokey detected!"
    jmp *

detected_stereo
    mva #3 SKCTL+$10            ; start second pokey here
    rts
.endp

songname
    ins 'songname.txt'
    .by eol

    ini $2000

; --------------------------------------------

    org $80

left_chn_copy    .ds     9
left_chn_pos     .ds     9
left_bptr        .ds     2
left_cur_pos     .ds     1
left_chn_bits    .ds     1

left_bit_data    .byte   1

.proc left_get_byte
    lda left_song_data+1
    inc left_song_ptr
    bne skip
    inc left_song_ptr+1
skip
    rts
.endp
left_song_ptr = left_get_byte + 1

right_chn_copy    .ds     9
right_chn_pos     .ds     9
right_bptr        .ds     2
right_cur_pos     .ds     1
right_chn_bits    .ds     1

right_bit_data    .byte   1

.proc right_get_byte
    lda right_song_data+1
    inc right_song_ptr
    bne skip
    inc right_song_ptr+1
skip
    rts
.endp
right_song_ptr = right_get_byte + 1


left_POKEY = $D200
right_POKEY = $D210

    org $2000
left_buffers
    .ds 256 * 9

right_buffers
    .ds 256 * 9

left_song_data
        ins     'left.lz16'
left_song_end

right_song_data
        ins     'right.lz16'
right_song_end


start

    mva #3 SKCTL
    mva #3 SKCTL+$10

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Song Initialization - this runs in the first tick:
;
.proc left_init_song
    ; Init all channels:
    ldx #8
    ldy #0
clear
    ; Read just init value and store into buffer and POKEY
    jsr left_get_byte
    sta left_POKEY, x
    sty left_chn_copy, x
cbuf
    sta left_buffers + 255
    inc cbuf + 2
    dex
    bpl clear

    ; Initialize buffer pointer:
    sty left_bptr
    sty left_cur_pos
.endp

.proc right_init_song
    ; Init all channels:
    ldx #8
    ldy #0
clear
    ; Read just init value and store into buffer and POKEY
    jsr right_get_byte
    sta right_POKEY, x
    sty right_chn_copy, x
cbuf
    sta right_buffers + 255
    inc cbuf + 2
    dex
    bpl clear

    ; Initialize buffer pointer:
    sty right_bptr
    sty right_cur_pos
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

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Play one frame of the song
;
.proc left_play_frame
    lda #>left_buffers
    sta left_bptr+1

    lda left_song_data
    sta left_chn_bits
    ldx #8

    ; Loop through all "channels", one for each POKEY register
chn_loop:
    lsr left_chn_bits
    bcs skip_chn       ; C=1 : skip this channel

    lda left_chn_copy, x    ; Get status of this stream
    bne do_copy_byte   ; If > 0 we are copying bytes

    ; We are decoding a new match/literal
    lsr left_bit_data       ; Get next bit
    bne got_bit
    jsr left_get_byte       ; Not enough bits, refill!
    ror                ; Extract a new bit and add a 1 at the high bit (from C set above)
    sta left_bit_data       ;
got_bit:
    jsr left_get_byte       ; Always read a byte, it could mean "match size/offset" or "literal byte"
    bcs store          ; Bit = 1 is "literal", bit = 0 is "match"

    sta left_chn_pos, x     ; Store in "copy pos"

    jsr left_get_byte
    sta left_chn_copy, x    ; Store in "copy length"

                        ; And start copying first byte
do_copy_byte:
    dec left_chn_copy, x     ; Decrease match length, increase match position
    inc left_chn_pos, x
    ldy left_chn_pos, x

    ; Now, read old data, jump to data store
    lda (left_bptr), y

store:
    ldy left_cur_pos
    sta left_POKEY, x        ; Store to output and buffer
    sta (left_bptr), y

skip_chn:
    ; Increment channel buffer pointer
    inc left_bptr+1

    dex
    bpl chn_loop        ; Next channel

    inc left_cur_pos
.endp

.proc right_play_frame
    lda #>right_buffers
    sta right_bptr+1

    lda right_song_data
    sta right_chn_bits
    ldx #8

    ; Loop through all "channels", one for each POKEY register
chn_loop:
    lsr right_chn_bits
    bcs skip_chn       ; C=1 : skip this channel

    lda right_chn_copy, x    ; Get status of this stream
    bne do_copy_byte   ; If > 0 we are copying bytes

    ; We are decoding a new match/literal
    lsr right_bit_data       ; Get next bit
    bne got_bit
    jsr right_get_byte       ; Not enough bits, refill!
    ror                ; Extract a new bit and add a 1 at the high bit (from C set above)
    sta right_bit_data       ;
got_bit:
    jsr right_get_byte       ; Always read a byte, it could mean "match size/offset" or "literal byte"
    bcs store          ; Bit = 1 is "literal", bit = 0 is "match"

    sta right_chn_pos, x     ; Store in "copy pos"

    jsr right_get_byte
    sta right_chn_copy, x    ; Store in "copy length"

                        ; And start copying first byte
do_copy_byte:
    dec right_chn_copy, x     ; Decrease match length, increase match position
    inc right_chn_pos, x
    ldy right_chn_pos, x

    ; Now, read old data, jump to data store
    lda (right_bptr), y

store:
    ldy right_cur_pos
    sta right_POKEY, x        ; Store to output and buffer
    sta (right_bptr), y

skip_chn:
    ; Increment channel buffer pointer
    inc right_bptr+1

    dex
    bpl chn_loop        ; Next channel

    inc right_cur_pos
.endp

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Check for ending of song and jump to the next frame
;
.proc check_end_song
    lda left_song_ptr + 1
    cmp #>left_song_end
    jne wait_frame
    lda left_song_ptr
    cmp #<left_song_end
    jne wait_frame
.endp

end_loop
    ldx #8
    lda #0
silence
    sta left_POKEY,x
    sta right_POKEY,x
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

    run start

