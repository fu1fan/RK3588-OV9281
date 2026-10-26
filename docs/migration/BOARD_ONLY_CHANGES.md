# 板上变更与镜像修复的对应关系

本文件记录原开发板上的历史解法与当前镜像内实现的对应关系。整个过程没有连接或改写开发板。

| 项目 | 现有板上方案 | 本次状态 |
|---|---|---|
| 看门狗导致约 40–90 秒复位 | elf2-wdctl + S80watchdog | 已固化为源码 + rootfs 安装链，见下 |
| Wi-Fi STA 开机关联与 DHCP | S36wifistation | 已固化为源码 + rootfs 安装链，见下 |
| CIF dummy buffer 导致 STREAMON / ENOMEM | 启流前写 is_use_dummybuf=0 | 改为内核修复，不再需要 sysfs 绕过 |
| Web 预览、用户态 AE、录像等应用 | elf2-camweb / S85elf2-camweb 等 | 独立应用，仍未导入 |

前两项已改为本仓库源码，由 `.ci/rootfs_services.py` 交叉编译/校验后写入最终 rootfs；板上
`S80watchdog`、`S36wifistation` 的原始脚本内容未从开发板回读，这里是按 Issue 描述重新实现的
等价（更保守）实现。第三项是内核修复，与板上 sysfs 绕过无关。

源码内的修复对应用户态行为变化如下，但**都没有在开发板上验证**：

- 复位稳定性、看门狗超时后是否真的复位：未测。
- AX200 关联、DHCP 获取地址、断线重连：未测。
- `/dev/video11` 启流、dummy buffer 尺寸、帧完整性：未测。

本文档只说明源码与镜像内容层面的改动，不等于硬件验收。
