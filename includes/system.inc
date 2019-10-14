; This file is for system related macros and functions.

!macro basic_sys {	
	!byte $0b,$08,$01,$00,$9e,$32,$30,$36,$31,$00,$00,$00
	; This is basically "1 SYS2061"
	; Begin with "*=$0801" in front of this
}

; This is to ensure the functions in this system go to
; a good place in RAM. Be sure that nothing else 
; overwrites this.
*=$8000

bound: !byte $00, $00
brandom: !byte $00, $00
; this bounds the rng value to bound
; assumes bound < 512 which it is for our graph demo purposes
getrandom_bounded:
-	jsr getrandom
	lda random
	sta brandom
	lda random+1
	and #$01 ; we only ever need the first bit of the higher byte
	         ; as we know what we're doing
	sta brandom+1

	lda bound+1
	cmp brandom+1
	bcc - ; upper byte of bound was less than random -> random too big, again
	beq + ; upper byte of bound was the same as random -> needs to compare lower byte
	jmp ++ ; upper byte of bound was more than random -> all okay, go away
+	lda bound
	cmp brandom
	bcc - ; lower byte of bound was more than random -> random too big, again
++	rts   ; all okay, go back, bounded in brandom and brandom+1

; the rng code from 
; https://codebase64.org/doku.php?id=base:two_very_fast_16bit_pseudo_random_generators_as_lfsr
;© 2007 Hanno Behrens (pebbles@schattenlauf.de)
;LGPL Licence

getrandom:
		jsr rand64k       ;Factors of 65535: 3 5 17 257
		jsr rand32k       ;Factors of 32767: 7 31 151 are independent and can be combined
		lda sr1+1         ;can be left out 
		eor sr2+1         ;if you dont use
		tay               ;y as suggested
		lda sr1           ;mix up lowbytes of SR1
		eor sr2           ;and SR2 to combine both 
		sta random
		tya
		sta random+1
		rts
 
;periode with 65535
;10+12+13+15
rand64k:
		lda sr1+1
		asl
		asl
		eor sr1+1
		asl
		eor sr1+1
		asl
		asl
		eor sr1+1
		asl
		rol sr1         ;shift this left, "random" bit comes from low
		rol sr1+1
		rts
 
;periode with 32767
;13+14
rand32k:
		lda sr2+1
		asl
		eor sr2+1
		asl
		asl
		ror sr2         ;shift this right, random bit comes from high - nicer when eor with sr1
		rol sr2+1
		rts
 
;feel free to set seeds as wished, if put in zeropage some speed-boost is 
;the result. For example sr1=$5c sr2=5e would fit
sr1:	!word $a55a
sr2:	!word $7653
random: !byte $00, $00