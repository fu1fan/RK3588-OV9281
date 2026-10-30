# 版本化应用 SDK

系统流水线在系统镜像验证和可选增量复验后执行 `.ci/export_sdk.py`，将 SDK 发布为
`docker.cnb.cool/fu1fan/rk3588-ov9281/app-sdk:build-<CNB_BUILD_ID>`。
消费者必须固定构建日志输出的 `@sha256:...`，不能依赖可变标签。

产物镜像不运行服务，`/sdk` 包含：

- `toolchain/`：该次系统构建使用的 AArch64 工具链；
- `sysroot/`：工具链开发 sysroot，叠加该次最终 rootfs 的 lib、usr/lib、usr/include；
- `system.json`：从最终 rootfs 读取的系统身份；
- `manifest.json`：系统提交、标签、内核版本、依赖清单和最终 rootfs 校验值。

导出时校验系统构建状态和 rootfs 身份，修正 sysroot 内的绝对软链，并把 SDK
复制到另一临时路径编译链接 C、C++、Vulkan 样例，失败禁止发布 SDK。
这是应用 SDK，不包含完整内核源码或保证可用于编译内核模块的开发树。update.img、
内核配置、Module.symvers 等仍由原有系统附件发布流程交付。

使用 VibraReveal 的 `elf2-sdk`：在 `10-env/sdk.lock.json` 中同时记录 image digest
和 system_commit，执行 `elf2-sdk use` 或 `elf2-sdk run cmake ...`。首次拉取后提取到
本地缓存，以后直接使用缓存；不要求系统构建容器持续运行。

注意：运行时镜像不一定包含所有第三方开发头文件。新增库时应同时补充对应开发文件，
增加真实编译链接验证，不能把编译通过视为开发板 ABI/运行验收。只在配套系统上部署。
