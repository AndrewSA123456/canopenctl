#ifndef __MAIN_H
#define __MAIN_H

#include "globals.h"

#include "CANOpen.h"
typedef enum
{
	NMT_OPERATIONAL = 0x1,
	NMT_STOPPED = 0x2,
	NMT_PRE_OPERATIONAL = 0x80,
	NMT_RESET_APP = 0x81,
	NMT_RESET_COMM = 0x82,
	CMD_READ,
	CMD_WRITE,
	CMD_INVALID
} CommandType_t;

typedef struct
{
	char *interface;
	uint32_t node_id;
	CommandType_t command;
	uint16_t index;
	uint8_t subindex;
	dataTypeOD_t dataType;
	uint8_t *value;
	size_t valueSize;
} CanOpenShellArg;


#endif //__MAIN_H
