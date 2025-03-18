# 关于此 fbb_ws63

此仓库是基于官方开源的 fbb_ws63 sdk 修改适配 XFusion 的版本。

- 更多文档参见：[Coral 科洛社区 - 从 ws63 开始](https://www.coral-zone.cc/#/document?path=/document/zh_CN/get-started/starting_with_ws63.html)
- 与 XFusion 对接部分参见：[x-eks-fusion/port_xf_for_nearlink: NearLink port of XFusion.](https://github.com/x-eks-fusion/port_xf_for_nearlink)
- 原版参见：[fbb_ws63: fbb_ws63 代码仓为支持 ws63 和 ws63e 解决方案 SDK。](https://gitee.com/HiSpark/fbb_ws63)

## 注意事项

### 备忘录

1. sdk 本身自带的 bug.

   1. 【已修复】

      1. `致命错误： asm/memmap_config.h：没有那个文件或目录` 报错。

         sdk 覆盖了某些标准库实现，在使用某些头文件（`#include <limits.h>`）时会出现如上报错，这是由于 `kernel/liteos/liteos_v208.5.0/CMakeLists.txt` 内的 `LOS_HEADER` 缺少包含路径。

         见：`🐞 fix(liteos): 修复缺少包含路径`

      1. `🐞 fix(mid-sle): 补全缺失的头文件`：

         补全中间层 sle 中，传输管理文件中缺失的头文件包含：
         文件 `include/middleware/services/bts/sle/sle_transmition_manager.h` 中，漏加 `errcode.h` 文件的包含，会导致报错。

      1. `🐞 fix(drivers-i2c): 补全sdk缺失的接口`：i2c 驱动缺少控制停止位、起始位的接口，现已补充。

         i2c 驱动中，master 角色增加读写接口，具体为增加带可设置停止位、起始位的读写 api，原驱动文件中的读写接口均固定了停止与起始位控制，会导致部分读写时序异常不能被传感器识别。

      1. `🐞 fix(drivers-spi): 修复未返回传输结果`： spi 读取时超时时未返回实际读取个数，现已补充。

         spi v151 驱动中，修改 hal_spi_v151_write 实现的部分行为，具体为：

         超时返回时，增加更新 data->rx_bytes 的行为，告知调用侧剩余未收到的数据量。

   1. 【未修复】

      1. 暂无。

1. 其他新增或修改。

   1. 【新增】

      1. `✨ feat(tools): 补全lzma_tool`：ota 打包固件包时会用到。

         文件见：`src/tools/bin/lzma_tool`.

      1. `✨ feat(linker): 支持使用xf段`：为 XFusion 自动初始化提供支持。

         文件见：`src/drivers/boards/ws63/evb/linker/ws63_liteos_app_linker/linker.prelds`.

         修改：在 `.text` 段内添加 `.xf_auto_init`。

   1. 【修改】

      1. `🦄 refactor(app-ws63): 默认不使能内存占用报告`：关闭内存占用报告，不然运行时定时输出内存情况。

         文件：

         1. `src/application/ws63/ws63_liteos_application/main.c`.
         1. `src/application/Kconfig`.

         修改：加了开关宏。详见对应 commit。

      1. `🦄 refactor(build-config-ws63): 使能lwip的GW功能`：不使能时，DHCPc(station) 向 DHCPs(AP) 申请 IP 时无法获取网关。

         文件：`src/build/config/target_config/ws63/config.py`.

         修改：`defines` 内加入 `CONFIG_DHCPS_GW`.

      1. `🔧 build(app-samples): 支持构建xf`：XFusion 源码作为 samples 的一部分编译。

         文件：

         1. `src/application/samples/CMakeLists.txt`.
         1. `src/application/samples/Kconfig`.

         修改：添加配置，可以控制是否将 XFusion 源码加入构建。详见对应 commit。

      1. `🔧 build(build-config-ws63): 更新默认配置`：为适配 XFusion 而更新配置。

         文件：`src/build/config/target_config/ws63/menuconfig/acore/ws63_liteos_app.config`

         修改：详见对应 commit。
