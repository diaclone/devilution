#include "diablo.h"

BOOL invflag;
void *pInvCels;
BOOL drawsbarflag;
int sgdwLastTime; // check name

const InvXY InvRect[73] = {
	// clang-format off
	//  X,   Y
	{ 452,  31 }, // helmet
	{ 480,  31 }, // helmet
	{ 452,  59 }, // helmet
	{ 480,  59 }, // helmet
	{ 365, 205 }, // left ring
	{ 567, 205 }, // right ring
	{ 524,  59 }, // amulet
	{ 337, 104 }, // left hand
	{ 366, 104 }, // left hand
	{ 337, 132 }, // left hand
	{ 366, 132 }, // left hand
	{ 337, 160 }, // left hand
	{ 366, 160 }, // left hand
	{ 567, 104 }, // right hand
	{ 596, 104 }, // right hand
	{ 567, 132 }, // right hand
	{ 596, 132 }, // right hand
	{ 567, 160 }, // right hand
	{ 596, 160 }, // right hand
	{ 452, 104 }, // chest
	{ 480, 104 }, // chest
	{ 452, 132 }, // chest
	{ 480, 132 }, // chest
	{ 452, 160 }, // chest
	{ 480, 160 }, // chest
	{ 337, 250 }, // inv row 1
	{ 366, 250 }, // inv row 1
	{ 394, 250 }, // inv row 1
	{ 423, 250 }, // inv row 1
	{ 451, 250 }, // inv row 1
	{ 480, 250 }, // inv row 1
	{ 509, 250 }, // inv row 1
	{ 538, 250 }, // inv row 1
	{ 567, 250 }, // inv row 1
	{ 596, 250 }, // inv row 1
	{ 337, 279 }, // inv row 2
	{ 366, 279 }, // inv row 2
	{ 394, 279 }, // inv row 2
	{ 423, 279 }, // inv row 2
	{ 451, 279 }, // inv row 2
	{ 480, 279 }, // inv row 2
	{ 509, 279 }, // inv row 2
	{ 538, 279 }, // inv row 2
	{ 567, 279 }, // inv row 2
	{ 596, 279 }, // inv row 2
	{ 337, 308 }, // inv row 3
	{ 366, 308 }, // inv row 3
	{ 394, 308 }, // inv row 3
	{ 423, 308 }, // inv row 3
	{ 451, 308 }, // inv row 3
	{ 480, 308 }, // inv row 3
	{ 509, 308 }, // inv row 3
	{ 538, 308 }, // inv row 3
	{ 567, 308 }, // inv row 3
	{ 596, 308 }, // inv row 3
	{ 337, 336 }, // inv row 4
	{ 366, 336 }, // inv row 4
	{ 394, 336 }, // inv row 4
	{ 423, 336 }, // inv row 4
	{ 451, 336 }, // inv row 4
	{ 480, 336 }, // inv row 4
	{ 509, 336 }, // inv row 4
	{ 538, 336 }, // inv row 4
	{ 567, 336 }, // inv row 4
	{ 596, 336 }, // inv row 4
	{ 205, 385 }, // belt
	{ 234, 385 }, // belt
	{ 263, 385 }, // belt
	{ 292, 385 }, // belt
	{ 321, 385 }, // belt
	{ 350, 385 }, // belt
	{ 379, 385 }, // belt
	{ 408, 385 }  // belt
	// clang-format on
};

/* data */

int AP2x2Tbl[10] = { 8, 28, 6, 26, 4, 24, 2, 22, 0, 20 }; // weak

void FreeInvGFX()
{
	MemFreeDbg(pInvCels);
}

void InitInv()
{
	if (plr[myplr]._pClass == PC_WARRIOR) {
		pInvCels = LoadFileInMem("Data\\Inv\\Inv.CEL", 0);
	} else if (plr[myplr]._pClass == PC_ROGUE) {
		pInvCels = LoadFileInMem("Data\\Inv\\Inv_rog.CEL", 0);
	} else if (plr[myplr]._pClass == PC_SORCERER) {
		pInvCels = LoadFileInMem("Data\\Inv\\Inv_Sor.CEL", 0);
	}

	invflag = 0;
	drawsbarflag = FALSE;
}

void InvDrawSlotBack(int X, int Y, int W, int H)
{
	BYTE *dst;

	/// ASSERT: assert(gpBuffer);

	dst = &gpBuffer[X + PitchTbl[Y]];

#ifdef USE_ASM
	__asm {
		mov		edi, dst
		xor		edx, edx
		xor		ebx, ebx
		mov		dx, word ptr H
		mov		bx, word ptr W
	label1:
		mov		ecx, ebx
	label2:
		mov		al, [edi]
		cmp		al, PAL16_BLUE
		jb		label5
		cmp		al, PAL16_BLUE + 15
		ja		label3
		sub		al, PAL16_BLUE - PAL16_BEIGE
		jmp		label4
	label3:
		cmp		al, PAL16_GRAY
		jb		label5
		sub		al, PAL16_GRAY - PAL16_BEIGE
	label4:
		mov		[edi], al
	label5:
		inc		edi
		loop	label2
		sub		edi, 768
		sub		edi, ebx
		dec		edx
		jnz		label1
	}
#else
	int wdt, hgt;
	BYTE pix;

	for (hgt = H; hgt; hgt--, dst -= 768 + W) {
		for (wdt = W; wdt; wdt--) {
			pix = *dst;
			if (pix >= PAL16_BLUE) {
				if (pix <= PAL16_BLUE + 15)
					pix -= PAL16_BLUE - PAL16_BEIGE;
				else if (pix >= PAL16_GRAY)
					pix -= PAL16_GRAY - PAL16_BEIGE;
			}
			*dst++ = pix;
		}
	}
#endif
}

void DrawInv()
{
	BOOL invtest[40];
	int frame, frame_width, colour, screen_x, screen_y, i, j, ii;

	CelDecodeOnly(384, 511, (BYTE *)pInvCels, 1, 320);

	if (plr[myplr].InvBody[INVLOC_HEAD]._itype != ITYPE_NONE) {
		InvDrawSlotBack(517, 219, 2 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX);

		frame = plr[myplr].InvBody[INVLOC_HEAD]._iCurs + CURSOR_FIRSTITEM;
		frame_width = InvItemWidth[frame];

		if (pcursinvitem == INVITEM_HEAD) {
			colour = ICOL_WHITE;
			if (plr[myplr].InvBody[INVLOC_HEAD]._iMagical != ITEM_QUALITY_NORMAL) {
				colour = ICOL_BLUE;
			}
			if (!plr[myplr].InvBody[INVLOC_HEAD]._iStatFlag) {
				colour = ICOL_RED;
			}
			CelDecodeClr(colour, 517, 219, (BYTE *)pCursCels, frame, frame_width, 0, 8);
		}

		if (plr[myplr].InvBody[INVLOC_HEAD]._iStatFlag) {
			CelDrawHdrOnly(517, 219, (BYTE *)pCursCels, frame, frame_width, 0, 8);
		} else {
			CelDrawHdrLightRed(517, 219, (BYTE *)pCursCels, frame, frame_width, 0, 8, 1);
		}
	}

	if (plr[myplr].InvBody[INVLOC_RING_LEFT]._itype != ITYPE_NONE) {
		InvDrawSlotBack(432, 365, INV_SLOT_SIZE_PX, INV_SLOT_SIZE_PX);

		frame = plr[myplr].InvBody[INVLOC_RING_LEFT]._iCurs + CURSOR_FIRSTITEM;
		frame_width = InvItemWidth[frame];

		if (pcursinvitem == INVITEM_RING_LEFT) {
			colour = ICOL_WHITE;
			if (plr[myplr].InvBody[INVLOC_RING_LEFT]._iMagical != ITEM_QUALITY_NORMAL) {
				colour = ICOL_BLUE;
			}
			if (!plr[myplr].InvBody[INVLOC_RING_LEFT]._iStatFlag) {
				colour = ICOL_RED;
			}
			CelDecodeClr(colour, 432, 365, (BYTE *)pCursCels, frame, frame_width, 0, 8);
		}

		if (plr[myplr].InvBody[INVLOC_RING_LEFT]._iStatFlag) {
			CelDrawHdrOnly(432, 365, (BYTE *)pCursCels, frame, frame_width, 0, 8);
		} else {
			CelDrawHdrLightRed(432, 365, (BYTE *)pCursCels, frame, frame_width, 0, 8, 1);
		}
	}

	if (plr[myplr].InvBody[INVLOC_RING_RIGHT]._itype != ITYPE_NONE) {
		InvDrawSlotBack(633, 365, INV_SLOT_SIZE_PX, INV_SLOT_SIZE_PX);

		frame = plr[myplr].InvBody[INVLOC_RING_RIGHT]._iCurs + CURSOR_FIRSTITEM;
		frame_width = InvItemWidth[frame];

		if (pcursinvitem == INVITEM_RING_RIGHT) {
			colour = ICOL_WHITE;
			if (plr[myplr].InvBody[INVLOC_RING_RIGHT]._iMagical != ITEM_QUALITY_NORMAL) {
				colour = ICOL_BLUE;
			}
			if (!plr[myplr].InvBody[INVLOC_RING_RIGHT]._iStatFlag) {
				colour = ICOL_RED;
			}
			CelDecodeClr(colour, 633, 365, (BYTE *)pCursCels, frame, frame_width, 0, 8);
		}

		if (plr[myplr].InvBody[INVLOC_RING_RIGHT]._iStatFlag) {
			CelDrawHdrOnly(633, 365, (BYTE *)pCursCels, frame, frame_width, 0, 8);
		} else {
			CelDrawHdrLightRed(633, 365, (BYTE *)pCursCels, frame, frame_width, 0, 8, 1);
		}
	}

	if (plr[myplr].InvBody[INVLOC_AMULET]._itype != ITYPE_NONE) {
		InvDrawSlotBack(589, 220, INV_SLOT_SIZE_PX, INV_SLOT_SIZE_PX);

		frame = plr[myplr].InvBody[INVLOC_AMULET]._iCurs + CURSOR_FIRSTITEM;
		frame_width = InvItemWidth[frame];

		if (pcursinvitem == INVITEM_AMULET) {
			colour = ICOL_WHITE;
			if (plr[myplr].InvBody[INVLOC_AMULET]._iMagical != ITEM_QUALITY_NORMAL) {
				colour = ICOL_BLUE;
			}
			if (!plr[myplr].InvBody[INVLOC_AMULET]._iStatFlag) {
				colour = ICOL_RED;
			}
			CelDecodeClr(colour, 589, 220, (BYTE *)pCursCels, frame, frame_width, 0, 8);
		}

		if (plr[myplr].InvBody[INVLOC_AMULET]._iStatFlag) {
			CelDrawHdrOnly(589, 220, (BYTE *)pCursCels, frame, frame_width, 0, 8);
		} else {
			CelDrawHdrLightRed(589, 220, (BYTE *)pCursCels, frame, frame_width, 0, 8, 1);
		}
	}

	if (plr[myplr].InvBody[INVLOC_HAND_LEFT]._itype != ITYPE_NONE) {
		InvDrawSlotBack(401, 320, 2 * INV_SLOT_SIZE_PX, 3 * INV_SLOT_SIZE_PX);

		frame = plr[myplr].InvBody[INVLOC_HAND_LEFT]._iCurs + CURSOR_FIRSTITEM;
		frame_width = InvItemWidth[frame];
		// calc item offsets for weapons smaller than 2x3 slots
		screen_x = frame_width == INV_SLOT_SIZE_PX ? 415 : 401;
		screen_y = InvItemHeight[frame] == (3 * INV_SLOT_SIZE_PX) ? 320 : 306;

		if (pcursinvitem == INVITEM_HAND_LEFT) {
			colour = ICOL_WHITE;
			if (plr[myplr].InvBody[INVLOC_HAND_LEFT]._iMagical != ITEM_QUALITY_NORMAL) {
				colour = ICOL_BLUE;
			}
			if (!plr[myplr].InvBody[INVLOC_HAND_LEFT]._iStatFlag) {
				colour = ICOL_RED;
			}
			CelDecodeClr(colour, screen_x, screen_y, (BYTE *)pCursCels, frame, frame_width, 0, 8);
		}

		if (plr[myplr].InvBody[INVLOC_HAND_LEFT]._iStatFlag) {
			CelDrawHdrOnly(screen_x, screen_y, (BYTE *)pCursCels, frame, frame_width, 0, 8);
		} else {
			CelDrawHdrLightRed(screen_x, screen_y, (BYTE *)pCursCels, frame, frame_width, 0, 8, 1);
		}

		if (plr[myplr].InvBody[INVLOC_HAND_LEFT]._iLoc == ILOC_TWOHAND) {
			InvDrawSlotBack(631, 320, 2 * INV_SLOT_SIZE_PX, 3 * INV_SLOT_SIZE_PX);
			light_table_index = 0;
			cel_transparency_active = 1;

			CelDecodeHdrLightTrans(
			    frame_width == INV_SLOT_SIZE_PX
			        ? &gpBuffer[SCREENXY(581, 160)]
			        : &gpBuffer[SCREENXY(567, 160)],
			    (BYTE *)pCursCels, frame, frame_width, 0, 8);

			cel_transparency_active = 0;
		}
	}
	if (plr[myplr].InvBody[INVLOC_HAND_RIGHT]._itype != ITYPE_NONE) {
		InvDrawSlotBack(631, 320, 2 * INV_SLOT_SIZE_PX, 3 * INV_SLOT_SIZE_PX);

		frame = plr[myplr].InvBody[INVLOC_HAND_RIGHT]._iCurs + CURSOR_FIRSTITEM;
		frame_width = InvItemWidth[frame];
		// calc item offsets for weapons smaller than 2x3 slots
		screen_x = frame_width == INV_SLOT_SIZE_PX ? 645 : 633;
		screen_y = InvItemHeight[frame] == 3 * INV_SLOT_SIZE_PX ? 320 : 306;

		if (pcursinvitem == INVITEM_HAND_RIGHT) {
			colour = ICOL_WHITE;
			if (plr[myplr].InvBody[INVLOC_HAND_RIGHT]._iMagical != ITEM_QUALITY_NORMAL) {
				colour = ICOL_BLUE;
			}
			if (!plr[myplr].InvBody[INVLOC_HAND_RIGHT]._iStatFlag) {
				colour = ICOL_RED;
			}
			CelDecodeClr(colour, screen_x, screen_y, (BYTE *)pCursCels, frame, frame_width, 0, 8);
		}

		if (plr[myplr].InvBody[INVLOC_HAND_RIGHT]._iStatFlag) {
			CelDrawHdrOnly(screen_x, screen_y, (BYTE *)pCursCels, frame, frame_width, 0, 8);
		} else {
			CelDrawHdrLightRed(screen_x, screen_y, (BYTE *)pCursCels, frame, frame_width, 0, 8, 1);
		}
	}

	if (plr[myplr].InvBody[INVLOC_CHEST]._itype != ITYPE_NONE) {
		InvDrawSlotBack(517, 320, 2 * INV_SLOT_SIZE_PX, 3 * INV_SLOT_SIZE_PX);

		frame = plr[myplr].InvBody[INVLOC_CHEST]._iCurs + CURSOR_FIRSTITEM;
		frame_width = InvItemWidth[frame];

		if (pcursinvitem == INVITEM_CHEST) {
			colour = ICOL_WHITE;
			if (plr[myplr].InvBody[INVLOC_CHEST]._iMagical != ITEM_QUALITY_NORMAL) {
				colour = ICOL_BLUE;
			}
			if (!plr[myplr].InvBody[INVLOC_CHEST]._iStatFlag) {
				colour = ICOL_RED;
			}
			CelDecodeClr(colour, 517, 320, (BYTE *)pCursCels, frame, frame_width, 0, 8);
		}

		if (plr[myplr].InvBody[INVLOC_CHEST]._iStatFlag) {
			CelDrawHdrOnly(517, 320, (BYTE *)pCursCels, frame, frame_width, 0, 8);
		} else {
			CelDrawHdrLightRed(517, 320, (BYTE *)pCursCels, frame, frame_width, 0, 8, 1);
		}
	}

	for (i = 0; i < NUM_INV_GRID_ELEM; i++) {
		if (plr[myplr].InvGrid[i] != 0) {
			InvDrawSlotBack(
			    InvRect[i + SLOTXY_INV_FIRST].X + 64,
			    InvRect[i + SLOTXY_INV_FIRST].Y + 159,
			    INV_SLOT_SIZE_PX,
			    INV_SLOT_SIZE_PX);
		}
	}

	for (j = 0; j < NUM_INV_GRID_ELEM; j++) {
		if (plr[myplr].InvGrid[j] > 0) // first slot of an item
		{
			ii = plr[myplr].InvGrid[j] - 1;

			invtest[j] = TRUE;

			frame = plr[myplr].InvList[ii]._iCurs + CURSOR_FIRSTITEM;
			frame_width = InvItemWidth[frame];
			if (pcursinvitem == ii + INVITEM_INV_FIRST) {
				colour = ICOL_WHITE;
				if (plr[myplr].InvList[ii]._iMagical != ITEM_QUALITY_NORMAL) {
					colour = ICOL_BLUE;
				}
				if (!plr[myplr].InvList[ii]._iStatFlag) {
					colour = ICOL_RED;
				}
				CelDecodeClr(
				    colour,
				    InvRect[j + SLOTXY_INV_FIRST].X + 64,
				    InvRect[j + SLOTXY_INV_FIRST].Y + 159,
				    (BYTE *)pCursCels, frame, frame_width, 0, 8);
			}

			if (plr[myplr].InvList[ii]._iStatFlag) {
				CelDrawHdrOnly(
				    InvRect[j + SLOTXY_INV_FIRST].X + 64,
				    InvRect[j + SLOTXY_INV_FIRST].Y + 159,
				    (BYTE *)pCursCels, frame, frame_width, 0, 8);
			} else {
				CelDrawHdrLightRed(
				    InvRect[j + SLOTXY_INV_FIRST].X + 64,
				    InvRect[j + SLOTXY_INV_FIRST].Y + 159,
				    (BYTE *)pCursCels, frame, frame_width, 0, 8, 1);
			}
		}
	}
}
// 4B8CB8: using guessed type char pcursinvitem;
// 69BEF8: using guessed type int light_table_index;
// 69CF94: using guessed type int cel_transparency_active;
// 41B8C4: using guessed type int var_A0[40];

void DrawInvBelt()
{
	int i, frame, frame_width, colour;
	BYTE fi, ff;

	if (talkflag) {
		return;
	}

	DrawPanelBox(205, 21, 232, 28, 269, 517);

	for (i = 0; i < MAXBELTITEMS; i++) {
		if (plr[myplr].SpdList[i]._itype == ITYPE_NONE) {
			continue;
		}

		InvDrawSlotBack(InvRect[i + 65].X + 64, InvRect[i + 65].Y + 159, 28, 28);
		frame = plr[myplr].SpdList[i]._iCurs + CURSOR_FIRSTITEM;
		frame_width = InvItemWidth[frame];

		if (pcursinvitem == i + 47) {
			colour = ICOL_WHITE;
			if (plr[myplr].SpdList[i]._iMagical)
				colour = ICOL_BLUE;
			if (!plr[myplr].SpdList[i]._iStatFlag)
				colour = ICOL_RED;
			CelDecodeClr(colour, InvRect[i + 65].X + 64, InvRect[i + 65].Y + 159, (BYTE *)pCursCels, frame, frame_width, 0, 8);
		}

		if (plr[myplr].SpdList[i]._iStatFlag)
			CelDrawHdrOnly(InvRect[i + 65].X + 64, InvRect[i + 65].Y + 159, (BYTE *)pCursCels, frame, frame_width, 0, 8);
		else
			CelDrawHdrLightRed(InvRect[i + 65].X + 64, InvRect[i + 65].Y + 159, (BYTE *)pCursCels, frame, frame_width, 0, 8, 1);

		if (AllItemsList[plr[myplr].SpdList[i].IDidx].iUsable
		    && plr[myplr].SpdList[i]._iStatFlag
		    && plr[myplr].SpdList[i]._itype != ITYPE_GOLD) {
			fi = i + 49;
			ff = fontframe[gbFontTransTbl[fi]];
			CPrintString(InvRect[i + 65].X + 64 + PitchTbl[InvRect[i + 65].Y + 159] - fontkern[ff] + 28, ff, 0);
		}
	}
}
// 4B8960: using guessed type int talkflag;
// 4B8CB8: using guessed type char pcursinvitem;

BOOL AutoPlace(int pnum, int ii, int sx, int sy, BOOL saveflag)
{
	int i, j, xx, yy;
	BOOL done;

	done = TRUE;
	yy = 10 * (ii / 10);
	if (yy < 0) {
		yy = 0;
	}
	for (j = 0; j < sy && done; j++) {
		if (yy >= 40) {
			done = FALSE;
		}
		xx = ii % 10;
		if (xx < 0) {
			xx = 0;
		}
		for (i = 0; i < sx && done; i++) {
			if (xx < 10) {
				done = plr[pnum].InvGrid[xx + yy] == 0;
			} else {
				done = FALSE;
			}
			xx++;
		}
		yy += 10;
	}
	if (done && saveflag) {
		plr[pnum].InvList[plr[pnum]._pNumInv] = plr[pnum].HoldItem;
		plr[pnum]._pNumInv++;
		yy = 10 * (ii / 10);
		if (yy < 0) {
			yy = 0;
		}
		for (j = 0; j < sy; j++) {
			xx = ii % 10;
			if (xx < 0) {
				xx = 0;
			}
			for (i = 0; i < sx; i++) {
				if (i != 0 || j != sy - 1) {
					plr[pnum].InvGrid[xx + yy] = -plr[pnum]._pNumInv;
				} else {
					plr[pnum].InvGrid[xx + yy] = plr[pnum]._pNumInv;
				}
				xx++;
			}
			yy += 10;
		}
		CalcPlrScrolls(pnum);
	}
	return done;
}

BOOL SpecialAutoPlace(int pnum, int ii, int sx, int sy, BOOL saveflag)
{
	int i, j, xx, yy;
	BOOL done;

	done = TRUE;
	yy = 10 * (ii / 10);
	if (yy < 0) {
		yy = 0;
	}
	for (j = 0; j < sy && done; j++) {
		if (yy >= 40) {
			done = FALSE;
		}
		xx = ii % 10;
		if (xx < 0) {
			xx = 0;
		}
		for (i = 0; i < sx && done; i++) {
			if (xx < 10) {
				done = plr[pnum].InvGrid[xx + yy] == 0;
			} else {
				done = FALSE;
			}
			xx++;
		}
		yy += 10;
	}
	if (!done) {
		if (sx <= 1 && sy <= 1) {
			for (i = 0; i < MAXBELTITEMS; i++) {
				if (plr[pnum].SpdList[i]._itype == ITYPE_NONE) {
					done = TRUE;
					break;
				}
			}
		} else {
			done = FALSE;
		}
	}
	if (done && saveflag) {
		plr[pnum].InvList[plr[pnum]._pNumInv] = plr[pnum].HoldItem;
		plr[pnum]._pNumInv++;
		yy = 10 * (ii / 10);
		if (yy < 0) {
			yy = 0;
		}
		for (j = 0; j < sy; j++) {
			xx = ii % 10;
			if (xx < 0) {
				xx = 0;
			}
			for (i = 0; i < sx; i++) {
				if (i != 0 || j != sy - 1) {
					plr[pnum].InvGrid[xx + yy] = -plr[pnum]._pNumInv;
				} else {
					plr[pnum].InvGrid[xx + yy] = plr[pnum]._pNumInv;
				}
				xx++;
			}
			yy += 10;
		}
		CalcPlrScrolls(pnum);
	}
	return done;
}

BOOL GoldAutoPlace(int pnum)
{
	BOOL done;
	int i, ii;
	int xx, yy;

	done = FALSE;
	for (i = 0; i < plr[pnum]._pNumInv && !done; i++) {
		if (plr[pnum].InvList[i]._itype == ITYPE_GOLD) {
			if (plr[pnum].HoldItem._ivalue + plr[pnum].InvList[i]._ivalue <= 5000) {
				plr[pnum].InvList[i]._ivalue = plr[pnum].HoldItem._ivalue + plr[pnum].InvList[i]._ivalue;
				if (plr[pnum].InvList[i]._ivalue >= 2500)
					plr[pnum].InvList[i]._iCurs = ICURS_GOLD_LARGE;
				else if (plr[pnum].InvList[i]._ivalue <= 1000)
					plr[pnum].InvList[i]._iCurs = ICURS_GOLD_SMALL;
				else
					plr[pnum].InvList[i]._iCurs = ICURS_GOLD_MEDIUM;
				plr[pnum]._pGold = CalculateGold(pnum);
				done = TRUE;
			}
		}
	}
	if (done)
		return done;

	for (i = 0; i < plr[pnum]._pNumInv && !done; i++) {
		if (plr[pnum].InvList[i]._itype == ITYPE_GOLD && plr[pnum].InvList[i]._ivalue < 5000) {
			if (plr[pnum].HoldItem._ivalue + plr[pnum].InvList[i]._ivalue <= 5000) {
				plr[pnum].InvList[i]._ivalue = plr[pnum].HoldItem._ivalue + plr[pnum].InvList[i]._ivalue;
				if (plr[pnum].InvList[i]._ivalue >= 2500)
					plr[pnum].InvList[i]._iCurs = ICURS_GOLD_LARGE;
				else if (plr[pnum].InvList[i]._ivalue <= 1000)
					plr[pnum].InvList[i]._iCurs = ICURS_GOLD_SMALL;
				else
					plr[pnum].InvList[i]._iCurs = ICURS_GOLD_MEDIUM;
				plr[pnum]._pGold = CalculateGold(pnum);
				done = TRUE;
			}
		}
	}
	if (done)
		return done;

	for (i = 39; i >= 0 && !done; i--) {
		yy = 10 * (i / 10);
		xx = i % 10;
		if (!plr[pnum].InvGrid[xx + yy]) {
			ii = plr[pnum]._pNumInv;
			plr[pnum].InvList[ii] = plr[pnum].HoldItem;
			plr[pnum]._pNumInv = plr[pnum]._pNumInv + 1;
			plr[pnum].InvGrid[xx + yy] = plr[pnum]._pNumInv;
			if (plr[pnum].HoldItem._ivalue >= 2500)
				plr[pnum].InvList[ii]._iCurs = ICURS_GOLD_LARGE;
			else if (plr[pnum].HoldItem._ivalue <= 1000)
				plr[pnum].InvList[ii]._iCurs = ICURS_GOLD_SMALL;
			else
				plr[pnum].InvList[ii]._iCurs = ICURS_GOLD_MEDIUM;
			plr[pnum]._pGold = CalculateGold(pnum);
			done = TRUE;
		}
	}

	return done;
}

int WeaponAutoPlace(int pnum)
{
	if (plr[pnum].HoldItem._iLoc != ILOC_TWOHAND) {
		if (plr[pnum].InvBody[INVLOC_HAND_LEFT]._itype != ITYPE_NONE && plr[pnum].InvBody[INVLOC_HAND_LEFT]._iClass == ICLASS_WEAPON)
			return FALSE;
		if (plr[pnum].InvBody[INVLOC_HAND_RIGHT]._itype != ITYPE_NONE && plr[pnum].InvBody[INVLOC_HAND_RIGHT]._iClass == ICLASS_WEAPON)
			return FALSE;
		if (plr[pnum].InvBody[INVLOC_HAND_LEFT]._itype == ITYPE_NONE) {
			NetSendCmdChItem(TRUE, INVLOC_HAND_LEFT);
			plr[pnum].InvBody[INVLOC_HAND_LEFT] = plr[pnum].HoldItem;
			return TRUE;
		}
		if (plr[pnum].InvBody[INVLOC_HAND_RIGHT]._itype == ITYPE_NONE && plr[pnum].InvBody[INVLOC_HAND_LEFT]._iLoc != ILOC_TWOHAND) {
			NetSendCmdChItem(TRUE, INVLOC_HAND_RIGHT);
			plr[pnum].InvBody[INVLOC_HAND_RIGHT] = plr[pnum].HoldItem;
			return TRUE;
		}
	} else if (plr[pnum].InvBody[INVLOC_HAND_LEFT]._itype == ITYPE_NONE && plr[pnum].InvBody[INVLOC_HAND_RIGHT]._itype == ITYPE_NONE) {
		NetSendCmdChItem(TRUE, INVLOC_HAND_LEFT);
		plr[pnum].InvBody[INVLOC_HAND_LEFT] = plr[pnum].HoldItem;
		return TRUE;
	}

	return FALSE;
}

int SwapItem(ItemStruct *a, ItemStruct *b)
{
	ItemStruct h;

	h = *a;
	*a = *b;
	*b = h;

	return h._iCurs + CURSOR_FIRSTITEM;
}

void CheckInvPaste(int pnum, int mx, int my)
{
	int v3;              // ebx
	int v4;              // edi
	int v5;              // eax
	int v6;              // esi
	signed int v7;       // edi
	int v8;              // edx
	int v9;              // edx
	signed int v10;      // edi
	char v11;            // al
	signed int v12;      // ecx
	int v13;             // eax
	int v14;             // eax
	char *v15;           // edi
	int v16;             // esi
	int v17;             // ecx
	int v18;             // edx
	int v21;             // esi
	ItemStruct *v22;     // edi
	ItemStruct *v23;     // ecx
	int v24;             // eax
	int v25;             // eax
	int v26;             // edx
	ItemStruct *v27;     // esi
	int v28;             // eax
	int v29;             // ecx
	int v30;             // esi
	int v31;             // eax
	int v32;             // eax
	int v33;             // ecx
	int v34;             // eax
	int v35;             // ecx
	char *v36;           // eax
	int v37;             // edx
	int v38;             // ecx
	int v39;             // edi
	int v40;             // esi
	int v41;             // ebx
	int v42;             // edx
	int v43;             // eax
	int v44;             // eax
	signed int v45;      // ecx
	int v46;             // edx
	char *v47;           // eax
	int v48;             // edi
	int v49;             // eax
	int v50;             // ecx
	char *v51;           // esi
	char v52;            // cl
	int v53;             // ecx
	int v54;             // eax
	int v55;             // edi
	int v56;             // edx
	int v57;             // esi
	int v58;             // ebx
	int v59;             // eax
	int v60;             // esi
	ItemStruct tempitem; // [esp+Ch] [ebp-190h]
	int v62;             // [esp+17Ch] [ebp-20h]
	int p;               // [esp+180h] [ebp-1Ch]
	int v64;             // [esp+184h] [ebp-18h]
	int v65;             // [esp+188h] [ebp-14h]
	int v66;             // [esp+18Ch] [ebp-10h]
	int v67;             // [esp+190h] [ebp-Ch]
	int v68;             // [esp+194h] [ebp-8h]
	int v69;             // [esp+198h] [ebp-4h]
	int cursor_id;       // [esp+1A4h] [ebp+8h]
	int cursor_ida;      // [esp+1A4h] [ebp+8h]

	p = pnum;
	v3 = pnum;
	v4 = mx;
	SetICursor(plr[pnum].HoldItem._iCurs + CURSOR_FIRSTITEM);
	v5 = my + (icursH >> 1);
	v6 = v4 + (icursW >> 1);
	v64 = icursW28;
	v7 = 0;
	v67 = icursH28;
	v68 = 0;
	do {
		if (v7)
			goto LABEL_18;
		v8 = InvRect[v68].X;
		if (v6 >= v8 && v6 < v8 + 28) {
			v9 = InvRect[v68].Y;
			if (v5 >= v9 - 29 && v5 < v9) {
				v7 = 1;
				--v68;
			}
		}
		if (v68 != 24)
			goto LABEL_13;
		if (!(v64 & 1))
			v6 -= 14;
		if (!(v67 & 1)) {
			v5 -= 14;
		LABEL_13:
			if (v68 == 64 && !(v67 & 1))
				v5 += 14;
		}
		++v68;
	} while ((unsigned int)v68 < 0x49);
	if (!v7)
		return;
LABEL_18:
	v10 = v68;
	v69 = ILOC_UNEQUIPABLE;
	if (v68 >= 0 && v68 <= ILOC_ARMOR)
		v69 = ILOC_HELM;
	if (v68 >= ILOC_HELM && v68 <= ILOC_RING)
		v69 = ILOC_RING;
	if (v68 == ILOC_AMULET)
		v69 = ILOC_AMULET;
	if (v68 >= ILOC_UNEQUIPABLE && v68 <= 18)
		v69 = ILOC_ONEHAND;
	if (v68 >= 19 && v68 <= 24)
		v69 = ILOC_ARMOR;
	if (v68 >= 65 && v68 <= 72)
		v69 = ILOC_BELT;
	v11 = plr[v3].HoldItem._iLoc;
	v12 = 0;
	if ((char)v11 == v69)
		v12 = 1;
	if (v69 == 1 && v11 == ILOC_TWOHAND) {
		v69 = ILOC_TWOHAND;
		v12 = 1;
	}
	if (v11 != 7 || v69 != ILOC_BELT) {
	LABEL_50:
		if (v69 != ILOC_UNEQUIPABLE)
			goto LABEL_81;
		v66 = 0;
		cursor_id = 1;
		v13 = (v68 - 25) / 10;
		if (plr[v3].HoldItem._itype == ITYPE_GOLD) {
			_LOBYTE(v13) = plr[0].InvGrid[10 * v13 + v3 * 21720 + (v68 - 25) % 10];
			if (!(_BYTE)v13)
				goto LABEL_93;
			v13 = (char)v13;
			if ((char)v13 <= 0) {
				v13 = -v13;
			} else if (*(int *)((char *)&plr[0].InvBody[v13 + 6]._itype + v3 * 21720) == ITYPE_GOLD) {
				goto LABEL_93;
			}
			v66 = v13;
		LABEL_93:
			v21 = p;
			if (p == myplr) {
				PlaySFX(ItemInvSnds[ItemCAnimTbl[plr[v3].HoldItem._iCurs]]);
				v10 = v68;
			}
			cursor_ida = 1;
			switch (v69) {
			case ILOC_ONEHAND:
				if (v10 > 12) {
					if (plr[v3].InvBody[INVLOC_HAND_RIGHT]._itype == ITYPE_NONE) {
						v25 = plr[v3].InvBody[INVLOC_HAND_LEFT]._itype;
						if (v25 == ITYPE_NONE)
							goto LABEL_232;
						if (plr[v3].InvBody[INVLOC_HAND_LEFT]._iLoc == ILOC_TWOHAND) {
							NetSendCmdDelItem(FALSE, 4u);
							NetSendCmdChItem(FALSE, 5u);
							SwapItem(&plr[v3].InvBody[INVLOC_HAND_RIGHT], &plr[v3].InvBody[INVLOC_HAND_LEFT]);
							v23 = &plr[v3].InvBody[INVLOC_HAND_RIGHT];
						LABEL_99:
							v24 = SwapItem(v23, &plr[v3].HoldItem);
						LABEL_172:
							cursor_ida = v24;
							goto LABEL_226;
						}
						if (v25 == ITYPE_NONE || plr[v3].InvBody[INVLOC_HAND_LEFT]._iClass != plr[v3].HoldItem._iClass) {
						LABEL_232:
							NetSendCmdChItem(FALSE, 5u);
							v22 = &plr[v3].InvBody[INVLOC_HAND_RIGHT];
						LABEL_158:
							qmemcpy(v22, &plr[v3].HoldItem, sizeof(ItemStruct));
							goto LABEL_226;
						}
					} else if (plr[v3].InvBody[INVLOC_HAND_LEFT]._itype == ITYPE_NONE
					    || plr[v3].InvBody[INVLOC_HAND_LEFT]._iClass != plr[v3].HoldItem._iClass) {
						goto LABEL_114;
					}
				} else {
					if (plr[v3].InvBody[INVLOC_HAND_LEFT]._itype == ITYPE_NONE) {
						if (plr[v3].InvBody[INVLOC_HAND_RIGHT]._itype != ITYPE_NONE
						    && plr[v3].InvBody[INVLOC_HAND_RIGHT]._iClass == plr[v3].HoldItem._iClass) {
						LABEL_114:
							NetSendCmdChItem(FALSE, 5u);
							v23 = &plr[v3].InvBody[INVLOC_HAND_RIGHT];
							goto LABEL_99;
						}
						NetSendCmdChItem(FALSE, 4u);
						v22 = &plr[v3].InvBody[INVLOC_HAND_LEFT];
						goto LABEL_158;
					}
					if (plr[v3].InvBody[INVLOC_HAND_RIGHT]._itype != ITYPE_NONE
					    && plr[v3].InvBody[INVLOC_HAND_RIGHT]._iClass == plr[v3].HoldItem._iClass) {
						goto LABEL_114;
					}
				}
				NetSendCmdChItem(FALSE, 4u);
				v23 = &plr[v3].InvBody[INVLOC_HAND_LEFT];
				goto LABEL_99;
			case ILOC_TWOHAND:
				NetSendCmdDelItem(FALSE, 5u);
				if (plr[v3].InvBody[INVLOC_HAND_LEFT]._itype == ITYPE_NONE)
					goto LABEL_147;
				v26 = plr[v3].InvBody[INVLOC_HAND_RIGHT]._itype;
				if (v26 == -1)
					goto LABEL_146;
				qmemcpy(&tempitem, &plr[v3].HoldItem, sizeof(tempitem));
				v27 = &plr[v3].InvBody[INVLOC_HAND_RIGHT];
				if (v26 != ITYPE_SHIELD)
					v27 = &plr[v3].InvBody[INVLOC_HAND_LEFT];
				v28 = p;
				qmemcpy(&plr[v3].HoldItem, v27, sizeof(plr[v3].HoldItem));
				v29 = plr[v3].HoldItem._iCurs + CURSOR_FIRSTITEM;
				if (v28 == myplr)
					SetCursor_(v29);
				else
					SetICursor(v29);
				v67 = 0;
				v30 = 0;
				do {
					if (v67)
						break;
					v31 = AutoPlace(p, v30++, icursW28, icursH28, 1);
					v67 = v31;
				} while (v30 < 40);
				v32 = p;
				qmemcpy(&plr[v3].HoldItem, &tempitem, sizeof(plr[v3].HoldItem));
				v33 = plr[v3].HoldItem._iCurs + CURSOR_FIRSTITEM;
				if (v32 == myplr)
					SetCursor_(v33);
				else
					SetICursor(v33);
				if (!v67)
					return;
				if (plr[v3].InvBody[INVLOC_HAND_RIGHT]._itype == ITYPE_SHIELD)
					plr[v3].InvBody[INVLOC_HAND_RIGHT]._itype = ITYPE_NONE;
				else
					plr[v3].InvBody[INVLOC_HAND_LEFT]._itype = ITYPE_NONE;
			LABEL_146:
				if (plr[v3].InvBody[INVLOC_HAND_LEFT]._itype != ITYPE_NONE)
					goto LABEL_149;
			LABEL_147:
				if (plr[v3].InvBody[INVLOC_HAND_RIGHT]._itype == ITYPE_NONE) {
					NetSendCmdChItem(FALSE, 4u);
					qmemcpy(&plr[v3].InvBody[INVLOC_HAND_LEFT], &plr[v3].HoldItem, sizeof(plr[v3].InvBody[INVLOC_HAND_LEFT]));
				} else {
				LABEL_149:
					NetSendCmdChItem(FALSE, 4u);
					if (plr[v3].InvBody[INVLOC_HAND_LEFT]._itype == ITYPE_NONE)
						SwapItem(&plr[v3].InvBody[INVLOC_HAND_LEFT], &plr[v3].InvBody[INVLOC_HAND_RIGHT]);
					cursor_ida = SwapItem(&plr[v3].InvBody[INVLOC_HAND_LEFT], &plr[v3].HoldItem);
				}
				if (plr[v3].InvBody[INVLOC_HAND_LEFT]._itype == ITYPE_STAFF) {
					v34 = plr[v3].InvBody[INVLOC_HAND_LEFT]._iSpell;
					if (v34) {
						if (plr[v3].InvBody[INVLOC_HAND_LEFT]._iCharges > 0) {
							plr[v3]._pRSpell = v34;
							_LOBYTE(plr[v3]._pRSplType) = RSPLTYPE_CHARGES;
							drawpanflag = 255;
						}
					}
				}
				goto LABEL_226;
			case ILOC_ARMOR:
				NetSendCmdChItem(FALSE, 6u);
				if (plr[v3].InvBody[INVLOC_CHEST]._itype == ITYPE_NONE) {
					v22 = &plr[v3].InvBody[INVLOC_CHEST];
					goto LABEL_158;
				}
				v23 = &plr[v3].InvBody[INVLOC_CHEST];
				goto LABEL_99;
			case ILOC_HELM:
				NetSendCmdChItem(FALSE, 0);
				if (plr[v3].InvBody[INVLOC_HEAD]._itype == ITYPE_NONE) {
					v22 = plr[v3].InvBody;
					goto LABEL_158;
				}
				v23 = plr[v3].InvBody;
				goto LABEL_99;
			case ILOC_RING:
				if (v10 == 4) {
					NetSendCmdChItem(FALSE, 1u);
					if (plr[v3].InvBody[INVLOC_RING_LEFT]._itype == ITYPE_NONE) {
						v22 = &plr[v3].InvBody[INVLOC_RING_LEFT];
						goto LABEL_158;
					}
					v23 = &plr[v3].InvBody[INVLOC_RING_LEFT];
				} else {
					NetSendCmdChItem(FALSE, 2u);
					if (plr[v3].InvBody[INVLOC_RING_RIGHT]._itype == ITYPE_NONE) {
						v22 = &plr[v3].InvBody[INVLOC_RING_RIGHT];
						goto LABEL_158;
					}
					v23 = &plr[v3].InvBody[INVLOC_RING_RIGHT];
				}
				goto LABEL_99;
			case ILOC_AMULET:
				NetSendCmdChItem(FALSE, 3u);
				if (plr[v3].InvBody[INVLOC_AMULET]._itype == ITYPE_NONE) {
					v22 = &plr[v3].InvBody[INVLOC_AMULET];
					goto LABEL_158;
				}
				v23 = &plr[v3].InvBody[INVLOC_AMULET];
				goto LABEL_99;
			case ILOC_UNEQUIPABLE:
				v35 = plr[v3].HoldItem._itype;
				if (v35 == ITYPE_GOLD) {
					if (!v66) {
						v36 = &plr[0].InvGrid[10 * ((v68 - 25) / 10) + v3 * 21720 + (v68 - 25) % 10];
						if (*v36 <= 0) {
							v42 = 368 * plr[v3]._pNumInv + v3 * 21720;
							qmemcpy((char *)plr[0].InvList + v42, &plr[v3].HoldItem, 0x170u);
							++plr[v3]._pNumInv;
							*v36 = plr[v3]._pNumInv;
							v43 = plr[v3].HoldItem._ivalue;
							plr[v3]._pGold += v43;
							if (v43 <= 5000) {
								if (v43 < 2500) {
									if (v43 > 1000)
										*(int *)((char *)&plr[0].InvList[0]._iCurs + v42) = 5;
									else
										*(int *)((char *)&plr[0].InvList[0]._iCurs + v42) = 4;
								} else {
									*(int *)((char *)&plr[0].InvList[0]._iCurs + v42) = 6;
								}
							}
							goto LABEL_226;
						}
						v37 = plr[v3].HoldItem._ivalue;
						v38 = 368 * (*v36 - 1) + v3 * 21720;
						v39 = *(int *)((char *)&plr[0].InvList[0]._ivalue + v38);
						v40 = v37 + v39;
						if (v37 + v39 <= 5000) {
							*(int *)((char *)&plr[0].InvList[0]._ivalue + v38) = v40;
							plr[v3]._pGold += plr[v3].HoldItem._ivalue;
							if (v40 < 2500) {
								if (v40 > 1000)
									*(int *)((char *)&plr[0].InvList[0]._iCurs + v38) = 5;
								else
									*(int *)((char *)&plr[0].InvList[0]._iCurs + v38) = 4;
							} else {
								*(int *)((char *)&plr[0].InvList[0]._iCurs + v38) = 6;
							}
							goto LABEL_226;
						}
						plr[v3]._pGold += 5000 - v39;
						plr[v3].HoldItem._ivalue = v37 - (5000 - v39);
						*(int *)((char *)&plr[0].InvList[0]._ivalue + v38) = 5000;
						*(int *)((char *)&plr[0].InvList[0]._iCurs + v38) = 6;
						v41 = plr[v3].HoldItem._ivalue;
						if (v41 >= 2500) {
							cursor_ida = 18;
							goto LABEL_226;
						}
						v24 = (v41 > 1000) + 16;
						goto LABEL_172;
					}
				} else if (!v66) {
					qmemcpy((char *)&plr[0].InvList[plr[v3]._pNumInv++] + v3 * 21720, &plr[v3].HoldItem, 0x170u);
					v66 = plr[v3]._pNumInv;
				LABEL_191:
					v48 = v67;
					v49 = 10 * ((v68 - 25) / 10 - ((v67 - 1) >> 1));
					if (v49 < 0)
						v49 = 0;
					v65 = 0;
					if (v67 > 0) {
						v69 = (v68 - 25) % 10 - ((v64 - 1) >> 1);
						do {
							v50 = v69;
							if (v69 < 0)
								v50 = 0;
							v67 = 0;
							if (v64 > 0) {
								v51 = &plr[v3].InvGrid[v50 + v49];
								do {
									if (v67 || v65 != v48 - 1)
										v52 = -(char)v66;
									else
										v52 = v66;
									*v51++ = v52;
									++v67;
								} while (v67 < v64);
							}
							v49 += 10;
							++v65;
						} while (v65 < v48);
					}
					goto LABEL_226;
				}
				v44 = v66 - 1;
				if (v35 == ITYPE_GOLD)
					plr[v3]._pGold += plr[v3].HoldItem._ivalue;
				cursor_ida = SwapItem((ItemStruct *)((char *)&plr[0].InvList[v44] + v3 * 21720), &plr[v3].HoldItem);
				if (plr[v3].HoldItem._itype == ITYPE_GOLD)
					plr[v3]._pGold = CalculateGold(v21);
				v45 = 0;
				v46 = -v66;
				do {
					v47 = &plr[v3].InvGrid[v45];
					if (*v47 == v66)
						*v47 = 0;
					if (*v47 == v46)
						*v47 = 0;
					++v45;
				} while (v45 < 40);
				goto LABEL_191;
			case ILOC_BELT:
				v53 = v3 * 21720 + 368 * (v68 - 65);
				if (plr[v3].HoldItem._itype != ITYPE_GOLD) {
					if (*(int *)((char *)&plr[0].SpdList[0]._itype + v53) == ITYPE_NONE) {
						qmemcpy((char *)plr[0].SpdList + v53, &plr[v3].HoldItem, 0x170u);
					} else {
						cursor_ida = SwapItem((ItemStruct *)((char *)plr[0].SpdList + v53), &plr[v3].HoldItem);
						if (plr[v3].HoldItem._itype == ITYPE_GOLD)
							plr[v3]._pGold = CalculateGold(p);
					}
					goto LABEL_225;
				}
				v54 = *(int *)((char *)&plr[0].SpdList[0]._itype + v53);
				if (v54 != ITYPE_NONE) {
					if (v54 == ITYPE_GOLD) {
						v55 = *(int *)((char *)&plr[0].SpdList[0]._ivalue + v53);
						v56 = plr[v3].HoldItem._ivalue;
						v57 = v55 + v56;
						if (v55 + v56 <= 5000) {
							*(int *)((char *)&plr[0].SpdList[0]._ivalue + v53) = v57;
							plr[v3]._pGold += plr[v3].HoldItem._ivalue;
							if (v57 < 2500) {
								if (v57 > 1000)
									*(int *)((char *)&plr[0].SpdList[0]._iCurs + v53) = 5;
								else
									*(int *)((char *)&plr[0].SpdList[0]._iCurs + v53) = 4;
							} else {
								*(int *)((char *)&plr[0].SpdList[0]._iCurs + v53) = 6;
							}
							goto LABEL_225;
						}
						plr[v3]._pGold += 5000 - v55;
						plr[v3].HoldItem._ivalue = v56 - (5000 - v55);
						*(int *)((char *)&plr[0].SpdList[0]._ivalue + v53) = 5000;
						*(int *)((char *)&plr[0].SpdList[0]._iCurs + v53) = 6;
						v58 = plr[v3].HoldItem._ivalue;
						if (v58 >= 2500) {
							cursor_ida = 18;
							goto LABEL_225;
						}
						v59 = (v58 > 1000) + 16;
					} else {
						plr[v3]._pGold += plr[v3].HoldItem._ivalue;
						v59 = SwapItem((ItemStruct *)((char *)plr[0].SpdList + v53), &plr[v3].HoldItem);
					}
					cursor_ida = v59;
					goto LABEL_225;
				}
				qmemcpy((char *)plr[0].SpdList + v53, &plr[v3].HoldItem, 0x170u);
				plr[v3]._pGold += plr[v3].HoldItem._ivalue;
			LABEL_225:
				drawsbarflag = TRUE;
			LABEL_226:
				v60 = p;
				CalcPlrInv(p, 1u);
				if (v60 == myplr) {
					if (cursor_ida == 1)
						SetCursorPos(MouseX + (cursW >> 1), MouseY + (cursH >> 1));
					SetCursor_(cursor_ida);
				}
				return;
			default:
				goto LABEL_226;
			}
		}
		v62 = (v68 - 25) % 10;
		v14 = 10 * (v13 - ((v67 - 1) >> 1));
		if (v14 < 0)
			v14 = 0;
		v65 = 0;
		if (v67 <= 0)
			goto LABEL_93;
		v15 = &plr[v3].InvGrid[v14];
		while (1) {
			if (cursor_id == CURSOR_NONE)
				return;
			if (v14 >= 40)
				cursor_id = 0;
			v16 = v62 - ((v64 - 1) >> 1);
			if (v16 < 0)
				v16 = 0;
			v17 = 0;
			if (v64 > 0)
				break;
		LABEL_79:
			v14 += 10;
			v15 += 10;
			if (++v65 >= v67) {
				v12 = cursor_id;
				v10 = v68;
				goto LABEL_81;
			}
		}
		while (1) {
			if (cursor_id == CURSOR_NONE)
				goto LABEL_79;
			if (v16 >= 10)
				goto LABEL_233;
			_LOBYTE(v18) = v15[v16];
			if ((_BYTE)v18) {
				v18 = (char)v18;
				if ((v18 & 0x80u) != 0)
					v18 = -v18;
				if (!v66) {
					v66 = v18;
					goto LABEL_78;
				}
				if (v66 != v18)
				LABEL_233:
					cursor_id = 0;
			}
		LABEL_78:
			++v16;
			if (++v17 >= v64)
				goto LABEL_79;
		}
	}
	if (v64 == 1 && v67 == 1) {
		v12 = 1;
		if (!AllItemsList[plr[v3].HoldItem.IDidx].iUsable)
			v12 = 0;
		if (!plr[v3].HoldItem._iStatFlag)
			v12 = 0;
		if (plr[v3].HoldItem._itype == ITYPE_GOLD) {
			v12 = 0;
			goto LABEL_50;
		}
	}
LABEL_81:
	if (!v12)
		return;
	if (v69 == ILOC_UNEQUIPABLE || v69 == ILOC_BELT || plr[v3].HoldItem._iStatFlag) {
		goto LABEL_93;
	}
	if (plr[v3]._pClass == PC_WARRIOR) {
		PlaySFX(PS_WARR13);
	} else if (plr[v3]._pClass == PC_ROGUE) {
		PlaySFX(PS_ROGUE13);
	} else if (plr[v3]._pClass == PC_SORCERER) {
		PlaySFX(PS_MAGE13);
	}
}
// 4B8C9C: using guessed type int cursH;
// 4B8CB4: using guessed type int icursH;
// 4B8CBC: using guessed type int icursW;
// 52571C: using guessed type int drawpanflag;

void CheckInvSwap(int pnum, BYTE bLoc, int idx, WORD wCI, int seed, BOOL bId)
{
	PlayerStruct *p;

	RecreateItem(MAXITEMS, idx, wCI, seed, 0);

	p = &plr[pnum];
	p->HoldItem = item[MAXITEMS];

	if (bId) {
		p->HoldItem._iIdentified = TRUE;
	}

	if (bLoc < NUM_INVLOC) {
		p->InvBody[bLoc] = p->HoldItem;

		if (bLoc == INVLOC_HAND_LEFT && p->HoldItem._iLoc == ILOC_TWOHAND) {
			p->InvBody[INVLOC_HAND_RIGHT]._itype = ITYPE_NONE;
		} else if (bLoc == INVLOC_HAND_RIGHT && p->HoldItem._iLoc == ILOC_TWOHAND) {
			p->InvBody[INVLOC_HAND_LEFT]._itype = ITYPE_NONE;
		}
	}

	CalcPlrInv(pnum, TRUE);
}

void CheckInvCut(int pnum, int mx, int my)
{
	int r;
	BOOL done;
	char ii;
	int iv, i, j, offs;

	if (plr[pnum]._pmode > PM_WALK3) {
		return;
	}

	if (dropGoldFlag) {
		dropGoldFlag = FALSE;
		dropGoldValue = 0;
	}

	done = FALSE;

	// TODO: this loop is compiled differently (via InvRect pointers)
	for (r = 0; (DWORD)r < NUM_XY_SLOTS && !done; r++) {
		// check which inventory rectangle the mouse is in, if any
		if (mx >= InvRect[r].X
		    && mx < InvRect[r].X + (INV_SLOT_SIZE_PX + 1)
		    && my >= InvRect[r].Y - (INV_SLOT_SIZE_PX + 1)
		    && my < InvRect[r].Y) {
			done = TRUE;
			r--;
		}
	}

	if (!done) {
		// not on an inventory slot rectangle
		return;
	}

	plr[pnum].HoldItem._itype = ITYPE_NONE;

	if (
	    r >= SLOTXY_HEAD_FIRST
	    && r <= SLOTXY_HEAD_LAST
	    && plr[pnum].InvBody[INVLOC_HEAD]._itype != ITYPE_NONE) {
		NetSendCmdDelItem(FALSE, INVLOC_HEAD);
		plr[pnum].HoldItem = plr[pnum].InvBody[INVLOC_HEAD];
		plr[pnum].InvBody[INVLOC_HEAD]._itype = ITYPE_NONE;
	}

	if (
	    r == SLOTXY_RING_LEFT
	    && plr[pnum].InvBody[INVLOC_RING_LEFT]._itype != ITYPE_NONE) {
		NetSendCmdDelItem(FALSE, INVLOC_RING_LEFT);
		plr[pnum].HoldItem = plr[pnum].InvBody[INVLOC_RING_LEFT];
		plr[pnum].InvBody[INVLOC_RING_LEFT]._itype = ITYPE_NONE;
	}

	if (
	    r == SLOTXY_RING_RIGHT
	    && plr[pnum].InvBody[INVLOC_RING_RIGHT]._itype != ITYPE_NONE) {
		NetSendCmdDelItem(FALSE, INVLOC_RING_RIGHT);
		plr[pnum].HoldItem = plr[pnum].InvBody[INVLOC_RING_RIGHT];
		plr[pnum].InvBody[INVLOC_RING_RIGHT]._itype = ITYPE_NONE;
	}

	if (
	    r == SLOTXY_AMULET
	    && plr[pnum].InvBody[INVLOC_AMULET]._itype != ITYPE_NONE) {
		NetSendCmdDelItem(FALSE, INVLOC_AMULET);
		plr[pnum].HoldItem = plr[pnum].InvBody[INVLOC_AMULET];
		plr[pnum].InvBody[INVLOC_AMULET]._itype = ITYPE_NONE;
	}

	if (
	    r >= SLOTXY_HAND_LEFT_FIRST
	    && r <= SLOTXY_HAND_LEFT_LAST
	    && plr[pnum].InvBody[INVLOC_HAND_LEFT]._itype != ITYPE_NONE) {
		NetSendCmdDelItem(FALSE, INVLOC_HAND_LEFT);
		plr[pnum].HoldItem = plr[pnum].InvBody[INVLOC_HAND_LEFT];
		plr[pnum].InvBody[INVLOC_HAND_LEFT]._itype = ITYPE_NONE;
	}

	if (
	    r >= SLOTXY_HAND_RIGHT_FIRST
	    && r <= SLOTXY_HAND_RIGHT_LAST
	    && plr[pnum].InvBody[INVLOC_HAND_RIGHT]._itype != ITYPE_NONE) {
		NetSendCmdDelItem(FALSE, INVLOC_HAND_RIGHT);
		plr[pnum].HoldItem = plr[pnum].InvBody[INVLOC_HAND_RIGHT];
		plr[pnum].InvBody[INVLOC_HAND_RIGHT]._itype = ITYPE_NONE;
	}

	if (
	    r >= SLOTXY_CHEST_FIRST
	    && r <= SLOTXY_CHEST_LAST
	    && plr[pnum].InvBody[INVLOC_CHEST]._itype != ITYPE_NONE) {
		NetSendCmdDelItem(FALSE, INVLOC_CHEST);
		plr[pnum].HoldItem = plr[pnum].InvBody[INVLOC_CHEST];
		plr[pnum].InvBody[INVLOC_CHEST]._itype = ITYPE_NONE;
	}

	if (r >= SLOTXY_INV_FIRST && r <= SLOTXY_INV_LAST) {
		ii = plr[pnum].InvGrid[r - SLOTXY_INV_FIRST];
		if (ii) {
			iv = ii;
			if (ii <= 0) {
				iv = -ii;
			}

			for (i = 0; i < NUM_INV_GRID_ELEM; i++) {
				if (plr[pnum].InvGrid[i] == iv || plr[pnum].InvGrid[i] == -iv) {
					plr[pnum].InvGrid[i] = 0;
				}
			}

			iv--;

			plr[pnum].HoldItem = plr[pnum].InvList[iv];
			plr[pnum]._pNumInv--;

			if (plr[pnum]._pNumInv > 0 && plr[pnum]._pNumInv != iv) {
				plr[pnum].InvList[iv] = plr[pnum].InvList[plr[pnum]._pNumInv];

				for (j = 0; j < NUM_INV_GRID_ELEM; j++) {
					if (plr[pnum].InvGrid[j] == plr[pnum]._pNumInv + 1) {
						plr[pnum].InvGrid[j] = iv + 1;
					}
					if (plr[pnum].InvGrid[j] == -(plr[pnum]._pNumInv + 1)) {
						plr[pnum].InvGrid[j] = -iv - 1;
					}
				}
			}
		}
	}

	if (r >= SLOTXY_BELT_FIRST) {
		offs = r - SLOTXY_BELT_FIRST;
		if (plr[pnum].SpdList[offs]._itype != ITYPE_NONE) {
			plr[pnum].HoldItem = plr[pnum].SpdList[offs];
			plr[pnum].SpdList[offs]._itype = ITYPE_NONE;
			drawsbarflag = TRUE;
		}
	}

	if (plr[pnum].HoldItem._itype != ITYPE_NONE) {
		if (plr[pnum].HoldItem._itype == ITYPE_GOLD) {
			plr[pnum]._pGold = CalculateGold(pnum);
		}

		CalcPlrInv(pnum, TRUE);
		CheckItemStats(pnum);

		if (pnum == myplr) {
			PlaySFX(IS_IGRAB);
			SetCursor_(plr[pnum].HoldItem._iCurs + CURSOR_FIRSTITEM);
			SetCursorPos(mx - (cursW >> 1), MouseY - (cursH >> 1));
		}
	}
}

void inv_update_rem_item(int pnum, BYTE iv)
{
	BOOL Loadgfx;

	if (iv < NUM_INVLOC) {
		plr[pnum].InvBody[iv]._itype = ITYPE_NONE;
	}

	Loadgfx = FALSE;

	if (plr[pnum]._pmode != PM_DEATH) {
		Loadgfx = TRUE;
	}

	CalcPlrInv(pnum, Loadgfx);
}

void RemoveInvItem(int pnum, int iv)
{
	int i, j;

	iv++;

	for (i = 0; i < NUM_INV_GRID_ELEM; i++) {
		if (plr[pnum].InvGrid[i] == iv || plr[pnum].InvGrid[i] == -iv) {
			plr[pnum].InvGrid[i] = 0;
		}
	}

	iv--;
	plr[pnum]._pNumInv--;

	if (plr[pnum]._pNumInv > 0 && plr[pnum]._pNumInv != iv) {
		plr[pnum].InvList[iv] = plr[pnum].InvList[plr[pnum]._pNumInv];

		for (j = 0; j < NUM_INV_GRID_ELEM; j++) {
			if (plr[pnum].InvGrid[j] == plr[pnum]._pNumInv + 1) {
				plr[pnum].InvGrid[j] = iv + 1;
			}
			if (plr[pnum].InvGrid[j] == -(plr[pnum]._pNumInv + 1)) {
				plr[pnum].InvGrid[j] = -(iv + 1);
			}
		}
	}

	CalcPlrScrolls(pnum);

	if (plr[pnum]._pRSplType == RSPLTYPE_SCROLL) {
		if (plr[pnum]._pRSpell != SPL_INVALID) {
			// BUGFIX: Cast the literal `1` to `unsigned __int64` to make that bitshift 64bit
			// this causes the last 4 skills to not reset correctly after use
			if (!(
			        plr[pnum]._pScrlSpells
			        & (1 << (plr[pnum]._pRSpell - 1)))) {
				plr[pnum]._pRSpell = SPL_INVALID;
			}

			drawpanflag = 255;
		}
	}
}

void RemoveSpdBarItem(int pnum, int iv)
{
	plr[pnum].SpdList[iv]._itype = ITYPE_NONE;

	CalcPlrScrolls(pnum);

	if (plr[pnum]._pRSplType == RSPLTYPE_SCROLL) {
		if (plr[pnum]._pRSpell != SPL_INVALID) {
			// BUGFIX: Cast the literal `1` to `unsigned __int64` to make that bitshift 64bit
			// this causes the last 4 skills to not reset correctly after use
			if (!(
			        plr[pnum]._pScrlSpells
			        & (1 << (plr[pnum]._pRSpell - 1)))) {
				plr[pnum]._pRSpell = SPL_INVALID;
			}
		}
	}
	drawpanflag = 255;
}

void CheckInvItem()
{
	if (pcurs >= CURSOR_FIRSTITEM) {
		CheckInvPaste(myplr, MouseX, MouseY);
	} else {
		CheckInvCut(myplr, MouseX, MouseY);
	}
}

void CheckInvScrn()
{
	if (MouseX > 190 && MouseX < 437
	    && MouseY > 352 && MouseY < 385) {
		CheckInvItem();
	}
}

void CheckItemStats(int pnum)
{
	PlayerStruct *p = &plr[pnum];

	p->HoldItem._iStatFlag = FALSE;

	if (p->_pStrength >= p->HoldItem._iMinStr
	    && p->_pMagic >= p->HoldItem._iMinMag
	    && p->_pDexterity >= p->HoldItem._iMinDex) {
		p->HoldItem._iStatFlag = TRUE;
	}
}

void CheckBookLevel(int pnum)
{
	int slvl;

	if (plr[pnum].HoldItem._iMiscId == IMISC_BOOK) {
		plr[pnum].HoldItem._iMinMag = spelldata[plr[pnum].HoldItem._iSpell].sMinInt;
		slvl = plr[pnum]._pSplLvl[plr[pnum].HoldItem._iSpell];
		while (slvl) {
			plr[pnum].HoldItem._iMinMag += 20 * plr[pnum].HoldItem._iMinMag / 100;
			slvl--;
			if (plr[pnum].HoldItem._iMinMag + 20 * plr[pnum].HoldItem._iMinMag / 100 > 255) {
				plr[pnum].HoldItem._iMinMag = -1;
				slvl = 0;
			}
		}
	}
}

void CheckQuestItem(int pnum)
{
	if (plr[pnum].HoldItem.IDidx == IDI_OPTAMULET)
		quests[QTYPE_BLIND]._qactive = 3;
	if (plr[pnum].HoldItem.IDidx == IDI_MUSHROOM && quests[QTYPE_BLKM]._qactive == 2 && quests[QTYPE_BLKM]._qvar1 == QS_MUSHSPAWNED) {
		sfxdelay = 10;
		if (plr[pnum]._pClass == PC_WARRIOR) { // BUGFIX: Voice for this quest might be wrong in MP
			sfxdnum = PS_WARR95;
		} else if (plr[pnum]._pClass == PC_ROGUE) {
			sfxdnum = PS_ROGUE95;
		} else if (plr[pnum]._pClass == PC_SORCERER) {
			sfxdnum = PS_MAGE95;
		}
		quests[QTYPE_BLKM]._qvar1 = QS_MUSHPICKED;
	}
	if (plr[pnum].HoldItem.IDidx == IDI_ANVIL) {
		if (quests[QTYPE_ANVIL]._qactive == 1) {
			quests[QTYPE_ANVIL]._qactive = 2;
			quests[QTYPE_ANVIL]._qvar1 = 1;
		}
		if (quests[QTYPE_ANVIL]._qlog == 1) {
			sfxdelay = 10;
			if (plr[myplr]._pClass == PC_WARRIOR) {
				sfxdnum = PS_WARR89;
			} else if (plr[myplr]._pClass == PC_ROGUE) {
				sfxdnum = PS_ROGUE89;
			} else if (plr[myplr]._pClass == PC_SORCERER) {
				sfxdnum = PS_MAGE89;
			}
		}
	}
	if (plr[pnum].HoldItem.IDidx == IDI_GLDNELIX) {
		sfxdelay = 30;
		if (plr[myplr]._pClass == PC_WARRIOR) {
			sfxdnum = PS_WARR88;
		} else if (plr[myplr]._pClass == PC_ROGUE) {
			sfxdnum = PS_ROGUE88;
		} else if (plr[myplr]._pClass == PC_SORCERER) {
			sfxdnum = PS_MAGE88;
		}
	}
	if (plr[pnum].HoldItem.IDidx == IDI_ROCK) {
		if (quests[QTYPE_INFRA]._qactive == 1) {
			quests[QTYPE_INFRA]._qactive = 2;
			quests[QTYPE_INFRA]._qvar1 = 1;
		}
		if (quests[QTYPE_INFRA]._qlog == 1) {
			sfxdelay = 10;
			if (plr[myplr]._pClass == PC_WARRIOR) {
				sfxdnum = PS_WARR87;
			} else if (plr[myplr]._pClass == PC_ROGUE) {
				sfxdnum = PS_ROGUE87;
			} else if (plr[myplr]._pClass == PC_SORCERER) {
				sfxdnum = PS_MAGE87;
			}
		}
	}
	if (plr[pnum].HoldItem.IDidx == IDI_ARMOFVAL) {
		quests[QTYPE_BLOOD]._qactive = 3;
		sfxdelay = 20;
		if (plr[myplr]._pClass == PC_WARRIOR) {
			sfxdnum = PS_WARR91;
		} else if (plr[myplr]._pClass == PC_ROGUE) {
			sfxdnum = PS_ROGUE91;
		} else if (plr[myplr]._pClass == PC_SORCERER) {
			sfxdnum = PS_MAGE91;
		}
	}
}
// 52A554: using guessed type int sfxdelay;

void InvGetItem(int pnum, int ii)
{
	int i;

	if (dropGoldFlag) {
		dropGoldFlag = FALSE;
		dropGoldValue = 0;
	}

	if (dItem[item[ii]._ix][item[ii]._iy]) {
		if (myplr == pnum && pcurs >= CURSOR_FIRSTITEM)
			NetSendCmdPItem(TRUE, CMD_SYNCPUTITEM, plr[myplr].WorldX, plr[myplr].WorldY);
		item[ii]._iCreateInfo &= ~0x8000;
		plr[pnum].HoldItem = item[ii];
		CheckQuestItem(pnum);
		CheckBookLevel(pnum);
		CheckItemStats(pnum);
		dItem[item[ii]._ix][item[ii]._iy] = 0;
		i = 0;
		while (i < numitems) {
			if (itemactive[i] == ii) {
				DeleteItem(itemactive[i], i);
				i = 0;
			} else {
				i++;
			}
		}
		pcursitem = -1;
		SetCursor_(plr[pnum].HoldItem._iCurs + CURSOR_FIRSTITEM);
	}
}
// 4B8CC0: using guessed type char pcursitem;

void AutoGetItem(int pnum, int ii)
{
	int i, idx;
	int w, h;
	BOOL done;

	if (dropGoldFlag) {
		dropGoldFlag = FALSE;
		dropGoldValue = 0;
	}

	if (ii != MAXITEMS && !dItem[item[ii]._ix][item[ii]._iy]) {
		return;
	}

	item[ii]._iCreateInfo &= 0x7FFF;
	plr[pnum].HoldItem = item[ii];
	CheckQuestItem(pnum);
	CheckBookLevel(pnum);
	CheckItemStats(pnum);
	SetICursor(plr[pnum].HoldItem._iCurs + CURSOR_FIRSTITEM);
	if (plr[pnum].HoldItem._itype == ITYPE_GOLD) {
		done = GoldAutoPlace(pnum);
	} else {
		done = FALSE;
		if (((plr[pnum]._pgfxnum & 0xF) == ANIM_ID_UNARMED || (plr[pnum]._pgfxnum & 0xF) == ANIM_ID_UNARMED_SHIELD) && plr[pnum]._pmode <= PM_WALK3) {
			if (plr[pnum].HoldItem._iStatFlag) {
				if (plr[pnum].HoldItem._iClass == ICLASS_WEAPON) {
					done = WeaponAutoPlace(pnum);
					if (done)
						CalcPlrInv(pnum, TRUE);
				}
			}
		}
		if (!done) {
			w = icursW28;
			h = icursH28;
			if (w == 1 && h == 1) {
				idx = plr[pnum].HoldItem.IDidx;
				if (plr[pnum].HoldItem._iStatFlag && AllItemsList[idx].iUsable) {
					for (i = 0; i < 8 && !done; i++) {
						if (plr[pnum].SpdList[i]._itype == ITYPE_NONE) {
							plr[pnum].SpdList[i] = plr[pnum].HoldItem;
							CalcPlrScrolls(pnum);
							drawsbarflag = TRUE;
							done = TRUE;
						}
					}
				}
				for (i = 30; i <= 39 && !done; i++) {
					done = AutoPlace(pnum, i, w, h, TRUE);
				}
				for (i = 20; i <= 29 && !done; i++) {
					done = AutoPlace(pnum, i, w, h, TRUE);
				}
				for (i = 10; i <= 19 && !done; i++) {
					done = AutoPlace(pnum, i, w, h, TRUE);
				}
				for (i = 0; i <= 9 && !done; i++) {
					done = AutoPlace(pnum, i, w, h, TRUE);
				}
			}
			if (w == 1 && h == 2) {
				for (i = 29; i >= 20 && !done; i--) {
					done = AutoPlace(pnum, i, w, h, TRUE);
				}
				for (i = 9; i >= 0 && !done; i--) {
					done = AutoPlace(pnum, i, w, h, TRUE);
				}
				for (i = 19; i >= 10 && !done; i--) {
					done = AutoPlace(pnum, i, w, h, TRUE);
				}
			}
			if (w == 1 && h == 3) {
				for (i = 0; i < 20 && !done; i++) {
					done = AutoPlace(pnum, i, w, h, TRUE);
				}
			}
			if (w == 2 && h == 2) {
				for (i = 0; i < 10 && !done; i++) {
					done = AutoPlace(pnum, AP2x2Tbl[i], w, h, TRUE);
				}
				for (i = 21; i < 29 && !done; i += 2) {
					done = AutoPlace(pnum, i, w, h, TRUE);
				}
				for (i = 1; i < 9 && !done; i += 2) {
					done = AutoPlace(pnum, i, w, h, TRUE);
				}
				for (i = 10; i < 19 && !done; i++) {
					done = AutoPlace(pnum, i, w, h, TRUE);
				}
			}
			if (w == 2 && h == 3) {
				for (i = 0; i < 9 && !done; i++) {
					done = AutoPlace(pnum, i, w, h, TRUE);
				}
				for (i = 10; i < 19 && !done; i++) {
					done = AutoPlace(pnum, i, w, h, TRUE);
				}
			}
		}
	}
	if (done) {
		dItem[item[ii]._ix][item[ii]._iy] = 0;
		i = 0;
		while (i < numitems) {
			if (itemactive[i] == ii) {
				DeleteItem(itemactive[i], i);
				i = 0;
			} else {
				i++;
			}
		}
	} else {
		if (pnum == myplr) {
			if (plr[pnum]._pClass == PC_WARRIOR) {
				PlaySFX(random(0, 3) + PS_WARR14);
			} else if (plr[pnum]._pClass == PC_ROGUE) {
				PlaySFX(random(0, 3) + PS_ROGUE14);
			} else if (plr[pnum]._pClass == PC_SORCERER) {
				PlaySFX(random(0, 3) + PS_MAGE14);
			}
		}
		plr[pnum].HoldItem = item[ii];
		RespawnItem(ii, 1);
		NetSendCmdPItem(TRUE, CMD_RESPAWNITEM, item[ii]._ix, item[ii]._iy);
		plr[pnum].HoldItem._itype = ITYPE_NONE;
	}
}
// 48E9A8: using guessed type int AP2x2Tbl[10];

int FindGetItem(int indx, WORD ci, int iseed)
{
	int i, ii;

	i = 0;
	if (numitems <= 0)
		return -1;

	while (1) {
		ii = itemactive[i];
		if (item[ii].IDidx == indx && item[ii]._iSeed == iseed && item[ii]._iCreateInfo == ci)
			break;

		i++;

		if (i >= numitems)
			return -1;
	}

	return ii;
}

void SyncGetItem(int x, int y, int idx, WORD ci, int iseed)
{
	int i, ii;

	if (dItem[x][y]) {
		ii = dItem[x][y] - 1;
		if (item[ii].IDidx == idx
		    && item[ii]._iSeed == iseed
		    && item[ii]._iCreateInfo == ci) {
			FindGetItem(idx, ci, iseed);
		} else {
			ii = FindGetItem(idx, ci, iseed);
		}
	} else {
		ii = FindGetItem(idx, ci, iseed);
	}

	if (ii != -1) {
		dItem[item[ii]._ix][item[ii]._iy] = 0;
		i = 0;
		while (i < numitems) {
			if (itemactive[i] == ii) {
				DeleteItem(itemactive[i], i);
				FindGetItem(idx, ci, iseed);
				FindGetItem(idx, ci, iseed); /* check idx */
				i = 0;
			} else {
				i++;
			}
		}
		FindGetItem(idx, ci, iseed);
	}
}

BOOL CanPut(int x, int y)
{
	char oi, oi2;

	if (dItem[x][y])
		return FALSE;
	if (nSolidTable[dPiece[x][y]])
		return FALSE;

	if (dObject[x][y]) {
		if (object[dObject[x][y] > 0 ? dObject[x][y] - 1 : -1 - dObject[x][y]]._oSolidFlag)
			return FALSE;
	}

	oi = dObject[x + 1][y + 1];
	if (oi > 0 && object[oi - 1]._oSelFlag) {
		return FALSE;
	}
	if (oi < 0 && object[-(oi + 1)]._oSelFlag) {
		return FALSE;
	}

	oi = dObject[x + 1][y];
	if (oi > 0) {
		oi2 = dObject[x][y + 1];
		if (oi2 > 0 && object[oi - 1]._oSelFlag && object[oi2 - 1]._oSelFlag)
			return FALSE;
	}

	if (!currlevel && (dMonster[x][y] || dMonster[x + 1][y + 1]))
		return FALSE;

	return TRUE;
}

BOOL TryInvPut()
{
	int dir;

	if (numitems >= 127)
		return FALSE;

	dir = GetDirection(plr[myplr].WorldX, plr[myplr].WorldY, cursmx, cursmy);
	if (CanPut(plr[myplr].WorldX + offset_x[dir], plr[myplr].WorldY + offset_y[dir])) {
		return TRUE;
	}

	dir = (dir - 1) & 7;
	if (CanPut(plr[myplr].WorldX + offset_x[dir], plr[myplr].WorldY + offset_y[dir])) {
		return TRUE;
	}

	dir = (dir + 2) & 7;
	if (CanPut(plr[myplr].WorldX + offset_x[dir], plr[myplr].WorldY + offset_y[dir])) {
		return TRUE;
	}

	return CanPut(plr[myplr].WorldX, plr[myplr].WorldY);
}

void DrawInvMsg(char *msg)
{
	DWORD dwTicks;

	dwTicks = GetTickCount();
	if (dwTicks - sgdwLastTime >= 5000) {
		sgdwLastTime = dwTicks;
		ErrorPlrMsg(msg);
	}
}

int InvPutItem(int pnum, int x, int y)
{
	BOOL done;
	int d, ii;
	int i, j, l;
	int xx, yy;

	if (numitems >= 127)
		return -1;

	if (FindGetItem(plr[pnum].HoldItem.IDidx, plr[pnum].HoldItem._iCreateInfo, plr[pnum].HoldItem._iSeed) != -1) {
		DrawInvMsg("A duplicate item has been detected.  Destroying duplicate...");
		SyncGetItem(x, y, plr[pnum].HoldItem.IDidx, plr[pnum].HoldItem._iCreateInfo, plr[pnum].HoldItem._iSeed);
	}

	d = GetDirection(plr[pnum].WorldX, plr[pnum].WorldY, x, y);
	xx = x - plr[pnum].WorldX;
	yy = y - plr[pnum].WorldY;
	if (abs(xx) > 1 || abs(yy) > 1) {
		x = plr[pnum].WorldX + offset_x[d];
		y = plr[pnum].WorldY + offset_y[d];
	}
	if (!CanPut(x, y)) {
		d = (d - 1) & 7;
		x = plr[pnum].WorldX + offset_x[d];
		y = plr[pnum].WorldY + offset_y[d];
		if (!CanPut(x, y)) {
			d = (d + 2) & 7;
			x = plr[pnum].WorldX + offset_x[d];
			y = plr[pnum].WorldY + offset_y[d];
			if (!CanPut(x, y)) {
				done = FALSE;
				for (l = 1; l < 50 && !done; l++) {
					for (j = -l; j <= l && !done; j++) {
						yy = j + plr[pnum].WorldY;
						for (i = -l; i <= l && !done; i++) {
							xx = i + plr[pnum].WorldX;
							if (CanPut(xx, yy)) {
								done = TRUE;
								x = xx;
								y = yy;
							}
						}
					}
				}
				if (!done)
					return -1;
			}
		}
	}

	CanPut(x, y); //if (!CanPut(x, y)) {
	//	assertion_failed(1524, "C:\\Diablo\\Direct\\inv.cpp", "CanPut(x,y)");
	//}

	ii = itemavail[0];
	dItem[x][y] = ii + 1;
	itemavail[0] = itemavail[MAXITEMS - (numitems + 1)];
	itemactive[numitems] = ii;
	item[ii] = plr[pnum].HoldItem;
	item[ii]._ix = x;
	item[ii]._iy = y;
	RespawnItem(ii, 1);
	numitems++;
	SetCursor_(CURSOR_HAND);
	return ii;
}

int SyncPutItem(int pnum, int x, int y, int idx, WORD icreateinfo, int iseed, int Id, int dur, int mdur, int ch, int mch, int ivalue, unsigned int ibuff)
{
	BOOL done;
	int d, ii;
	int i, j, l;
	int xx, yy;

	if (numitems >= 127)
		return -1;

	if (FindGetItem(idx, icreateinfo, iseed) != -1) {
		DrawInvMsg("A duplicate item has been detected from another player.");
		SyncGetItem(x, y, idx, icreateinfo, iseed);
	}

	d = GetDirection(plr[pnum].WorldX, plr[pnum].WorldY, x, y);
	xx = x - plr[pnum].WorldX;
	yy = y - plr[pnum].WorldY;
	if (abs(xx) > 1 || abs(yy) > 1) {
		x = plr[pnum].WorldX + offset_x[d];
		y = plr[pnum].WorldY + offset_y[d];
	}
	if (!CanPut(x, y)) {
		d = (d - 1) & 7;
		x = plr[pnum].WorldX + offset_x[d];
		y = plr[pnum].WorldY + offset_y[d];
		if (!CanPut(x, y)) {
			d = (d + 2) & 7;
			x = plr[pnum].WorldX + offset_x[d];
			y = plr[pnum].WorldY + offset_y[d];
			if (!CanPut(x, y)) {
				done = FALSE;
				for (l = 1; l < 50 && !done; l++) {
					for (j = -l; j <= l && !done; j++) {
						yy = j + plr[pnum].WorldY;
						for (i = -l; i <= l && !done; i++) {
							xx = i + plr[pnum].WorldX;
							if (CanPut(xx, yy)) {
								done = TRUE;
								x = xx;
								y = yy;
							}
						}
					}
				}
				if (!done)
					return -1;
			}
		}
	}

	CanPut(x, y);

	ii = itemavail[0];
	dItem[x][y] = ii + 1;
	itemavail[0] = itemavail[MAXITEMS - (numitems + 1)];
	itemactive[numitems] = ii;

	if (idx == IDI_EAR) {
		RecreateEar(ii, icreateinfo, iseed, Id, dur, mdur, ch, mch, ivalue, ibuff);
	} else {
		RecreateItem(ii, idx, icreateinfo, iseed, ivalue);
		if (Id)
			item[ii]._iIdentified = TRUE;
		item[ii]._iDurability = dur;
		item[ii]._iMaxDur = mdur;
		item[ii]._iCharges = ch;
		item[ii]._iMaxCharges = mch;
	}

	item[ii]._ix = x;
	item[ii]._iy = y;
	RespawnItem(ii, 1);
	numitems++;
	return ii;
}

char CheckInvHLight()
{
	int r, ii, nGold;
	ItemStruct *pi;
	PlayerStruct *p;
	char rv;

	for (r = 0; (DWORD)r < 73; r++) {
		if (MouseX >= InvRect[r].X
		    && MouseX < InvRect[r].X + 29
		    && MouseY >= InvRect[r].Y - 29
		    && MouseY < InvRect[r].Y) {
			break;
		}
	}

	if ((DWORD)r >= 73)
		return -1;

	rv = -1;
	infoclr = COL_WHITE;
	pi = NULL;
	p = &plr[myplr];
	ClearPanel();
	if (r >= 0 && r <= 3) {
		rv = INVLOC_HEAD;
		pi = &p->InvBody[rv];
	} else if (r == 4) {
		rv = INVLOC_RING_LEFT;
		pi = &p->InvBody[rv];
	} else if (r == 5) {
		rv = INVLOC_RING_RIGHT;
		pi = &p->InvBody[rv];
	} else if (r == 6) {
		rv = INVLOC_AMULET;
		pi = &p->InvBody[rv];
	} else if (r >= 7 && r <= 12) {
		rv = INVLOC_HAND_LEFT;
		pi = &p->InvBody[rv];
	} else if (r >= 13 && r <= 18) {
		pi = &p->InvBody[INVLOC_HAND_LEFT];
		if (pi->_itype == ITYPE_NONE || pi->_iLoc != ILOC_TWOHAND) {
			rv = INVLOC_HAND_RIGHT;
			pi = &p->InvBody[rv];
		} else {
			rv = INVLOC_HAND_LEFT;
		}
	} else if (r >= 19 && r <= 24) {
		rv = INVLOC_CHEST;
		pi = &p->InvBody[rv];
	} else if (r >= 25 && r <= 64) {
		r = abs(p->InvGrid[r - 25]);
		if (!r)
			return -1;
		ii = r - 1;
		rv = ii + 7;
		pi = &p->InvList[ii];
	} else if (r >= 65) {
		r -= 65;
		drawsbarflag = TRUE;
		pi = &p->SpdList[r];
		if (pi->_itype == ITYPE_NONE)
			return -1;
		rv = r + 47;
	}

	if (pi->_itype == ITYPE_NONE)
		return -1;

	if (pi->_itype == ITYPE_GOLD) {
		nGold = pi->_ivalue;
		sprintf(infostr, "%i gold %s", nGold, get_pieces_str(nGold));
	} else {
		if (pi->_iMagical == ITEM_QUALITY_MAGIC) {
			infoclr = COL_BLUE;
		} else if (pi->_iMagical == ITEM_QUALITY_UNIQUE) {
			infoclr = COL_GOLD;
		}
		strcpy(infostr, pi->_iName);
		if (pi->_iIdentified) {
			strcpy(infostr, pi->_iIName);
			PrintItemDetails(pi);
		} else {
			PrintItemDur(pi);
		}
	}

	return rv;
}
// 4B883C: using guessed type int infoclr;

void RemoveScroll(int pnum)
{
	int i;

	for (i = 0; i < plr[pnum]._pNumInv; i++) {
		if (plr[pnum].InvList[i]._itype != ITYPE_NONE
		    && (plr[pnum].InvList[i]._iMiscId == IMISC_SCROLL || plr[pnum].InvList[i]._iMiscId == IMISC_SCROLLT)
		    && plr[pnum].InvList[i]._iSpell == plr[pnum]._pRSpell) {
			RemoveInvItem(pnum, i);
			CalcPlrScrolls(pnum);
			return;
		}
	}
	for (i = 0; i < MAXBELTITEMS; i++) {
		if (plr[pnum].SpdList[i]._itype != ITYPE_NONE
		    && (plr[pnum].SpdList[i]._iMiscId == IMISC_SCROLL || plr[pnum].SpdList[i]._iMiscId == IMISC_SCROLLT)
		    && plr[pnum].SpdList[i]._iSpell == plr[pnum]._pRSpell) {
			RemoveSpdBarItem(pnum, i);
			CalcPlrScrolls(pnum);
			return;
		}
	}
}

BOOL UseScroll()
{
	int i;

	if (pcurs != CURSOR_HAND)
		return FALSE;
	if (leveltype == DTYPE_TOWN && !spelldata[plr[myplr]._pRSpell].sTownSpell)
		return FALSE;

	for (i = 0; i < plr[myplr]._pNumInv; i++) {
		if (plr[myplr].InvList[i]._itype != ITYPE_NONE
		    && (plr[myplr].InvList[i]._iMiscId == IMISC_SCROLL || plr[myplr].InvList[i]._iMiscId == IMISC_SCROLLT)
		    && plr[myplr].InvList[i]._iSpell == plr[myplr]._pRSpell) {
			return TRUE;
		}
	}
	for (i = 0; i < MAXBELTITEMS; i++) {
		if (plr[myplr].SpdList[i]._itype != ITYPE_NONE
		    && (plr[myplr].SpdList[i]._iMiscId == IMISC_SCROLL || plr[myplr].SpdList[i]._iMiscId == IMISC_SCROLLT)
		    && plr[myplr].SpdList[i]._iSpell == plr[myplr]._pRSpell) {
			return TRUE;
		}
	}

	return FALSE;
}

void UseStaffCharge(int pnum)
{
	if (plr[pnum].InvBody[INVLOC_HAND_LEFT]._itype != ITYPE_NONE
	    && plr[pnum].InvBody[INVLOC_HAND_LEFT]._iMiscId == IMISC_STAFF
	    && plr[pnum].InvBody[INVLOC_HAND_LEFT]._iSpell == plr[pnum]._pRSpell
	    && plr[pnum].InvBody[INVLOC_HAND_LEFT]._iCharges > 0) {
		plr[pnum].InvBody[INVLOC_HAND_LEFT]._iCharges--;
		CalcPlrStaff(pnum);
	}
}

BOOL UseStaff()
{
	if (pcurs == CURSOR_HAND) {
		if (plr[myplr].InvBody[INVLOC_HAND_LEFT]._itype != ITYPE_NONE
		    && plr[myplr].InvBody[INVLOC_HAND_LEFT]._iMiscId == IMISC_STAFF
		    && plr[myplr].InvBody[INVLOC_HAND_LEFT]._iSpell == plr[myplr]._pRSpell
		    && plr[myplr].InvBody[INVLOC_HAND_LEFT]._iCharges > 0) {
			return TRUE;
		}
	}

	return FALSE;
}

void StartGoldDrop()
{
	initialDropGoldIndex = pcursinvitem;
	if (pcursinvitem <= 46)
		initialDropGoldValue = plr[myplr].InvBody[pcursinvitem]._ivalue;
	else
		initialDropGoldValue = plr[myplr].InvBody[pcursinvitem]._iMaxDur;
	dropGoldFlag = TRUE;
	dropGoldValue = 0;
	if (talkflag)
		control_reset_talk();
}
// 4B8960: using guessed type int talkflag;
// 4B8CB8: using guessed type char pcursinvitem;

BOOL UseInvItem(int pnum, int cii)
{
	int c, idata, it;
	ItemStruct *Item;
	BOOL speedlist;

	if (plr[pnum]._pInvincible && !plr[pnum]._pHitPoints && pnum == myplr)
		return TRUE;

	if (pcurs != 1 || stextflag)
		return TRUE;

	if (cii <= 5)
		return FALSE;

	if (cii <= 46) {
		c = cii - 7;
		speedlist = FALSE;
		Item = &plr[pnum].InvList[c];
	} else {
		if (talkflag)
			return TRUE;
		c = cii - 47;
		speedlist = TRUE;
		Item = &plr[pnum].SpdList[c];
	}

	switch (Item->IDidx) {
	case 17:
		sfxdelay = 10;
		if (plr[pnum]._pClass == PC_WARRIOR) {
			sfxdnum = PS_WARR95;
		} else if (plr[pnum]._pClass == PC_ROGUE) {
			sfxdnum = PS_ROGUE95;
		} else if (plr[pnum]._pClass == PC_SORCERER) {
			sfxdnum = PS_MAGE95;
		}
		break;
	case 19:
		PlaySFX(IS_IBOOK);
		sfxdelay = 10;
		if (plr[pnum]._pClass == PC_WARRIOR) {
			sfxdnum = PS_WARR29;
		} else if (plr[pnum]._pClass == PC_ROGUE) {
			sfxdnum = PS_ROGUE29;
		} else if (plr[pnum]._pClass == PC_SORCERER) {
			sfxdnum = PS_MAGE29;
		}
		break;
	default:
		if (!AllItemsList[Item->IDidx].iUsable)
			return FALSE;

		if (!Item->_iStatFlag) {
			if (plr[pnum]._pClass == PC_WARRIOR) {
				PlaySFX(PS_WARR13);
			} else if (plr[pnum]._pClass == PC_ROGUE) {
				PlaySFX(PS_ROGUE13);
			} else if (plr[pnum]._pClass == PC_SORCERER) {
				PlaySFX(PS_MAGE13);
			}
			return TRUE;
		}

		it = Item->_iMiscId;

		if (it == IMISC_NONE && Item->_itype == ITYPE_GOLD) {
			StartGoldDrop();
			return TRUE;
		}

		if (dropGoldFlag) {
			dropGoldFlag = FALSE;
			dropGoldValue = 0;
		}

		if (it == IMISC_SCROLL && currlevel == 0 && !spelldata[Item->_iSpell].sTownSpell
		    || it == IMISC_SCROLLT && currlevel == 0 && !spelldata[Item->_iSpell].sTownSpell) {
			return TRUE;
		}

		idata = ItemCAnimTbl[Item->_iCurs];
		if (it == IMISC_BOOK)
			PlaySFX(IS_RBOOK);
		else if (pnum == myplr)
			PlaySFX(ItemInvSnds[idata]);

		UseItem(pnum, Item->_iMiscId, Item->_iSpell);

		if (speedlist) {
			RemoveSpdBarItem(pnum, c);
		} else if (plr[pnum].InvList[c]._iMiscId != IMISC_MAPOFDOOM) {
			RemoveInvItem(pnum, c);
		}
		break;
	}

	return TRUE;
}
// 4B8960: using guessed type int talkflag;
// 52A554: using guessed type int sfxdelay;
// 6AA705: using guessed type char stextflag;

void DoTelekinesis()
{
	if (pcursobj != -1)
		NetSendCmdParam1(TRUE, CMD_OPOBJT, pcursobj);
	if (pcursitem != -1)
		NetSendCmdGItem(TRUE, CMD_REQUESTAGITEM, myplr, myplr, pcursitem);
	if (pcursmonst != -1 && !M_Talker(pcursmonst) && !monster[pcursmonst].mtalkmsg)
		NetSendCmdParam1(TRUE, CMD_KNOCKBACK, pcursmonst);
	SetCursor_(CURSOR_HAND);
}
// 4B8CC0: using guessed type char pcursitem;
// 4B8CC1: using guessed type char pcursobj;

int CalculateGold(int pnum)
{
	int i, gold;

	gold = 0;
	for (i = 0; i < MAXBELTITEMS; i++) {
		if (plr[pnum].SpdList[i]._itype == ITYPE_GOLD) {
			gold += plr[pnum].SpdList[i]._ivalue;
			drawpanflag = 255;
		}
	}
	for (i = 0; i < plr[pnum]._pNumInv; i++) {
		if (plr[pnum].InvList[i]._itype == ITYPE_GOLD)
			gold += plr[pnum].InvList[i]._ivalue;
	}

	return gold;
}
// 52571C: using guessed type int drawpanflag;

BOOL DropItemBeforeTrig()
{
	if (TryInvPut()) {
		NetSendCmdPItem(TRUE, CMD_PUTITEM, cursmx, cursmy);
		SetCursor_(CURSOR_HAND);
		return TRUE;
	}

	return FALSE;
}
