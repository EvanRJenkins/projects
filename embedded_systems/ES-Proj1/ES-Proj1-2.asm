;-------------------------------------------------------------------------------
	.cdecls C,LIST,"msp430g2553.h" 			; Include device header file
	.def RESET
;-------------------------------------------------------------------------------
	.text 								    ; Assemble into program memory
	.retain 							    ; Override ELF conditional linking
										    ; and retain current section
	.retainrefs 						    ; Additionally retain any sections
										    ; that have references to current
											; section
;-------------------------------------------------------------------------------
RESET 		mov.w  #0280h,SP 			    ; Initialize stack pointer
StopWDT 	mov.w  #WDTPW+WDTHOLD,&WDTCTL 	; Turn off watchdog timer
SetupP1 	bis.b  #0x41,&P1DIR 			; Set DDR to output for 1.0 and 1.6
	
Init 		xor.b  #001h,&P1OUT 		    ; Toggle 1.0
	 		bic.w  #0xFFFF,R15 			    ; Clear 1.0 delay counter 		       

Mainloop 	inc.w  R15 					    ; ++ 1.0 delay counter
			cmp.w  #0xFFFF,R15				; Compare 1.0 delay counter to max
			jz     Init						; Jump to Init if 1.0 counter full
			inc.w  R14 					    ; Otherwise ++ 1.6 delay counter
			cmp.w  #0xFFFF,R14				; Compare 1.6 delay counter to max
			jz     LED2  					; Jump to LED2 if 1.6 counter full
			jmp    Mainloop					; Otherwise repeat ++ cycle

LED2        xor.b  #0x40,&P1OUT 		 	; Toggle 1.6
			inv.b  R14
			mov.w  #0x7000,R14 			    ; Reset 1.5 delay counter  
			jmp    Mainloop 		        ; Repeat ++ cycle
;-------------------------------------------------------------------------------
; Stack Pointer definition
;-------------------------------------------------------------------------------
	.global __STACK_END
	.sect .stack
;-------------------------------------------------------------------------------
; Interrupt Vectors
;-------------------------------------------------------------------------------
	.sect ".reset" 				                 ; MSP430 RESET Vector
	.short RESET


	.end
