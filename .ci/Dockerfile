FROM ubuntu:22.04
ARG DEBIAN_FRONTEND=noninteractive
ENV LANG=C.UTF-8 LC_ALL=C.UTF-8 TZ=Asia/Shanghai
RUN apt-get update && apt-get install -y --no-install-recommends \
    bash bc bison build-essential ca-certificates ccache cpio curl device-tree-compiler \
    dpkg-dev e2fsprogs expect fakeroot file flex git jq kmod libgmp-dev libmpc-dev \
    libncurses-dev libssl-dev lz4 lzop pkg-config python2 python3 rsync swig \
    u-boot-tools unzip wget xz-utils zip \
    && ln -s /usr/bin/python2 /usr/local/bin/python \
    && dpkg-query -W > /etc/elf2-ci-packages.txt \
    && rm -rf /var/lib/apt/lists/*
COPY .ci/fetch_deps.py /usr/local/lib/elf2-fetch-deps.py
COPY docs/migration/dependencies.json /opt/elf2-dependencies.json
RUN python3 /usr/local/lib/elf2-fetch-deps.py \
    --manifest /opt/elf2-dependencies.json --destination /opt/elf2-deps \
    --downloads /tmp/elf2-downloads \
    && rm -rf /tmp/elf2-downloads

