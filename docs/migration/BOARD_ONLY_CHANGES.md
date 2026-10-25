# 本轮未合入的板上变更

本次迁移不处理这些系统修复，也没有连接或改写开发板。

| 项目 | 现有板上方案 | 本次状态 |
|---|---|---|
| 看门狗导致约 40–90 秒复位 | elf2-wdctl + S80watchdog | 未合入 |
| Wi-Fi STA 开机关联与 DHCP | S36wifistation | 未合入 |
| CIF dummy buffer 导致 STREAMON / ENOMEM | 启流前写 is_use_dummybuf=0 | 未合入 |
| Web 预览、用户态 AE、录像等应用 | elf2-camweb / S85elf2-camweb 等 | 独立应用，本轮未导入 |

这份材料保留既有基础系统行为，不能因为源码上传或编译成功就声称以上问题已在镜像中修复。
