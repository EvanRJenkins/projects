;-------------------------------------------------------------------------------
	.cdecls C,LIST,"msp430g2553.h" 			      ; Include device header file
	.def RESET
;-------------------------------------------------------------------------------
	.text 								                    ; Assemble into program memory
	.retain 							                    ; Override ELF conditional linking
										                        ; and retain current section
	.retainrefs 						                  ; Additionally retain any sections
										                        ; that have references to current
											                      ; section
;-------------------------------------------------------------------------------
RESET 		mov.w  #0x00280,SP 			          ; Modes: Immediate and Register
StopWDT 	mov.w  #WDTPW+WDTHOLD,&WDTCTL   	; Modes: Immediate and Absolute
SetupP1 	bis.b  #0x41,&P1DIR 			        ; Modes: Immediate and Absolute

Init 		xor.b  #001h,&P1OUT 		            ; Modes: Immediate and Absolute
	 		bic.w  #0xFFFF,1(R14) 		      	    ; Modes: Immediate and Indexed	
			mov.w   #Test,R12                     ; Modes: Immediate and Register
            mov.w   @R12+,R13               ; Mode:  Indirect autoincrement
            mov.w   @R12,R14                ; Mode:  Indirect	       

Mainloop 	inc.w  R15 					              ; 
			cmp.w  #0xFFFF,R15				            ; Modes: Immediate and Register
			jz     Init						                ; 
			inc.w  R14 					                  ; 
			cmp.w  #0xFFFF,R14				            ; 
			jz     LED2  					                ; Mode:  Symbolic
			jmp    Mainloop					              ; Mode:  Symbolic

LED2        xor.b  #0x40,&P1OUT 		 	      ; Modes: Immediate and Absolute
			inv.b  R14
			mov.w  #0x7000,R14 			              ; Modes: Immediate and Register
			jmp    Mainloop 		                  ; Mode:  Symbolic
;
Test:      .word   0x1234, 0xABCD           ; Test nums for using all modes
;
;-------------------------------------------------------------------------------
; Stack Pointer definition
;-------------------------------------------------------------------------------
	.global __STACK_END
	.sect .stack
;-------------------------------------------------------------------------------
; Interrupt Vectors
;-------------------------------------------------------------------------------
	.sect ".reset" 				                   ; MSP430 RESET Vector
	.short RESET


	.end
