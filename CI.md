# Tag 自动构建

## 触发

将流水线配置推送到仓库后，从需要构建的提交推送版本 Tag：

```bash
git tag -a system-v0.1.0-rc.1 -m "ELF2 system image candidate"
git push origin system-v0.1.0-rc.1
```

格式为 `system-vMAJOR.MINOR.PATCH[-suffix]`。普通分支 push 不运行系统构建。手动 `web_trigger_clean` 入口可从仓库页面发起无缓存编译，产物附在该提交下。

## 构建过程

CNB 分配 amd64 / 16 核 / 32 GiB 容器。`.ci/Dockerfile` 构建 Ubuntu 22.04 主机环境，包含 Python 2/3 和 ccache；CNB 按 Dockerfile、依赖获取脚本和依赖清单缓存环境镜像。三份 SHA256 固定依赖在环境镜像内展开，避免每次重新下载。

CI 先将当前 Tag 的精确源码同步到独立缓存工作区，再运行厂商 `build.sh`。内核、U-Boot 对象文件原地缓存；增加的 `RK_CI_AARCH64_TOOLCHAIN` 仅在 CI 显式设置时生效，普通手动构建仍使用原工具链发现逻辑。

每次从未修改的基础 rootfs 开始。第一次 `all` 负责构建各组件，随后将本次内核的剥符号模块、depmod 索引和构建身份写入 rootfs，再重新打包 `update.img`。不依赖 loop 挂载。最后解开最终镜像，逐字节哈希比较 boot / recovery / rootfs / Loader，检查模块版本和必需内核配置。

rootfs 用户空间仍来自固定预编译基线；没有声称从源码重建全部 Buildroot。板上看门狗、Wi-Fi STA、dummy buffer 和相机应用修复不在本轮范围内。

## 增量缓存

- 固定分组的 CNB copy-on-write 卷分别保存构建工作区和 ccache。
- 工作区兼容键包含构建脚本、环境包版本、工具链/依赖清单和板级/Kconfig 配置，不包含 Tag 或完整 Commit。
- 使用 Git 同步当前提交，保留未变源码时间戳，正确处理删除文件和切换旧 Tag；不把旧缓存源码当作当前源码。
- 工作区中的 `.o`、依赖文件和生成头文件被保留；构建记录报告复用对象数量、上次成功提交及 ccache 统计。
- 同一缓存串行更新，发布成功才写成功标记；CNB 仅在流水线成功后合并 copy-on-write 改动。中断标记存在时丢弃该工作区重建。
- 节点目录缓存不是跨节点共享：换节点时允许冷编译。固定依赖镜像可以跨节点复用；跨节点中间文件快照暂未启用，应先比较传输与重编成本。
- 干净构建不复用对象，并禁用 ccache。

## 发布和失败证据

通过校验后自动创建对应 Tag 的预发布 Release，不设为 latest，不删除已有 Release。上传 Tag 前缀的 `update.img`、`boot.img`、`recovery.img`、DTB、配置、Module.symvers、构建信息、压缩日志和 SHA256SUMS，并核对服务端 SHA256。

若流水线临时令牌缺少 Release 权限，自动退回固定 Commit 附件；不同 Tag 的文件名前缀避免冲突。已有同名且同 SHA256 的附件可跳过，内容不同则停止，不覆盖已发布镜像。

失败阶段尽量将日志和失败信息保存在提交附件；失败阶段本身失败不替代原始构建错误。厂商脚本子进程只接收白名单环境变量，不接收 CNB_TOKEN。

构建成功不代表已通过实际开发板验收。刷写和硬件验证独立进行。

## 本地验证

在 Linux 主机上安装 README 中的主机依赖以及 ccache / kmod / e2fsprogs，恢复固定依赖，使用独立缓存目录运行：

```bash
python3 -m unittest discover -s .ci/tests -v
ELF2_DEPENDENCY_ROOT=/path/to/restored-dependencies \
ELF2_WORK_CACHE=/path/to/owned-build-cache \
ELF2_CCACHE_DIR=/path/to/owned-ccache python3 .ci/build.py
python3 .ci/build.py --finish
```

`.ci/publish.py` 只在具备 CNB 流水线环境时发布；本地构建不会主动上传。改变源码后应先提交，因为构建针对 Git HEAD 的准确内容。
