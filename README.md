# ELF2 RK3588 / OV9281 基础系统构建材料

从现有 ELF2 Linux SDK 摘取的基础系统源码、板级配置和必要依赖。快照日期：2026-10-25。

本次只做材料迁移与构建验证，不包含 CNB 流水线、Docker 构建环境或自动部署开发。

## 构建范围

- 板型：飞凌 ELF2，RK3588，Buildroot 方案。
- Linux 5.10.209；U-Boot 2017.09。
- 配置入口：`elf2_sdkbuild_defconfig`。
- 内核配置：`kernel/arch/arm64/configs/elf2_kernel_defconfig`。
- 设备树：`kernel/arch/arm64/boot/dts/rockchip/elf2-kernel.dts` 及其引用文件。
- OV9281：保留当前驱动和 `csi2_dphy5 → mipi5_csi2 → rkcif_mipi_lvds5` 设备树修正。
- 内核、U-Boot、恢复镜像及完整 `update.img` 的原厂构建入口均保留。

系统 rootfs 目前沿用固定的预编译基线，不是已经验证可从零重建全部用户空间的软件发行版。Buildroot 源码、原有配置和本地软件包材料同时保留，供后续开发。

## 获取必要依赖

源码在 Git；固定工具链、基础 rootfs 和厂商二进制材料位于本仓库 `sdk-dependencies-20261025` 版本附件。

下载附件后，在仓库根目录执行：

```bash
sha256sum -c docs/migration/SHA256SUMS
tar -xJf elf2-aarch64-toolchain-20261025.tar.xz
tar -xJf elf2-base-rootfs-sanitized-20261025.tar.xz
tar -xJf elf2-vendor-build-assets-20261025.tar.xz
```

三个包都需要恢复。它们按 SDK 原有相对路径展开，不能仅克隆 Git 就跳过依赖。文件大小和 SHA256 见 `docs/migration/dependencies.json`；逐文件清单见 `docs/migration/file-inventory.tsv.gz`。

## 手动构建

构建主机使用 Linux x86_64。已有环境为 WSL2 Ubuntu 24.04；厂商 SDK 面向 Ubuntu 22.04。主机需要 bash、make、gcc/g++、bison、flex、cpio、unzip、zip、dtc、lz4、lzop、u-boot-tools、OpenSSL/ncurses 开发包、swig、GMP/MPC 开发包、expect、fakeroot 和 Python 2.7。厂商脚本调用 `python2`，部分脚本通过 `python` 调用 Python 2；应先在构建主机准备好。依赖归档只提供目标交叉工具链，不提供主机 apt 包或容器。

```bash
# 避免继承 Windows 中含空格的 PATH。
export PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin
./build.sh elf2_sdkbuild_defconfig
./build.sh all
```

完整镜像输出在 `output/update/Image/update.img`。常用分阶段命令：

```bash
./build.sh kernel       # 内核、DTB 和 boot.img
./build.sh uboot        # U-Boot / Loader
./build.sh updateimg    # 打包已准备好的各分区，不能替代前面的编译/安装步骤
```

**厂商脚本的现有行为：**显式运行 `./build.sh rootfs`、`buildroot` 或 `recovery`，会移动 `prebuilts/elfboard/` 中对应预编译目录，转入源码编译。要复现当前预编译基线流程，使用上面的 `all` 入口；需要重新从预编译依赖开始时，重新解开相应附件。修改 overlay 不保证自动更新预编译 rootfs。

## 本次导入的边界

- 保留 SDK 当前已有的内核配置、OV9281 设备树、cgroup 和 dnsmasq 改动。
- 以原厂归档成员清单为基础读取当前文件，另补入新增的 `S01cgroup`、`S80dnsmasq`；不导入原目录的编译缓存或完整烧写镜像。
- 排除 Ubuntu/Debian/Yocto/RTOS/UEFI 路线、NPU 模型转换工具，以及未纳入当前构建的示例；当前 NPU 配置仍会构建 `rknn_common_test`，其代码和第三方依赖保留。
- rootfs 和相同的 Wi-Fi 配置模板已清除预置联网信息；镜像内旧 SSH 主机密钥及公钥已移除，原有 `S50sshd` 会在启动时重新生成。原 SDK 未被修改。
- 原厂示例签名私钥均已排除；含完整测试私钥的两份可选 TEE 静态库，以及 rootfs 中带私钥的 Qt SSL 示例也不公开分发。安全启动、TA 重签等非本次基础构建功能需要自行准备密钥和相应依赖。原许可证文件均保留，各组件分别遵循原许可证。
- 没有把开发板上的看门狗喂狗、Wi-Fi STA 自启、CIF dummy buffer 绕过措施和相机 Web 服务合入本次基础系统。

构建结果和未验证范围见 `docs/migration/VALIDATION.md`；板上尚未回写镜像的项目见 `docs/migration/BOARD_ONLY_CHANGES.md`。
