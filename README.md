# STM32F103 Keil Flash Algorithm for W25Q64

![Language](https://img.shields.io/badge/language-C-blue.svg) ![Platform](https://img.shields.io/badge/platform-STM32-green.svg) ![Toolchain](https://img.shields.io/badge/toolchain-Keil_MDK-orange.svg)

这是一个用于 Keil MDK 的外部 Flash 下载算法（FLM），基于 **STM32F103RCT6** 实现，用于烧录连接在 SPI1 上的 **W25Q64** 闪存芯片。

此算法解决了官方算法不支持自定义引脚、速度慢以及编译报错（L6235E）等问题。采用**寄存器直接操作**，无 HAL 库依赖，代码体积小，速度快。

## 🚀 特性 (Features)

* **纯寄存器实现**：不依赖 HAL 库，无需中断向量表，极简启动。
* **高兼容性**：解决了常见的 Scatter 文件配置错误 (`L6235E: More than one section...`)。
* **速度优化**：
    * 默认使用 HSI 时钟，SPI 运行在 **4MHz** 。
    * 如不稳定可调整分频系数
* **支持功能**：`Init`, `UnInit`, `EraseSector`, `EraseChip`, `ProgramPage`, `Verify`。

## 🔌 引脚定义 (Pinout)

本项目默认使用 **SPI1** 接口，引脚映射如下：

| STM32 Pin | W25Q64 Pin | Description |
| :--- | :--- | :--- |
| **PA4** | CS | 片选 (Software Control) |
| **PA5** | CLK | 时钟 |
| **PA6** | MISO | 数据输出 |
| **PA7** | MOSI | 数据输入 |

> **注意**：如果你的硬件连接不同，请修改 `FlashPrg.c` 中的 `Init` 函数。

## 📥 安装与使用 (Installation)

### 方法 1：直接使用 (Pre-built)
1. 下载 `Binary` 文件夹下的 `W25Q64_STM32F103.FLM` 文件。
2. 将文件复制到 Keil 安装目录的 Flash 文件夹中：
   * 路径通常为：`C:\Keil_v5\ARM\Flash`
3. 打开你的 STM32 应用程序工程：
   * 点击 `Options for Target` (魔法棒) -> `Debug` -> `Settings` -> `Flash Download`。
   * 点击 `Add`，在列表中找到 `W25Q64_STM32F103` 并添加。

### 方法 2：自行编译 (Build from Source)
1. 使用 Keil MDK 5 打开 `Project` 目录下的工程文件。
2. 确保已选择 `STM32F103RCT6` 芯片。
3. 点击 `Rebuild`。
4. 编译成功后，脚本会自动将生成出的 `.axf` 重命名为 `.FLM` (需配置 User Command) 或手动在 Output 目录寻找。

## 🛠️ 配置说明 (Configuration)

### 修改 SPI 速度
在 `FlashPrg.c` 的 `Init` 函数中：

```c
// 当前设置：2分频 (4MHz @ HSI 8MHz) -> 推荐
SPI1_CR1 |= (0 << 3) | ...; 

// 如需降低至 2MHz：
// SPI1_CR1 |= (1 << 3) | ...;
```

## 📄 License
MIT License. 欢迎 Fork 和 Star！