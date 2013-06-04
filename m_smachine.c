/*
 * m_smachine.c
 *
 *  Created on: 12.04.2013
 *      Author: admin
 */
#include "post.h"
#include "m_bcp.h"
#include "m_smachine.h"
#include "eventer.h"
#include "m_accessor.h"
#include "m_lcd.h"
#include "trace.h"

sm_state_e state;
rom static char * ver = "SM0.01";
static mailbox_t mailbox;
static event_t event;


#define MYSELF	MODULE_SRVMACHINE

static int process_buffer(bd_t handler)
{
	int result = 0;
	bcp_header_t * hdr = (bcp_header_t *) bcp_buffer(handler)->buf;

	switch (TYPE(hdr->hdr_s.type)) {
	case TYPE_NPD1:
		switch (hdr->raw[RAW_QAC]) {
		case QAC_GETVER:
			switch (hdr->hdr_s.packtype_u.npd1.data) {
			case (MYSELF + 1):
				/* readers ver */
				hdr->hdr_s.type = TYPE_NPDL;
				hdr->raw[RAW_DATA] = hdr->hdr_s.packtype_u.npd1.data;
				hdr->hdr_s.packtype_u.npdl.len = strlenpgm(ver) + 3;
				strcpypgm2ram((char *) &hdr->raw[RAW_DATA + 1], ver);
				bcp_send_buffer(handler);
				break;
			default:
				result = -1;
			}
			break;

		default:
			result = -1;
		}
		break;

	default:
		result = -1;
	}

	bcp_release_buffer(handler);
	TRACE("\n\rSM: buffer released (rsp sent)");

	return result;
}

static void sm_lcd_prompt(void)
{
	sprintf(LCD_STRING_0, "���������� �����");
	sprintf(LCD_STRING_1, "                ");
	LCD_decode(LCD_ALL);
	LCDUpdate();
}

void sm_init(void)
{
	// XXX check INs

	sm_lcd_prompt();

	mail_subscribe(MYSELF, &mailbox);
	state = SM_READY;

}

BOOL sm_is_ready(void) {
	return TRUE;	// XXX check it
}

static void sm_indicator_off(void)
{
	P_REL2 = 0;
}

static void sm_control_off(void)
{
	P_REL1 = 0;
}

static void sm_indicator(BOOL on)
{
	static DWORD t;

	if(on) {
		t = TickGet(); 
		P_REL2 = 1;
	} else if(P_REL2 && (TickGet() - t) > ((TICK_SECOND / 100L) * (DWORD)AppConfig.sm_sig_indicator_duration)) {
		P_REL2 = 0;
	}
}

static void sm_control(BOOL on)
{
	static DWORD t;

	if(on) {
		t = TickGet();
		P_REL1 = 1;
	} else if(P_REL1 && (TickGet() - t) > ((TICK_SECOND / 100L) * (DWORD)AppConfig.sm_sig_control_duration)) {
		P_REL1 = 0;
	}
}

 
void sm_module(void)
{
	bd_t ipacket;
	static DWORD t;

	if (mail_reciev(MYSELF, &ipacket))
			process_buffer(ipacket);

	sm_indicator(FALSE);
	sm_control(FALSE);

	switch(state) {
	case SM_INIT:
		break;
	case SM_READY:
		if(event_recieve(MYSELF, &event)) {
			if(event & EVT_SM_PREPARE) {
				state = SM_PREPARE;
			}
		}
		break;
	case SM_PREPARE:
		if(event_recieve(MYSELF, &event)) {
			switch(event) {
			case EVT_SM_ENABLE_INDICATOR | EVT_SM_ENABLE_CONTROL:
				sm_indicator(TRUE);
			case EVT_SM_ENABLE_CONTROL:
				sm_control(TRUE);
				t = TickGet();
				state = SM_WORK;
				break;
			case EVT_SM_ENABLE_INDICATOR:
				sm_indicator(TRUE);
				state = SM_READY;
				break;
			case EVT_SM_DISABLE:
				sm_indicator_off();
				state = SM_READY;
				break;
			}
		}
		break;
	case SM_WORK:
		if((TickGet() - t) > ((TICK_SECOND / 100L) * (DWORD)AppConfig.sm_service_time)) {
			event_send(MODULE_ACCESSOR, EVT_AC_TOUT);
			sm_lcd_prompt();
			state = SM_FINAL;
		} else if (P_IN1) {
			sm_control_off();
//			sm_indicator_off();
			event_send(MODULE_ACCESSOR, EVT_AC_DONE);
			sm_lcd_prompt();
			state = SM_FINAL;
		}
		break;
	case SM_FINAL:
		state = SM_READY;	// XXX
		break;
	case SM_FAILURE:
		break;
	}
}