#!/bin/bash
set -e      # 遇到错误立即退出

# 创建镜像
dd if=/dev/zero of=disk.img bs=1M count=10     # 创建一个10mb的.img

# 绑定设备
LOOPDEV=$(sudo losetup -f)
sudo losetup -P $LOOPDEV disk.img

# 格式化
# sudo ./myfs-format $LOOPDEV
