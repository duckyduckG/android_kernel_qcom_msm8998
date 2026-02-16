ifeq ($(CONFIG_BUILD_ARM64_DT_OVERLAY),y)
dtbo-$(CONFIG_ARCH_SDM660) += \
	sdm660-mtp-overlay.dtbo \
	sdm636-mtp-overlay.dtbo

sdm660-mtp-overlay.dtbo-base := sdm660-mtp.dtb
sdm636-mtp-overlay.dtbo-base := sdm636-mtp.dtb
endif

always		:= $(dtb-y)
subdir-y	:= $(dts-dirs)
clean-files	:= *.dtb
