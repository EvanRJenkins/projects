;-------------------------------------------------------------------------------
	.cdecls C,LIST,"msp430g2553.h" 			  ; Include device header file
	.def RESET
;-------------------------------------------------------------------------------
	.text 								              	; Assemble into program memory
	.retain 							              	; Override ELF conditional linking
										                  	; and retain current section
	.retainrefs 						            	; Additionally retain any sections
										                  	; that have references to current
											                  ; section
;-------------------------------------------------------------------------------
RESET 		mov.w #0280h,SP 			      	; Initialize stackpointer
StopWDT 	mov.w #WDTPW+WDTHOLD,&WDTCTL 	; Stop WDT
SetupP1 	bis.b #0x41,&P1DIR 			  	  ; P1.0 and P1.6 output
	
Mainloop 	xor.b #001h,&P1OUT 			    	; Toggle P1.0
Wait 		mov.w #065500,R15 			        ; Delay to R15
L1 			dec.w R15 					          	; Decrement R15
			jnz 		L1 					              ; Delay over?
            xor.b #0x40,&P1OUT 		  		; Toggle P1.0
			jmp 		Mainloop 		             	; Again
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
