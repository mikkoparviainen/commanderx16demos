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

; uses the same x and y as the pixel draw
; length of the line - has to be two bytes
; as for now, no check on going over the end of the row
hline_len: !byte $00, $00
hline:
	pha ; save a, x and y registers
	txa
	pha
	tya
	pha
	+PIXPOS x, y

	lda pixlo
	sta veralo
	lda pixmid
	sta veramid
	lda pixhi
	ora #$10 ; set the increment to 1
	sta verahi ; veralo:veramid:verahi = pos of pixel, inc=1

	lda colour
	ldx hline_len+1 ; get the high byte of the loop to x register
	ldy hline_len ; get the low byte of the loop to y register

-	sta veradat
	dey
	bne -
	cpx #00
	beq + ; high byte of counter is zero, so we're done
	dex   ; not done, so decrease x...
	jmp - ; and start again. y register is zero and
	      ; gets decreased in the loop so runs again

+	pla
	tay
	pla
	tax
	pla
	rts


; The line drawing routine.
; Inputs: colour, x0, y0, x1, y1
; most of them on zero page to be fast, all are two bytes
; See also https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm
; The last one is the one used here so the least jumps
x0=$10
y0=$12
x1=$14
y1=$16
; temporary positions in the zero page
dx=$18
dy=$1A
D=$1C
step=$001E ; steps are +1 or -1 so one byte is enough
line:
	pha ; save a, x and y registers
	txa
	pha
	tya
	pha

	jmp line_low

;assumes: abs(y1 - y0) < abs(x1 - x0) && x0 < x1
line_low:
	sec
	lda x1
	sbc x0
	sta dx
	lda x1+1
	sbc x0+1
	sta dx+1 ; dx = x1 - x0

	sec
	lda y1
	sbc y0
	sta dy
	lda y1+1
	sbc y0+1
	sta dy+1 ; dy = y1 - y0

	lda #01
	sta step

	lda #0
	cmp dy+1
	bmi ++ ; if dy > 0 we're okay
	lda #$FF ; dy < 0 -> switch
	sta step ; step = -1
	lda dy+1
	eor #$FF
	sta dy+1
	lda dy
	eor #$FF
	clc
	adc #$01
	sta dy
	lda dy+1
	adc #$00
	sta dy+1 ; dy = -dy
++  lda dy
	asl
	sta D
	lda dy+1
	rol
	sta D+1 ; D = dy*2

	sec
	lda D
	sbc dx
	sta D
	lda D+1
	sbc dx+1
	sta D+1 ; D = dy*2 - dx

	lda y0
	sta y
	lda y0+1
	sta y+1 ; y = y0

	lda x0
	sta x
	lda x0+1
	sta x+1 ; x = x0

	asl dx
	rol dx+1 ; dx = 2*dx

	asl dy
	rol dy+1 ; dy = 2*dy

-	lda x+1
	cmp x1+1
	bne ++ ;high bytes differ, continue
	lda x ; high bytes are the same
	cmp x1 
	beq + ; low bytes are also the same -> we're done

++	clc
	adc #$01
	sta x
	lda x+1
	adc #00 
	sta x+1 ; x += 1

	jsr draw_pixel

	lda #0
	cmp D+1
	bpl ++ ; 0 > D
	clc
	lda y
	adc step
	sta y
	lda y+1
	adc #$00
	sta y+1 ; y = y+step

	lda D
	sec
	sbc dx
	sta D
	lda D+1
	sbc dx+1
	sta D+1 ; D = D-2*dx

++	lda D
	clc
	adc dy
	sta D
	lda D+1
	adc dy+1
	sta D+1 ; D = D + 2*dy

	jmp - ; and loop

+	pla
	tay
	pla
	tax
	pla
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