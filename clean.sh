#!/bin/bash

export CROSS_COMPILE=aarch64-linux-gnu-
export TEEC_EXPORT=../../../optee_client/out/export/usr
export PLATFORM=vexpress-juno
export TA_DEV_KIT_DIR=../../../optee_os/out/arm-plat-vexpress/export-ta_arm64

make clean

# 检查文件是否存在
if [ -f "error.log" ]; then
  # 删除文件
  rm error.log
  rm error.out
  echo "Remove error logs"
else
  echo "No error logs"
fi