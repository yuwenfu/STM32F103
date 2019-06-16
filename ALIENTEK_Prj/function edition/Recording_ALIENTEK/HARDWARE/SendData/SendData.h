#ifndef __SendData_H
#define __SendData_H
#include "sys.h"


u8 DataSendInit(void);
u8 DataSend(u8 type,u8 *pname,u16 rate);

void create_new_pathname(u8 *pname);
void create_nrev_pathname(u8 *pname);
void create_nsendmsc_pathname(u8 *pname);
void create_nsendrec_pathname(u8 *pname);
void create_nsendtxt_pathname(u8 *pname);

u8 musictransfer(u8 number,u16 rate);
u8 txttransfer(u8 number,u16 rate);
u8 recordertransfer(u8 number,u16 rate);
#endif

