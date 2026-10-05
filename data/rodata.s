@ The cartridge's read-only data, after the script it sends to the games
@ (data/event_script.s) and before the graphics (data/graphics.s).

	.include "asm/macros/mg.inc"

	.section .rodata

	.global sDistributionType
sDistributionType:
	.byte 2
	.align 1, 0

@ The serial numbers of the games this cartridge accepts, ending in 0xFFFF;
@ the save code uses the same list to find each game's save block.
	.global sAcceptedSerialNos
sAcceptedSerialNos:
	.global sSaveBlockIds
sSaveBlockIds:
	.2byte 0x0002, 0xFFFF
	.space 0x1e

	.global sRfuConnectCallbacks
sRfuConnectCallbacks:
	.4byte StartDistribution
	.space 15 * 4

	.global sSaveBlockSizes
sSaveBlockSizes:
	.4byte 0

	.global sExpiryDate
sExpiryDate:
	.byte 0x00, 0x01, 0x01, 0x00

	.global sSaveMagic
sSaveMagic:
	.ascii "AURO"
	.space 0x4

@ The font of the debug and setup screens. Its tiles are the characters' codes:
@ ASCII, with Japanese kana in place of some symbols and above 0x80.
	.global sDebugGfx
sDebugGfx:
	.incbin "graphics/debug/tiles.4bpp"
	.space 0x8

	.global sDebugBgPalette
sDebugBgPalette:
	.incbin "graphics/debug/palette.gbapal"

	.global sDebugObjPalette
sDebugObjPalette:
	.incbin "graphics/debug/obj_palette.gbapal"

	.global sText_EmptyRankingRow
sText_EmptyRankingRow:
	.string "----- -----   -"
	.align 2, 0

	.global sFmt_3d
sFmt_3d:
	.asciz "%3d"
	.global sFmt_05d
sFmt_05d:
	.asciz "%05d"
	.align 2, 0
	.global sFmt_02X
sFmt_02X:
	.asciz "%02X"
	.align 2, 0
	.global sFmt_04X
sFmt_04X:
	.asciz "%04X"
	.align 2, 0
	.global sFmt_08X
sFmt_08X:
	.asciz "%08X"
	.align 2, 0
	.global sFmt_d
sFmt_d:
	.asciz "%d"
	.align 2, 0
	.global sFmt_02d
sFmt_02d:
	.asciz "%02d"
	.align 2, 0
	.global sFmt_03d
sFmt_03d:
	.asciz "%03d"
	.align 2, 0

@ The WONDER CARD the card server sends, and an unused Japanese one.

	.global sWonderCard
sWonderCard:
	wonder_card 0x3e8, 0xffff, 0, 0, 2, 0, 0
	.string "         “AURORA TICKET”", 40
	.string "     Exchange Card 2004 FALL", 40
	.string "Go to the second floor of the POKéMON", 40
	.string "CENTER and meet the delivery person in", 40
	.string "green. Receive the AURORA TICKET and", 40
	.string "then save the game!!", 40
	.string "Do not toss this Exchange Card", 40
	.string "before receiving the AURORA TICKET!!", 40
	.align 2, 0

	.global sWonderCard_Japanese
sWonderCard_Japanese:
	@ "AURORA TICKET exchange card, summer 2004. Start your game and go to the
	@ second floor of the POKéMON CENTER! You can get the AURORA TICKET! Once
	@ you have it, save right away! Don't throw this card away before you get
	@ the ticket!"
	wonder_card 0x3e8, 0xffff, 0, 0, 2, 0, 0
	.string "オーロラチケット　ひきかえ　カード", 40
	.string "2004　なつ", 40
	.string "ゲームを　はじめて　ポケモンセンターの", 40
	.string "2かいへ　いこう！　オーロラチケットを", 40
	.string "もらうことが　できるよ！", 40
	.string "チケットを　もらったら　すぐレポート！", 40
	.string "チケットを　もらうまえに　このカードを", 40
	.string "すてないで　ください！", 40
	.align 2, 0

@ The WONDER NEWS the news server sends, each between two dates (BCD year,
@ month, day, hour), up to one starting with 0xFF.
	.global sContentEntries
sContentEntries:

	@ POKéMON Movie News, Preview 1: New information on the movie Pocket
	@ Monsters Advanced Generation, opening Saturday July 17, is coming! Be sure
	@ to check out the trailer, in cinemas nationwide from Saturday March 6, and
	@ the new poster!!
	.byte 0x04, 0x02, 0x27, 0x00 @ start: 2004-02-27 00:00
	.byte 0x04, 0x03, 0x05, 0x23 @ end: 2004-03-05 23:00
	.2byte 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF @ questionnaire
	wonder_news 2, 2, 7
	.string "ポケモンえいがニュース　じゅんびごう　1", 40
	.string "7がつ17にち　どようび　から", 40
	.string "こうかい　される", 40
	.string "げきじょうばん　ポケットモンスター", 40
	.string "アドバンス　ジェネレーションの", 40
	.string "しんじょうほうが　はっぴょう　されるよ！", 40
	.string "3がつ6にち　どようびから　ぜんこくの", 40
	.string "えいがかんで　じょうえい　される", 40
	.string "よこくへんと　あたらしい　ポスターを", 40
	.string "かならず　チェック　しよう！！", 40
	.string "", 40

	@ POKéMON Movie News, Preview 2: The title of the POKéMON movie opening
	@ Saturday July 17 is decided: Pocket Monsters Advanced Generation "Destiny
	@ Deoxys" (the visitor from the sky). Deoxys!? What kind of POKéMON could it
	@ be? Wait for the news on March 15!
	.byte 0x04, 0x03, 0x06, 0x00 @ start: 2004-03-06 00:00
	.byte 0x04, 0x03, 0x14, 0x23 @ end: 2004-03-14 23:00
	.2byte 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF @ questionnaire
	wonder_news 3, 2, 7
	.string "ポケモンえいがニュース　じゅんびごう　2", 40
	.string "7がつ17にち　どようび　から", 40
	.string "こうかい　される　ポケモン　えいがの", 40
	.string "せいしきな　タイトルが　けってい！", 40
	.string "げきじょうばん　ポケットモンスター", 40
	.string "アドバンス　ジェネレーション", 40
	.string "「れっくうの　ほうもんしゃ　デオキシス」", 40
	.string "デオキシス！？", 40
	.string "いったい　どんな　ポケモンなんだろう？", 40
	.string "3がつ15にちの　ニュースを　まて！", 40
	.string "", 40

	@ Latest POKéMON Movie News 1: Deoxys can change its form! In FireRed it has
	@ Attack Forme, in LeafGreen Defense Forme and in Ruby and Sapphire Normal
	@ Forme: three forms. For details, look up the official POKéMON site on the
	@ Internet!!
	.byte 0x04, 0x03, 0x15, 0x00 @ start: 2004-03-15 00:00
	.byte 0x04, 0x04, 0x14, 0x23 @ end: 2004-04-14 23:00
	.2byte 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF @ questionnaire
	wonder_news 4, 2, 7
	.string "ポケモンえいが　さいしんニュース　1", 40
	.string "デオキシスは　すがたを　かえることが", 40
	.string "はんめい！", 40
	.string "ポケットモンスター", 40
	.string "ファイアレッドでは　アタック　フォルム", 40
	.string "リーフグリーンでは　ディフェンス", 40
	.string "フォルム　ルビー・サファイアでは", 40
	.string "ノーマル　フォルム　と　3つの　すがた", 40
	.string "くわしくは　インターネットで", 40
	.string "ポケモン　オフィシャル　サイトを", 40
	.string "けんさくして　みてみよう！！", 40

	@ Latest POKéMON Movie News 2: With the special advance movie ticket, on
	@ sale from April 17 to July 16, you can get the AURORA TICKET to meet
	@ Deoxys!! For details, see CoroCoro Comic, out April 15, or look up the
	@ official POKéMON movie site on the Internet!
	.byte 0x04, 0x04, 0x15, 0x00 @ start: 2004-04-15 00:00
	.byte 0x04, 0x04, 0x16, 0x23 @ end: 2004-04-16 23:00
	.2byte 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF @ questionnaire
	wonder_news 5, 2, 7
	.string "ポケモンえいが　さいしん　ニュース　2", 40
	.string "4がつ17にち　から　7がつ16にちまで", 40
	.string "はつばい　される", 40
	.string "えいがの　とくべつ　まえうりけんで", 40
	.string "デオキシスに　あうための", 40
	.string "オーロラ　チケットが　ゲット　できる！！", 40
	.string "くわしくは　4がつ15にち　はつばい", 40
	.string "コロコロ　コミックや　インターネットで", 40
	.string "ポケモン　えいが　こうしき　サイトを", 40
	.string "けんさくして　みて　みよう！", 40
	.string "", 40

	@ Latest POKéMON Movie News 3: From April 17, cinemas nationwide are showing
	@ a new trailer for "Destiny Deoxys", with Deoxys in its three formes!! You
	@ can't miss it!
	.byte 0x04, 0x04, 0x17, 0x00 @ start: 2004-04-17 00:00
	.byte 0x04, 0x05, 0x20, 0x23 @ end: 2004-05-20 23:00
	.2byte 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF @ questionnaire
	wonder_news 6, 2, 7
	.string "ポケモンえいが　さいしんニュース　3", 40
	.string "4がつ17にちから　ぜんこくの", 40
	.string "えいがかんで", 40
	.string "げきじょうばん　ポケットモンスター", 40
	.string "アドバンス　ジェネレーション", 40
	.string "「れっくうの　ほうもんしゃ　デオキシス」", 40
	.string "の　あたらしい　よこくへんが", 40
	.string "じょうえい　されている", 40
	.string "そこにはデオキシスが　3つの　フォルムで", 40
	.string "とうじょう！！", 40
	.string "これは　ぜったいに　みのがせない！", 40

	@ Latest POKéMON Movie News 4: All Nippon Airways' new POKéMON Jet design
	@ has been unveiled!! The main characters of the POKéMON movies so far are
	@ all on it! Munchlax (Gonbe), a new POKéMON in the summer movie, makes its
	@ first appearance on it too!
	.byte 0x04, 0x05, 0x21, 0x00 @ start: 2004-05-21 00:00
	.byte 0x04, 0x06, 0x14, 0x23 @ end: 2004-06-14 23:00
	.2byte 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF @ questionnaire
	wonder_news 7, 2, 7
	.string "ポケモンえいが　さいしんニュース　4", 40
	.string "ぜんにっくう　ポケモン　ジェットの", 40
	.string "あたらしい　デザインの　きたいが", 40
	.string "ひろうされた！！", 40
	.string "これまでの　ポケモンえいがの", 40
	.string "おもな　キャラクターが　せいぞろい！", 40
	.string "なつの　えいがに　とうじょうする", 40
	.string "あたらしい　ポケモン　「ゴンベ」も", 40
	.string "この　きたいで　おめみえ　したよ！", 40
	.string "", 40
	.string "", 40

	@ Latest POKéMON Movie News 5: From Saturday June 19, the AURORA TICKET for
	@ meeting Deoxys is distributed. Read the "Deoxys Complete Guide" that comes
	@ with the special advance ticket before you go!! Also look up the official
	@ POKéMON movie site on the Internet!
	.byte 0x04, 0x06, 0x15, 0x00 @ start: 2004-06-15 00:00
	.byte 0x04, 0x08, 0x31, 0x23 @ end: 2004-08-31 23:00
	.2byte 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF @ questionnaire
	wonder_news 8, 2, 7
	.string "ポケモンえいが　さいしんニュース　5", 40
	.string "6がつ19にち　どようびより", 40
	.string "デオキシスに　あえる　「オーロラ", 40
	.string "チケット」の　はいしんが　スタート　する", 40
	.string "とくべつ　まえうりけんに　ついている", 40
	.string "「デオキシス　かんぜんゲット　ガイド」を", 40
	.string "よく　よんで　でかけよう！！", 40
	.string "インターネットの", 40
	.string "ポケモン　えいが　こうしき　サイトも", 40
	.string "けんさくして　みて　みよう！", 40
	.string "", 40

	@ end of the list
	.byte 0xFF, 0xFF, 0xFF, 0xFF
	.space 0x1CC - 4

@ The news sent when no other news is current.

	.global sDefaultContentEntry
sDefaultContentEntry:
	@ "Ranking News: You are ranked 999th of 999 players. Top 5: rank, name, ID,
	@ wins." The cartridge fills in the numbers (BuildRankingNews).
	.byte 0x00, 0x00, 0x00, 0x00 @ start: none
	.byte 0xFF, 0xFF, 0xFF, 0xFF @ end: none
	.2byte 0x142C, 0x1812, 0x1610, 0x080A @ questionnaire
	wonder_news 1, 2, 0
	.string "ランキング　ニュース", 40
	.string "あなたの　じゅんいは", 40
	.string "999にん　ちゅう　999い　です。", 40
	.string "", 40
	.string "じょうい　5い　までの　せいせき", 40
	.string "ランク　　なまえ　　　ID　　　　かち", 40
	.string "　1い　じょい01　00001　999", 40
	.string "　2い　じょい02　00002　998", 40
	.string "　3い　じょい03　00003　997", 40
	.string "　4い　じょい04　00004　996", 40
	.string "　5い　じょい05　00005　995", 40

	.global sText_NoBattleRecord
sText_NoBattleRecord:
	@ "You have no battle record."
	.string "あなたの　たいせん　せいせき　は", 40
	.string "ありません。", 40

	.global sMainCallbacks
sMainCallbacks:
	.4byte MainState_None
	.4byte TitleScreen_Main
	.4byte SlotTasks_Main
	.4byte SetupMenu_Main

	.global sDebugSlotPos
sDebugSlotPos:
	.2byte 2, 4
	.2byte 2, 6
	.2byte 2, 8
	.2byte 2, 10
	.global sDebugLabelPos_Serial
sDebugLabelPos_Serial:
	.2byte 6, 2
	.global sDebugLabelPos_Time
sDebugLabelPos_Time:
	.2byte 13, 2
	.global sDebugLabelPos_MaxTime
sDebugLabelPos_MaxTime:
	.2byte 18, 2
	.global sDebugLabelPos_Strength
sDebugLabelPos_Strength:
	.2byte 23, 2
	.global sDebugSerialPos
sDebugSerialPos:
	.2byte 8, 4
	.2byte 8, 6
	.2byte 8, 8
	.2byte 8, 10
	.global sDebugTimePos
sDebugTimePos:
	.2byte 13, 4
	.2byte 13, 6
	.2byte 13, 8
	.2byte 13, 10
	.global sDebugMaxTimePos
sDebugMaxTimePos:
	.2byte 18, 4
	.2byte 18, 6
	.2byte 18, 8
	.2byte 18, 10
	.global sDebugStrengthPos
sDebugStrengthPos:
	.2byte 23, 4
	.2byte 23, 6
	.2byte 23, 8
	.2byte 23, 10

	@ unused
	.byte 0x03, 0x00, 0x00, 0x00

	.global sText_Slot
sText_Slot:
	.asciz "SLOT"
	.align 2, 0

	.global sText_Serial
sText_Serial:
	.asciz "SERIAL"
	.align 2, 0

	.global sText_SlotTime
sText_SlotTime:
	.asciz "NOW"

	.global sText_MaxTime
sText_MaxTime:
	.asciz "MAX"

	.global sText_Strength
sText_Strength:
	.asciz "LS"
	.align 2, 0

@ The range of each field of the setup menu's clock (BCD year, month, day,
@ weekday, hour, minute, second) and POKéMON AI settings
	.global sRtcMin
sRtcMin:
	.byte 0x00, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00
	.global sRtcMax
sRtcMax:
	.byte 0x99, 0x12, 0x31, 0x06, 0x23, 0x59, 0x59
	.global sAiSettingsMin
sAiSettingsMin:
	.byte 0, 0, 1
	.global sAiSettingsMax
sAiSettingsMax:
	.byte 2, 5, 55

	.global sText_Border
sText_Border:
	.asciz "/----------------------------/"
	.align 2, 0

	.global sText_SetupMenu
sText_SetupMenu:
	.asciz "/         Setup Menu         /"
	.align 2, 0

	.global sText_Time
sText_Time:
	.asciz "    TIME"
	.align 2, 0

	.global sText_PokemonAi
sText_PokemonAi:
	.asciz "    POKEMON AI"
	.align 2, 0

	.global sText_TimeSetup
sText_TimeSetup:
	.asciz "/         Time Setup         /"
	.align 2, 0

	.global sText_Year
sText_Year:
	.asciz "    YEAR"
	.align 2, 0

	.global sText_Month
sText_Month:
	.asciz "    MONTH"
	.align 2, 0

	.global sText_Day
sText_Day:
	.asciz "    DAY"

	.global sText_Week
sText_Week:
	.asciz "    WEEK"
	.align 2, 0

	.global sText_Hour
sText_Hour:
	.asciz "    HOUR"
	.align 2, 0

	.global sText_Minute
sText_Minute:
	.asciz "    MINITE"
	.align 2, 0

	.global sText_Second
sText_Second:
	.asciz "    SECOND"
	.align 2, 0

	.global sText_Set
sText_Set:
	.asciz "    SET"

	.global sText_PokemonAiSetup
sText_PokemonAiSetup:
	.asciz "/      POKEMON AI Setup      /"
	.align 2, 0

	.global sText_Mode
sText_Mode:
	.asciz "    Mode"
	.align 2, 0

	.global sText_Level
sText_Level:
	.asciz "    Level"
	.align 2, 0

	.global sText_Pair
sText_Pair:
	.asciz "    Pair"
	.align 2, 0

	.global sText_Dashes
sText_Dashes:
	.asciz "--"
	.align 2, 0

@ White-on-black and black-on-white: FlashPalette swaps between them to flash
@ the setup menu's screen.
	.global sFlashPalettes
sFlashPalettes:
	.2byte 0x0000, 0x7FFF, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000
	.2byte 0x7FFF, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000

	.global sTitlePos
sTitlePos:
	.2byte 6, 5
	.global sVersionPos
sVersionPos:
	.2byte 11, 8
	.global sPressAPos
sPressAPos:
	.2byte 8, 14

	.global sText_Title
sText_Title:
	.byte 0xF8, 0xAE, 0xB2, 0xBD, 0x3F, 0xAF, 0xC4, 0x20, 0xC3, 0xBD, 0xC4, 0x20, 0x3D, 0xDB, 0xF4, 0xD7, 0xD1, 0x00 @ ジョイスポット テスト プログラム ("JoySpot test program")
	.global sText_Version
sText_Version:
	.asciz "Ver. 0.4"
	.global sText_PressA
sText_PressA:
	.asciz "Press A Button"
	.align 2, 0

@ The name the cartridge shows to the games.
	.global sRfuUserName
sRfuUserName:
	@ "Terminal"
	.string "ターミナル$"
	.align 2, 0

	.global sNumberPos
sNumberPos:
	.byte 35, 44
	.2byte 0
	.byte 35, 60
	.2byte 0
	.byte 35, 76
	.2byte 0
	.byte 35, 92
	.2byte 0
	.byte 35, 108
	.2byte 0
	.byte 131, 44
	.2byte 0
	.byte 131, 60
	.2byte 0
	.byte 131, 76
	.2byte 0
	.byte 131, 92
	.2byte 0
	.byte 131, 108
	.2byte 0

	.global sBgCnt
sBgCnt:
	.2byte 0x0004, 0x0105, 0x0206, 0x0307

	@ unused
	.incbin "graphics/unused/palette.gbapal"

@ The Mystery Gift servers: a name the games show, and the script the
@ server runs. The commands are named as in pret's pokeemerald.

	.global sNewsServerName
sNewsServerName:
	@ "News Server"
	.string "ニュース　サーバ$"
	.align 2, 0

	.global sNewsServerScript
sNewsServerScript:
	server_cmd SVR_LOAD_CLIENT_SCRIPT, 0x20, sClientScript_SendGameData
	server_cmd SVR_SEND
	server_cmd SVR_RECV, MG_LINKID_GAME_DATA
	server_cmd SVR_COPY_GAME_DATA
	server_cmd SVR_CHECK_GAME_DATA
	server_cmd SVR_GOTO_IF_EQ, 0, sServerScript_InvalidGameData
	server_cmd SVR_FIND_NEWS  @ 0: none for this game
	server_cmd SVR_GOTO_IF_EQ, 0, sServerScript_NoNews
	server_cmd SVR_COPY_FOUND_NEWS
	server_cmd SVR_GOTO, 0, sServerScript_SendNews

	.global sCardServerName
sCardServerName:
	@ "Card Server"
	.string "カード　サーバ$"

	.global sCardServerScript
sCardServerScript:
	server_cmd SVR_COPY_CARD, 0, sWonderCard
	server_cmd SVR_SET_RAM_SCRIPT, 0x400, sAuroraTicketScript
	server_cmd SVR_LOAD_CLIENT_SCRIPT, 0x20, sClientScript_SendGameData
	server_cmd SVR_SEND
	server_cmd SVR_RECV, MG_LINKID_GAME_DATA
	server_cmd SVR_COPY_GAME_DATA
	server_cmd SVR_CHECK_GAME_DATA
	server_cmd SVR_GOTO_IF_EQ, 0, sServerScript_InvalidGameData
	server_cmd SVR_CHECK_EXISTING_CARD  @ 0: no card, 1: this card, 2: another card
	server_cmd SVR_GOTO_IF_EQ, 2, sServerScript_AskToss
	server_cmd SVR_GOTO_IF_EQ, 0, sServerScript_SendCard
	server_cmd SVR_GOTO, 0, sServerScript_HasCard

	.global sMgScripts
sMgScripts:
	.4byte 0, 1, sCardServerScript, sCardServerName
	.4byte 14, 0, sNewsServerScript, sNewsServerName

sText_CardCancelled:
	@ "Card loading was canceled."
	.string "カードの　よみこみを\nキャンセル　しました$"
	.align 2, 0

@ The scripts the servers send to the games to run.

sClientScript_Unused_RunReceived:
	client_cmd CLI_RECV, MG_LINKID_CLIENT_SCRIPT
	client_cmd CLI_COPY_RECV

sClientScript_SendGameData:
	client_cmd CLI_LOAD_GAME_DATA
	client_cmd CLI_SEND_LOADED
	client_cmd CLI_RECV, MG_LINKID_CLIENT_SCRIPT
	client_cmd CLI_COPY_RECV

sClientScript_InvalidGameData:
	client_cmd CLI_SEND_READY_END
	client_cmd CLI_RETURN, 10

sClientScript_NoNews:
	client_cmd CLI_SEND_READY_END
	client_cmd CLI_RETURN, 11

sClientScript_HasCard:
	client_cmd CLI_SEND_READY_END
	client_cmd CLI_RETURN

sClientScript_ReceiveCard:
	client_cmd CLI_RECV, MG_LINKID_CARD
	client_cmd CLI_SAVE_CARD
	client_cmd CLI_RECV, MG_LINKID_RAM_SCRIPT
	client_cmd CLI_SAVE_RAM_SCRIPT
	client_cmd CLI_SEND_READY_END
	client_cmd CLI_RETURN, 2

sClientScript_ReceiveNews:
	client_cmd CLI_RECV, MG_LINKID_NEWS
	client_cmd CLI_SAVE_NEWS
	client_cmd CLI_SEND_LOADED
	client_cmd CLI_RECV, MG_LINKID_CLIENT_SCRIPT
	client_cmd CLI_COPY_RECV

sClientScript_NewsAlreadyReceived:
	client_cmd CLI_SEND_READY_END
	client_cmd CLI_RETURN, 7

sClientScript_NewsReceived:
	client_cmd CLI_SEND_READY_END
	client_cmd CLI_RETURN, 3

sClientScript_AskToss:
	client_cmd CLI_ASK_TOSS
	client_cmd CLI_LOAD_TOSS_RESPONSE
	client_cmd CLI_SEND_LOADED
	client_cmd CLI_RECV, MG_LINKID_CLIENT_SCRIPT
	client_cmd CLI_COPY_RECV

sClientScript_Unused_Return9:
	client_cmd CLI_SEND_READY_END
	client_cmd CLI_RETURN, 9

sClientScript_Unused_Return5:
	client_cmd CLI_SEND_READY_END
	client_cmd CLI_RETURN, 5

sClientScript_ShowMessage:
	client_cmd CLI_RECV, MG_LINKID_DYNAMIC_MSG
	client_cmd CLI_COPY_MSG
	client_cmd CLI_SEND_READY_END
	client_cmd CLI_RETURN, 14

sClientScript_Unused_ShowMessage:
	client_cmd CLI_RECV, MG_LINKID_DYNAMIC_MSG
	client_cmd CLI_COPY_MSG
	client_cmd CLI_SEND_READY_END
	client_cmd CLI_RETURN, 13

sServerScript_InvalidGameData:
	server_cmd SVR_LOAD_CLIENT_SCRIPT, 0x10, sClientScript_InvalidGameData
	server_cmd SVR_SEND
	server_cmd SVR_RECV, MG_LINKID_READY_END
	server_cmd SVR_RETURN, 10

sServerScript_NoNews:
	server_cmd SVR_LOAD_CLIENT_SCRIPT, 0x10, sClientScript_NoNews
	server_cmd SVR_SEND
	server_cmd SVR_RECV, MG_LINKID_READY_END
	server_cmd SVR_RETURN, 11

sServerScript_Unused_Return9:
	server_cmd SVR_LOAD_CLIENT_SCRIPT, 0x10, sClientScript_Unused_Return9
	server_cmd SVR_SEND
	server_cmd SVR_RECV, MG_LINKID_READY_END
	server_cmd SVR_RETURN, 9

sServerScript_TossDeclined:
	server_cmd SVR_LOAD_CLIENT_SCRIPT, 0x20, sClientScript_ShowMessage
	server_cmd SVR_SEND
	server_cmd SVR_LOAD_MSG, 0x16, sText_CardCancelled
	server_cmd SVR_SEND
	server_cmd SVR_RECV, MG_LINKID_READY_END
	server_cmd SVR_RETURN, 9

sServerScript_NewsAlreadyReceived:
	server_cmd SVR_LOAD_CLIENT_SCRIPT, 0x10, sClientScript_NewsAlreadyReceived
	server_cmd SVR_SEND
	server_cmd SVR_RECV, MG_LINKID_READY_END
	server_cmd SVR_RETURN, 7

sServerScript_SendNews:
	server_cmd SVR_LOAD_CLIENT_SCRIPT, 0x28, sClientScript_ReceiveNews
	server_cmd SVR_SEND
	server_cmd SVR_LOAD_NEWS
	server_cmd SVR_SEND
	server_cmd SVR_RECV, MG_LINKID_RESPONSE
	server_cmd SVR_READ_RESPONSE  @ 1: the game already has the news
	server_cmd SVR_GOTO_IF_EQ, 1, sServerScript_NewsAlreadyReceived
	server_cmd SVR_LOAD_CLIENT_SCRIPT, 0x10, sClientScript_NewsReceived
	server_cmd SVR_SEND
	server_cmd SVR_RECV, MG_LINKID_READY_END
	server_cmd SVR_RETURN, 3

sServerScript_SendCard:
	server_cmd SVR_LOAD_CLIENT_SCRIPT, 0x30, sClientScript_ReceiveCard
	server_cmd SVR_SEND
	server_cmd SVR_LOAD_CARD
	server_cmd SVR_SEND
	server_cmd SVR_LOAD_RAM_SCRIPT
	server_cmd SVR_SEND
	server_cmd SVR_RECV, MG_LINKID_READY_END
	server_cmd SVR_RETURN, 2

sServerScript_AskToss:
	server_cmd SVR_LOAD_CLIENT_SCRIPT, 0x28, sClientScript_AskToss
	server_cmd SVR_SEND
	server_cmd SVR_RECV, MG_LINKID_RESPONSE
	server_cmd SVR_READ_RESPONSE  @ 0: the player tossed the old card
	server_cmd SVR_GOTO_IF_EQ, 0, sServerScript_SendCard
	server_cmd SVR_GOTO, 0, sServerScript_TossDeclined

sServerScript_Unused_Return5:
	server_cmd SVR_LOAD_CLIENT_SCRIPT, 0x10, sClientScript_Unused_Return5
	server_cmd SVR_SEND
	server_cmd SVR_RECV, MG_LINKID_READY_END
	server_cmd SVR_RETURN, 5

sServerScript_HasCard:
	server_cmd SVR_LOAD_CLIENT_SCRIPT, 0x10, sClientScript_HasCard
	server_cmd SVR_SEND
	server_cmd SVR_RECV, MG_LINKID_READY_END
	server_cmd SVR_RETURN

	@ unused: "Etsuko", a name
	.string "エツコ$"

	.global sMgServerInfoHeader
sMgServerInfoHeader:
	.asciz "GameFreak inc."
	.align 2, 0

@ CRC-16-CCITT (reflected, polynomial 0x8408)
	.global sCrcTable
sCrcTable:
	.2byte 0x0000, 0x1189, 0x2312, 0x329B, 0x4624, 0x57AD, 0x6536, 0x74BF
	.2byte 0x8C48, 0x9DC1, 0xAF5A, 0xBED3, 0xCA6C, 0xDBE5, 0xE97E, 0xF8F7
	.2byte 0x1081, 0x0108, 0x3393, 0x221A, 0x56A5, 0x472C, 0x75B7, 0x643E
	.2byte 0x9CC9, 0x8D40, 0xBFDB, 0xAE52, 0xDAED, 0xCB64, 0xF9FF, 0xE876
	.2byte 0x2102, 0x308B, 0x0210, 0x1399, 0x6726, 0x76AF, 0x4434, 0x55BD
	.2byte 0xAD4A, 0xBCC3, 0x8E58, 0x9FD1, 0xEB6E, 0xFAE7, 0xC87C, 0xD9F5
	.2byte 0x3183, 0x200A, 0x1291, 0x0318, 0x77A7, 0x662E, 0x54B5, 0x453C
	.2byte 0xBDCB, 0xAC42, 0x9ED9, 0x8F50, 0xFBEF, 0xEA66, 0xD8FD, 0xC974
	.2byte 0x4204, 0x538D, 0x6116, 0x709F, 0x0420, 0x15A9, 0x2732, 0x36BB
	.2byte 0xCE4C, 0xDFC5, 0xED5E, 0xFCD7, 0x8868, 0x99E1, 0xAB7A, 0xBAF3
	.2byte 0x5285, 0x430C, 0x7197, 0x601E, 0x14A1, 0x0528, 0x37B3, 0x263A
	.2byte 0xDECD, 0xCF44, 0xFDDF, 0xEC56, 0x98E9, 0x8960, 0xBBFB, 0xAA72
	.2byte 0x6306, 0x728F, 0x4014, 0x519D, 0x2522, 0x34AB, 0x0630, 0x17B9
	.2byte 0xEF4E, 0xFEC7, 0xCC5C, 0xDDD5, 0xA96A, 0xB8E3, 0x8A78, 0x9BF1
	.2byte 0x7387, 0x620E, 0x5095, 0x411C, 0x35A3, 0x242A, 0x16B1, 0x0738
	.2byte 0xFFCF, 0xEE46, 0xDCDD, 0xCD54, 0xB9EB, 0xA862, 0x9AF9, 0x8B70
	.2byte 0x8408, 0x9581, 0xA71A, 0xB693, 0xC22C, 0xD3A5, 0xE13E, 0xF0B7
	.2byte 0x0840, 0x19C9, 0x2B52, 0x3ADB, 0x4E64, 0x5FED, 0x6D76, 0x7CFF
	.2byte 0x9489, 0x8500, 0xB79B, 0xA612, 0xD2AD, 0xC324, 0xF1BF, 0xE036
	.2byte 0x18C1, 0x0948, 0x3BD3, 0x2A5A, 0x5EE5, 0x4F6C, 0x7DF7, 0x6C7E
	.2byte 0xA50A, 0xB483, 0x8618, 0x9791, 0xE32E, 0xF2A7, 0xC03C, 0xD1B5
	.2byte 0x2942, 0x38CB, 0x0A50, 0x1BD9, 0x6F66, 0x7EEF, 0x4C74, 0x5DFD
	.2byte 0xB58B, 0xA402, 0x9699, 0x8710, 0xF3AF, 0xE226, 0xD0BD, 0xC134
	.2byte 0x39C3, 0x284A, 0x1AD1, 0x0B58, 0x7FE7, 0x6E6E, 0x5CF5, 0x4D7C
	.2byte 0xC60C, 0xD785, 0xE51E, 0xF497, 0x8028, 0x91A1, 0xA33A, 0xB2B3
	.2byte 0x4A44, 0x5BCD, 0x6956, 0x78DF, 0x0C60, 0x1DE9, 0x2F72, 0x3EFB
	.2byte 0xD68D, 0xC704, 0xF59F, 0xE416, 0x90A9, 0x8120, 0xB3BB, 0xA232
	.2byte 0x5AC5, 0x4B4C, 0x79D7, 0x685E, 0x1CE1, 0x0D68, 0x3FF3, 0x2E7A
	.2byte 0xE70E, 0xF687, 0xC41C, 0xD595, 0xA12A, 0xB0A3, 0x8238, 0x93B1
	.2byte 0x6B46, 0x7ACF, 0x4854, 0x59DD, 0x2D62, 0x3CEB, 0x0E70, 0x1FF9
	.2byte 0xF78F, 0xE606, 0xD49D, 0xC514, 0xB1AB, 0xA022, 0x92B9, 0x8330
	.2byte 0x7BC7, 0x6A4E, 0x58D5, 0x495C, 0x3DE3, 0x2C6A, 0x1EF1, 0x0F78

	.global sNoContentHeader
sNoContentHeader:
	.byte 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF

@ An unused 16x16 font: ASCII, kana and kanji.
	.incbin "graphics/font/font.1bpp"

	.global sText_JspController
sText_JspController:
	.asciz "JSPCONTROLLER"
	.align 2, 0
	.global sText_ChildTkd
sText_ChildTkd:
	.asciz "CHILDTKD"
	.align 2, 0
