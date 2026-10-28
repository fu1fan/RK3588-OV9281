# ELF2 HDMI 原生模式与启动接管修复

## 板上证据

system-v0.1.0-systemfix.2 接 Waveshare 7inch HDMI LCD (C)：HDMI 实际输出 1920×1080@60Hz。EDID 初次读取失败，后续读取成功，原生时序为 `49000 1024 1072 1168 1312 600 603 613 624`，但 DRM 拒绝并记录 `CLOCK_RANGE`。

接屏启动日志在 `dw_hdmi_qp_bridge_attach` 内触发 SError，寄存器地址为 `0xbc4`，对应 `PKT_VSI_CONTENTS1`。旧代码把 HPD 与 DDC 控制寄存器非零当作 bootloader 已完成显示初始化的依据，然后读取包寄存器；这不足以保证视频控制器时钟有效。

Weston 启动脚本每次重写 profile，强制禁用 atomic，并在 `disp_type` 缺失时开启镜像。HDMI 60Hz 与 DSI 约56Hz 共用一个1080p缓冲区。板上临时启用 atomic、独立输出后，两个输出使用独立缓冲区；移动白条测试仍被用户观察到内部纹理，不能据此宣称撕裂或图像质量已完全修复。

## 改动依据

- 给 kernel 的 `display_subsystem` 增加 `hdmi0_phy_pll`。同树 `rk3588-nvr.dtsi` 已使用此绑定，VOP2 会选择 HDMI PHY PLL 并允许非 CEA 像素时钟；HDPTX 驱动支持动态 PLL 计算。保留模式与硬件校验，不强行放行模式，也不把某块屏幕的时序硬编码进配置。
- 移植 Rockchip develop-5.10 后续的 `CMU_STATUS & HDMI_CTRL_CLK_EN` 接管检查，仅在控制器时钟就绪时继承 bootloader 状态。这使只完成 DDC 初始化的设备走内核正常 modeset，避免提前读取 `PKT_VSI_CONTENTS1`。
- Weston 默认开启 atomic，并使用独立输出；只有明确设置 `disp_type=sync` 才启用镜像。缺失或读取失败的 bootloader 环境变量不再默认为镜像。现有 `async` 行为保留。
- 在最终 ext4 安装并逐字节核对 `elfboard_env.sh` 和 `weston.sh`，避免预构建 rootfs 留存旧配置。

上游参考：https://github.com/rockchip-linux/kernel/blob/develop-5.10/drivers/gpu/drm/bridge/synopsys/dw-hdmi-qp.c 。屏幕规格：https://docs.waveshare.net/7inch_HDMI_LCD_C/ 。

## 验证

固定 `.ci/Dockerfile` 环境中执行 `.ci/tests`、`system/tests`、`system/elf2-wdctl/tests`，共36项通过。新增测试从实际驱动提取接管条件，遍历HPD、DDC和32种时钟状态；对旧源码执行时触发断言失败，修复后通过。它验证控制流条件，不代替真实总线或接屏启动测试。

完整镜像构建、49MHz实际输出、接屏启动及动态画面验收结果须另行记录；软件测试通过不代表这些硬件验收已经通过。

固定 GCC 10.3 工具链下，dw-hdmi-qp.o 与 elf2-kernel.dtb 编译通过；fdtget 确认 display-subsystem 包含 hdmi0_phy_pll。
