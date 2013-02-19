// *********** ��������� ������� ������ ���������� **************
#ifndef __CRCMOD_h
  #define __CRCMOD_h

//#include <stdint.h>
#include "TCPIP Stack/TCPIP.h"
typedef WORD uint16_t;
typedef BYTE uint8_t;

// �-�� ���������� CRC16 �� ��������� MODBUS ��� ������� ������, (������� 0xA001)
// �� ������ ������� ������� ��������� puchMsg; usDataLen - ���-�� ���� � ���c��� 
uint16_t CRC16_MODBUS(uint8_t *puchMsg, uint16_t usDataLen); 

#endif
