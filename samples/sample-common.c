/*
 * sample-common.c
 *
 * Copyright (C) 2014 Ingenic Semiconductor Co.,Ltd
 */

#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#include <imp/imp_log.h>
#include <imp/imp_common.h>
#include <imp/imp_system.h>
#include <imp/imp_framesource.h>
#include <imp/imp_encoder.h>
#include <imp/imp_isp.h>
#include <imp/imp_osd.h>

#include "logodata_100x100_bgra.h"

#include "sample-common.h"

#define TAG "Sample-Common"

static const int S_RC_METHOD = ENC_RC_MODE_SMART;

//#define SHOW_FRM_BITRATE
#ifdef SHOW_FRM_BITRATE
#define FRM_BIT_RATE_TIME 2
#define STREAM_TYPE_NUM 3
static int frmrate_sp[STREAM_TYPE_NUM] = { 0 };
static int statime_sp[STREAM_TYPE_NUM] = { 0 };
static int bitrate_sp[STREAM_TYPE_NUM] = { 0 };
#endif

struct chn_conf chn[FS_CHN_NUM] = {
	{
		.index = CH0_INDEX,
		.enable = CHN0_EN,
        .payloadType = PT_H265,
		.fs_chn_attr = {
			.pixFmt = PIX_FMT_NV12,
			.outFrmRateNum = SENSOR_FRAME_RATE_NUM,
			.outFrmRateDen = SENSOR_FRAME_RATE_DEN,
			.nrVBs = 2,
			.type = FS_PHY_CHANNEL,

			.crop.enable = CROP_EN,
			.crop.top = 0,
			.crop.left = 0,
			.crop.width = SENSOR_WIDTH,
			.crop.height = SENSOR_HEIGHT,

			.scaler.enable = 0,

			.picWidth = SENSOR_WIDTH,
			.picHeight = SENSOR_HEIGHT,
		   },
		.framesource_chn =	{ DEV_ID_FS, CH0_INDEX, 0},
		.imp_encoder = { DEV_ID_ENC, CH0_INDEX, 0},
	},
	{
		.index = CH1_INDEX,
		.enable = CHN1_EN,
        .payloadType = PT_H265,
		.fs_chn_attr = {
			.pixFmt = PIX_FMT_NV12,
			.outFrmRateNum = SENSOR_FRAME_RATE_NUM,
			.outFrmRateDen = SENSOR_FRAME_RATE_DEN,
			.nrVBs = 2,
			.type = FS_PHY_CHANNEL,

			.crop.enable = 0,
			.crop.top = 0,
			.crop.left = 0,
			.crop.width = SENSOR_WIDTH,
			.crop.height = SENSOR_HEIGHT,

			.scaler.enable = 1,
			.scaler.outwidth = SENSOR_WIDTH_THIRD,
			.scaler.outheight = SENSOR_HEIGHT_THIRD,

			.picWidth = SENSOR_WIDTH_THIRD,
			.picHeight = SENSOR_HEIGHT_THIRD,
		   },
		.framesource_chn =	{ DEV_ID_FS, CH1_INDEX, 0},
		.imp_encoder = { DEV_ID_ENC, CH1_INDEX, 0},
	},
	{
		.index = CH2_INDEX,
		.enable = CHN2_EN,
        .payloadType = PT_H264,
		.fs_chn_attr = {
			.pixFmt = PIX_FMT_NV12,
			.outFrmRateNum = SENSOR_FRAME_RATE_NUM,
			.outFrmRateDen = SENSOR_FRAME_RATE_DEN,
			.nrVBs = 2,
			.type = FS_PHY_CHANNEL,

			.crop.enable = 0,
			.crop.top = 0,
			.crop.left = 0,
			.crop.width = SENSOR_WIDTH,
			.crop.height = SENSOR_HEIGHT,

			.scaler.enable = 1,
			.scaler.outwidth = SENSOR_WIDTH_SECOND,
			.scaler.outheight = SENSOR_HEIGHT_SECOND,

			.picWidth = SENSOR_WIDTH_SECOND,
			.picHeight = SENSOR_HEIGHT_SECOND,
		   },
		.framesource_chn =	{ DEV_ID_FS, CH2_INDEX, 0},
		.imp_encoder = { DEV_ID_ENC, CH2_INDEX, 0},
	},
};

struct chn_conf chn_ext_hsv[1] = {
	{
		.fs_chn_attr = {
			.pixFmt = PIX_FMT_HSV,
			.outFrmRateNum = SENSOR_FRAME_RATE_NUM,
			.outFrmRateDen = SENSOR_FRAME_RATE_DEN,
			.nrVBs = 2,
			.type = FS_EXT_CHANNEL,

			.crop.enable = 0,
			.crop.top = 0,
			.crop.left = 0,
			.crop.width = SENSOR_WIDTH_SECOND,
			.crop.height = SENSOR_HEIGHT_SECOND,

			.scaler.enable = 0,

			.picWidth = SENSOR_WIDTH_SECOND,
			.picHeight = SENSOR_HEIGHT_SECOND,
		},
	},
};

struct chn_conf chn_ext_bgra[1] = {
	{
		.fs_chn_attr = {
			.pixFmt = PIX_FMT_BGRA,
			.outFrmRateNum = SENSOR_FRAME_RATE_NUM,
			.outFrmRateDen = SENSOR_FRAME_RATE_DEN,
			.nrVBs = 2,
			.type = FS_EXT_CHANNEL,

			.crop.enable = 0,
			.crop.top = 0,
			.crop.left = 0,
			.crop.width = SENSOR_WIDTH_SECOND,
			.crop.height = SENSOR_HEIGHT_SECOND,

			.scaler.enable = 0,

			.picWidth = SENSOR_WIDTH_SECOND,
			.picHeight = SENSOR_HEIGHT_SECOND,
		},
	},
};

IMPSensorInfo sensor_info;
int sample_system_init()
{
	int ret = 0;

	memset(&sensor_info, 0, sizeof(IMPSensorInfo));
	memcpy(sensor_info.name, SENSOR_NAME, sizeof(SENSOR_NAME));
	sensor_info.cbus_type = SENSOR_CUBS_TYPE;
	memcpy(sensor_info.i2c.type, SENSOR_NAME, sizeof(SENSOR_NAME));
	sensor_info.i2c.addr = SENSOR_I2C_ADDR;

	IMP_LOG_DBG(TAG, "sample_system_init start\n");

	ret = IMP_ISP_Open();
	if(ret < 0){
		IMP_LOG_ERR(TAG, "failed to open ISP\n");
		return -1;
	}

	ret = IMP_ISP_AddSensor(&sensor_info);
	if(ret < 0){
		IMP_LOG_ERR(TAG, "failed to AddSensor\n");
		return -1;
	}

	ret = IMP_ISP_EnableSensor();
	if(ret < 0){
		IMP_LOG_ERR(TAG, "failed to EnableSensor\n");
		return -1;
	}

	ret = IMP_System_Init();
	if(ret < 0){
		IMP_LOG_ERR(TAG, "IMP_System_Init failed\n");
		return -1;
	}

	/* enable turning, to debug graphics */
	ret = IMP_ISP_EnableTuning();
	if(ret < 0){
		IMP_LOG_ERR(TAG, "IMP_ISP_EnableTuning failed\n");
		return -1;
	}
    IMP_ISP_Tuning_SetContrast(128);
    IMP_ISP_Tuning_SetSharpness(128);
    IMP_ISP_Tuning_SetSaturation(128);
    IMP_ISP_Tuning_SetBrightness(128);
#if 1
    ret = IMP_ISP_Tuning_SetISPRunningMode(IMPISP_RUNNING_MODE_DAY);
    if (ret < 0){
        IMP_LOG_ERR(TAG, "failed to set running mode\n");
        return -1;
    }
#endif
#if 0
    ret = IMP_ISP_Tuning_SetSensorFPS(SENSOR_FRAME_RATE_NUM, SENSOR_FRAME_RATE_DEN);
    if (ret < 0){
        IMP_LOG_ERR(TAG, "failed to set sensor fps\n");
        return -1;
    }
#endif
	IMP_LOG_DBG(TAG, "ImpSystemInit success\n");

	return 0;
}

int sample_system_exit()
{
	int ret = 0;

	IMP_LOG_DBG(TAG, "sample_system_exit start\n");


	IMP_System_Exit();

	ret = IMP_ISP_DisableSensor();
	if(ret < 0){
		IMP_LOG_ERR(TAG, "failed to EnableSensor\n");
		return -1;
	}

	ret = IMP_ISP_DelSensor(&sensor_info);
	if(ret < 0){
		IMP_LOG_ERR(TAG, "failed to AddSensor\n");
		return -1;
	}

	ret = IMP_ISP_DisableTuning();
	if(ret < 0){
		IMP_LOG_ERR(TAG, "IMP_ISP_DisableTuning failed\n");
		return -1;
	}

	if(IMP_ISP_Close()){
		IMP_LOG_ERR(TAG, "failed to open ISP\n");
		return -1;
	}

	IMP_LOG_DBG(TAG, " sample_system_exit success\n");

	return 0;
}

int sample_framesource_streamon()
{
	int ret = 0, i = 0;
	/* Enable channels */
	for (i = 0; i < FS_CHN_NUM; i++) {
		if (chn[i].enable) {
			ret = IMP_FrameSource_EnableChn(chn[i].index);
			if (ret < 0) {
				IMP_LOG_ERR(TAG, "IMP_FrameSource_EnableChn(%d) error: %d\n", ret, chn[i].index);
				return -1;
			}
		}
	}
	return 0;
}

int sample_framesource_ext_hsv_streamon()
{
	int ret = 0;
	/* Enable channels */
	ret = IMP_FrameSource_EnableChn(3);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_FrameSource_EnableChn(%d) error: %d\n", ret, 3);
		return -1;
	}
	return 0;
}

int sample_framesource_ext_bgra_streamon()
{
	int ret = 0;
	/* Enable channels */
	ret = IMP_FrameSource_EnableChn(3);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_FrameSource_EnableChn(%d) error: %d\n", ret, 3);
		return -1;
	}
	return 0;
}

int sample_framesource_streamoff()
{
	int ret = 0, i = 0;
	/* Enable channels */
	for (i = 0; i < FS_CHN_NUM; i++) {
		if (chn[i].enable){
			ret = IMP_FrameSource_DisableChn(chn[i].index);
			if (ret < 0) {
				IMP_LOG_ERR(TAG, "IMP_FrameSource_DisableChn(%d) error: %d\n", ret, chn[i].index);
				return -1;
			}
		}
	}
	return 0;
}

int sample_framesource_ext_hsv_streamoff()
{
	int ret = 0;
	/* Enable channels */
	ret = IMP_FrameSource_DisableChn(3);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_FrameSource_DisableChn(%d) error: %d\n", ret, 3);
		return -1;
	}
	return 0;
}

int sample_framesource_ext_bgra_streamoff()
{
	int ret = 0;
	/* Enable channels */
	ret = IMP_FrameSource_DisableChn(3);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_FrameSource_DisableChn(%d) error: %d\n", ret, 3);
		return -1;
	}
	return 0;
}

static void *get_frame(void *args)
{
	int index = (int)args;
	int chnNum = chn[index].index;
	int i = 0, ret = 0;
	IMPFrameInfo *frame = NULL;
	char framefilename[64];
	int fd = -1;

	if (PIX_FMT_NV12 == chn[index].fs_chn_attr.pixFmt) {
		sprintf(framefilename, "frame%dx%d.nv12", chn[index].fs_chn_attr.picWidth, chn[index].fs_chn_attr.picHeight);
	} else {
		sprintf(framefilename, "frame%dx%d.raw", chn[index].fs_chn_attr.picWidth, chn[index].fs_chn_attr.picHeight);
	}

	fd = open(framefilename, O_RDWR | O_CREAT, 0x644);
	if (fd < 0) {
		IMP_LOG_ERR(TAG, "open %s failed:%s\n", framefilename, strerror(errno));
		goto err_open_framefilename;
	}

	ret = IMP_FrameSource_SetFrameDepth(chnNum, chn[index].fs_chn_attr.nrVBs * 2);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_FrameSource_SetFrameDepth(%d,%d) failed\n", chnNum, chn[index].fs_chn_attr.nrVBs * 2);
		goto err_IMP_FrameSource_SetFrameDepth_1;
	}

	for (i = 0; i < NR_FRAMES_TO_SAVE; i++) {
		ret = IMP_FrameSource_GetFrame(chnNum, &frame);
		if (ret < 0) {
			IMP_LOG_ERR(TAG, "IMP_FrameSource_GetFrame(%d) i=%d failed\n", chnNum, i);
			goto err_IMP_FrameSource_GetFrame_i;
		}
		if (write(fd, (void *)frame->virAddr, frame->size) != frame->size) {
			IMP_LOG_ERR(TAG, "chnNum=%d write frame i=%d failed\n", chnNum, i);
			goto err_write_frame;
		}
		ret = IMP_FrameSource_ReleaseFrame(chnNum, frame);
		if (ret < 0) {
			IMP_LOG_ERR(TAG, "IMP_FrameSource_ReleaseFrame(%d) i=%d failed\n", chnNum, i);
			goto err_IMP_FrameSource_ReleaseFrame_i;
		}
	}

	IMP_FrameSource_SetFrameDepth(chnNum, 0);

	return (void *)0;

err_IMP_FrameSource_ReleaseFrame_i:
err_write_frame:
	IMP_FrameSource_ReleaseFrame(chnNum, frame);
err_IMP_FrameSource_GetFrame_i:
	goto err_IMP_FrameSource_SetFrameDepth_1;
	IMP_FrameSource_SetFrameDepth(chnNum, 0);
err_IMP_FrameSource_SetFrameDepth_1:
	close(fd);
err_open_framefilename:
	return (void *)-1;
}

int sample_get_frame()
{
	unsigned int i;
	int ret;
	pthread_t tid[FS_CHN_NUM];

	for (i = 0; i < FS_CHN_NUM; i++) {
		if (chn[i].enable) {
			ret = pthread_create(&tid[i], NULL, get_frame, (void *)i);
			if (ret < 0) {
				IMP_LOG_ERR(TAG, "Create ChnNum%d get_frame failed\n", chn[i].index);
				return -1;
			}
		}
	}

	for (i = 0; i < FS_CHN_NUM; i++) {
		if (chn[i].enable) {
			pthread_join(tid[i],NULL);
		}
	}

	return 0;
}

int sample_framesource_init()
{
	int i, ret;

	for (i = 0; i < FS_CHN_NUM; i++) {
		if (chn[i].enable) {
			ret = IMP_FrameSource_CreateChn(chn[i].index, &chn[i].fs_chn_attr);
			if(ret < 0){
				IMP_LOG_ERR(TAG, "IMP_FrameSource_CreateChn(chn%d) error !\n", chn[i].index);
				return -1;
			}

			ret = IMP_FrameSource_SetChnAttr(chn[i].index, &chn[i].fs_chn_attr);
			if (ret < 0) {
				IMP_LOG_ERR(TAG, "IMP_FrameSource_SetChnAttr(chn%d) error !\n",  chn[i].index);
				return -1;
			}
		}
	}

	return 0;
}

int sample_framesource_ext_hsv_init()
{
	int ret;
	ret = IMP_FrameSource_CreateChn(3, &chn_ext_hsv[0].fs_chn_attr);
	if(ret < 0){
		IMP_LOG_ERR(TAG, "IMP_FrameSource_CreateChn(chn%d) error !\n", 3);
		return -1;
	}

	ret = IMP_FrameSource_SetChnAttr(3, &chn_ext_hsv[0].fs_chn_attr);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_FrameSource_SetChnAttr(chn%d) error !\n", 3);
		return -1;
	}
	return 0;
}

int sample_framesource_ext_bgra_init()
{
	int ret;
	ret = IMP_FrameSource_CreateChn(3, &chn_ext_bgra[0].fs_chn_attr);
	if(ret < 0){
		IMP_LOG_ERR(TAG, "IMP_FrameSource_CreateChn(chn%d) error !\n", 3);
		return -1;
	}

	ret = IMP_FrameSource_SetChnAttr(3, &chn_ext_bgra[0].fs_chn_attr);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_FrameSource_SetChnAttr(chn%d) error !\n", 3);
		return -1;
	}
	return 0;
}

int sample_framesource_exit()
{
	int ret,i;

	for (i = 0; i <  FS_CHN_NUM; i++) {
		if (chn[i].enable) {
			/*Destroy channel */
			ret = IMP_FrameSource_DestroyChn(chn[i].index);
			if (ret < 0) {
				IMP_LOG_ERR(TAG, "IMP_FrameSource_DestroyChn(%d) error: %d\n", chn[i].index, ret);
				return -1;
			}
		}
	}
	return 0;
}

int sample_framesource_ext_hsv_exit()
{
	int ret;

	ret = IMP_FrameSource_DestroyChn(3);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_FrameSource_DestroyChn() error: %d\n", ret);
		return -1;
	}
	return 0;
}

int sample_framesource_ext_bgra_exit()
{
	int ret;

	ret = IMP_FrameSource_DestroyChn(3);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_FrameSource_DestroyChn() error: %d\n", ret);
		return -1;
	}
	return 0;
}

int sample_jpeg_init()
{
	int i, ret;
	IMPEncoderAttr *enc_attr;
	IMPEncoderCHNAttr channel_attr;
	IMPFSChnAttr *imp_chn_attr_tmp;

	for (i = 0; i <  FS_CHN_NUM; i++) {
		if (chn[i].enable) {
			imp_chn_attr_tmp = &chn[i].fs_chn_attr;
			memset(&channel_attr, 0, sizeof(IMPEncoderCHNAttr));
			enc_attr = &channel_attr.encAttr;
			enc_attr->enType = PT_JPEG;
			enc_attr->bufSize = 0;
			enc_attr->profile = 0;
			enc_attr->picWidth = imp_chn_attr_tmp->picWidth;
			enc_attr->picHeight = imp_chn_attr_tmp->picHeight;

			/* Create Channel */
			ret = IMP_Encoder_CreateChn(3 + chn[i].index, &channel_attr);
			if (ret < 0) {
				IMP_LOG_ERR(TAG, "IMP_Encoder_CreateChn(%d) error: %d\n",
							chn[i].index, ret);
				return -1;
			}

			/* Resigter Channel */
			ret = IMP_Encoder_RegisterChn(i, 3 + chn[i].index);
			if (ret < 0) {
				IMP_LOG_ERR(TAG, "IMP_Encoder_RegisterChn(0, %d) error: %d\n",
							chn[i].index, ret);
				return -1;
			}
		}
	}

	return 0;
}

int sample_encoder_init()
{
	int i, ret, chnNum = 0;
	IMPEncoderAttr *enc_attr;
	IMPEncoderRcAttr *rc_attr;
	IMPFSChnAttr *imp_chn_attr_tmp;
	IMPEncoderCHNAttr channel_attr;

	for (i = 0; i <  FS_CHN_NUM; i++) {
		if (chn[i].enable) {
			imp_chn_attr_tmp = &chn[i].fs_chn_attr;
			memset(&channel_attr, 0, sizeof(IMPEncoderCHNAttr));
			enc_attr = &channel_attr.encAttr;
			enc_attr->enType = chn[i].payloadType;
			enc_attr->bufSize = 0;
			enc_attr->profile = 1;
			enc_attr->picWidth = imp_chn_attr_tmp->picWidth;
			enc_attr->picHeight = imp_chn_attr_tmp->picHeight;
            if (chn[i].payloadType == PT_JPEG) {
                chnNum = 3 + chn[i].index;
            } else if (chn[i].payloadType == PT_H264) {
                chnNum = chn[i].index;
                rc_attr = &channel_attr.rcAttr;
                rc_attr->outFrmRate.frmRateNum = imp_chn_attr_tmp->outFrmRateNum;
                rc_attr->outFrmRate.frmRateDen = imp_chn_attr_tmp->outFrmRateDen;
                rc_attr->maxGop = 2 * rc_attr->outFrmRate.frmRateNum / rc_attr->outFrmRate.frmRateDen;
                if (S_RC_METHOD == ENC_RC_MODE_CBR) {
                    rc_attr->attrRcMode.rcMode = ENC_RC_MODE_CBR;
                    rc_attr->attrRcMode.attrH264Cbr.outBitRate = BITRATE_720P_Kbs * (imp_chn_attr_tmp->picWidth * imp_chn_attr_tmp->picHeight) / (1280 * 720);
                    rc_attr->attrRcMode.attrH264Cbr.maxQp = 45;
                    rc_attr->attrRcMode.attrH264Cbr.minQp = 15;
                    rc_attr->attrRcMode.attrH264Cbr.iBiasLvl = 0;
                    rc_attr->attrRcMode.attrH264Cbr.frmQPStep = 3;
                    rc_attr->attrRcMode.attrH264Cbr.gopQPStep = 15;
                    rc_attr->attrRcMode.attrH264Cbr.adaptiveMode = false;
                    rc_attr->attrRcMode.attrH264Cbr.gopRelation = false;

                    rc_attr->attrHSkip.hSkipAttr.skipType = IMP_Encoder_STYPE_N1X;
                    rc_attr->attrHSkip.hSkipAttr.m = 0;
                    rc_attr->attrHSkip.hSkipAttr.n = 0;
                    rc_attr->attrHSkip.hSkipAttr.maxSameSceneCnt = 0;
                    rc_attr->attrHSkip.hSkipAttr.bEnableScenecut = 0;
                    rc_attr->attrHSkip.hSkipAttr.bBlackEnhance = 0;
                    rc_attr->attrHSkip.maxHSkipType = IMP_Encoder_STYPE_N1X;
                } else if (S_RC_METHOD == ENC_RC_MODE_VBR) {
                    rc_attr->attrRcMode.rcMode = ENC_RC_MODE_VBR;
                    rc_attr->attrRcMode.attrH264Vbr.maxQp = 45;
                    rc_attr->attrRcMode.attrH264Vbr.minQp = 15;
                    rc_attr->attrRcMode.attrH264Vbr.staticTime = 2;
                    rc_attr->attrRcMode.attrH264Vbr.maxBitRate = BITRATE_720P_Kbs * (imp_chn_attr_tmp->picWidth * imp_chn_attr_tmp->picHeight) / (1280 * 720);
                    rc_attr->attrRcMode.attrH264Vbr.iBiasLvl = 0;
                    rc_attr->attrRcMode.attrH264Vbr.changePos = 80;
                    rc_attr->attrRcMode.attrH264Vbr.qualityLvl = 2;
                    rc_attr->attrRcMode.attrH264Vbr.frmQPStep = 3;
                    rc_attr->attrRcMode.attrH264Vbr.gopQPStep = 15;
                    rc_attr->attrRcMode.attrH264Vbr.gopRelation = false;

                    rc_attr->attrHSkip.hSkipAttr.skipType = IMP_Encoder_STYPE_N1X;
                    rc_attr->attrHSkip.hSkipAttr.m = 0;
                    rc_attr->attrHSkip.hSkipAttr.n = 0;
                    rc_attr->attrHSkip.hSkipAttr.maxSameSceneCnt = 0;
                    rc_attr->attrHSkip.hSkipAttr.bEnableScenecut = 0;
                    rc_attr->attrHSkip.hSkipAttr.bBlackEnhance = 0;
                    rc_attr->attrHSkip.maxHSkipType = IMP_Encoder_STYPE_N1X;
                } else if (S_RC_METHOD == ENC_RC_MODE_SMART) {
                    rc_attr->attrRcMode.rcMode = ENC_RC_MODE_SMART;
                    rc_attr->attrRcMode.attrH264Smart.maxQp = 45;
                    rc_attr->attrRcMode.attrH264Smart.minQp = 15;
                    rc_attr->attrRcMode.attrH264Smart.staticTime = 2;
                    rc_attr->attrRcMode.attrH264Smart.maxBitRate = BITRATE_720P_Kbs * (imp_chn_attr_tmp->picWidth * imp_chn_attr_tmp->picHeight) / (1280 * 720);
                    rc_attr->attrRcMode.attrH264Smart.iBiasLvl = 0;
                    rc_attr->attrRcMode.attrH264Smart.changePos = 80;
                    rc_attr->attrRcMode.attrH264Smart.qualityLvl = 2;
                    rc_attr->attrRcMode.attrH264Smart.frmQPStep = 3;
                    rc_attr->attrRcMode.attrH264Smart.gopQPStep = 15;
                    rc_attr->attrRcMode.attrH264Smart.gopRelation = false;

                    rc_attr->attrHSkip.hSkipAttr.skipType = IMP_Encoder_STYPE_N1X;
                    rc_attr->attrHSkip.hSkipAttr.m = rc_attr->maxGop - 1;
                    rc_attr->attrHSkip.hSkipAttr.n = 1;
                    rc_attr->attrHSkip.hSkipAttr.maxSameSceneCnt = 6;
                    rc_attr->attrHSkip.hSkipAttr.bEnableScenecut = 0;
                    rc_attr->attrHSkip.hSkipAttr.bBlackEnhance = 0;
                    rc_attr->attrHSkip.maxHSkipType = IMP_Encoder_STYPE_N1X;
                } else { /* fixQp */
                    rc_attr->attrRcMode.rcMode = ENC_RC_MODE_FIXQP;
                    rc_attr->attrRcMode.attrH264FixQp.qp = 35;

                    rc_attr->attrHSkip.hSkipAttr.skipType = IMP_Encoder_STYPE_N1X;
                    rc_attr->attrHSkip.hSkipAttr.m = 0;
                    rc_attr->attrHSkip.hSkipAttr.n = 0;
                    rc_attr->attrHSkip.hSkipAttr.maxSameSceneCnt = 0;
                    rc_attr->attrHSkip.hSkipAttr.bEnableScenecut = 0;
                    rc_attr->attrHSkip.hSkipAttr.bBlackEnhance = 0;
                    rc_attr->attrHSkip.maxHSkipType = IMP_Encoder_STYPE_N1X;
                }
            } else { //PT_H265
                chnNum = chn[i].index;
                rc_attr = &channel_attr.rcAttr;
                rc_attr->outFrmRate.frmRateNum = imp_chn_attr_tmp->outFrmRateNum;
                rc_attr->outFrmRate.frmRateDen = imp_chn_attr_tmp->outFrmRateDen;
                rc_attr->maxGop = 2 * rc_attr->outFrmRate.frmRateNum / rc_attr->outFrmRate.frmRateDen;
                if (S_RC_METHOD == ENC_RC_MODE_CBR) {
                    rc_attr->attrRcMode.attrH265Cbr.maxQp = 45;
                    rc_attr->attrRcMode.attrH265Cbr.minQp = 15;
                    rc_attr->attrRcMode.attrH265Cbr.staticTime = 2;
                    rc_attr->attrRcMode.attrH265Cbr.outBitRate = BITRATE_720P_Kbs * (imp_chn_attr_tmp->picWidth * imp_chn_attr_tmp->picHeight) / (1280 * 720);
                    rc_attr->attrRcMode.attrH265Cbr.iBiasLvl = 0;
                    rc_attr->attrRcMode.attrH265Cbr.frmQPStep = 3;
                    rc_attr->attrRcMode.attrH265Cbr.gopQPStep = 15;
                    rc_attr->attrRcMode.attrH265Cbr.flucLvl = 2;

                    rc_attr->attrHSkip.hSkipAttr.skipType = IMP_Encoder_STYPE_N1X;
                    rc_attr->attrHSkip.hSkipAttr.m = 0;
                    rc_attr->attrHSkip.hSkipAttr.n = 0;
                    rc_attr->attrHSkip.hSkipAttr.maxSameSceneCnt = 0;
                    rc_attr->attrHSkip.hSkipAttr.bEnableScenecut = 0;
                    rc_attr->attrHSkip.hSkipAttr.bBlackEnhance = 0;
                    rc_attr->attrHSkip.maxHSkipType = IMP_Encoder_STYPE_N1X;
                } else if (S_RC_METHOD == ENC_RC_MODE_VBR) {
                    rc_attr->attrRcMode.rcMode = ENC_RC_MODE_VBR;
                    rc_attr->attrRcMode.attrH265Vbr.maxQp = 45;
                    rc_attr->attrRcMode.attrH265Vbr.minQp = 15;
                    rc_attr->attrRcMode.attrH265Vbr.staticTime = 2;
                    rc_attr->attrRcMode.attrH265Vbr.maxBitRate = BITRATE_720P_Kbs * (imp_chn_attr_tmp->picWidth * imp_chn_attr_tmp->picHeight) / (1280 * 720);
                    rc_attr->attrRcMode.attrH265Vbr.iBiasLvl = 0;
                    rc_attr->attrRcMode.attrH265Vbr.changePos = 80;
                    rc_attr->attrRcMode.attrH265Vbr.qualityLvl = 2;
                    rc_attr->attrRcMode.attrH265Vbr.frmQPStep = 3;
                    rc_attr->attrRcMode.attrH265Vbr.gopQPStep = 15;
                    rc_attr->attrRcMode.attrH265Vbr.flucLvl = 2;

                    rc_attr->attrHSkip.hSkipAttr.skipType = IMP_Encoder_STYPE_N1X;
                    rc_attr->attrHSkip.hSkipAttr.m = 0;
                    rc_attr->attrHSkip.hSkipAttr.n = 0;
                    rc_attr->attrHSkip.hSkipAttr.maxSameSceneCnt = 0;
                    rc_attr->attrHSkip.hSkipAttr.bEnableScenecut = 0;
                    rc_attr->attrHSkip.hSkipAttr.bBlackEnhance = 0;
                    rc_attr->attrHSkip.maxHSkipType = IMP_Encoder_STYPE_N1X;
                } else if (S_RC_METHOD == ENC_RC_MODE_SMART) {
                    rc_attr->attrRcMode.rcMode = ENC_RC_MODE_SMART;
                    rc_attr->attrRcMode.attrH265Smart.maxQp = 45;
                    rc_attr->attrRcMode.attrH265Smart.minQp = 15;
                    rc_attr->attrRcMode.attrH265Smart.staticTime = 2;
                    rc_attr->attrRcMode.attrH265Smart.maxBitRate = BITRATE_720P_Kbs * (imp_chn_attr_tmp->picWidth * imp_chn_attr_tmp->picHeight) / (1280 * 720);
                    rc_attr->attrRcMode.attrH265Smart.iBiasLvl = 0;
                    rc_attr->attrRcMode.attrH265Smart.changePos = 80;
                    rc_attr->attrRcMode.attrH265Smart.qualityLvl = 2;
                    rc_attr->attrRcMode.attrH265Smart.frmQPStep = 3;
                    rc_attr->attrRcMode.attrH265Smart.gopQPStep = 15;
                    rc_attr->attrRcMode.attrH265Smart.flucLvl = 2;

                    rc_attr->attrHSkip.hSkipAttr.skipType = IMP_Encoder_STYPE_N1X;
                    rc_attr->attrHSkip.hSkipAttr.m = rc_attr->maxGop - 1;
                    rc_attr->attrHSkip.hSkipAttr.n = 1;
                    rc_attr->attrHSkip.hSkipAttr.maxSameSceneCnt = 6;
                    rc_attr->attrHSkip.hSkipAttr.bEnableScenecut = 0;
                    rc_attr->attrHSkip.hSkipAttr.bBlackEnhance = 0;
                    rc_attr->attrHSkip.maxHSkipType = IMP_Encoder_STYPE_N1X;
                } else { /* fixQp */
                    rc_attr->attrRcMode.rcMode = ENC_RC_MODE_FIXQP;
                    rc_attr->attrRcMode.attrH265FixQp.qp = 35;

                    rc_attr->attrHSkip.hSkipAttr.skipType = IMP_Encoder_STYPE_N1X;
                    rc_attr->attrHSkip.hSkipAttr.m = 0;
                    rc_attr->attrHSkip.hSkipAttr.n = 0;
                    rc_attr->attrHSkip.hSkipAttr.maxSameSceneCnt = 0;
                    rc_attr->attrHSkip.hSkipAttr.bEnableScenecut = 0;
                    rc_attr->attrHSkip.hSkipAttr.bBlackEnhance = 0;
                    rc_attr->attrHSkip.maxHSkipType = IMP_Encoder_STYPE_N1X;
                }
            }

			ret = IMP_Encoder_CreateChn(chnNum, &channel_attr);
			if (ret < 0) {
				IMP_LOG_ERR(TAG, "IMP_Encoder_CreateChn(%d) error !\n", chnNum);
				return -1;
			}

			ret = IMP_Encoder_RegisterChn(chn[i].index, chnNum);
			if (ret < 0) {
				IMP_LOG_ERR(TAG, "IMP_Encoder_RegisterChn(%d, %d) error: %d\n", chn[i].index, chnNum, ret);
				return -1;
			}
		}
	}

	return 0;
}

int sample_encoder_exit(void)
{
    int ret = 0, i = 0, chnNum = 0;
    IMPEncoderCHNStat chn_stat;

	for (i = 0; i <  FS_CHN_NUM; i++) {
		if (chn[i].enable) {
            if (chn[i].payloadType == PT_JPEG) {
                chnNum = 3 + chn[i].index;
            } else {
                chnNum = chn[i].index;
            }
            memset(&chn_stat, 0, sizeof(IMPEncoderCHNStat));
            ret = IMP_Encoder_Query(chnNum, &chn_stat);
            if (ret < 0) {
                IMP_LOG_ERR(TAG, "IMP_Encoder_Query(%d) error: %d\n", chnNum, ret);
                return -1;
            }

            if (chn_stat.registered) {
                ret = IMP_Encoder_UnRegisterChn(chnNum);
                if (ret < 0) {
                    IMP_LOG_ERR(TAG, "IMP_Encoder_UnRegisterChn(%d) error: %d\n", chnNum, ret);
                    return -1;
                }

                ret = IMP_Encoder_DestroyChn(chnNum);
                if (ret < 0) {
                    IMP_LOG_ERR(TAG, "IMP_Encoder_DestroyChn(%d) error: %d\n", chnNum, ret);
                    return -1;
                }
            }
        }
    }

    return 0;
}

IMPRgnHandle *sample_osd_init(int grpNum)
{
	int ret;
	IMPRgnHandle *prHander;
	IMPRgnHandle rHanderFont;
	IMPRgnHandle rHanderLogo;
	IMPRgnHandle rHanderCover;
	IMPRgnHandle rHanderRect;

	prHander = malloc(4 * sizeof(IMPRgnHandle));
	if (prHander <= 0) {
		IMP_LOG_ERR(TAG, "malloc() error !\n");
		return NULL;
	}

	rHanderFont = IMP_OSD_CreateRgn(NULL);
	if (rHanderFont == INVHANDLE) {
		IMP_LOG_ERR(TAG, "IMP_OSD_CreateRgn TimeStamp error !\n");
		return NULL;
	}

	rHanderLogo = IMP_OSD_CreateRgn(NULL);
	if (rHanderLogo == INVHANDLE) {
		IMP_LOG_ERR(TAG, "IMP_OSD_CreateRgn Logo error !\n");
		return NULL;
	}

	rHanderCover = IMP_OSD_CreateRgn(NULL);
	if (rHanderCover == INVHANDLE) {
		IMP_LOG_ERR(TAG, "IMP_OSD_CreateRgn Cover error !\n");
		return NULL;
	}

	rHanderRect = IMP_OSD_CreateRgn(NULL);
	if (rHanderRect == INVHANDLE) {
		IMP_LOG_ERR(TAG, "IMP_OSD_CreateRgn Rect error !\n");
		return NULL;
	}


	ret = IMP_OSD_RegisterRgn(rHanderFont, grpNum, NULL);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IVS IMP_OSD_RegisterRgn failed\n");
		return NULL;
	}

	ret = IMP_OSD_RegisterRgn(rHanderLogo, grpNum, NULL);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IVS IMP_OSD_RegisterRgn failed\n");
		return NULL;
	}

	ret = IMP_OSD_RegisterRgn(rHanderCover, grpNum, NULL);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IVS IMP_OSD_RegisterRgn failed\n");
		return NULL;
	}

	ret = IMP_OSD_RegisterRgn(rHanderRect, grpNum, NULL);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IVS IMP_OSD_RegisterRgn failed\n");
		return NULL;
	}


	IMPOSDRgnAttr rAttrFont;
	memset(&rAttrFont, 0, sizeof(IMPOSDRgnAttr));
	rAttrFont.type = OSD_REG_PIC;
	rAttrFont.rect.p0.x = 10;
	rAttrFont.rect.p0.y = 10;
	rAttrFont.rect.p1.x = rAttrFont.rect.p0.x + 20 * OSD_REGION_WIDTH- 1;   //p0 is start，and p1 well be epual p0+width(or heigth)-1
	rAttrFont.rect.p1.y = rAttrFont.rect.p0.y + OSD_REGION_HEIGHT - 1;
#ifdef SUPPORT_RGB555LE
	rAttrFont.fmt = PIX_FMT_RGB555LE;
#else
	rAttrFont.fmt = PIX_FMT_BGRA;
#endif
	rAttrFont.data.picData.pData = NULL;
	ret = IMP_OSD_SetRgnAttr(rHanderFont, &rAttrFont);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_OSD_SetRgnAttr TimeStamp error !\n");
		return NULL;
	}

	IMPOSDGrpRgnAttr grAttrFont;

	if (IMP_OSD_GetGrpRgnAttr(rHanderFont, grpNum, &grAttrFont) < 0) {
		IMP_LOG_ERR(TAG, "IMP_OSD_GetGrpRgnAttr Logo error !\n");
		return NULL;

	}
	memset(&grAttrFont, 0, sizeof(IMPOSDGrpRgnAttr));
	grAttrFont.show = 0;

	/* Disable Font global alpha, only use pixel alpha. */
	grAttrFont.gAlphaEn = 1;
	grAttrFont.fgAlhpa = 0xff;
	grAttrFont.layer = 3;
	if (IMP_OSD_SetGrpRgnAttr(rHanderFont, grpNum, &grAttrFont) < 0) {
		IMP_LOG_ERR(TAG, "IMP_OSD_SetGrpRgnAttr Logo error !\n");
		return NULL;
	}

	IMPOSDRgnAttr rAttrLogo;
	memset(&rAttrLogo, 0, sizeof(IMPOSDRgnAttr));
	int picw = 100;
	int pich = 100;
	rAttrLogo.type = OSD_REG_PIC;
	rAttrLogo.rect.p0.x = SENSOR_WIDTH - 100;
	rAttrLogo.rect.p0.y = SENSOR_HEIGHT - 100;
	rAttrLogo.rect.p1.x = rAttrLogo.rect.p0.x+picw-1;     //p0 is start，and p1 well be epual p0+width(or heigth)-1
	rAttrLogo.rect.p1.y = rAttrLogo.rect.p0.y+pich-1;
	rAttrLogo.fmt = PIX_FMT_BGRA;
	rAttrLogo.data.picData.pData = logodata_100x100_bgra;
	ret = IMP_OSD_SetRgnAttr(rHanderLogo, &rAttrLogo);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_OSD_SetRgnAttr Logo error !\n");
		return NULL;
	}
	IMPOSDGrpRgnAttr grAttrLogo;

	if (IMP_OSD_GetGrpRgnAttr(rHanderLogo, grpNum, &grAttrLogo) < 0) {
		IMP_LOG_ERR(TAG, "IMP_OSD_GetGrpRgnAttr Logo error !\n");
		return NULL;

	}
	memset(&grAttrLogo, 0, sizeof(IMPOSDGrpRgnAttr));
	grAttrLogo.show = 0;

	/* Set Logo global alpha to 0x7f, it is semi-transparent. */
	grAttrLogo.gAlphaEn = 1;
	grAttrLogo.fgAlhpa = 0x7f;
	grAttrLogo.layer = 2;

	if (IMP_OSD_SetGrpRgnAttr(rHanderLogo, grpNum, &grAttrLogo) < 0) {
		IMP_LOG_ERR(TAG, "IMP_OSD_SetGrpRgnAttr Logo error !\n");
		return NULL;
	}

	IMPOSDRgnAttr rAttrCover;
	memset(&rAttrCover, 0, sizeof(IMPOSDRgnAttr));
	rAttrCover.type = OSD_REG_COVER;
	rAttrCover.rect.p0.x = SENSOR_WIDTH/2-100;
	rAttrCover.rect.p0.y = SENSOR_HEIGHT/2-100;
	rAttrCover.rect.p1.x = rAttrCover.rect.p0.x+SENSOR_WIDTH/2-1+50;     //p0 is start，and p1 well be epual p0+width(or heigth)-1
	rAttrCover.rect.p1.y = rAttrCover.rect.p0.y+SENSOR_HEIGHT/2-1+50;
	rAttrCover.fmt = PIX_FMT_BGRA;
	rAttrCover.data.coverData.color = OSD_BLACK;
	ret = IMP_OSD_SetRgnAttr(rHanderCover, &rAttrCover);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_OSD_SetRgnAttr Cover error !\n");
		return NULL;
	}
	IMPOSDGrpRgnAttr grAttrCover;

	if (IMP_OSD_GetGrpRgnAttr(rHanderCover, grpNum, &grAttrCover) < 0) {
		IMP_LOG_ERR(TAG, "IMP_OSD_GetGrpRgnAttr Cover error !\n");
		return NULL;

	}
	memset(&grAttrCover, 0, sizeof(IMPOSDGrpRgnAttr));
	grAttrCover.show = 0;

	/* Disable Cover global alpha, it is absolutely no transparent. */
	grAttrCover.gAlphaEn = 1;
	grAttrCover.fgAlhpa = 0x7f;
	grAttrCover.layer = 2;
	if (IMP_OSD_SetGrpRgnAttr(rHanderCover, grpNum, &grAttrCover) < 0) {
		IMP_LOG_ERR(TAG, "IMP_OSD_SetGrpRgnAttr Cover error !\n");
		return NULL;
	}

	IMPOSDRgnAttr rAttrRect;
	memset(&rAttrRect, 0, sizeof(IMPOSDRgnAttr));

	rAttrRect.type = OSD_REG_RECT;
	rAttrRect.rect.p0.x = 300;
	rAttrRect.rect.p0.y = 300;
	rAttrRect.rect.p1.x = rAttrRect.rect.p0.x + 600 - 1;
	rAttrRect.rect.p1.y = rAttrRect.rect.p0.y + 300 - 1;
	rAttrRect.fmt = PIX_FMT_MONOWHITE;
	rAttrRect.data.lineRectData.color = OSD_GREEN;
	rAttrRect.data.lineRectData.linewidth = 5;
	ret = IMP_OSD_SetRgnAttr(rHanderRect, &rAttrRect);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_OSD_SetRgnAttr Rect error !\n");
		return NULL;
	}
	IMPOSDGrpRgnAttr grAttrRect;

	if (IMP_OSD_GetGrpRgnAttr(rHanderRect, grpNum, &grAttrRect) < 0) {
		IMP_LOG_ERR(TAG, "IMP_OSD_GetGrpRgnAttr Rect error !\n");
		return NULL;

	}
	memset(&grAttrRect, 0, sizeof(IMPOSDGrpRgnAttr));
	grAttrRect.show = 0;
	grAttrRect.layer = 1;
	grAttrRect.scalex = 1;
	grAttrRect.scaley = 1;
	if (IMP_OSD_SetGrpRgnAttr(rHanderRect, grpNum, &grAttrRect) < 0) {
		IMP_LOG_ERR(TAG, "IMP_OSD_SetGrpRgnAttr Rect error !\n");
		return NULL;
	}


	ret = IMP_OSD_Start(grpNum);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_OSD_Start TimeStamp, Logo, Cover and Rect error !\n");
		return NULL;
	}

	prHander[0] = rHanderFont;
	prHander[1] = rHanderLogo;
	prHander[2] = rHanderCover;
	prHander[3] = rHanderRect;
	return prHander;
}

int sample_osd_exit(IMPRgnHandle *prHander,int grpNum)
{
	int ret;

	ret = IMP_OSD_ShowRgn(prHander[0], grpNum, 0);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_OSD_ShowRgn close timeStamp error\n");
	}

	ret = IMP_OSD_ShowRgn(prHander[1], grpNum, 0);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_OSD_ShowRgn close Logo error\n");
	}

	ret = IMP_OSD_ShowRgn(prHander[2], grpNum, 0);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_OSD_ShowRgn close cover error\n");
	}

	ret = IMP_OSD_ShowRgn(prHander[3], grpNum, 0);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_OSD_ShowRgn close Rect error\n");
	}


	ret = IMP_OSD_UnRegisterRgn(prHander[0], grpNum);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_OSD_UnRegisterRgn timeStamp error\n");
	}

	ret = IMP_OSD_UnRegisterRgn(prHander[1], grpNum);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_OSD_UnRegisterRgn logo error\n");
	}

	ret = IMP_OSD_UnRegisterRgn(prHander[2], grpNum);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_OSD_UnRegisterRgn Cover error\n");
	}

	ret = IMP_OSD_UnRegisterRgn(prHander[3], grpNum);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_OSD_UnRegisterRgn Rect error\n");
	}


	IMP_OSD_DestroyRgn(prHander[0]);
	IMP_OSD_DestroyRgn(prHander[1]);
	IMP_OSD_DestroyRgn(prHander[2]);
	IMP_OSD_DestroyRgn(prHander[3]);

	ret = IMP_OSD_DestroyGroup(grpNum);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_OSD_DestroyGroup(0) error\n");
		return -1;
	}
	free(prHander);
	prHander = NULL;

	return 0;
}

static int save_stream(int fd, IMPEncoderStream *stream)
{
	int ret, i, nr_pack = stream->packCount;

	for (i = 0; i < nr_pack; i++) {
		ret = write(fd, (void *)stream->pack[i].virAddr,
					stream->pack[i].length);
		if (ret != stream->pack[i].length) {
			IMP_LOG_ERR(TAG, "stream write ret(%d) != stream->pack[%d].length(%d) error:%s\n", ret, i, stream->pack[i].length, strerror(errno));
			return -1;
		}
	}
	return 0;
}

static int save_stream_by_name(char *stream_prefix, int idx, IMPEncoderStream *stream)
{
    int stream_fd = -1;
    char stream_path[128];
	int ret, i, nr_pack = stream->packCount;

    sprintf(stream_path, "%s_%d", stream_prefix, idx);

    IMP_LOG_DBG(TAG, "Open Stream file %s ", stream_path);
    stream_fd = open(stream_path, O_RDWR | O_CREAT | O_TRUNC, 0777);
    if (stream_fd < 0) {
        IMP_LOG_ERR(TAG, "failed: %s\n", strerror(errno));
        return -1;
    }
    IMP_LOG_DBG(TAG, "OK\n");

	for (i = 0; i < nr_pack; i++) {
		ret = write(stream_fd, (void *)stream->pack[i].virAddr, stream->pack[i].length);
		if (ret != stream->pack[i].length) {
            close(stream_fd);
			IMP_LOG_ERR(TAG, "stream write ret(%d) != stream->pack[%d].length(%d) error:%s\n", ret, i, stream->pack[i].length, strerror(errno));
			return -1;
		}
	}

    close(stream_fd);

	return 0;
}

static void *get_video_stream(void *args)
{
	int val, i, chnNum, ret;
	char stream_path[64];
    IMPPayloadType payloadType;
    int stream_fd = -1, totalSaveCnt = 0;

	val = (int)args;
    chnNum = val & 0xffff;
    payloadType = (val >> 16) & 0xffff;

    ret = IMP_Encoder_StartRecvPic(chnNum);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_Encoder_StartRecvPic(%d) failed\n", chnNum);
		return ((void *)-1);
	}

	sprintf(stream_path, "%s/stream-%d.%s", STREAM_FILE_PATH_PREFIX, chnNum,
            (payloadType == PT_H264) ? "h264" : ((payloadType == PT_H265) ? "h265" : "jpeg"));

    if (payloadType == PT_JPEG) {
        totalSaveCnt = ((NR_FRAMES_TO_SAVE / 50) > 0) ? (NR_FRAMES_TO_SAVE / 50) : 1;
    } else {
        IMP_LOG_DBG(TAG, "Video ChnNum=%d Open Stream file %s ", chnNum, stream_path);
        stream_fd = open(stream_path, O_RDWR | O_CREAT | O_TRUNC, 0777);
        if (stream_fd < 0) {
            IMP_LOG_ERR(TAG, "failed: %s\n", strerror(errno));
            return ((void *)-1);
        }
        IMP_LOG_DBG(TAG, "OK\n");
        totalSaveCnt = NR_FRAMES_TO_SAVE;
    }

	for (i = 0; i < totalSaveCnt; i++) {
		ret = IMP_Encoder_PollingStream(chnNum, 1000);
		if (ret < 0) {
			IMP_LOG_ERR(TAG, "IMP_Encoder_PollingStream(%d) timeout\n", chnNum);
			continue;
		}

		IMPEncoderStream stream;

		/* Get H264 or H265 Stream */
		ret = IMP_Encoder_GetStream(chnNum, &stream, 1);
#ifdef SHOW_FRM_BITRATE
		int i, len = 0;
		for (i = 0; i < stream.packCount; i++) {
			len += stream.pack[i].length;
		}
		bitrate_sp[chnNum] += len;
		frmrate_sp[chnNum]++;

		int64_t now = IMP_System_GetTimeStamp() / 1000;
		if(((int)(now - statime_sp[chnNum]) / 1000) >= FRM_BIT_RATE_TIME){
			double fps = (double)frmrate_sp[chnNum] / ((double)(now - statime_sp[chnNum]) / 1000);
			double kbr = (double)bitrate_sp[chnNum] * 8 / (double)(now - statime_sp[chnNum]);

			printf("\rstreamNum[%d]:FPS: %0.2f,Bitrate: %0.2f(kbps)", chnNum, fps, kbr);
			fflush(stdout);

			frmrate_sp[chnNum] = 0;
			bitrate_sp[chnNum] = 0;
			statime_sp[chnNum] = now;
		}
#endif
		if (ret < 0) {
			IMP_LOG_ERR(TAG, "IMP_Encoder_GetStream(%d) failed\n", chnNum);
			return ((void *)-1);
		}
		//IMP_LOG_DBG(TAG, "i=%d, stream.packCount=%d, stream.%sRefType=%d\n", i, stream.packCount,
        //payloadtype == PT_H264 ? "h264" : "h265", payloadType == PT_H264 ? stream.h264RefType : stream.h265RefType);

        if (payloadType == PT_JPEG) {
            ret = save_stream_by_name(stream_path, i, &stream);
            if (ret < 0) {
                return ((void *)ret);
            }
        }
#if 1
		else {
            ret = save_stream(stream_fd, &stream);
            if (ret < 0) {
                close(stream_fd);
                return ((void *)ret);
            }
		}
#endif
        IMP_Encoder_ReleaseStream(chnNum, &stream);
	}

	close(stream_fd);

	ret = IMP_Encoder_StopRecvPic(chnNum);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_Encoder_StopRecvPic(%d) failed\n", chnNum);
		return ((void *)-1);
	}

	return ((void *)0);
}

int sample_get_video_stream()
{
	unsigned int i;
	int ret;
	pthread_t tid[FS_CHN_NUM];

	for (i = 0; i < FS_CHN_NUM; i++) {
		if (chn[i].enable) {
            int arg = 0;
            if (chn[i].payloadType == PT_JPEG) {
                arg = (chn[i].payloadType << 16 | (3 + chn[i].index));
            } else {
                arg = (chn[i].payloadType << 16 | chn[i].index);
            }
			ret = pthread_create(&tid[i], NULL, get_video_stream, (void *)arg);
			if (ret < 0) {
				IMP_LOG_ERR(TAG, "Create ChnNum%d get_video_stream failed\n", (chn[i].payloadType == PT_JPEG) ? (3 + chn[i].index) : chn[i].index);
			}
		}
	}

	for (i = 0; i < FS_CHN_NUM; i++) {
		if (chn[i].enable) {
			pthread_join(tid[i],NULL);
		}
	}

	return 0;
}

int sample_get_jpeg_snap()
{
	int i, ret;
	char snap_path[64];

	for (i = 0; i < FS_CHN_NUM; i++) {
		if (chn[i].enable) {
			ret = IMP_Encoder_StartRecvPic(3 + chn[i].index);
			if (ret < 0) {
				IMP_LOG_ERR(TAG, "IMP_Encoder_StartRecvPic(%d) failed\n", 3 + chn[i].index);
				return -1;
			}

			sprintf(snap_path, "%s/snap-%d.jpg",
					SNAP_FILE_PATH_PREFIX, chn[i].index);

			IMP_LOG_ERR(TAG, "Open Snap file %s ", snap_path);
			int snap_fd = open(snap_path, O_RDWR | O_CREAT | O_TRUNC, 0777);
			if (snap_fd < 0) {
				IMP_LOG_ERR(TAG, "failed: %s\n", strerror(errno));
				return -1;
			}
			IMP_LOG_DBG(TAG, "OK\n");

			/* Polling JPEG Snap, set timeout as 1000msec */
			ret = IMP_Encoder_PollingStream(3 + chn[i].index, 1000);
			if (ret < 0) {
				IMP_LOG_ERR(TAG, "Polling stream timeout\n");
				continue;
			}

			IMPEncoderStream stream;
			/* Get JPEG Snap */
			ret = IMP_Encoder_GetStream(chn[i].index + 3, &stream, 1);
			if (ret < 0) {
				IMP_LOG_ERR(TAG, "IMP_Encoder_GetStream() failed\n");
				return -1;
			}

			ret = save_stream(snap_fd, &stream);
			if (ret < 0) {
				close(snap_fd);
				return ret;
			}

			IMP_Encoder_ReleaseStream(3 + chn[i].index, &stream);

			close(snap_fd);

			ret = IMP_Encoder_StopRecvPic(3 + chn[i].index);
			if (ret < 0) {
				IMP_LOG_ERR(TAG, "IMP_Encoder_StopRecvPic() failed\n");
				return -1;
			}
		}
	}
	return 0;
}

int sample_get_video_stream_byfd()
{
    int streamFd[FS_CHN_NUM], vencFd[FS_CHN_NUM], maxVencFd = 0;
	char stream_path[FS_CHN_NUM][128];
    fd_set readfds;
    struct timeval selectTimeout;
    int saveStreamCnt[FS_CHN_NUM], totalSaveStreamCnt[FS_CHN_NUM];
    int i = 0, ret = 0, chnNum = 0;
    memset(streamFd, 0, sizeof(streamFd));
    memset(vencFd, 0, sizeof(vencFd));
    memset(stream_path, 0, sizeof(stream_path));
    memset(saveStreamCnt, 0, sizeof(saveStreamCnt));
    memset(totalSaveStreamCnt, 0, sizeof(totalSaveStreamCnt));

	for (i = 0; i < FS_CHN_NUM; i++) {
        streamFd[i] = -1;
        vencFd[i] = -1;
        saveStreamCnt[i] = 0;
        if (chn[i].enable) {
            if (chn[i].payloadType == PT_JPEG) {
                chnNum = 3 + chn[i].index;
                totalSaveStreamCnt[i] = (NR_FRAMES_TO_SAVE / 50 > 0) ? NR_FRAMES_TO_SAVE / 50 : NR_FRAMES_TO_SAVE;
            } else {
                chnNum = chn[i].index;
                totalSaveStreamCnt[i] = NR_FRAMES_TO_SAVE;
            }
            sprintf(stream_path[i], "%s/stream-%d.%s", STREAM_FILE_PATH_PREFIX, chnNum,
                    (chn[i].payloadType == PT_H264) ? "h264" : ((chn[i].payloadType == PT_H265) ? "h265" : "jpeg"));

            if (chn[i].payloadType != PT_JPEG) {
                streamFd[i] = open(stream_path[i], O_RDWR | O_CREAT | O_TRUNC, 0777);
                if (streamFd[i] < 0) {
                    IMP_LOG_ERR(TAG, "open %s failed:%s\n", stream_path[i], strerror(errno));
                    return -1;
                }
            }

            vencFd[i] = IMP_Encoder_GetFd(chnNum);
            if (vencFd[i] < 0) {
                IMP_LOG_ERR(TAG, "IMP_Encoder_GetFd(%d) failed\n", chnNum);
                return -1;
            }

            if (maxVencFd < vencFd[i]) {
                maxVencFd = vencFd[i];
            }

            ret = IMP_Encoder_StartRecvPic(chnNum);
            if (ret < 0) {
                IMP_LOG_ERR(TAG, "IMP_Encoder_StartRecvPic(%d) failed\n", chnNum);
                return -1;
            }
        }
    }

    while(1) {
        int breakFlag = 1;
        for (i = 0; i < FS_CHN_NUM; i++) {
            breakFlag &= (saveStreamCnt[i] >= totalSaveStreamCnt[i]);
        }
        if (breakFlag) {
            break;  // save frame enough
        }

        FD_ZERO(&readfds);
        for (i = 0; i < FS_CHN_NUM; i++) {
            if (chn[i].enable && saveStreamCnt[i] < totalSaveStreamCnt[i]) {
                FD_SET(vencFd[i], &readfds);
            }
        }
        selectTimeout.tv_sec = 2;
        selectTimeout.tv_usec = 0;

        ret = select(maxVencFd + 1, &readfds, NULL, NULL, &selectTimeout);
        if (ret < 0) {
            IMP_LOG_ERR(TAG, "select failed:%s\n", strerror(errno));
            return -1;
        } else if (ret == 0) {
            continue;
        } else {
            for (i = 0; i < FS_CHN_NUM; i++) {
                if (chn[i].enable && FD_ISSET(vencFd[i], &readfds)) {
                    IMPEncoderStream stream;

                    if (chn[i].payloadType == PT_JPEG) {
                        chnNum = 3 + chn[i].index;
                    } else {
                        chnNum = chn[i].index;
                    }
                    /* Get H264 or H265 Stream */
                    ret = IMP_Encoder_GetStream(chnNum, &stream, 1);
                    if (ret < 0) {
                        IMP_LOG_ERR(TAG, "IMP_Encoder_GetStream(%d) failed\n", chnNum);
                        return -1;
                    }

                    if (chn[i].payloadType == PT_JPEG) {
                        ret = save_stream_by_name(stream_path[i], saveStreamCnt[i], &stream);
                        if (ret < 0) {
                            return -1;
                        }
                    } else {
                        ret = save_stream(streamFd[i], &stream);
                        if (ret < 0) {
                            close(streamFd[i]);
                            return -1;
                        }
                    }

                    IMP_Encoder_ReleaseStream(chnNum, &stream);
                    saveStreamCnt[i]++;
                }
            }
        }
    }

	for (i = 0; i < FS_CHN_NUM; i++) {
        if (chn[i].enable) {
            if (chn[i].payloadType == PT_JPEG) {
                chnNum = 3 + chn[i].index;
            } else {
                chnNum = chn[i].index;
            }
            IMP_Encoder_StopRecvPic(chnNum);
            close(streamFd[i]);
        }
    }

    return 0;
}
