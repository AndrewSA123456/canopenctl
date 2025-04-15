#ifndef __MAIN_H
#define __MAIN_H

#include "globals.h"

typedef struct
{
	uint32_t id;
	uint8_t len;
	uint8_t data[8];
} canFrame_t;
void CO_sdo_handler(canFrame_t *canRequestMsg);

#define SDO_CS_Pos (5)
#define SDO_CS_Msk ((uint8_t)(0x7 << SDO_CS_Pos))

#define SDO_CS_ABORT ((uint8_t)0x4 << SDO_CS_Pos)

#define SDO_CCS_DL_INIT ((uint8_t)(0x1 << SDO_CS_Pos))
#define SDO_CCS_DL_SEG ((uint8_t)(0x0 << SDO_CS_Pos))
#define SDO_CCS_UL_INIT ((uint8_t)(0x2 << SDO_CS_Pos))
#define SDO_CCS_UL_SEG ((uint8_t)(0x3 << SDO_CS_Pos))

#define SDO_SCS_DL_INIT ((uint8_t)(0x3 << SDO_CS_Pos))
#define SDO_SCS_DL_SEG ((uint8_t)(0x20))
#define SDO_SCS_DL_SEG_TOGGLED ((uint8_t)(0x30))
#define SDO_SCS_UL_INIT ((uint8_t)(0x2 << SDO_CS_Pos))
#define SDO_SCS_UL_SEG ((uint8_t)(0x0))
#define SDO_SCS_UL_SEG_TOGGLED ((uint8_t)(0x10))

#define SDO_E_Pos (1)
#define SDO_E_Msk ((uint8_t)(0x1 << SDO_E_Pos))

#define SDO_S_Pos (0)
#define SDO_S_Msk ((uint8_t)(0x1 << SDO_S_Pos))

#define SDO_N_INIT_Pos (2)
#define SDO_N_INIT_Msk ((uint8_t)(0x3 << SDO_S_Pos))

#define SDO_N_Pos (1)
#define SDO_N_Msk ((uint8_t)(0x7 << SDO_S_Pos))

#define SDO_T_Pos (4)
#define SDO_T_Msk ((uint8_t)(0x1 << SDO_S_Pos))

#define SDO_C_Pos (0)
#define SDO_C_Msk ((uint8_t)(0x1 << SDO_C_Pos))

#define ABORT_SDO_INVALID_SIZE_DATA (0x06070010U)

enum
{
	SDO_REQUEST_DL = 1,
	SDO_REQUEST_UL
};
enum
{
	SDO_REQUEST_OK,
	SDO_REQUEST_ERROR
};
#define WAIT_COUNT 1000
#define SDO_REQUEST_ID(nodeID) ((uint32_t)(0x600 + nodeID))

#endif //__MAIN_H
