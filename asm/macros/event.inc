@ The Pokémon Gen 3 field script commands used by the script the cartridge
@ sends to the games (data/event_script.s), named as in pret's pokeemerald and
@ pokefirered.

	.macro end
	.byte 0x02
	.endm

	.macro callstd function:req
	.byte 0x09, \function
	.endm

	.macro setorcopyvar destvar:req, source:req
	.byte 0x1A
	.2byte \destvar, \source
	.endm

	.macro compare_var_to_value var:req, value:req
	.byte 0x21
	.2byte \var, \value
	.endm

	.macro setflag flag:req
	.byte 0x29
	.2byte \flag
	.endm

	.macro checkflag flag:req
	.byte 0x2B
	.2byte \flag
	.endm

	.macro checkitemspace item:req, quantity:req
	.byte 0x46
	.2byte \item, \quantity
	.endm

	.macro checkitem item:req, quantity:req
	.byte 0x47
	.2byte \item, \quantity
	.endm

	.macro faceplayer
	.byte 0x5A
	.endm

	.macro waitmessage
	.byte 0x66
	.endm

	.macro lock
	.byte 0x6A
	.endm

	.macro release
	.byte 0x6C
	.endm

	.macro waitbuttonpress
	.byte 0x6D
	.endm

	@ The script runs from a buffer, so its jumps and messages are given as
	@ addresses relative to the one set by setvaddress (its own).
	.macro setvaddress addr:req
	.byte 0xB8
	.4byte \addr
	.endm

	.macro vgoto_if condition:req, dest:req
	.byte 0xBB, \condition
	.4byte \dest
	.endm

	.macro vmessage text:req
	.byte 0xBD
	.4byte \text
	.endm

@ Conditions: compare_var_to_value sets the result to LT, EQ or GT, and
@ checkflag to TRUE or FALSE.
	.set LT, 0
	.set EQ, 1
	.set GT, 2
	.set FALSE, 0
	.set TRUE, 1

@ Variables
	.set VAR_0x8000, 0x8000
	.set VAR_0x8001, 0x8001
	.set VAR_RESULT, 0x800D

@ callstd functions
	.set STD_OBTAIN_ITEM, 0

@ Items
	.set ITEM_AURORA_TICKET, 371

@ Flags (pokefirered's)
	.set FLAG_RECEIVED_AURORA_TICKET, 0x2A7
	.set FLAG_FOUGHT_DEOXYS, 0x2E4
	.set FLAG_ENABLE_SHIP_BIRTH_ISLAND, 0x84B
