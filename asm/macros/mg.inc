@ Mystery Gift data: the server's and the games' (client) script commands,
@ WONDER CARDs and WONDER NEWS. Named as in pret's pokeemerald
@ (mystery_gift_server.h, mystery_gift_client.h, global.h).

@ A server command (MysteryGiftServer_Run in src/mg.c).
	.macro server_cmd instr:req, parameter=0, ptr=0
	.4byte \instr, \parameter, \ptr
	.endm

@ A command for the game to run.
	.macro client_cmd instr:req, parameter=0
	.4byte \instr, \parameter
	.endm

@ Followed by the title, subtitle, four lines of body text and two of footer,
@ 40 bytes each.
	.macro wonder_card flagId:req, iconSpecies:req, idNumber:req, type:req, bgType:req, sendType:req, maxStamps:req
	.2byte \flagId, \iconSpecies
	.4byte \idNumber
	.byte \type | (\bgType << 2) | (\sendType << 6), \maxStamps
	.endm

@ Followed by the title and ten lines of body text, 40 bytes each.
	.macro wonder_news id:req, sendType:req, bgType:req
	.2byte \id
	.byte \sendType, \bgType
	.endm

@ Server commands
	.set SVR_RETURN, 0
	.set SVR_SEND, 1
	.set SVR_RECV, 2
	.set SVR_GOTO, 3
	.set SVR_GOTO_IF_EQ, 4
	.set SVR_COPY_GAME_DATA, 5
	.set SVR_CHECK_GAME_DATA, 6
	.set SVR_CHECK_EXISTING_CARD, 7
	.set SVR_READ_RESPONSE, 8
	.set SVR_CHECK_EXISTING_STAMPS, 9
	.set SVR_GET_CARD_STAT, 10
	.set SVR_CHECK_QUESTIONNAIRE, 11
	.set SVR_COMPARE, 12
	.set SVR_LOAD_CARD, 13
	.set SVR_LOAD_NEWS, 14
	.set SVR_LOAD_RAM_SCRIPT, 15
	.set SVR_LOAD_STAMP, 16
	.set SVR_LOAD_UNK_2, 17
	.set SVR_LOAD_CLIENT_SCRIPT, 18
	.set SVR_LOAD_EREADER_TRAINER, 19
	.set SVR_LOAD_MSG, 20
	.set SVR_COPY_STAMP, 21
	.set SVR_COPY_CARD, 22
	.set SVR_COPY_NEWS, 23
	.set SVR_SET_RAM_SCRIPT, 24
	.set SVR_SET_CLIENT_SCRIPT, 25
	@ This cartridge's own
	.set SVR_LOAD_UNK_1, 29         @ sends MG_LINKID_UNK_1
	.set SVR_FIND_NEWS, 30          @ finds the current WONDER NEWS for the game
	.set SVR_COPY_FOUND_NEWS, 31    @ copies it to send
	.set SVR_NOP, 32

@ Client commands (the ones used here)
	.set CLI_RETURN, 1
	.set CLI_RECV, 2
	.set CLI_SEND_LOADED, 3
	.set CLI_COPY_RECV, 4
	.set CLI_LOAD_GAME_DATA, 8
	.set CLI_SAVE_NEWS, 9
	.set CLI_SAVE_CARD, 10
	.set CLI_COPY_MSG, 12
	.set CLI_ASK_TOSS, 13
	.set CLI_LOAD_TOSS_RESPONSE, 14
	.set CLI_SAVE_RAM_SCRIPT, 17
	.set CLI_SEND_READY_END, 20

@ What a link transfer carries
	.set MG_LINKID_CLIENT_SCRIPT, 0x10
	.set MG_LINKID_GAME_DATA, 0x11
	.set MG_LINKID_GAME_STAT, 0x12
	.set MG_LINKID_RESPONSE, 0x13
	.set MG_LINKID_READY_END, 0x14
	.set MG_LINKID_DYNAMIC_MSG, 0x15
	.set MG_LINKID_CARD, 0x16
	.set MG_LINKID_NEWS, 0x17
	.set MG_LINKID_STAMP, 0x18
	.set MG_LINKID_RAM_SCRIPT, 0x19
	.set MG_LINKID_EREADER_TRAINER, 0x1A
	.set MG_LINKID_UNK_1, 0x1B
	.set MG_LINKID_UNK_2, 0x1C
