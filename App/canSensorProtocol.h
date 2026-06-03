#ifndef __CANSENSORPROTOCOL_H
#define __CANSENSORPROTOCOL_H

#include "can.h"
#include <stdint.h>

#define CAN_SENSOR_COUNT               4U
#define CAN_SENSOR_REQUEST_DLC         8U
#define CAN_SENSOR_RESPONSE_DLC        8U
#define CAN_SENSOR_DEFAULT_POLL_MS     5000U
#define CAN_SENSOR_MIN_POLL_MS         100U
#define CAN_SENSOR_INTER_FRAME_MS      20U
#define CAN_SENSOR_TX_TIMEOUT_MS       10U

#define CAN_SENSOR_REQ_MAGIC           0xA5U
#define CAN_SENSOR_RESP_MAGIC          0x5AU
#define CAN_SENSOR_READ_CMD            0x01U
#define CAN_SENSOR_READ_RESP_CMD       0x81U
#define CAN_SENSOR_VALUE_SCALE         1000.0f

typedef enum
{
    CAN_SENSOR_FREE_CHLORINE = 1,
    CAN_SENSOR_DISSOLVED_OXYGEN,
    CAN_SENSOR_IRON_ION,
    CAN_SENSOR_COPPER_ION,
} CanSensorType_t;

typedef enum
{
    CAN_SENSOR_PARSE_OK = 0,
    CAN_SENSOR_PARSE_INVALID_ARG,
    CAN_SENSOR_PARSE_UNKNOWN_ID,
    CAN_SENSOR_PARSE_BAD_DLC,
    CAN_SENSOR_PARSE_BAD_MAGIC,
    CAN_SENSOR_PARSE_BAD_CHECKSUM,
    CAN_SENSOR_PARSE_ERROR_STATUS,
} CanSensorParseStatus_t;

typedef struct
{
    CanSensorType_t type;
    const char *name;
    const char *unit;
    uint32_t request_id;
    uint32_t response_id;
    uint8_t type_code;
} CanSensorDescriptor_t;

typedef struct
{
    CanSensorType_t type;
    const char *name;
    float value;
    uint16_t raw_value;
    uint8_t status;
    uint8_t sequence;
    uint32_t tick;
} CanSensorSample_t;

const CanSensorDescriptor_t *CanSensorProtocol_GetTable(uint8_t *count);
const CanSensorDescriptor_t *CanSensorProtocol_FindByType(CanSensorType_t type);
const CanSensorDescriptor_t *CanSensorProtocol_FindByResponseId(uint32_t response_id);

uint8_t CanSensorProtocol_BuildReadRequest(const CanSensorDescriptor_t *sensor,
                                           uint8_t sequence,
                                           uint8_t *data,
                                           uint8_t len);

CanSensorParseStatus_t CanSensorProtocol_ParseResponse(const CanFrame_t *frame,
                                                       CanSensorSample_t *sample);

const char *CanSensorProtocol_ParseStatusText(CanSensorParseStatus_t status);
uint8_t CanSensorProtocol_Checksum(const uint8_t *data, uint8_t len_without_checksum);

#endif
