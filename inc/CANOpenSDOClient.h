#ifndef __CANOPEN_SDO_CLIENT_H
#define __CANOPEN_SDO_CLIENT_H

#include "globals.h"


int CO_sdo_request(int sockDesc,
    uint16_t cmd,
    uint32_t nodeID,
    uint16_t index,
    uint8_t subindex,
    uint8_t **buff,
    size_t *buffSize);
    
#endif //__CANOPEN_SDO_CLIENT_H