# 2026-10-27 系统修复验证

基线：`ac46aa736ab77106707db5c75168c8507fad5ffe`。本轮修复延续 NPC 的 rootfs 安装链与构建框架，不覆盖旧 Tag、不烧录开发板。

## 修正内容与依据

- Wi-Fi：恢复 `/etc/wifistation.conf`；运行状态统一保存在 `/var/run/wifistation`。按 NUL 分隔的实际 argv 校验进程名称及私有配置/PID 路径；stop 使用启动时保存的接口，避免读取新配置后停错接口。串行执行 start/stop/restart，重复 start 复用正在运行的服务。
- DHCP：以 `udhcpc -f` 在后台持续运行，不使用会退出的 `-n`/`-q`。AP 不可达时不等待关联或租约，后续关联后仍可继续请求地址。原 ASSOC_TIMEOUT/DHCP_TIMEOUT 不再使用，模板已更新。
- 配置：SSID 写为十六进制，PSK 转义引号和反斜杠，运行目录与配置仅 root 可访问。支持 WPA-PSK 或 NONE；未增加 WPA3 SAE 支持。
- 看门狗：读取 `WDIOC_GETTIMEOUT`，喂狗间隔最多为实际超时的一半；SETTIMEOUT 失败时查询当前值，无法确定安全超时或首次喂狗失败则报错。运行期 ioctl 失败退出并保留已启用的 watchdog。后台父进程等待喂狗线程创建成功才报告成功；正常停止先 join 喂狗线程，避免关闭 FD 与 ioctl 并发。
- DW watchdog：撤回 NPC 的“仅把 wdd->timeout 改成 30”补丁。该补丁没有设置硬件 TOP，却改变上报值；当前 `dw_wdt_set_timeout` 本身支持选择硬件 TOP，并在超出硬件范围时由 watchdog core 辅助喂狗。没有证据支持原补丁声称的“后续 set_timeout 全部失败”。本轮保留基线驱动接管逻辑，真实启动接管时序仍需板测。
- OV9281/CIF：枚举接受调用方的 `code=0`，同时返回实际 code。对每个传感器查询已清零的 active format，避免另一个传感器已有枚举结果时漏掉不支持枚举的设备。校验零尺寸及 u32/page-align 溢出，保留分配错误码。撤回无必要的 reuse 参数，维持原 dev_lock/共享缓冲区生命周期。

## 已执行验证

在 CNB 开发容器中执行：

```sh
python3 -m unittest discover -s .ci/tests -v
python3 -m unittest discover -s system/tests -v
python3 -m unittest discover -s system/elf2-wdctl/tests -v
```

分别 12、12、11 项通过，共 35 项，无跳过。CI 辅助测试包含 ext4 rootfs 文件安装/权限验证。

Wi-Fi 测试要求没有现存 STA 配置/运行目录的临时 root 环境。测试直接执行未改写的正式脚本，使用真实 `/etc`、`/var/run`、`/proc`；网络命令为保留真实 argv 的 C 桩程序，不修改真实网卡。覆盖延迟关联、重复启动、stop/restart、配置变化、字面密码及旧 PID 不误杀其他进程。该测试不是实际无线协议测试。

CIF 测试提取源码中的完整函数并编译执行，使用仓库 media-bus 常量和 V4L2/分配器桩，覆盖 `code=0`、明确不匹配 code、索引越界、多传感器回退、RAW/RGB 尺寸、零尺寸、溢出及分配失败。它不模拟 DMA 或实际出流。

将关键新测试回跑基线源码，确认以下三项按预期失败：正式路径的 Wi-Fi 启动、实际超时缩短后的看门狗喂狗、OV9281 `code=0` 枚举。

使用开发容器的 AArch64 GCC 14 编译通过：

```sh
make -C kernel O=/tmp/elf2-repair-kbuild ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- elf2_kernel_defconfig
make -C kernel O=/tmp/elf2-repair-kbuild ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- -j8 \
  drivers/media/i2c/ov9281.o drivers/media/platform/rockchip/cif/capture.o drivers/watchdog/dw_wdt.o
aarch64-linux-gnu-gcc -O2 -Wall -Wextra -Werror -std=gnu99 \
  system/elf2-wdctl/elf2-wdctl.c -lpthread -o /tmp/elf2-wdctl-aarch64
```

上述编译为源码检查，产物不用于发布；正式镜像仍须使用 `.ci/Dockerfile` 固定工具链，以匹配目标 rootfs ABI。

## 尚未验证

本轮未构建或发布新的完整 update.img。旧 `system-v0.1.0-systemfix.1` 不含本轮修正。

仍需固定工具链的完整镜像构建、最终镜像/附件哈希校验，以及开发板上的复位稳定性、nowayout 行为、AX200 关联/DHCP/重连、OV9281 STREAMON 和帧完整性测试。不能用本轮主机测试或对象编译替代这些验收。
