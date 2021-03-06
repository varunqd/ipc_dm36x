#ifndef AR0331_WDR
 
#include "drv_imgs_AR0331_3.1MP.h"
#include <drv_gpio.h>

DRV_ImgsObj gDRV_imgsObj;

#define INPUT_CLK    24  //EXTCLK
#define PLL_M        198  //pll_multiplier
#define PLL_pre_div  8   //pre_pll_clk_div
#define PLL_P1       1   //vt_sys_clk_div
#define PLL_P2       8   //vt_pix_clk_div
#define OUT_CLK ((INPUT_CLK * PLL_M) / (PLL_pre_div * PLL_P1 * PLL_P2))//74.25MHz

#define LINE_LENGTH (0x3e8) //1000,1080p 
#define ROW_TIME    29.6297 //Xiangdong //27     // (2*LINE_LENGTH / OUT_CLK)//26.9us,two paths readout,

#define rev2_linear

int DRV_imgsEnable_AR0331(Bool enable);

int DRV_imgsCheckId_AR0331()
{
  int status;
  Uint16  regAddr;
  Uint16 regValue;

  regAddr = CHIP_VERSION_ADDR;
  regValue = 0;

  status = DRV_i2c16Read16(&gDRV_imgsObj.i2cHndl, &regAddr, &regValue, 1);

  if(status!=OSA_SOK) {
    OSA_ERROR("DRV_i2cRead16()\n");
    return status;
  }
  if(regValue!=IMGS_CHIP_ID)
    return OSA_EFAIL;

  return OSA_SOK;
}

int DRV_imgsOpen_AR0331(DRV_ImgsConfig *config)
{
  int status, retry=10;
  Uint16 width, height;
  
  memset(&gDRV_imgsObj, 0, sizeof(gDRV_imgsObj));

  DRV_imgGetWidthHeight(config->sensorMode, &width, &height);

  width+=IMGS_H_PAD;
  height+=IMGS_V_PAD;

  DRV_imgsCalcFrameTime_AR0331(config->fps, width, height, config->binEnable);

  status = DRV_i2cOpen(&gDRV_imgsObj.i2cHndl, IMGS_I2C_ADDR);
  if(status!=OSA_SOK) {
    OSA_ERROR("DRV_i2cOpen()\n");
    return status;
  }

  #ifdef BOARD_AP_IPNC
  DRV_gpioSetMode(IMGS_RESET_GPIO, DRV_GPIO_DIR_OUT);
  DRV_gpioSet(IMGS_RESET_GPIO);
  DRV_gpioClr(IMGS_RESET_GPIO);
  OSA_waitMsecs(50);
  DRV_gpioSet(IMGS_RESET_GPIO);
  OSA_waitMsecs(50);
  #endif
  do {
    status = DRV_imgsCheckId_AR0331();
    if(status==OSA_SOK)
      break;
    OSA_waitMsecs(10);
  } while(retry--);

  if(status!=OSA_SOK) {
    OSA_ERROR("DRV_imgsCheckId()\n");
    DRV_i2cClose(&gDRV_imgsObj.i2cHndl);
    return status;
  }
return 0;
}

int DRV_imgsClose_AR0331()
{
  int status;

  status = DRV_imgsEnable_AR0331(FALSE);
  status |= DRV_i2cClose(&gDRV_imgsObj.i2cHndl);

  return status;
}

char* DRV_imgsGetImagerName_AR0331()
{
  return "Aptina_AR0331_3.1MP";
}

int DRV_imgsSpecificSetting_AR0331(void)
{
	return 0;
}

int DRV_imgsSet50_60Hz_AR0331(Bool is50Hz)
{
 /*
 int fps;

  if( gDRV_imgsObj.curFrameTime.fps==30
    ||gDRV_imgsObj.curFrameTime.fps==25
    ) {

    if(is50Hz)
      fps = 25;
    else
      fps = 30;

    DRV_imgsSetFramerate(fps);
  }
  return 0;
*/
return 0;
}

int DRV_imgsSetFramerate_AR0331(int fps)
{
/*
	Uint16 regAddr[4];
	Uint16 regValue[4];
	int i = 0;
	int j;
	int status = 0;

  switch(fps){
      case 30:
         i = 0;
		       regAddr[i] = AR0331_PLL_Multiplier; regValue[i] = 0x0031; i++; 	
		       regAddr[i] = AR0331_Pre_PLL_CLK_DIV; regValue[i] = 0x0002; i++; 	
			regAddr[i] = AR0331_VT_SYS_CLK_DIV; regValue[i] = 0x0001; i++; 	
			regAddr[i] = AR0331_VT_PIX_CLK_DIV; regValue[i] = 0x0008; i++; 
        break;
      case 24:
         i = 0;
		       regAddr[i] = AR0331_PLL_Multiplier; regValue[i] = 0x002D; i++; 	
		       regAddr[i] = AR0331_Pre_PLL_CLK_DIV; regValue[i] = 0x0002; i++; 
			regAddr[i] = AR0331_VT_SYS_CLK_DIV; regValue[i] = 0x0001; i++; 	
			regAddr[i] = AR0331_VT_PIX_CLK_DIV; regValue[i] = 0x000A; i++; 	
        break;
      case 15:
         i = 0;
		       regAddr[i] = AR0331_PLL_Multiplier; regValue[i] = 0x002D; i++; 	
		       regAddr[i] = AR0331_Pre_PLL_CLK_DIV; regValue[i] = 0x0002; i++; 
			regAddr[i] = AR0331_VT_SYS_CLK_DIV; regValue[i] = 0x0001; i++; 	
			regAddr[i] = AR0331_VT_PIX_CLK_DIV; regValue[i] = 0x0010; i++; 	
        break;
      case 8:
                 i = 0;
		       regAddr[i] = AR0331_PLL_Multiplier; regValue[i] = 0x002D; i++; 	
		       regAddr[i] = AR0331_Pre_PLL_CLK_DIV; regValue[i] = 0x0004; i++; 
			regAddr[i] = AR0331_VT_SYS_CLK_DIV; regValue[i] = 0x0001; i++; 	
			regAddr[i] = AR0331_VT_PIX_CLK_DIV; regValue[i] = 0x000F; i++; 	
        break;
      default: 
         break;
       }
for(j = 0; j < i ; j++)
{
	status = DRV_i2c16Write16(&gDRV_imgsObj.i2cHndl, regAddr, regValue, i);
	if(status!=OSA_SOK) {
		OSA_ERROR("DRV_i2c16Write16()\n");
		return status;
	}
	OSA_waitMsecs(1);
}
      OSA_waitMsecs(50);
    return status;
*/    
    return 0;
}

int DRV_imgsBinEnable_AR0331(Bool enable)
{
/*
  Uint8 regAddr[4];
  Uint16 regValue[4];
  int i, col_bin, row_bin, status;

  if(!enable) {
    col_bin = 0;
    row_bin = 0;
  } else {
    col_bin = gDRV_imgsObj.curFrameTime.col_bin;
    row_bin = gDRV_imgsObj.curFrameTime.row_bin;
  }

  i=0;
  regAddr[i]  = ROW_ADDR_MODE;
  regValue[i] = (gDRV_imgsObj.curFrameTime.row_skip & 0x7 ) | ((row_bin & 0x3) << 4);
  i++;

  regAddr[i]  = COL_ADDR_MODE;
  regValue[i] = (gDRV_imgsObj.curFrameTime.col_skip & 0x7 ) | ((col_bin & 0x3) << 4);
  i++;

  status = DRV_i2cWrite16(&gDRV_imgsObj.i2cHndl, regAddr, regValue, i);
  if(status!=OSA_SOK) {
    OSA_ERROR("DRV_i2cWrite16()\n");
    return status;
  }

  return status;
*/
return 0;
}

int DRV_imgsBinMode_AR0331(int binMode)
{
/*
  Uint8 regAddr[4];
  Uint16 regValue[4];
  int i, status;

  i=0;
  regAddr[i]  = READ_MODE_2;
  regValue[i] = binMode;
  i++;


  status = DRV_i2cWrite16(&gDRV_imgsObj.i2cHndl, regAddr, regValue, i);
  if(status!=OSA_SOK) {
    OSA_ERROR("DRV_i2cWrite16()\n");
    return status;
  }

  return status;
*/
return 0;
}

int DRV_imgsSetDgain_AR0331(int dgain)
{
    return OSA_SOK;
}

int DRV_imgsSetAgain_AR0331(int again, int setRegDirect)
{

  Uint16 regAddr;
  Uint16 regValue;

  int status=0;
  regAddr = AR0331_ANALOG_GAIN;

  if (again >= 8000)
  {
    regValue = 0x30;//x8
  }
  else if (again >= 5333)
  {
      regValue = 0x28;//5.3
  }
  else if (again >= 4000)
  {
      regValue = 0x20;//4
  }
  else if (again >= 2666)
  {
      regValue = 0x18;//2.6
  }
  else if (again >= 2000)
  {
      regValue = 0x10;//2
  }
  else if (again >= 1333)
  {
      regValue = 0x08;//1.3
  }
  else
  {
      regValue = 0x00;
  }

  //printf("analog:%d\n",regValue);
  status = DRV_i2c16Write16(&gDRV_imgsObj.i2cHndl, &regAddr, &regValue, 1);  
  if(status!=OSA_SOK) {
	OSA_ERROR("I2C write error\n");
	return status;
  }
  OSA_waitMsecs(10);

  return 0;
}

int DRV_imgsSetEshutter_AR0331(Uint32 eshutterInUsec, int setRegDirect)
{
  int status;
  Uint16 regAddr;
  Uint16 regValue;
  regAddr =AR0331_COARSE_IT_TIME_A;
  regValue = (float)(eshutterInUsec/ROW_TIME);
//  regValue = 0x134; // 8333/27
  status = DRV_i2c16Write16(&gDRV_imgsObj.i2cHndl, &regAddr, &regValue, 1);
  if(status!=OSA_SOK) {
    OSA_ERROR("DRV_i2c16Write16()\n");
    return status;
  }
// printf("eshutterInUsec:%d\n",eshutterInUsec);
  return status;
 
}
int AR0331_HDR_Enable_AR0331(int enable)
{
   return 0;
}

//Read AGain & exposure
int DRV_imgsGetAgain_AR0331(int *again)
{
/*
  Uint16 regAddr;
  Uint16 regValue;
  int status;

  regAddr = AR0331_GLOABL_GAIN;

  status = DRV_i2c16Read16(&gDRV_imgsObj.i2cHndl, &regAddr, &regValue, 1);

  if(status!=OSA_SOK) {
    OSA_ERROR("DRV_i2c16Read16()\n");
    return status;
  }

  *again = regValue;
  return status;
 */
 return 0;
}

int DRV_imgsGetEshutter_AR0331(Uint32 *eshutterInUsec)
{
/*
  int status;
  Uint16 regAddr;
  Uint16 regValue;

  regAddr = AR0331_COARSE_IT_TIME_A;

  status = DRV_i2c16Read16(&gDRV_imgsObj.i2cHndl, &regAddr, &regValue, 1);
  if(status!=OSA_SOK) {
    OSA_ERROR("DRV_i2c16Write16()\n");
    return status;
  }

 *eshutterInUsec = regValue*ROW_TIME;

  return status;
  */
  return 0;
}

int DRV_imgsSetDcSub_AR0331(Uint32 dcSub, int setRegDirect)
{

  return 0;
}

int DRV_imgsEnable_AR0331(Bool enable)
{
  int status=OSA_SOK;

  if(enable) {
    status = DRV_imgsSetRegs_AR0331();
    if(status!=OSA_SOK) {
      OSA_ERROR("DRV_imgsSetRegs()\n");
      return status;
    }
  }
  return status;
}

DRV_ImgsModeConfig      *DRV_imgsGetModeConfig_AR0331(int sensorMode)
{
  return &gDRV_imgsObj.curModeConfig;
}

DRV_ImgsIsifConfig      *DRV_imgsGetIsifConfig_AR0331(int sensorMode)
{
  return &gDRV_imgsIsifConfig_Common;
}

DRV_ImgsIpipeConfig     *DRV_imgsGetIpipeConfig_AR0331(int sensorMode, int vnfDemoCfg)
{
  if(vnfDemoCfg)
  	return &gDRV_imgsIpipeConfig_Vnfdemo;
  else
  	return &gDRV_imgsIpipeConfig_Common;
}

DRV_ImgsH3aConfig       *DRV_imgsGetH3aConfig_AR0331(int sensorMode, int aewbVendor)
{
  if(aewbVendor==1) {
  	return &gDRV_imgsH3aConfig_Appro;
  }
  else if(aewbVendor==2) {
  	return &gDRV_imgsH3aConfig_TI;
  }
  else {
  	return &gDRV_imgsH3aConfig_Common;
  }
}

DRV_ImgsLdcConfig       *DRV_imgsGetLdcConfig_AR0331(int sensorMode, Uint16 ldcInFrameWidth, Uint16 ldcInFrameHeight)
{
  sensorMode &= 0xFF;

  switch(sensorMode) {

    case DRV_IMGS_SENSOR_MODE_720x480:

      if(ldcInFrameWidth==864)
        return &gDRV_imgsLdcConfig_736x480_0_VS;

      if(ldcInFrameWidth==352)
        return &gDRV_imgsLdcConfig_736x480_1_VS;

      if(ldcInFrameWidth==736)
        return &gDRV_imgsLdcConfig_736x480_0;

      if(ldcInFrameWidth==288)
        return &gDRV_imgsLdcConfig_736x480_1;

      if(ldcInFrameWidth==768)
        return &gDRV_imgsLdcConfig_768x512_0;

      if(ldcInFrameWidth==320)
        return &gDRV_imgsLdcConfig_768x512_1;

      if(ldcInFrameWidth==928)
        return &gDRV_imgsLdcConfig_768x512_0_VS;

      if(ldcInFrameWidth==384)
        return &gDRV_imgsLdcConfig_768x512_1_VS;

      break;

    case DRV_IMGS_SENSOR_MODE_1280x720:

      if(ldcInFrameWidth==1280)
        return &gDRV_imgsLdcConfig_1280x736_0;

      if(ldcInFrameWidth==320)
        return &gDRV_imgsLdcConfig_1280x736_1;

      if(ldcInFrameWidth==640)
        return &gDRV_imgsLdcConfig_1280x736_2;

      if(ldcInFrameWidth==1536)
        return &gDRV_imgsLdcConfig_1280x736_0_VS;

      if(ldcInFrameWidth==384)
        return &gDRV_imgsLdcConfig_1280x736_1_VS;

      if(ldcInFrameWidth==768)
        return &gDRV_imgsLdcConfig_1280x736_2_VS;

      if(ldcInFrameWidth==1312)
        return &gDRV_imgsLdcConfig_1312x768_0;

      if(ldcInFrameWidth==352)
        return &gDRV_imgsLdcConfig_1312x768_1;

      if(ldcInFrameWidth==672)
        return &gDRV_imgsLdcConfig_1312x768_2;

      if(ldcInFrameWidth==1600)
        return &gDRV_imgsLdcConfig_1312x768_0_VS;

      if(ldcInFrameWidth==448)
        return &gDRV_imgsLdcConfig_1312x768_1_VS;

      if(ldcInFrameWidth==832)
        return &gDRV_imgsLdcConfig_1312x768_2_VS;

      break;

  }

  return NULL;
}

int DRV_imgsReset_AR0331()
{
  return 0;
}



int DRV_imgsSetRegs_AR0331()
 {
 
  Uint16  regAddr[500];
  Uint16 regValue[500];
  int status,i=0;
  int j;
  memset(regAddr, 0, 500*sizeof(Uint16));
  memset(regValue, 0,500*sizeof(Uint16));

#ifdef rev2_linear

        regAddr[i] = 0x301A; regValue[i] = 0x0001 	// RESET_REGISTER
; i++ ; regAddr[i] = 0x301A; regValue[i] = 0x10D8 	// RESET_REGISTER
; i++ ; regAddr[i] = 0x3088; regValue[i] = 0x8000 	// SEQ_CTRL_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x4A03 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x4316 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x0443 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x1645 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x4045 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x6017 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x5045 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x404B 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x1244 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x6134 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x4A31 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x4342 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x4560 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x2714 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x3DFF 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x3DFF 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x3DEA 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x2704 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x3D10 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x2705 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x3D10 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x2715 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x3527 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x053D 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x1045 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x4027 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x0427 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x143D 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0xFF3D 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0xFF3D 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0xEA62 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x2728 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x3627 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x083D 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x6444 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x2C2C 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x2C2C 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x4B01 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x432D 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x4643 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x1647 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x435F 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x4F50 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x2604 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x2694 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x2027 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0xFC53 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x0D5C 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x0D57 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x5417 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x0955 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x5649 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x5307 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x5303 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x4D28 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x6C4C 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x0928 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x2C28 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x294E 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x5C09 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x4500 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x4580 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x26B6 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x27F8 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x1702 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x27FA 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x5C0B 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x1718 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x26B2 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x5C03 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x1744 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x27F2 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x1702 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x2809 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x1710 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x1628 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x084D 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x1A26 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x9316 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x1627 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0xFA45 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0xA017 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x0727 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0xFB17 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x2945 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x8017 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x0827 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0xFA17 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x285D 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x170E 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x2691 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x5301 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x1740 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x5302 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x1710 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x2693 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x2692 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x484D 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x4E28 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x094C 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x0B17 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x5F27 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0xF217 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x1428 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x0816 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x4D1A 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x1616 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x27FA 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x2603 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x5C01 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x4540 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x2798 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x172A 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x4A0A 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x4316 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x0B43 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x279C 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x4560 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x1707 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x279D 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x1725 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x4540 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x1708 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x2798 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x5D53 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x0D26 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x455C 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x014B 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x1244 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x5251 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x1702 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x6018 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x4A03 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x4316 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x0443 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x1658 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x4316 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x5943 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x165A 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x4316 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x5B43 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x4540 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x279C 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x4560 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x1707 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x279D 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x1725 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x4540 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x1710 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x2798 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x1720 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x224B 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x1244 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x2C2C 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x3086; regValue[i] = 0x2C2C 	// SEQ_DATA_PORT
; i++ ; regAddr[i] = 0x301A; regValue[i] = 0x10D8 	// RESET_; i++ ; regAddr[i] ISTER
; i++ ; regAddr[i] = 0x30B0; regValue[i] = 0x0000 	// DIGITAL_TEST
; i++ ; regAddr[i] = 0x30BA; regValue[i] = 0x06EC 	// DIGITAL_CTRL
; i++ ; regAddr[i] = 0x31AC; regValue[i] = 0x0C0C 	// DATA_FORMAT_BITS
; i++ ; regAddr[i] = 0x302A; regValue[i] = PLL_P2 	// VT_PIX_CLK_DIV
; i++ ; regAddr[i] = 0x302C; regValue[i] = PLL_P1 	// VT_SYS_CLK_DIV
; i++ ; regAddr[i] = 0x302E; regValue[i] = PLL_pre_div	// PRE_PLL_CLK_DIV
; i++ ; regAddr[i] = 0x3030; regValue[i] = PLL_M 	// PLL_MULTIPLIER
; i++ ; regAddr[i] = 0x3036; regValue[i] = 0x000C 	// OP_PIX_CLK_DIV
; i++ ; regAddr[i] = 0x3038; regValue[i] = 0x0001 	// OP_SYS_CLK_DIV
; i++ ; regAddr[i] = 0x3002; regValue[i] = 0x00E4 	// Y_ADDR_START
; i++ ; regAddr[i] = 0x3004; regValue[i] = 0x0042 	// X_ADDR_START
; i++ ; regAddr[i] = 0x3006; regValue[i] = 0x0523 	// Y_ADDR_END
; i++ ; regAddr[i] = 0x3008; regValue[i] = 0x07C9 	// X_ADDR_END
; i++ ; regAddr[i] = 0x300A; regValue[i] = 0x0465 	// FRAME_LENGTH_LINES
; i++ ; regAddr[i] = 0x300C; regValue[i] = 0x44c;//Xiangdong: change back to default;0x0428 //0x044C 	// LINE_LENGTH_PCK
; i++ ; regAddr[i] = 0x3012; regValue[i] = 0x0416 	// COARSE_INTEGRATION_TIME
; i++ ; regAddr[i] = 0x30A2; regValue[i] = 0x0001 	// X_ODD_INC
; i++ ; regAddr[i] = 0x30A6; regValue[i] = 0x0001 	// Y_ODD_INC
; i++ ; regAddr[i] = 0x3040; regValue[i] = 0xc000 	// READ_MODE
; i++ ; regAddr[i] = 0x31AE; regValue[i] = 0x0301 	// SERIAL_FORMAT
; i++ ; regAddr[i] = 0x3082; regValue[i] = 0x0009 	// OPERATION_MODE_CTRL
; i++ ; regAddr[i] = 0x318C; regValue[i] = 0x0000 	// HDR_MC_CTRL2
; i++ ; regAddr[i] = 0x301A; regValue[i] = 0x10D8 	// RESET_; i++ ; regAddr[i] ISTER
; i++ ; regAddr[i] = 0x2400; regValue[i] = 0x0003 	// ALTM_CONTROL
; i++ ; regAddr[i] = 0x301E; regValue[i] = 0x00A8 	// DATA_PEDESTAL
; i++ ; regAddr[i] = 0x301A; regValue[i] = 0x10DC 	// RESET_; i++ ; regAddr[i] ISTER
; i++ ; regAddr[i] = 0x3200; regValue[i] = 0x0000 	// ADACD_CONTROL
; i++ ; regAddr[i] = 0x31D0; regValue[i] = 0x0000 	// COMPANDING
; i++ ; regAddr[i] = 0x30FE; regValue[i] = 0x0080 	// NOISE_PEDESTAL
; i++ ; regAddr[i] = 0x31E0; regValue[i] = 0x0200 	// PIX_DEF_ID
; i++ ; regAddr[i] = 0x320A; regValue[i] = 0x0080 	// ADACD_PEDESTAL
; i++ ; regAddr[i] = 0x2450; regValue[i] = 0x0000 	// ALTM_OUT_PEDESTAL
; i++ ; regAddr[i] = 0x301E; regValue[i] = 0x00A8 	// DATA_PEDESTAL
; i++ ; regAddr[i] = 0x3064; regValue[i] = 0x1982 	// SMIA_TEST
; i++ ; regAddr[i] = 0x3064; regValue[i] = 0x1982 	// SMIA_TEST
; i++ ; regAddr[i] = 0x3180; regValue[i] = 0x8089 	// DELTA_DK_CONTROL
; i++ ; regAddr[i] = 0x30F4; regValue[i] = 0x4000 	// RESERVED_MFR_30F4
; i++ ; regAddr[i] = 0x3ED4; regValue[i] = 0x8F6C 	// RESERVED_MFR_3ED4
; i++ ; regAddr[i] = 0x3ED6; regValue[i] = 0x6666 	// RESERVED_MFR_3ED6
; i++ ; regAddr[i] = 0x3EDA; regValue[i] = 0x8899 	// RESERVED_MFR_3EDA
; i++ ; regAddr[i] = 0x3EE6; regValue[i] = 0x00F0 	// RESERVED_MFR_3EE6
; i++ ; regAddr[i] = 0x3ED2; regValue[i] = 0x9F46 	// DAC_LD_6_7
; i++ ; regAddr[i] = 0x301A; regValue[i] = 0x10DC 	// RESET_; i++ ; regAddr[i] ISTER
; i++ ; regAddr[i] = 0x3060; regValue[i] = 0x0012 	// ANALOG_GAIN
; i++ ; regAddr[i] = 0x305A; regValue[i] = 0x009A 	// RED_GAIN
; i++ ; regAddr[i] = 0x3056; regValue[i] = 0x0083 	// GREEN1_GAIN
; i++ ; regAddr[i] = 0x305C; regValue[i] = 0x0083 	// GREEN2_GAIN
; i++ ; regAddr[i] = 0x3058; regValue[i] = 0x00CD 	// BLUE_GAIN
; i++ ; regAddr[i] = 0x3012; regValue[i] = 0x0445 	// COARSE_INTEGRATION_TIME
; i++ ; regAddr[i] = 0x3060; regValue[i] = 0x000E 	// ANALOG_GAIN
; i++ ; regAddr[i] = 0x305A; regValue[i] = 0x0093 	// RED_GAIN
; i++ ; regAddr[i] = 0x3056; regValue[i] = 0x0080 	// GREEN1_GAIN
; i++ ; regAddr[i] = 0x305C; regValue[i] = 0x0080 	// GREEN2_GAIN
; i++ ; regAddr[i] = 0x3058; regValue[i] = 0x00D3 	// BLUE_GAIN
; i++ ; regAddr[i] = 0x3060; regValue[i] = 0x000C 	// ANALOG_GAIN
; i++ ; regAddr[i] = 0x305A; regValue[i] = 0x0092 	// RED_GAIN
; i++ ; regAddr[i] = 0x3056; regValue[i] = 0x0080 	// GREEN1_GAIN
; i++ ; regAddr[i] = 0x305C; regValue[i] = 0x0080 	// GREEN2_GAIN
; i++ ; regAddr[i] = 0x3058; regValue[i] = 0x00D5 	// BLUE_GAIN
; i++ ; regAddr[i] = 0x3060; regValue[i] = 0x000B 	// ANALOG_GAIN
; i++ ; regAddr[i] = 0x305A; regValue[i] = 0x0091 	// RED_GAIN
; i++ ; regAddr[i] = 0x3056; regValue[i] = 0x0080 	// GREEN1_GAIN
; i++ ; regAddr[i] = 0x305C; regValue[i] = 0x0080 	// GREEN2_GAIN
; i++ ; regAddr[i] = 0x3058; regValue[i] = 0x00D8 	// BLUE_GAIN
; i++ ; regAddr[i] = 0x3060; regValue[i] = 0x000B 	// ANALOG_GAIN
; i++ ; regAddr[i] = 0x305A; regValue[i] = 0x0091 	// RED_GAIN
; i++ ; regAddr[i] = 0x3056; regValue[i] = 0x0080 	// GREEN1_GAIN
; i++ ; regAddr[i] = 0x305C; regValue[i] = 0x0080 	// GREEN2_GAIN
; i++ ; regAddr[i] = 0x3058; regValue[i] = 0x00D9 	// BLUE_GAIN
; i++ ; regAddr[i] = 0x3060; regValue[i] = 0x000B 	// ANALOG_GAIN
; i++ ; regAddr[i] = 0x305A; regValue[i] = 0x0091 	// RED_GAIN
; i++ ; regAddr[i] = 0x3056; regValue[i] = 0x0080 	// GREEN1_GAIN
; i++ ; regAddr[i] = 0x305C; regValue[i] = 0x0080 	// GREEN2_GAIN
; i++ ; regAddr[i] = 0x3058; regValue[i] = 0x00DA 	// BLUE_GAIN
; i++ ; regAddr[i] = 0x3060; regValue[i] = 0x000B 	// ANALOG_GAIN
; i++ ; regAddr[i] = 0x305A; regValue[i] = 0x0091 	// RED_GAIN
; i++ ; regAddr[i] = 0x3056; regValue[i] = 0x0080 	// GREEN1_GAIN
; i++ ; regAddr[i] = 0x305C; regValue[i] = 0x0080 	// GREEN2_GAIN
; i++ ; regAddr[i] = 0x3058; regValue[i] = 0x00D9 	// BLUE_GAIN
; i++ ; regAddr[i] = 0x3060; regValue[i] = 0x000B 	// ANALOG_GAIN
; i++ ; regAddr[i] = 0x305A; regValue[i] = 0x0091 	// RED_GAIN
; i++ ; regAddr[i] = 0x3056; regValue[i] = 0x0080 	// GREEN1_GAIN
; i++ ; regAddr[i] = 0x305C; regValue[i] = 0x0080 	// GREEN2_GAIN
; i++ ; regAddr[i] = 0x3058; regValue[i] = 0x00D8 	// BLUE_GAIN
; i++ ; regAddr[i] = 0x3060; regValue[i] = 0x000B 	// ANALOG_GAIN
; i++ ; regAddr[i] = 0x305A; regValue[i] = 0x0092 	// RED_GAIN
; i++ ; regAddr[i] = 0x3056; regValue[i] = 0x0080 	// GREEN1_GAIN
; i++ ; regAddr[i] = 0x305C; regValue[i] = 0x0080 	// GREEN2_GAIN
; i++ ; regAddr[i] = 0x3058; regValue[i] = 0x00D6 	// BLUE_GAIN
; i++ ; regAddr[i] = 0x3060; regValue[i] = 0x000B 	// ANALOG_GAIN
; i++ ; regAddr[i] = 0x305A; regValue[i] = 0x0092 	// RED_GAIN
; i++ ; regAddr[i] = 0x3056; regValue[i] = 0x0080 	// GREEN1_GAIN
; i++ ; regAddr[i] = 0x305C; regValue[i] = 0x0080 	// GREEN2_GAIN
; i++ ; regAddr[i] = 0x3058; regValue[i] = 0x00D5 	// BLUE_GAIN
; i++ ; regAddr[i] = 0x3060; regValue[i] = 0x000B 	// ANALOG_GAIN
; i++ ; regAddr[i] = 0x305A; regValue[i] = 0x0093 	// RED_GAIN
; i++ ; regAddr[i] = 0x3056; regValue[i] = 0x0080 	// GREEN1_GAIN
; i++ ; regAddr[i] = 0x305C; regValue[i] = 0x0080 	// GREEN2_GAIN
; i++ ; regAddr[i] = 0x3058; regValue[i] = 0x00D3 	// BLUE_GAIN
; i++ ; regAddr[i] = 0x3060; regValue[i] = 0x000B 	// ANALOG_GAIN
; i++ ; regAddr[i] = 0x305A; regValue[i] = 0x0093 	// RED_GAIN
; i++ ; regAddr[i] = 0x3056; regValue[i] = 0x0080 	// GREEN1_GAIN
; i++ ; regAddr[i] = 0x305C; regValue[i] = 0x0080 	// GREEN2_GAIN
; i++ ; regAddr[i] = 0x3058; regValue[i] = 0x00D1 	// BLUE_GAIN
; i++ ; regAddr[i] = 0x3060; regValue[i] = 0x000B 	// ANALOG_GAIN
; i++ ; regAddr[i] = 0x305A; regValue[i] = 0x0094 	// RED_GAIN
; i++ ; regAddr[i] = 0x3056; regValue[i] = 0x0080 	// GREEN1_GAIN
; i++ ; regAddr[i] = 0x305C; regValue[i] = 0x0080 	// GREEN2_GAIN
; i++ ; regAddr[i] = 0x3058; regValue[i] = 0x00D0 	// BLUE_GAIN
; i++ ; regAddr[i] = 0x3060; regValue[i] = 0x000B 	// ANALOG_GAIN
; i++ ; regAddr[i] = 0x305A; regValue[i] = 0x0095 	// RED_GAIN
; i++ ; regAddr[i] = 0x3056; regValue[i] = 0x0080 	// GREEN1_GAIN
; i++ ; regAddr[i] = 0x305C; regValue[i] = 0x0080 	// GREEN2_GAIN
; i++ ; regAddr[i] = 0x3058; regValue[i] = 0x00CE 	// BLUE_GAIN
; i++ ; regAddr[i] = 0x3060; regValue[i] = 0x000B 	// ANALOG_GAIN
; i++ ; regAddr[i] = 0x305A; regValue[i] = 0x0095 	// RED_GAIN
; i++ ; regAddr[i] = 0x3056; regValue[i] = 0x0080 	// GREEN1_GAIN
; i++ ; regAddr[i] = 0x305C; regValue[i] = 0x0080 	// GREEN2_GAIN
; i++ ; regAddr[i] = 0x3058; regValue[i] = 0x00CC 	// BLUE_GAIN
; i++ ; regAddr[i] = 0x3060; regValue[i] = 0x000B 	// ANALOG_GAIN
; i++ ; regAddr[i] = 0x305A; regValue[i] = 0x0096 	// RED_GAIN
; i++ ; regAddr[i] = 0x3056; regValue[i] = 0x0080 	// GREEN1_GAIN
; i++ ; regAddr[i] = 0x305C; regValue[i] = 0x0080 	// GREEN2_GAIN
; i++ ; regAddr[i] = 0x3058; regValue[i] = 0x00CA 	// BLUE_GAIN
; i++ ; regAddr[i] = 0x3060; regValue[i] = 0x000B 	// ANALOG_GAIN
; i++ ; regAddr[i] = 0x305A; regValue[i] = 0x0097 	// RED_GAIN
; i++ ; regAddr[i] = 0x3056; regValue[i] = 0x0080 	// GREEN1_GAIN
; i++ ; regAddr[i] = 0x305C; regValue[i] = 0x0080 	// GREEN2_GAIN
; i++ ; regAddr[i] = 0x3058; regValue[i] = 0x00C9 	// BLUE_GAIN
; i++ ; regAddr[i] = 0x3060; regValue[i] = 0x000B 	// ANALOG_GAIN
; i++ ; regAddr[i] = 0x305A; regValue[i] = 0x0097 	// RED_GAIN
; i++ ; regAddr[i] = 0x3056; regValue[i] = 0x0080 	// GREEN1_GAIN
; i++ ; regAddr[i] = 0x305C; regValue[i] = 0x0080 	// GREEN2_GAIN
; i++ ; regAddr[i] = 0x3058; regValue[i] = 0x00C8 	// BLUE_GAIN
; i++ ; regAddr[i] = 0x3060; regValue[i] = 0x000B 	// ANALOG_GAIN
; i++ ; regAddr[i] = 0x305A; regValue[i] = 0x0097 	// RED_GAIN
; i++ ; regAddr[i] = 0x3056; regValue[i] = 0x0080 	// GREEN1_GAIN
; i++ ; regAddr[i] = 0x305C; regValue[i] = 0x0080 	// GREEN2_GAIN
; i++ ; regAddr[i] = 0x3058; regValue[i] = 0x00C7 	// BLUE_GAIN
; i++ ; regAddr[i] = 0x3060; regValue[i] = 0x000B 	// ANALOG_GAIN
; i++ ; regAddr[i] = 0x305A; regValue[i] = 0x0098 	// RED_GAIN
; i++ ; regAddr[i] = 0x3056; regValue[i] = 0x0080 	// GREEN1_GAIN
; i++ ; regAddr[i] = 0x305C; regValue[i] = 0x0080 	// GREEN2_GAIN
; i++ ; regAddr[i] = 0x3058; regValue[i] = 0x00C6 	// BLUE_GAIN
; i++ ; regAddr[i] = 0x3060; regValue[i] = 0x000B 	// ANALOG_GAIN
; i++ ; regAddr[i] = 0x305A; regValue[i] = 0x0098 	// RED_GAIN
; i++ ; regAddr[i] = 0x3056; regValue[i] = 0x0080 	// GREEN1_GAIN
; i++ ; regAddr[i] = 0x305C; regValue[i] = 0x0080 	// GREEN2_GAIN
; i++ ; regAddr[i] = 0x3058; regValue[i] = 0x00C5 	// BLUE_GAIN
; i++ ; regAddr[i] = 0x3060; regValue[i] = 0x000B 	// ANALOG_GAIN
; i++ ; regAddr[i] = 0x305A; regValue[i] = 0x0098 	// RED_GAIN
; i++ ; regAddr[i] = 0x3056; regValue[i] = 0x0080 	// GREEN1_GAIN
; i++ ; regAddr[i] = 0x305C; regValue[i] = 0x0080 	// GREEN2_GAIN
; i++ ; regAddr[i] = 0x3058; regValue[i] = 0x00C4 	// BLUE_GAIN
; i++ ; regAddr[i] = 0x3060; regValue[i] = 0x000B 	// ANALOG_GAIN
; i++ ; regAddr[i] = 0x305A; regValue[i] = 0x0098 	// RED_GAIN
; i++ ; regAddr[i] = 0x3056; regValue[i] = 0x0080 	// GREEN1_GAIN
; i++ ; regAddr[i] = 0x305C; regValue[i] = 0x0080 	// GREEN2_GAIN
; i++ ; regAddr[i] = 0x3058; regValue[i] = 0x00C4 	// BLUE_GAIN
; i++ ; regAddr[i] = 0x3060; regValue[i] = 0x000B 	// ANALOG_GAIN
; i++ ; regAddr[i] = 0x305A; regValue[i] = 0x0098 	// RED_GAIN
; i++ ; regAddr[i] = 0x3056; regValue[i] = 0x0080 	// GREEN1_GAIN
; i++ ; regAddr[i] = 0x305C; regValue[i] = 0x0080 	// GREEN2_GAIN
; i++ ; regAddr[i] = 0x3058; regValue[i] = 0x00C4 	// BLUE_GAIN
; i++ ; regAddr[i] = 0x3060; regValue[i] = 0x000B 	// ANALOG_GAIN
; i++ ; regAddr[i] = 0x305A; regValue[i] = 0x0098 	// RED_GAIN
; i++ ; regAddr[i] = 0x3056; regValue[i] = 0x0080 	// GREEN1_GAIN
; i++ ; regAddr[i] = 0x305C; regValue[i] = 0x0080 	// GREEN2_GAIN
; i++ ; regAddr[i] = 0x3058; regValue[i] = 0x00C5 	// BLUE_GAIN
; i++ ; regAddr[i] = 0x3060; regValue[i] = 0x000B 	// ANALOG_GAIN
; i++ ; regAddr[i] = 0x305A; regValue[i] = 0x0098 	// RED_GAIN
; i++ ; regAddr[i] = 0x3056; regValue[i] = 0x0080 	// GREEN1_GAIN
; i++ ; regAddr[i] = 0x305C; regValue[i] = 0x0080 	// GREEN2_GAIN
; i++ ; regAddr[i] = 0x3058; regValue[i] = 0x00C5 	// BLUE_GAIN
; i++ ; regAddr[i] = 0x3060; regValue[i] = 0x000B 	// ANALOG_GAIN
; i++ ; regAddr[i] = 0x305A; regValue[i] = 0x0098 	// RED_GAIN
; i++ ; regAddr[i] = 0x3056; regValue[i] = 0x0080 	// GREEN1_GAIN
; i++ ; regAddr[i] = 0x305C; regValue[i] = 0x0080 	// GREEN2_GAIN
; i++ ; regAddr[i] = 0x3058; regValue[i] = 0x00C6 	// BLUE_GAIN
; i++ ; regAddr[i] = 0x3060; regValue[i] = 0x000B 	// ANALOG_GAIN
; i++ ; regAddr[i] = 0x305A; regValue[i] = 0x0098 	// RED_GAIN
; i++ ; regAddr[i] = 0x3056; regValue[i] = 0x0080 	// GREEN1_GAIN
; i++ ; regAddr[i] = 0x305C; regValue[i] = 0x0080 	// GREEN2_GAIN
; i++ ; regAddr[i] = 0x3058; regValue[i] = 0x00C6 	// BLUE_GAIN
; i++ ; regAddr[i] = 0x3060; regValue[i] = 0x000B 	// ANALOG_GAIN
; i++ ; regAddr[i] = 0x305A; regValue[i] = 0x0098 	// RED_GAIN
; i++ ; regAddr[i] = 0x3056; regValue[i] = 0x0080 	// GREEN1_GAIN
; i++ ; regAddr[i] = 0x305C; regValue[i] = 0x0080 	// GREEN2_GAIN
; i++ ; regAddr[i] = 0x3058; regValue[i] = 0x00C7 	// BLUE_GAIN
; i++;
#endif

for(j=0; j < i; j++)
{
   status = DRV_i2c16Write16(&gDRV_imgsObj.i2cHndl, &regAddr[j], &regValue[j], 1);  
	if(status != OSA_SOK)
	{
	   printf("I2C write Error,index:%d\n",j);
	   return status;
	}
    if(regAddr[j] == 0x301A )	
       OSA_waitMsecs(100);

    OSA_waitMsecs(1);	
}
printf("Finished Linear Mode Init with AR0331 rev2.\n");

return 0;

}

#endif //#ifndef AR0331_WDR

DRV_ImgsFuncs  AR0331ImgsFuncs = 
{
    .imgsOpen            = DRV_imgsOpen_AR0331,
    .imgsClose           = DRV_imgsClose_AR0331,
    .imgsSetMirror       = NULL,
    .imgsGetImagerName   = DRV_imgsGetImagerName_AR0331,
    .imgsSetAgain        = DRV_imgsSetAgain_AR0331,
    .imgsSetDgain        = DRV_imgsSetDgain_AR0331,
    .imgsSetEshutter     = DRV_imgsSetEshutter_AR0331,
    .imgsSetDcSub        = DRV_imgsSetDcSub_AR0331,
    .imgsBinEnable       = DRV_imgsBinEnable_AR0331,
    .imgsBinMode         = DRV_imgsBinMode_AR0331,
    .imgsSetFramerate    = DRV_imgsSetFramerate_AR0331,
    .imgsSet50_60Hz      = DRV_imgsSet50_60Hz_AR0331, 
    .imgsEnable          = DRV_imgsEnable_AR0331,
    .imgsGetModeConfig   = DRV_imgsGetModeConfig_AR0331,
    .imgsGetIsifConfig   = DRV_imgsGetIsifConfig_AR0331,
    .imgsGetH3aConfig    = DRV_imgsGetH3aConfig_AR0331,
    .imgsGetIpipeConfig  = DRV_imgsGetIpipeConfig_AR0331,
    .imgsGetLdcConfig    = DRV_imgsGetLdcConfig_AR0331
};

