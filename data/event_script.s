@ The script the card server sends to FireRed and LeafGreen (sMgScripts):
@ the deliveryman at the POKéMON CENTER gives the player the AURORA TICKET.

	.include "asm/macros/event.inc"

	.section .rodata

	.global sAuroraTicketScript
sAuroraTicketScript:
	setvaddress sAuroraTicketScript
	lock
	faceplayer
	checkflag FLAG_RECEIVED_AURORA_TICKET
	vgoto_if TRUE, AuroraTicketScript_AlreadyReceived
	checkflag FLAG_FOUGHT_DEOXYS
	vgoto_if TRUE, AuroraTicketScript_AlreadyReceived
	checkitem ITEM_AURORA_TICKET, 1
	compare_var_to_value VAR_RESULT, TRUE
	vgoto_if EQ, AuroraTicketScript_AlreadyReceived
	vmessage AuroraTicketScript_Text_TicketForYou
	waitmessage
	waitbuttonpress
	checkitemspace ITEM_AURORA_TICKET, 1
	compare_var_to_value VAR_RESULT, FALSE
	vgoto_if EQ, AuroraTicketScript_NoRoom
	setorcopyvar VAR_0x8000, ITEM_AURORA_TICKET
	setorcopyvar VAR_0x8001, 1
	callstd STD_OBTAIN_ITEM
	setflag FLAG_ENABLE_SHIP_BIRTH_ISLAND
	setflag FLAG_RECEIVED_AURORA_TICKET
	vmessage AuroraTicketScript_Text_VermilionPort
	waitmessage
	waitbuttonpress
	release
	end

AuroraTicketScript_NoRoom:
	vmessage AuroraTicketScript_Text_KeyItemsPocketFull
	waitmessage
	waitbuttonpress
	release
	end

AuroraTicketScript_AlreadyReceived:
	vmessage AuroraTicketScript_Text_ThankYou
	waitmessage
	waitbuttonpress
	release
	end

AuroraTicketScript_Text_TicketForYou:
	.string "Thank you for using the MYSTERY\nGIFT System.\pYou must be {PLAYER}.\nThere is a ticket here for you.$"

AuroraTicketScript_Text_VermilionPort:
	.string "It appears to be for use at the\nVERMILION CITY port.\pWhy not give it a try and see what\nit is about?$"

AuroraTicketScript_Text_ThankYou:
	.string "Thank you for using the MYSTERY\nGIFT System.$"

AuroraTicketScript_Text_KeyItemsPocketFull:
	.string "Oh, I’m sorry, {PLAYER}. Your BAG’s\nKEY ITEMS POCKET is full.\pPlease store something on your PC,\nthen come back for this.$"
