# BlinkLED_WS

单颗 WS2812 状态灯，沿用 BlinkLED 的软件定时器与致命错误闪烁逻辑。
每 `blink_cycle` 毫秒切换亮灭；非中断致命错误时采用 125/125/500/500 ms 节奏。

参数：`spi`（LibXR::SPI）、`blink_cycle`（毫秒）、`red` / `green` / `blue`（0–255）。
MC02 默认蓝色亮度 16，250 ms 切换（完整周期 500 ms）。

SPI 必须独占，源时钟为 24 MHz；模块设为 /4、模式 0、MSB first，实际速率 6 MHz。
按 GRB 顺序编码为 0xC0/0xF0，每帧前后各保留 320 us 低电平复位。
发送缓冲区至少 504 字节。使用同步轮询传输，STM32SPI 的 DMA 阈值须大于帧长度，
避免定时器线程等待中断以及临时帧的异步生命周期问题。MC02 使用 UINT32_MAX。

中断内致命错误只停止心跳，与 BlinkLED 一致。该模块同时只能创建一个实例。
编码回归测试由工程 tools/communication_tests 中的 blink_led_ws_test 运行。
