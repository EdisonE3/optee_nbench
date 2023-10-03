export CROSS_COMPILE=aarch64-linux-gnu-
export TEEC_EXPORT=../../../optee_client/out/export/usr
export PLATFORM=vexpress-juno
export TA_DEV_KIT_DIR=../../../optee_os/out/arm-plat-vexpress/export-ta_arm64

make VERBOSE=1
