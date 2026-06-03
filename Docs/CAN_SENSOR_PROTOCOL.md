# CAN 水质传感器通信与验证说明

> 说明：目前没有具体传感器厂商/型号手册，因此本分支实现的是一套可直接联调的工程默认协议。若实际传感器已有固定 CAN 协议，只需要修改 `App/canSensorProtocol.c` 中的 `canSensorTable`、`CanSensorProtocol_BuildReadRequest()` 和 `CanSensorProtocol_ParseResponse()`。

## 1. 默认 CAN 参数

- CAN 外设：CAN1，PA11/CAN_RX，PA12/CAN_TX
- 默认模式：`CAN_MODE_NORMAL`
- 回环自测：在 `App/canTxTask.h` 中把 `CAN_SENSOR_CAN_MODE` 改为 `CAN_MODE_LOOPBACK`
- 波特率：500 kbps
  - APB1 = 42 MHz
  - SJW = 1TQ，BS1 = 7TQ，BS2 = 6TQ，Prescaler = 6
  - 42 MHz / ((1 + 7 + 6) * 6) = 500 kbps
- 帧格式：CAN2.0A 标准帧，数据帧，DLC = 8
- 接收方式：CAN FIFO0 消息挂起中断，ISR 内只取帧并写入 FreeRTOS 队列，解析在 `canRxTask` 内完成
- 发送方式：`canTxTask` 周期轮询四类传感器，每个传感器请求帧之间默认间隔 20 ms
- 任务优先级：`taskCanRx` 高于 `taskCanTx` 一级，收到帧后优先解析；STM32F4 bxCAN 接收不走 DMA

## 2. 默认传感器 ID 表

| 传感器 | 类型码 | 请求 StdId | 响应 StdId | 默认单位 |
| --- | ---: | ---: | ---: | --- |
| 余氯 | `0x01` | `0x101` | `0x181` | mg/L |
| 溶解氧 | `0x02` | `0x102` | `0x182` | mg/L |
| 铁离子 | `0x03` | `0x103` | `0x183` | mg/L |
| 铜离子 | `0x04` | `0x104` | `0x184` | mg/L |

## 3. 读取请求报文

主控每次读取某个传感器时发送 8 字节请求：

| Byte | 含义 |
| ---: | --- |
| 0 | 请求帧头，固定 `0xA5` |
| 1 | 传感器类型码 |
| 2 | 读命令，固定 `0x01` |
| 3 | 请求序号，主控每帧递增，传感器响应时原样返回 |
| 4 | 保留，固定 `0x00` |
| 5 | 保留，固定 `0x00` |
| 6 | 保留，固定 `0x00` |
| 7 | XOR 校验，Byte0 到 Byte6 逐字节异或 |

示例：

| 传感器 | StdId | Data[0..7] 示例 |
| --- | ---: | --- |
| 余氯 | `0x101` | `A5 01 01 00 00 00 00 A5` |
| 溶解氧 | `0x102` | `A5 02 01 01 00 00 00 A7` |
| 铁离子 | `0x103` | `A5 03 01 02 00 00 00 A5` |
| 铜离子 | `0x104` | `A5 04 01 03 00 00 00 A3` |

注：示例里的请求序号按发送顺序递增，因此校验也会随序号变化。

## 4. 响应报文

传感器返回 8 字节响应：

| Byte | 含义 |
| ---: | --- |
| 0 | 响应帧头，固定 `0x5A` |
| 1 | 传感器类型码 |
| 2 | 读响应命令，固定 `0x81` |
| 3 | 状态，`0x00` 表示正常，非 0 表示传感器错误 |
| 4 | 测量值低字节 |
| 5 | 测量值高字节 |
| 6 | 响应序号，应等于请求 Byte3 |
| 7 | XOR 校验，Byte0 到 Byte6 逐字节异或 |

测量值格式：

- `uint16_t raw = Byte4 + (Byte5 << 8)`
- `value = raw / 1000.0`
- 单位：mg/L
- 可表示范围：0.000 到 65.535 mg/L

示例：余氯传感器返回 1.234 mg/L，请求序号为 `0x10`：

- Raw = 1234 = `0x04D2`
- 响应 StdId = `0x181`
- Data = `5A 01 81 00 D2 04 10 1C`

## 5. 代码结构

- `Bsp/can.c`
  - `CAN1_Send_Frame()`：带 StdId 的标准帧发送，最多等待 `timeout_ms`
  - `HAL_CAN_RxFifo0MsgPendingCallback()`：CAN FIFO0 中断取帧，写入 `can1RxQueue`
  - `CAN1_Receive_Frame()`：任务从队列读取 CAN 帧
- `App/canSensorProtocol.c`
  - 协议 ID 表
  - 请求报文构造
  - 响应报文解析、校验、定点值转换
- `App/canTxTask.c`
  - 周期轮询四个传感器
  - 轮询周期接口：`CanTxTask_SetPollPeriodMs(uint32_t poll_period_ms)`
- `App/canRxTask.c`
  - 阻塞等待 CAN 接收队列
  - 解析成功后更新最新值缓存
  - 最新值读取接口：`CanRxTask_GetLatest(CanSensorType_t type, CanSensorSample_t *sample)`

## 6. 验证步骤

### 6.1 回环自测

1. 在 `App/canTxTask.h` 中临时加入或修改：

   ```c
   #define CAN_SENSOR_CAN_MODE CAN_MODE_LOOPBACK
   ```

2. 编译下载。
3. 串口查看日志，应能看到发送任务启动日志。
4. 因为回环收到的是请求帧，不是响应帧，默认解析层会忽略 `0x101` 到 `0x104` 请求 ID；如需验证解析链路，可用 CAN 工具或临时代码注入第 4 节响应帧。

### 6.2 CAN 分析仪验证发送帧

1. 接 CANH/CANL/GND，终端电阻 120 欧。
2. CAN 分析仪设置 500 kbps，标准帧。
3. 上电后应周期看到四个请求 ID：`0x101`、`0x102`、`0x103`、`0x104`。
4. 默认完整轮询周期为 2000 ms，每个请求帧间隔约 20 ms。

### 6.3 模拟传感器响应

用 CAN 分析仪发送一帧模拟响应，例如余氯 1.234 mg/L：

- StdId：`0x181`
- DLC：8
- Data：`5A 01 81 00 D2 04 10 1C`

串口应输出类似：

```text
can rx free_chlorine=1.234 mg/L status=0 seq=16
```

其他传感器可按 ID 表和类型码替换：

- 溶解氧：StdId `0x182`，Byte1 `0x02`
- 铁离子：StdId `0x183`，Byte1 `0x03`
- 铜离子：StdId `0x184`，Byte1 `0x04`

### 6.4 调整采样周期

在任意任务中调用：

```c
CanTxTask_SetPollPeriodMs(5000);
```

即可把四传感器轮询周期改为 5000 ms。最小周期由 `CAN_SENSOR_MIN_POLL_MS` 限制，默认 100 ms，防止误设置导致总线过载。

## 7. 接真实传感器时需要确认

1. 传感器波特率是否为 500 kbps。
2. 传感器使用标准帧还是扩展帧。
3. 每个传感器的节点 ID、请求 ID、响应 ID。
4. 读数据命令的真实载荷格式。
5. 返回值缩放系数、单位、字节序、错误状态定义。
6. 是否需要上电初始化、校准命令或地址配置命令。

确认这些后，优先改 `App/canSensorProtocol.c`，任务和 BSP 通信机制通常不需要再动。
