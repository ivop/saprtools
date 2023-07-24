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
SKCTL = $d20f

    org $2000
left_buffers
    .ds 256 * 9

left_song_data
        ins     'left.lz16'
left_song_end

right_buffers
    .ds 256 * 9

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

    lda 20
delay
    cmp 20
    beq delay
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
    jmp *

    run start

