/*
 * configs3.h
 *
 *  Created on: Sep 17, 2025
 *      Author: Vikti
 */

#ifndef SRC_CONFIGS3_H_
#define SRC_CONFIGS3_H_


/* DSP register bank FF=0x00*/
#define OV2640_R_BYPASS            0x05
#define OV2640_QS                  0x44
#define OV2640_CTRLI               0x50
#define OV2640_HSIZE               0x51
#define OV2640_VSIZE               0x52
#define OV2640_XOFFL               0x53
#define OV2640_YOFFL               0x54
#define OV2640_VHYX                0x55
#define OV2640_DPRP                0x56
#define OV2640_TEST                0x57
#define OV2640_ZMOW                0x5A
#define OV2640_ZMOH                0x5B
#define OV2640_ZMHH                0x5C
#define OV2640_BPADDR              0x7C
#define OV2640_BPDATA              0x7D
#define OV2640_CTRL2               0x86
#define OV2640_CTRL3               0x87
#define OV2640_SIZEL               0x8C
#define OV2640_HSIZE8              0xC0
#define OV2640_VSIZE8              0xC1
#define OV2640_CTRL0               0xC2
#define OV2640_CTRL1               0xC3
#define OV2640_R_DVP_SP            0xD3
#define OV2640_IMAGE_MODE          0xDA
#define OV2640_RESET               0xE0
#define OV2640_MS_SP               0xF0
#define OV2640_SS_ID               0xF7
#define OV2640_SS_CTRL             0xF7
#define OV2640_MC_BIST             0xF9
#define OV2640_MC_AL               0xFA
#define OV2640_MC_AH               0xFB
#define OV2640_MC_D                0xFC
#define OV2640_P_CMD               0xFD
#define OV2640_P_STATUS            0xFE
#define OV2640_BANK_SEL            0xFF

#define OV2640_CTRLI_LP_DP         0x80
#define OV2640_CTRLI_ROUND         0x40

#define OV2640_CTRL0_AEC_EN        0x80
#define OV2640_CTRL0_AEC_SEL       0x40
#define OV2640_CTRL0_STAT_SEL      0x20
#define OV2640_CTRL0_VFIRST        0x10
#define OV2640_CTRL0_YUV422        0x08
#define OV2640_CTRL0_YUV_EN        0x04
#define OV2640_CTRL0_RGB_EN        0x02
#define OV2640_CTRL0_RAW_EN        0x01

#define OV2640_CTRL2_DCW_EN        0x20
#define OV2640_CTRL2_SDE_EN        0x10
#define OV2640_CTRL2_UV_ADJ_EN     0x08
#define OV2640_CTRL2_UV_AVG_EN     0x04
#define OV2640_CTRL2_CMX_EN        0x01

#define OV2640_CTRL3_BPC_EN        0x80
#define OV2640_CTRL3_WPC_EN        0x40

#define OV2640_R_DVP_SP_AUTO_MODE  0x80

#define OV2640_R_BYPASS_DSP_EN         0x00
#define OV2640_R_BYPASS_DSP_BYPAS      0x01

#define OV2640_IMAGE_MODE_Y8_DVP_EN    0x40
#define OV2640_IMAGE_MODE_JPEG_EN      0x10
#define OV2640_IMAGE_MODE_YUV422       0x00
#define OV2640_IMAGE_MODE_RAW10        0x04
#define OV2640_IMAGE_MODE_RGB565       0x08
#define OV2640_IMAGE_MODE_HREF_VSYNC   0x02
#define OV2640_IMAGE_MODE_LBYTE_FIRST  0x01

#define OV2640_RESET_MICROC            0x40
#define OV2640_RESET_SCCB              0x20
#define OV2640_RESET_JPEG              0x10
#define OV2640_RESET_DVP               0x04
#define OV2640_RESET_IPU               0x02
#define OV2640_RESET_CIF               0x01

#define OV2640_MC_BIST_RESET           0x80
#define OV2640_MC_BIST_BOOT_ROM_SEL    0x40
#define OV2640_MC_BIST_12KB_SEL        0x20
#define OV2640_MC_BIST_12KB_MASK       0x30
#define OV2640_MC_BIST_512KB_SEL       0x08
#define OV2640_MC_BIST_512KB_MASK      0x0C
#define OV2640_MC_BIST_BUSY_BIT_R      0x02
#define OV2640_MC_BIST_MC_RES_ONE_SH_W 0x02
#define OV2640_MC_BIST_LAUNCH          0x01


typedef enum {
    BANK_DSP, BANK_SENSOR, BANK_MAX
} ov2640_bank_t;

/* Sensor register bank FF=0x01*/
#define OV2640_GAIN                0x00
#define OV2640_COM1                0x03
#define OV2640_REG04               0x04
#define OV2640_REG08               0x08
#define OV2640_COM2                0x09
#define OV2640_REG_PID             0x0A
#define OV2640_REG_VER             0x0B
#define OV2640_COM3                0x0C
#define OV2640_COM4                0x0D
#define OV2640_AEC                 0x10
#define OV2640_CLKRC               0x11
#define OV2640_COM7                0x12
#define OV2640_COM8                0x13
#define OV2640_COM9                0x14 /* AGC gain ceiling */
#define OV2640_COM10               0x15
#define OV2640_HSTART              0x17
#define OV2640_HSTOP               0x18
#define OV2640_VSTART              0x19
#define OV2640_VSTOP               0x1A
#define OV2640_REG_MIDH            0x1C
#define OV2640_REG_MIDL            0x1D
#define OV2640_AEW                 0x24
#define OV2640_AEB                 0x25
#define OV2640_VV                  0x26
#define OV2640_REG2A               0x2A
#define OV2640_FRARL               0x2B
#define OV2640_ADDVSL              0x2D
#define OV2640_ADDVSH              0x2E
#define OV2640_YAVG                0x2F
#define OV2640_HSDY                0x30
#define OV2640_HEDY                0x31
#define OV2640_REG32               0x32
#define OV2640_ARCOM2              0x34
#define OV2640_REG45               0x45
#define OV2640_FLL                 0x46
#define OV2640_FLH                 0x47
#define OV2640_COM19               0x48
#define OV2640_ZOOMS               0x49
#define OV2640_COM22               0x4B
#define OV2640_COM25               0x4E
#define OV2640_BD50                0x4F
#define OV2640_BD60                0x50
#define OV2640_REG5D               0x5D
#define OV2640_REG5E               0x5E
#define OV2640_REG5F               0x5F
#define OV2640_REG60               0x60
#define OV2640_HISTO_LOW           0x61
#define OV2640_HISTO_HIGH          0x62

#define OV2640_REG04_DEFAULT       0x28
#define OV2640_REG04_HFLIP_IMG     0x80
#define OV2640_REG04_VFLIP_IMG     0x40
#define OV2640_REG04_VREF_EN       0x10
#define OV2640_REG04_HREF_EN       0x08
#define OV2640_REG04_SET(x)        (REG04_DEFAULT|x)

#define OV2640_COM2_STDBY          0x10
#define OV2640_COM2_OUT_DRIVE_1x   0x00
#define OV2640_COM2_OUT_DRIVE_2x   0x01
#define OV2640_COM2_OUT_DRIVE_3x   0x02
#define OV2640_COM2_OUT_DRIVE_4x   0x03

#define OV2640_COM3_DEFAULT        0x38
#define OV2640_COM3_BAND_50Hz      0x04
#define OV2640_COM3_BAND_60Hz      0x00
#define OV2640_COM3_BAND_AUTO      0x02
#define OV2640_COM3_BAND_SET(x)    (COM3_DEFAULT|x)

#define OV2640_COM7_SRST           0x80
#define OV2640_COM7_RES_UXGA       0x00 /* UXGA */
#define OV2640_COM7_RES_SVGA       0x40 /* SVGA */
#define OV2640_COM7_RES_CIF        0x20 /* CIF  */
#define OV2640_COM7_ZOOM_EN        0x04 /* Enable Zoom */
#define OV2640_COM7_COLOR_BAR      0x02 /* Enable Color Bar Test */

#define OV2640_COM8_DEFAULT        0xC2
#define OV2640_COM8_BNDF_EN        0x20 /* Enable Banding filter */
#define OV2640_COM8_AGC_EN         0x04 /* AGC Auto/Manual control selection */
#define OV2640_COM8_AEC_EN         0x01 /* Auto/Manual Exposure control */
#define OV2640_COM8_SET(x)         (COM8_DEFAULT|x)

#define OV2640_COM9_DEFAULT        0x08
#define OV2640_COM9_AGC_GAIN_2x    0x00 /* AGC:    2x */
#define OV2640_COM9_AGC_GAIN_4x    0x01 /* AGC:    4x */
#define OV2640_COM9_AGC_GAIN_8x    0x02 /* AGC:    8x */
#define OV2640_COM9_AGC_GAIN_16x   0x03 /* AGC:   16x */
#define OV2640_COM9_AGC_GAIN_32x   0x04 /* AGC:   32x */
#define OV2640_COM9_AGC_GAIN_64x   0x05 /* AGC:   64x */
#define OV2640_COM9_AGC_GAIN_128x  0x06 /* AGC:  128x */
#define OV2640_COM9_AGC_SET(x)     (OV2640_COM9_DEFAULT|(x<<5))

#define OV2640_COM10_HREF_EN       0x80 /* HSYNC changes to HREF */
#define OV2640_COM10_HSYNC_EN      0x40 /* HREF changes to HSYNC */
#define OV2640_COM10_PCLK_NOTFREE  0x20 /* PCLK output option: not free running PCLK */
#define OV2640_COM10_PCLK_EDGE     0x10 /* Data is updated at the rising edge of PCLK */
#define OV2640_COM10_HREF_NEG      0x08 /* HREF negative */
#define OV2640_COM10_VSYNC_NEG     0x02 /* VSYNC negative */
#define OV2640_COM10_HSYNC_NEG     0x01 /* HSYNC negative */

#define OV2640_CTRL1_AWB           0x08 /* Enable AWB */

#define OV2640_VV_AGC_TH_SET(h,l)  ((h<<4)|(l&0x0F))

#define OV2640_REG32_UXGA          0x36
#define OV2640_REG32_SVGA          0x09
#define OV2640_REG32_CIF           0x89

#define OV2640_CLKRC_2X            0x80
#define OV2640_CLKRC_2X_UXGA       (0x01 | CLKRC_2X)
#define OV2640_CLKRC_2X_SVGA       CLKRC_2X
#define OV2640_CLKRC_2X_CIF        CLKRC_2X

#define OV_WIDTH (800)
#define OV_HEIGHT (600)
//#define OV_WIDTH (400)
//#define OV_HEIGHT (296)

uint8_t camera_config[][2] = {
{OV2640_BANK_SEL, BANK_SENSOR},
{OV2640_COM7, 0x80}, //RESET
{OV2640_BANK_SEL, BANK_DSP},
{0x2c, 0xff},
{0x2e, 0xdf},
{OV2640_BANK_SEL, BANK_SENSOR},
{0x3c, 0x32},

{OV2640_COM2, OV2640_COM2_OUT_DRIVE_1x}, //!

{OV2640_REG04, OV2640_REG04_DEFAULT},
{OV2640_COM8,  OV2640_COM8_DEFAULT  | OV2640_COM8_AGC_EN | OV2640_COM8_AEC_EN }, //No banding
{OV2640_COM9, OV2640_COM9_AGC_SET(OV2640_COM9_AGC_GAIN_8x)}, // Max gain


{0x2c, 0x0c},
{0x33, 0x78},
{0x3a, 0x33},
{0x3b, 0xfb},
{0x3e, 0x00},
{0x43, 0x11},
{0x16, 0x10},
{0x39, 0x92},
{0x35, 0xda},
{0x22, 0x1a},
{0x37, 0xc3},
{0x23, 0x00},

{OV2640_ARCOM2, 0xc0},
{0x06, 0x88},
{0x07, 0xc0},
{OV2640_COM4, 0x87},
{0x0e, 0x41},
{0x4c, 0x00},
{0x4a, 0x81},
{0x21, 0x99},
{OV2640_AEW, 0x68},  //! exposure
{OV2640_AEB, 0x58},  //! exposure
{OV2640_VV, OV2640_VV_AGC_TH_SET(8,2)},
{0x5c, 0x00},
{0x63, 0x00},
//{OV2640_HISTO_LOW, 0x70}, // default
//{OV2640_HISTO_HIGH, 0x80}, // default

{0x7c, 0x05},
{0x20, 0x80},
{0x28, 0x30},
{0x6c, 0x00},
{0x6d, 0x80},
{0x6e, 0x00},
{0x70, 0x02},
{0x71, 0x94},
{0x73, 0xc1},
{0x3d, 0x34},
{0x5a, 0x57},

{0x37, 0xc0},

{0x6d, 0x00},
{0x3d, 0x38},


{OV2640_BANK_SEL, BANK_DSP},
{0xe5, 0x7f},
{OV2640_MC_BIST, OV2640_MC_BIST_RESET | OV2640_MC_BIST_BOOT_ROM_SEL},
{0x41, 0x24},
{OV2640_RESET, OV2640_RESET_JPEG | OV2640_RESET_DVP},
{0x76, 0xff},
{0x33, 0xa0},
{0x42, 0x20},
{0x43, 0x18},
{0x4c, 0x00},
{OV2640_CTRL3, OV2640_CTRL3_WPC_EN | 0x10},
{0x88, 0x3f},
{0xd7, 0x03},
{0xd9, 0x10},

{0xc8, 0x08},
{0xc9, 0x80},
{OV2640_BPADDR, 0x00},
{OV2640_BPDATA, 0x00},
{OV2640_BPADDR, 0x03},
{OV2640_BPDATA, 0x48},
{OV2640_BPDATA, 0x48},
{OV2640_BPADDR, 0x08},
{OV2640_BPDATA, 0x20},
{OV2640_BPDATA, 0x10},
{OV2640_BPDATA, 0x0e},
{0x90, 0x00},
{0x91, 0x0e},
{0x91, 0x1a},
{0x91, 0x31},
{0x91, 0x5a},
{0x91, 0x69},
{0x91, 0x75},
{0x91, 0x7e},
{0x91, 0x88},
{0x91, 0x8f},
{0x91, 0x96},
{0x91, 0xa3},
{0x91, 0xaf},
{0x91, 0xc4},
{0x91, 0xd7},
{0x91, 0xe8},
{0x91, 0x20},


{0x92, 0x01}, //manual sharp?
{0x93, 0x3}, //nosharp? smooth?
//{0x93, 0xc2},
//{0x93, 0x00},
//{0x93, 0xc0},

{0x93, 0x06},

{0x93, 0xe3},
{0x93, 0x05},
{0x93, 0x05},
{0x93, 0x00},
{0x93, 0x04},
{0x93, 0x00},
{0x93, 0x00},
{0x93, 0x00},
{0x93, 0x00},
{0x93, 0x00},
{0x93, 0x00},
{0x93, 0x00},
{0x96, 0x00},
{0x97, 0x08},
{0x97, 0x19},
{0x97, 0x02},
{0x97, 0x0c},
{0x97, 0x24},
{0x97, 0x30},
{0x97, 0x28},
{0x97, 0x26},
{0x97, 0x02},
{0x97, 0x98},
{0x97, 0x80},
{0x97, 0x00},
{0x97, 0x00},
{0xa4, 0x00},
{0xa8, 0x00},
{0xc5, 0x11},
{0xc6, 0x51},
{0xbf, 0x80},
{0xc7, 0x10},
{0xb6, 0x66},
{0xb8, 0xa5},
{0xb7, 0x64},
{0xb9, 0x7c},
{0xb3, 0xaf},
{0xb4, 0x97},
{0xb5, 0xff},
{0xb0, 0xc5},
{0xb1, 0x94},
{0xb2, 0x0f},
{0xc4, 0x5c},
{0xc3, 0xfd},
{0x7f, 0x00},
{0xe5, 0x1f},
{0xe1, 0x67},
{0xdd, 0x7f},


{OV2640_IMAGE_MODE, OV2640_IMAGE_MODE_RGB565},//mode: rgb565

{OV2640_RESET, 0x00},
{OV2640_R_BYPASS, OV2640_R_BYPASS_DSP_EN},
{OV2640_R_BYPASS, OV2640_R_BYPASS_DSP_BYPAS},
{OV2640_BANK_SEL, BANK_SENSOR},

{OV2640_COM7, OV2640_COM7_RES_SVGA}, //cif or svga, pattern  | COM7_COLOR_BAR

{OV2640_COM1, 0x0a}, //for svga mode
//{OV2640_COM1, 0x06}, //for cif mode

{OV2640_REG32, OV2640_REG32_SVGA}, // for svga

{OV2640_HSTART, 0x11},
{OV2640_HSTOP, 0x11+50},

{OV2640_VSTART, 0x00},
{OV2640_VSTOP, 75},

{OV2640_FRARL,180 },//original: 0, line interval for frame time adjusting

{OV2640_BD50, 0xca},
{OV2640_BD60, 0xa8},
{0x5a, 0x23},
{0x6d, 0x00},
{0x3d, 0x38},
{0x39, 0x92},
{0x35, 0xda},
{0x22, 0x1a},
{0x37, 0xc3},
{0x23, 0x00},
//{OV2640_ARCOM2, 0xc0}, //original
{OV2640_ARCOM2, 0x20}, // datasheet default
{0x06, 0x88},
{0x07, 0xc0},
{OV2640_COM4, 0x87},
{0x0e, 0x41},
{0x4c, 0x00},

//FF00
{OV2640_BANK_SEL, BANK_DSP},
{OV2640_RESET, OV2640_RESET_DVP|OV2640_RESET_CIF},//cif dvp

{OV2640_HSIZE8, (OV_WIDTH)/8},
{OV2640_VSIZE8, (OV_HEIGHT)/8},

{OV2640_SIZEL, 0x00},

{OV2640_XOFFL, 0x00},
{OV2640_YOFFL, 0x00},
{OV2640_VHYX, 0x00},
{OV2640_TEST, 0x00},

//{OV2640_CTRL2, OV2640_CTRL2_DCW_EN|OV2640_CTRL2_UV_ADJ_EN|OV2640_CTRL2_UV_AVG_EN|OV2640_CTRL2_CMX_EN},//special effect kikapcsolva 0x2d
{OV2640_CTRL2, OV2640_CTRL2_DCW_EN|OV2640_CTRL2_CMX_EN},//special effect kikapcsolva 0x2d
{OV2640_CTRLI, OV2640_CTRLI_ROUND},

{OV2640_HSIZE, (OV_WIDTH)/4},
{OV2640_VSIZE, (OV_HEIGHT)/4},

{OV2640_XOFFL, 0x00},
{OV2640_YOFFL, 0x00},
{OV2640_VHYX, 0x00},
{OV2640_TEST, 0x00},

{OV2640_ZMOW, (OV_WIDTH)/4},
{OV2640_ZMOH, (OV_HEIGHT)/4},

{OV2640_ZMHH, 0x00},
{OV2640_BANK_SEL, BANK_SENSOR},

{OV2640_CLKRC, 0x01}, // Main CLK divisor

{OV2640_AEC, 0x33*3}, // Exposure control
{OV2640_REG45, 0x02}, // Exposure control, aec top
{OV2640_BANK_SEL, BANK_DSP},

{OV2640_R_DVP_SP, OV2640_R_DVP_SP_AUTO_MODE|0x02},// Pixel clock auto

{OV2640_R_BYPASS, OV2640_R_BYPASS_DSP_EN}, // Enable no bypass
{OV2640_RESET, OV2640_RESET_DVP},

{OV2640_IMAGE_MODE, OV2640_IMAGE_MODE_RGB565},//rgb565

//{0xd7, 0x01}, // if yuv
{0xd7, 0x03},  //if rgb565
//{0xe1, 0x67}, //if yuv
{0xe1, 0x77}, //if rgb565
{OV2640_RESET, 0x00},
{OV2640_RESET, OV2640_RESET_DVP},
{OV2640_IMAGE_MODE, OV2640_IMAGE_MODE_RGB565},//rgb565
//{0xd7, 0x01}, // if yuv
{0xd7, 0x03},  //if rgb565
//{0xe1, 0x67}, //if yuv
{0xe1, 0x77}, //if rgb565

{OV2640_RESET, 0x00},
{OV2640_BANK_SEL, BANK_SENSOR},

{OV2640_BANK_SEL, BANK_DSP},
{OV2640_CTRL3, 0x50},
{OV2640_CTRL1, 0xff},
{OV2640_CTRL0, OV2640_CTRL0_RGB_EN | OV2640_CTRL0_YUV_EN | OV2640_CTRL0_AEC_EN| OV2640_CTRL0_STAT_SEL },// OV2640_CTRL0_AEC_EN| OV2640_CTRL0_STAT_SEL | |OV2640_CTRL0_YUV422| OV2640_CTRL0_YUV_EN  0b11001110
{OV2640_BANK_SEL, BANK_SENSOR},
{OV2640_BANK_SEL, BANK_DSP},


{OV2640_BANK_SEL, BANK_SENSOR},
{OV2640_BANK_SEL, BANK_DSP},
{OV2640_BANK_SEL, BANK_SENSOR},
{OV2640_BANK_SEL, BANK_DSP},
{OV2640_BANK_SEL, BANK_SENSOR},
{OV2640_BANK_SEL, BANK_DSP},
{OV2640_BANK_SEL, BANK_SENSOR},
{OV2640_BANK_SEL, BANK_DSP},
{OV2640_BANK_SEL, BANK_SENSOR}};


#endif /* SRC_CONFIGS3_H_ */
