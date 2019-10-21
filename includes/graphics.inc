; graphics.inc

verareg=$9f20
veralo  = verareg+0
veramid = verareg+1
verahi  = verareg+2
veradat = verareg+3
veradat2= verareg+4
veractl = verareg+5
veraien = verareg+6
veraisr = verareg+7

vreg_cmp  = $F0000
vreg_pal  = $F1000
vreg_lay1 = $F2000
vreg_lay2 = $F3000
vreg_spr  = $F4000
vreg_sprd = $F5000

vera_reset = $FF81

pixlo = $FB
pixmid = pixlo+1
pixhi = pixmid+1

; This is to ensure the functions in this system go to
; a good place in RAM. Be sure that nothing else 
; overwrites this.
*=$7000

; This macro calculates the pixel position
; given in .x and .y and inserts it to pixlo-pixmid-pixhi
!macro PIXPOS .x, .y {
	; assumes that .y < 512 .x < 512
	lda #$00
	sta pixhi
	sta pixmid
	sta pixlo ; empty working area

	lda .y
	sta pixmid
	lda .y+1
	sta pixhi ; pixlo-pixmid-pixhi = 256 * .y because this is shifting by 8 bits

	lda .y
	asl
	asl
	asl
	asl
	asl
	asl ; shift left six times to get the lower byte of 64*.y
	sta pixlo ; pixlo = 64 * .y
	
	lda .y
	lsr
	lsr ; shift right two times to get the upper byte of 64*.y
	clc
	adc pixmid
	sta pixmid ; add that to the mid byte
	lda #0
	adc pixhi ; add possible carry to upper byte
	sta pixhi ; pixmid:pixhi += 64 * .y

	lda .y+1
	asl
	asl
	asl
	asl
	asl
	asl ; shift left six times to get the lower byte of the shifted upper byte
	clc
	adc pixmid
	lda #0    ; here we assume the .y+1 byte is either #$00 or #$01 because 
	adc pixhi ; pixel coordinates are less than 320 (or 240) anyway
	          ; so no bits to add to the pixhi

	; now pixlo:pixmid:pixhi = 320 * .y

	lda .x
	clc
	adc pixlo
	sta pixlo
	lda .x+1
	adc pixmid
	sta pixmid
	lda #0
	adc pixhi
	sta pixhi  ; pixlo:pixmid:pixhi += .x
}

; parameters for the pixel - faster to do it this way
x: !byte $00, $00
y: !byte $00, $00

draw_pixel:
	+PIXPOS x, y

	lda pixlo
	sta veralo
	lda pixmid
	sta veramid
	lda pixhi
	sta verahi ; veralo:veramid:verahi = pos of pixel, inc=0

	lda colour
	sta veradat ; print pixel

	rts

vera_init_320x240_8bpp:
	 lda #$0
	 sta veramid
	 lda #$0F
	 sta verahi ; $F0000 
	 lda #$01
	 sta veralo ; $F0001
	 lda #64 ; HSCALE = 64 -> 2 pixels per write
	 sta veradat
	 lda #$02 ; $F0002
	 sta veralo
	 lda #64 
	 sta veradat ; VSCALE = 64 -> 2 pixels per write

	 lda #$0
	 sta veralo
	 lda #$20
	 sta veramid
	 lda #$0F 
	 sta verahi ; $F2000, increment 0 - layer 0 register 0
	 lda #$E1 ; mode bitmap 8bpp, enabled 
	 sta veradat ; set mode
	 lda #$01 
	 sta veralo ; $F2001
	 lda #$00
	 sta veradat ; TILEW=0  -> 320 px
	 lda #$04
	 sta veralo
	 lda #0
	 sta veradat ; TILEBASE_LO = 0
	 lda #$05
	 sta veralo
	 lda #0
	 sta veradat ; TILEBASE_HI = 0

	 rts

clear_screen:
	lda #$0
	sta veralo
	sta veramid
	lda #$10
	sta verahi ; $00000, increment 1
	lda colour
	ldy #$FF
--	ldx #$F0
-	sta veradat
	dex
	bne -
	dey
    beq +
    jmp --
+   ldy #65
--	ldx #$F0
-	sta veradat
	dex
	bne -
	dey
    beq +
    jmp --

+	rts

colour: !byte $00