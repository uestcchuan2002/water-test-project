#include "canSensorProtocol.h"
#include <stddef.h>

static const CanSensorDescriptor_t canSensorTable[CAN_SENSOR_COUNT] =
{
    {
        CAN_SENSOR_FREE_CHLORINE,
        "free_chlorine",
        "mg/L",
        0x101U,
        0x181U,
        0x01U,
    },
    {
        CAN_SENSOR_DISSOLVED_OXYGEN,
        "dissolved_oxygen",
        "mg/L",
        0x102U,
        0x182U,
        0x02U,
    },
    {
        CAN_SENSOR_IRON_ION,
        "iron_ion",
        "mg/L",
        0x103U,
        0x183U,
        0x03U,
    },
    {
        CAN_SENSOR_COPPER_ION,
        "copper_ion",
        "mg/L",
        0x104U,
        0x184U,
        0x04U,
    },
};

/**
 * @brief 获取CAN传感器描述符表及其元素数量。
 *
 * 该函数返回指向全局CAN传感器描述符表的指针，并可选地通过输出参数返回表中元素的数量。
 *
 * @param[out] count 若不为NULL，则通过该指针返回CAN传感器描述符表中的元素个数；
 *                   若为NULL，则不写入任何值。
 *
 * @return 指向CAN传感器描述符表的常量指针，表内容不可修改。
 */
const CanSensorDescriptor_t *CanSensorProtocol_GetTable(uint8_t *count)
{
    if (count != NULL)
    {
        *count = CAN_SENSOR_COUNT;
    }

    return canSensorTable;
}

const CanSensorDescriptor_t *CanSensorProtocol_FindByType(CanSensorType_t type)
{
    for (uint8_t i = 0U; i < CAN_SENSOR_COUNT; i++)
    {
        if (canSensorTable[i].type == type)
        {
            return &canSensorTable[i];
        }
    }

    return NULL;
}

const CanSensorDescriptor_t *CanSensorProtocol_FindByResponseId(uint32_t response_id)
{
    for (uint8_t i = 0U; i < CAN_SENSOR_COUNT; i++)
    {
        if (canSensorTable[i].response_id == response_id)
        {
            return &canSensorTable[i];
        }
    }

    return NULL;
}

uint8_t CanSensorProtocol_Checksum(const uint8_t *data, uint8_t len_without_checksum)
{
    uint8_t checksum = 0U;

    if (data == NULL)
    {
        return 0U;
    }

    for (uint8_t i = 0U; i < len_without_checksum; i++)
    {
        checksum ^= data[i];
    }

    return checksum;
}

/**
 * @brief 构建CAN传感器读取请求数据帧
 *
 * 该函数根据指定的传感器描述符和序列号，构建一个符合CAN传感器协议的读取请求数据帧。
 * 请求帧包含固定格式的头部信息、命令码、序列号以及校验和。
 *
 * @param[in]  sensor   指向传感器描述符结构体的指针，包含传感器类型等信息
 * @param[in]  sequence 请求序列号，用于匹配请求与响应
 * @param[out] data     输出缓冲区，用于存放构建好的请求数据帧（至少8字节）
 * @param[in]  len      data缓冲区的长度，必须不小于CAN_SENSOR_REQUEST_DLC（通常为8）
 *
 * @return uint8_t
 *         - 0U：成功构建请求帧
 *         - 1U：参数无效（sensor或data为NULL，或len小于所需最小长度）
 */
uint8_t CanSensorProtocol_BuildReadRequest(const CanSensorDescriptor_t *sensor,
                                           uint8_t sequence,
                                           uint8_t *data,
                                           uint8_t len)
{
    /* 参数有效性检查：确保指针非空且缓冲区长度足够 */
    if ((sensor == NULL) || (data == NULL) || (len < CAN_SENSOR_REQUEST_DLC))
    {
        return 1U;
    }

    /* 填充CAN传感器读取请求帧的标准格式 */
    data[0] = CAN_SENSOR_REQ_MAGIC;
    data[1] = sensor->type_code;
    data[2] = CAN_SENSOR_READ_CMD;
    data[3] = sequence;
    data[4] = 0x00U;
    data[5] = 0x00U;
    data[6] = 0x00U;
    data[7] = CanSensorProtocol_Checksum(data, 7U);

    return 0U;
}

/**
 * @brief 解析CAN传感器协议的响应帧，并填充采样数据结构。
 *
 * 该函数验证传入的CAN帧是否为有效的传感器响应帧，包括ID、帧格式、数据长度、魔术字节、校验和等，
 * 并在验证通过后将解析出的原始值、状态、序列号等信息填充到sample结构体中。
 * 若帧中包含非零的状态字节，则视为传感器报告错误，返回相应错误码。
 *
 * @param[in]  frame   指向待解析的CAN帧的指针，必须为标准帧且包含完整的响应数据。
 * @param[out] sample  指向用于存储解析结果的采样数据结构的指针。
 *
 * @return CanSensorParseStatus_t 解析结果状态码：
 *         - CAN_SENSOR_PARSE_OK: 解析成功且传感器状态正常；
 *         - CAN_SENSOR_PARSE_INVALID_ARG: 输入参数为NULL；
 *         - CAN_SENSOR_PARSE_UNKNOWN_ID: 响应ID未匹配任何已知传感器；
 *         - CAN_SENSOR_PARSE_BAD_DLC: 帧格式或数据长度不符合协议要求；
 *         - CAN_SENSOR_PARSE_BAD_MAGIC: 魔术字节、类型码或命令码不匹配；
 *         - CAN_SENSOR_PARSE_BAD_CHECKSUM: 校验和验证失败；
 *         - CAN_SENSOR_PARSE_ERROR_STATUS: 传感器返回非零错误状态。
 */
CanSensorParseStatus_t CanSensorProtocol_ParseResponse(const CanFrame_t *frame,
                                                       CanSensorSample_t *sample)
{
    const CanSensorDescriptor_t *sensor;
    uint16_t rawValue;

    /* 检查输入参数有效性 */
    if ((frame == NULL) || (sample == NULL))
    {
        return CAN_SENSOR_PARSE_INVALID_ARG;
    }

    /* 根据响应ID查找对应的传感器描述符 */
    sensor = CanSensorProtocol_FindByResponseId(frame->std_id);
    if (sensor == NULL)
    {
        return CAN_SENSOR_PARSE_UNKNOWN_ID;
    }

    /* 验证CAN帧格式：必须为标准数据帧，且数据长度符合协议规定 */
    if ((frame->ide != CAN_ID_STD) ||
        (frame->rtr != CAN_RTR_DATA) ||
        (frame->dlc != CAN_SENSOR_RESPONSE_DLC))
    {
        return CAN_SENSOR_PARSE_BAD_DLC;
    }

    /* 验证协议头字段：魔术字节、传感器类型码和读取响应命令 */
    if ((frame->data[0] != CAN_SENSOR_RESP_MAGIC) ||
        (frame->data[1] != sensor->type_code) ||
        (frame->data[2] != CAN_SENSOR_READ_RESP_CMD))
    {
        return CAN_SENSOR_PARSE_BAD_MAGIC;
    }

    /* 验证数据帧的校验和（前7字节） */
    if (frame->data[7] != CanSensorProtocol_Checksum(frame->data, 7U))
    {
        return CAN_SENSOR_PARSE_BAD_CHECKSUM;
    }

    /* 从数据字节中组合16位原始值（小端序：data[4]=低字节，data[5]=高字节） */
    rawValue = (uint16_t)(((uint16_t)frame->data[5] << 8) | frame->data[4]);

    /* 填充采样结构体：类型、名称、原始值、换算后的浮点值、状态、序列号及时间戳 */
    sample->type = sensor->type;
    sample->name = sensor->name;
    sample->raw_value = rawValue;
    sample->value = (float)rawValue / CAN_SENSOR_VALUE_SCALE;
    sample->status = frame->data[3];
    sample->sequence = frame->data[6];
    sample->tick = frame->tick;

    /* 检查传感器返回的状态字节，非零表示传感器内部错误 */
    if (sample->status != 0U)
    {
        return CAN_SENSOR_PARSE_ERROR_STATUS;
    }

    return CAN_SENSOR_PARSE_OK;
}

const char *CanSensorProtocol_ParseStatusText(CanSensorParseStatus_t status)
{
    switch (status)
    {
        case CAN_SENSOR_PARSE_OK:
            return "ok";
        case CAN_SENSOR_PARSE_INVALID_ARG:
            return "invalid_arg";
        case CAN_SENSOR_PARSE_UNKNOWN_ID:
            return "unknown_id";
        case CAN_SENSOR_PARSE_BAD_DLC:
            return "bad_dlc";
        case CAN_SENSOR_PARSE_BAD_MAGIC:
            return "bad_magic";
        case CAN_SENSOR_PARSE_BAD_CHECKSUM:
            return "bad_checksum";
        case CAN_SENSOR_PARSE_ERROR_STATUS:
            return "sensor_error_status";
        default:
            return "unknown_status";
    }
}
