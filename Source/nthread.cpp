#include "diablo.h"
#include "../3rdParty/Storm/Source/storm.h"

char byte_679704; // weak
int gdwMsgLenTbl[MAX_PLRS];
#ifdef __cplusplus
static CCritSect sgMemCrit;
#endif
int gdwDeltaBytesSec;    // weak
char nthread_should_run; // weak
DWORD gdwTurnsInTransit; // weak
int glpMsgTbl[MAX_PLRS];
unsigned int glpNThreadId;
char sgbSyncCountdown;   // weak
int turn_upper_bit;      // weak
char byte_679758;        // weak
char sgbPacketCountdown; // weak
char sgbThreadIsRunning; // weak
int gdwLargestMsgSize;   // weak
int gdwNormalMsgSize;    // weak
int last_tick;           // weak

/* data */
static HANDLE sghThread = (HANDLE)0xFFFFFFFF; // idb

void nthread_terminate_game(const char *pszFcn)
{
	DWORD sErr; // eax

	sErr = SErrGetLastError();
	if (sErr != STORM_ERROR_INVALID_PLAYER) {
		if (sErr == STORM_ERROR_GAME_TERMINATED || sErr == STORM_ERROR_NOT_IN_GAME) {
			gbGameDestroyed = 1;
		} else {
			app_fatal("%s:\n%s", pszFcn, TraceLastError());
		}
	}
}
// 67862D: using guessed type char gbGameDestroyed;

int nthread_send_and_recv_turn(int cur_turn, int turn_delta)
{
	unsigned int new_cur_turn; // edi
	const char *lastStormFn;   // ecx
	int turn_tmp;              // eax
	int turn;                  // [esp+Ch] [ebp-8h]
	int curTurnsInTransit;     // [esp+10h] [ebp-4h]

	new_cur_turn = cur_turn;
	if (SNetGetTurnsInTransit(&curTurnsInTransit)) {
		if (curTurnsInTransit >= (unsigned int)gdwTurnsInTransit)
			return new_cur_turn;
		while (1) {
			++curTurnsInTransit;

			turn_tmp = turn_upper_bit | new_cur_turn & 0x7FFFFFFF;
			turn_upper_bit = 0;
			turn = turn_tmp;

			if (!SNetSendTurn((char *)&turn, sizeof(turn)))
				break;

			new_cur_turn += turn_delta;
			if (new_cur_turn >= 0x7FFFFFFF)
				new_cur_turn = (unsigned short)new_cur_turn;
			if (curTurnsInTransit >= (unsigned int)gdwTurnsInTransit)
				return new_cur_turn;
		}
		lastStormFn = "SNetSendTurn";
	} else {
		lastStormFn = "SNetGetTurnsInTransit";
	}
	nthread_terminate_game(lastStormFn);
	return 0;
}
// 679738: using guessed type int gdwTurnsInTransit;
// 679754: using guessed type int turn_upper_bit;

int nthread_recv_turns(int *pfSendAsync)
{
	BOOLEAN hasCountedDown; // zf

	*pfSendAsync = 0;
	if (--sgbPacketCountdown) {
		last_tick += 50;
		return 1;
	}
	hasCountedDown = sgbSyncCountdown-- == 1;
	sgbPacketCountdown = byte_679704;
	if (!hasCountedDown)
		goto LABEL_11;
	if (SNetReceiveTurns(0, MAX_PLRS, (char **)glpMsgTbl, (unsigned int *)gdwMsgLenTbl, (LPDWORD)player_state)) {
		if (!byte_679758) {
			byte_679758 = 1;
			last_tick = GetTickCount();
		}
		sgbSyncCountdown = 4;
		multi_msg_countdown();
	LABEL_11:
		*pfSendAsync = 1;
		last_tick += 50;
		return 1;
	}
	if (SErrGetLastError() != STORM_ERROR_NO_MESSAGES_WAITING)
		nthread_terminate_game("SNetReceiveTurns");
	byte_679758 = 0;
	sgbSyncCountdown = 1;
	sgbPacketCountdown = 1;
	return 0;
}
// 679704: using guessed type char byte_679704;
// 679750: using guessed type char sgbSyncCountdown;
// 679758: using guessed type char byte_679758;
// 679759: using guessed type char sgbPacketCountdown;
// 679764: using guessed type int last_tick;

void nthread_set_turn_upper_bit()
{
	turn_upper_bit = 0x80000000;
}
// 679754: using guessed type int turn_upper_bit;

void nthread_start(BOOL set_turn_upper_bit)
{
	char *err;                   // eax
	unsigned int largestMsgSize; // esi
	unsigned int normalMsgSize;  // eax
	char *err2;                  // eax
	_SNETCAPS caps;              // [esp+8h] [ebp-24h]

	last_tick = GetTickCount();
	sgbPacketCountdown = 1;
	sgbSyncCountdown = 1;
	byte_679758 = 1;
	if (set_turn_upper_bit)
		nthread_set_turn_upper_bit();
	else
		turn_upper_bit = 0;
	caps.size = 36;
	if (!SNetGetProviderCaps(&caps)) {
		err = TraceLastError();
		app_fatal("SNetGetProviderCaps:\n%s", err);
	}
	gdwTurnsInTransit = caps.defaultturnsintransit;
	if (!caps.defaultturnsintransit)
		gdwTurnsInTransit = 1;
	if (caps.defaultturnssec <= 0x14u && caps.defaultturnssec)
		byte_679704 = 0x14u / caps.defaultturnssec;
	else
		byte_679704 = 1;
	largestMsgSize = 512;
	if (caps.maxmessagesize < 0x200u)
		largestMsgSize = caps.maxmessagesize;
	gdwDeltaBytesSec = (unsigned int)caps.bytessec >> 2;
	gdwLargestMsgSize = largestMsgSize;
	if (caps.maxplayers > 4u)
		caps.maxplayers = 4;
	normalMsgSize = (3 * (caps.bytessec * (unsigned int)(unsigned char)byte_679704 / 0x14) >> 2) / caps.maxplayers;
	gdwNormalMsgSize = normalMsgSize;
	if (normalMsgSize < 0x80) {
		do {
			byte_679704 *= 2;
			normalMsgSize *= 2;
		} while (normalMsgSize < 0x80);
		gdwNormalMsgSize = normalMsgSize;
	}
	if (normalMsgSize > largestMsgSize)
		gdwNormalMsgSize = largestMsgSize;
	if ((unsigned char)gbMaxPlayers > 1u) {
		sgbThreadIsRunning = 0;
#ifdef __cplusplus
		sgMemCrit.Enter();
#endif
		nthread_should_run = 1;
		sghThread = (HANDLE)_beginthreadex(NULL, 0, nthread_handler, NULL, 0, &glpNThreadId);
		if (sghThread == (HANDLE)-1) {
			err2 = TraceLastError();
			app_fatal("nthread2:\n%s", err2);
		}
		SetThreadPriority(sghThread, THREAD_PRIORITY_HIGHEST);
	}
}
// 679660: using guessed type char gbMaxPlayers;
// 679704: using guessed type char byte_679704;
// 679730: using guessed type int gdwDeltaBytesSec;
// 679734: using guessed type char nthread_should_run;
// 679738: using guessed type int gdwTurnsInTransit;
// 679750: using guessed type char sgbSyncCountdown;
// 679754: using guessed type int turn_upper_bit;
// 679758: using guessed type char byte_679758;
// 679759: using guessed type char sgbPacketCountdown;
// 67975A: using guessed type char sgbThreadIsRunning;
// 67975C: using guessed type int gdwLargestMsgSize;
// 679760: using guessed type int gdwNormalMsgSize;
// 679764: using guessed type int last_tick;

unsigned int __stdcall nthread_handler(void *a1)
{
	signed int delta; // esi
	int received;     // [esp+Ch] [ebp-4h]

	if (nthread_should_run) {
		while (1) {
#ifdef __cplusplus
			sgMemCrit.Enter();
#endif
			if (!nthread_should_run)
				break;
			nthread_send_and_recv_turn(0, 0);
			if (nthread_recv_turns(&received))
				delta = last_tick - GetTickCount();
			else
				delta = 50;
#ifdef __cplusplus
			sgMemCrit.Leave();
#endif
			if (delta > 0)
				Sleep(delta);
			if (!nthread_should_run)
				return 0;
		}
#ifdef __cplusplus
		sgMemCrit.Leave();
#endif
	}
	return 0;
}
// 679734: using guessed type char nthread_should_run;
// 679764: using guessed type int last_tick;

void nthread_cleanup()
{
	nthread_should_run = 0;
	gdwTurnsInTransit = 0;
	gdwNormalMsgSize = 0;
	gdwLargestMsgSize = 0;
	if (sghThread != (HANDLE)-1 && glpNThreadId != GetCurrentThreadId()) {
#ifdef __cplusplus
		if (!sgbThreadIsRunning)
			sgMemCrit.Leave();
#endif
		if (WaitForSingleObject(sghThread, 0xFFFFFFFF) == -1) {
			app_fatal("nthread3:\n(%s)", TraceLastError());
		}
		CloseHandle(sghThread);
		sghThread = (HANDLE)-1;
	}
}
// 679734: using guessed type char nthread_should_run;
// 679738: using guessed type int gdwTurnsInTransit;
// 67975A: using guessed type char sgbThreadIsRunning;
// 67975C: using guessed type int gdwLargestMsgSize;
// 679760: using guessed type int gdwNormalMsgSize;

void nthread_ignore_mutex(BOOL bStart)
{
	if (sghThread != (HANDLE)-1) {
#ifdef __cplusplus
		if (bStart)
			sgMemCrit.Leave();
		else
			sgMemCrit.Enter();
#endif
		sgbThreadIsRunning = bStart;
	}
}
// 67975A: using guessed type char sgbThreadIsRunning;

BOOL nthread_has_500ms_passed(BOOL unused)
{
	DWORD currentTickCount; // eax
	int ticksElapsed;       // ecx

	currentTickCount = GetTickCount();
	ticksElapsed = currentTickCount - last_tick;
	if (gbMaxPlayers == 1 && ticksElapsed > 500) {
		last_tick = currentTickCount;
		ticksElapsed = 0;
	}
	return ticksElapsed >= 0;
}
// 679660: using guessed type char gbMaxPlayers;
// 679764: using guessed type int last_tick;
