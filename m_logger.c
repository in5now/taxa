/*
 * m_logger.c
 *
 *  Created on: 09.03.2013
 *      Author: admin
 */

#include "m_logger.h"

static BOOL slog_need_format(void);

static DWORD slog_pos;

static void slog_get_time(time_t *time)
{
	QWORD qw_time = TickGetFullScale();

	time->hours = (DWORD)(qw_time/TICK_HOUR);
	qw_time %= TICK_HOUR;
	time->minutes = (BYTE)(qw_time/TICK_MINUTE);
	qw_time %= TICK_MINUTE;
	time->seconds = (BYTE)(qw_time/TICK_SECOND);
}

void slog_init(void)
{
	if (slog_need_format()) {
		slog_format();
	} else {
		XEEBeginRead(SLOG_START);
		for (slog_pos = 0; (XEERead() != SLOG_EOF) && (slog_pos < SLOG_LEN); slog_pos++)
			;
		XEEEndRead();
	}
}

void slog_format(void)
{
	DWORD addr = SLOG_START;

	for (XEEBeginWrite(addr); addr < SLOG_START + SLOG_LEN; addr++) {
		if(addr % PAGE_SIZE == 0)
			XEEBeginWrite(addr);
		XEEWrite(SLOG_EOF);
	}
	XEEEndWrite();
	slog_pos = 0;
}

static BOOL slog_need_format(void)
{
	/*
	 * check's first slog page
	 */
	DWORD addr;
	XEEBeginRead(SLOG_START);
	for (addr = SLOG_START; addr < SLOG_START + PAGE_SIZE; addr++) {
		if (XEERead() ^ 0xFF) {
			XEEEndRead();
			return FALSE;
		}
	}
	XEEEndRead();
	return TRUE;
}

static int slog_put_timestamp(void)
{
	int put;
	time_t time;
	BYTE str_buf[13];
	char *str_time;

	slog_get_time(&time);
	sprintf(str_buf, "%05i:%02i:%02i ", (int)time.hours, (int)time.minutes, (int)time.seconds);

	for (put = 0, str_time = str_buf; (*str_time) && (slog_pos < SLOG_LEN); slog_pos++, put++, str_time++) {
		if((SLOG_START + slog_pos) % PAGE_SIZE == 0)
			XEEBeginWrite(SLOG_START + slog_pos);
		XEEWrite(*str_time);
	}

	return put;
}

int slog_puts(const BYTE *str)
{
	int put = slog_put_timestamp();

	for (put = 0; (*str) && (slog_pos < SLOG_LEN); slog_pos++, put++, str++) {
		if((SLOG_START + slog_pos) % PAGE_SIZE == 0)
			XEEBeginWrite(SLOG_START + slog_pos);
		XEEWrite(*str);
	}
	XEEEndWrite();

	return put;
}

int slog_putrs(const rom BYTE *str)
{
	int put = slog_put_timestamp();

	for (XEEBeginWrite(SLOG_START + slog_pos); (*str) && (slog_pos < SLOG_LEN); slog_pos++, put++, str++) {
		if((SLOG_START + slog_pos) % PAGE_SIZE == 0)
			XEEBeginWrite(SLOG_START + slog_pos);
		XEEWrite(*str);
	}
	XEEEndWrite();

	return put;
}


int slog_gets(DWORD pos, BYTE *buf, BYTE len)
{
	int got = 0;

	if (pos >= SLOG_LEN)
		return -1;
	if (!len)
		return 0;

	for (XEEBeginRead(pos + SLOG_START); got < len; got++, buf++)
		if ((*buf = XEERead()) == SLOG_EOF)
			break;
	XEEEndRead();

	return got;
}

void slog_flush(void)
{
	static BYTE buf[PAGE_SIZE/4];
	int read, all = 0;

	putrsUSART("\n\r<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n\r");

	while((read = slog_gets(all, buf, sizeof(buf))) > 0) {
		buf[read] = '\0';
		putsUSART(buf);
		all += read;
	}

	putrsUSART("\n\r>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n\r");

	uitoa((WORD)(SLOG_LEN - all), buf);

	putsUSART(buf);
	putrsUSART(" bytes free\n\r");
}
