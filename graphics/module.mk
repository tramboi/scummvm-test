MODULE := graphics

MODULE_OBJS := \
	conversion.o \
	cursorman.o \
	dither.o \
	font.o \
	fontman.o \
	fonts/consolefont.o \
	fonts/newfont_big.o \
	fonts/newfont.o \
	fonts/scummfont.o \
	iff.o \
	imagedec.o \
	jpeg.o \
	pict.o \
	primitives.o \
	scaler.o \
	scaler/thumbnail_intern.o \
	sjis.o \
	surface.o \
	thumbnail.o \
	VectorRenderer.o \
	VectorRendererSpec.o \
	video/avi_decoder.o \
	video/coktel_decoder.o \
	video/dxa_decoder.o \
	video/flic_decoder.o \
	video/mpeg_player.o \
	video/qt_decoder.o \
	video/smk_decoder.o \
	video/video_decoder.o \
	video/codecs/cinepak.o \
	video/codecs/mjpeg.o \
	video/codecs/msrle.o \
	video/codecs/msvideo1.o \
	video/codecs/qdm2.o \
	video/codecs/qtrle.o \
	video/codecs/rpza.o \
	video/codecs/smc.o \
	video/codecs/indeo3.o

ifdef USE_SCALERS
MODULE_OBJS += \
	scaler/2xsai.o \
	scaler/aspect.o \
	scaler/downscaler.o \
	scaler/scale2x.o \
	scaler/scale3x.o \
	scaler/scalebit.o

ifdef USE_ARM_SCALER_ASM
MODULE_OBJS += \
	scaler/downscalerARM.o \
	scaler/scale2xARM.o \
	scaler/Normal2xARM.o
endif

ifdef USE_HQ_SCALERS
MODULE_OBJS += \
	scaler/hq2x.o \
	scaler/hq3x.o

ifdef USE_NASM
MODULE_OBJS += \
	scaler/hq2x_i386.o \
	scaler/hq3x_i386.o
endif

endif

endif

# Include common rules
include $(srcdir)/rules.mk
