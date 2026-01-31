#include "main.h"
#include <string.h>
#include "CameraConfigs.h"
#include "Camera.h"
#include "dcmipp.h"
#include "i2c.h"
#include "Display/Display.h"
#include "SimpleLogger/SimpleLogger.h"
#include "NeuralNetwork/NeuralNetwork.h"
#include "hpdma.h"

// Defines
#define CAM_I2C_ADDRESS        (0x60)
#define CAM_TIMEOUT_MS         (300)
#define CAM_WIDTH              (256)
#define CAM_HEIGHT             (256)
#define CAM_CONTENTS_HEIGHT    (190)
#define CAM_BYTES_PER_PIXEL    (3)
#define CAM_FRAMEBUFFER_SIZE   (CAM_WIDTH * CAM_HEIGHT * CAM_BYTES_PER_PIXEL)
#define CAM_DMA_SIZE		   (0xFFFC)

// Variables
__attribute((aligned(32))) uint8_t cam_full_frame_buffer[CAM_FRAMEBUFFER_SIZE];
static CAM_State_Type cam_state = CAM_STATE_RESET;
static uint8_t* cam_frame_buffer;
static uint32_t cam_start_tick = 0;
static bool cam_stop_flag = false;
static uint8_t cam_dma_cycle = 0;

//Static functions

static bool CAM_Config(const uint8_t camera_config[][2], int config_size)
{
    HAL_StatusTypeDef ret;
    uint8_t val;
    uint8_t addr;

    //Reset according to OV2640 HW application notes
    HAL_GPIO_WritePin(GPIOH, OV_RST_Pin, GPIO_PIN_RESET);
    HAL_Delay(3);
    HAL_GPIO_WritePin(GPIOH, OV_RST_Pin, GPIO_PIN_SET);
    HAL_Delay(3);

    addr = 0xff;
    val = 0x01;
    ret = HAL_I2C_Mem_Write(&hi2c4, CAM_I2C_ADDRESS, addr, 1, &val, 1, 50);
    addr = 0x12;
    val = 0x80;
    ret = HAL_I2C_Mem_Write(&hi2c4, CAM_I2C_ADDRESS, addr, 1, &val, 1, 50);
    HAL_Delay(100);

    uint8_t pid = 0; // expected 0x26
    uint8_t vid = 0; // expected 0x42
    ret = HAL_I2C_Mem_Read(&hi2c4, CAM_I2C_ADDRESS, 0x0a, 1, &pid, 1, 50);
    ret = HAL_I2C_Mem_Read(&hi2c4, CAM_I2C_ADDRESS, 0x0b, 1, &vid, 1, 50);
    if (ret != HAL_OK)
    {
    	Log("CAM", "I2C config failed, HAL error");
    	return false;
    }

    if (vid != 0x42 || pid != 0x26)
    {
    	Log("CAM", "I2C config failed, invalid VID/PID value");
        return false;
    }

    for (int counter = 0; counter * 2 < config_size; counter++)
    {
        ret = HAL_I2C_Mem_Write(&hi2c4, CAM_I2C_ADDRESS, camera_config[counter][0], 1,
                                (uint8_t*)&camera_config[counter][1], 1, 50);

        if (ret != HAL_OK)
        {
        	Log("CAM", "I2C config failed, HAL error");
        	return false;
        }

        if (camera_config[counter][0] == 0x12 && camera_config[counter][1] == 0x80)
        {
            HAL_Delay(100);
        }
        if (camera_config[counter][0] == 0xe0)
        {
            HAL_Delay(100);
        }

        HAL_Delay(2);
    }

    HAL_Delay(50);
    Log("CAM", "I2C config completed.");
    return true;
}

static uint8_t* CAM_GetDMASrcAddress(void)
{
	uint8_t* base_addr = cam_frame_buffer;
	base_addr += cam_dma_cycle * CAM_DMA_SIZE;

	return base_addr;
}

static uint8_t* CAM_GetDMADestAddress(void)
{
	uint8_t* base_addr = NN_GetInputAddres();
	base_addr += (CAM_HEIGHT - CAM_CONTENTS_HEIGHT) / 2 * CAM_WIDTH * CAM_BYTES_PER_PIXEL;
	base_addr += cam_dma_cycle * CAM_DMA_SIZE;

	return base_addr;
}

static uint16_t CAM_GetDMASize(void)
{
	return cam_dma_cycle < 2 ? CAM_DMA_SIZE : (CAM_FRAMEBUFFER_SIZE - 2 * CAM_DMA_SIZE);
}

static void CAM_CheckTimeout(void)
{
	if (cam_state == CAM_STATE_RESET || cam_state == CAM_STATE_READY || cam_state == CAM_STATE_ERROR)
		return;

    if ((HAL_GetTick() - cam_start_tick) > CAM_TIMEOUT_MS)
    {
    	Log("CAM", "timeout during state: %d", cam_state);
        cam_state = CAM_STATE_ERROR;
    }
}

static bool CAM_CheckStop(void)
{
	if (cam_stop_flag)
	{
		if (HAL_DCMIPP_PIPE_Stop(&hdcmipp, DCMIPP_PIPE1) != HAL_OK)
		{
			Log("CAM", "capture stop failed, HAL error.");
		}
		else
		{
			Log("CAM", "capture stopped.");
		}
		cam_state = CAM_STATE_READY;
		cam_stop_flag = false;
		return true;
	}

	return false;
}

//Callbacks
void HAL_DCMIPP_PIPE_FrameEventCallback(DCMIPP_HandleTypeDef *hdcmipp, uint32_t Pipe)
{
    (void)hdcmipp;
    BSP_LED_Toggle(LED_BLUE);

    if (Pipe != DCMIPP_PIPE1)
    {
    	Log("CAM", "frame callback with invalid pipe. Ignoring");
    	return;
    }

	static uint32_t lastTime=0;
	uint32_t time=HAL_GetTick();
	uint32_t dt=time-lastTime;
	lastTime=time;

    if (cam_state == CAM_STATE_WAITING_CAPTURE)
    {
		cam_state = CAM_STATE_WAITING_COPY;
		Log("CAM", "frame callback, starting DMA copy.");
		if (dt > 0){ Log("CAM", "dt: %dms, fps: %d",dt,1000/dt);}
    	if (HAL_DMA_Start_IT(&handle_HPDMA1_Channel15, (uint32_t)CAM_GetDMASrcAddress(), (uint32_t)CAM_GetDMADestAddress(), CAM_GetDMASize()) != HAL_OK)
    	{
    		Log("CAM", "DMA start failed.");
    		cam_state = CAM_STATE_ERROR;
    	}
	}
    /*else
    {
    	Log("CAM", "frame callback during invalid state: %d.", cam_state);
    }*/
}

void HAL_DCMIPP_ErrorCallback(DCMIPP_HandleTypeDef *hdcmipp)
{
	(void)hdcmipp;
	if (cam_state != CAM_STATE_RESET && cam_state != CAM_STATE_READY)
	{
		Log("CAM", "DCMI-PP error callback. Cannot capture frame.");
		cam_state = CAM_STATE_ERROR;
	}
	else
	{
		Log("CAM", "DCMI-PP error callback during invalid state: %d.", cam_state);
	}
}

void HAL_DMA_CPLT_Callback(DMA_HandleTypeDef *handle)
{
	(void)handle;

	SCB_InvalidateDCache_by_Addr(NN_GetInputAddres(), CAM_FRAMEBUFFER_SIZE);
	if (CAM_GetDMASrcAddress()[0] != CAM_GetDMADestAddress()[0])
	{
		Log("CAM", "First byte mismatch");
	}

	if (cam_state != CAM_STATE_WAITING_COPY)
	{
		Log("CAM", "DMA callback during invalid state: %d.", cam_state);
		return;
	}

	if (cam_dma_cycle < 2)
	{
		cam_dma_cycle++;
		//Log("CAM", "DMA callback, next cycle");
    	if (HAL_DMA_Start_IT(&handle_HPDMA1_Channel15, (uint32_t)CAM_GetDMASrcAddress(), (uint32_t)CAM_GetDMADestAddress(), CAM_GetDMASize()) != HAL_OK)
    	{
    		Log("CAM", "DMA start failed.");
    		cam_state = CAM_STATE_ERROR;
    	}

	}
	else
	{
		Log("CAM", "DMA callback, DMA done, starting inference");
		cam_state = CAM_STATE_WAITING_INFERENCE;
		cam_dma_cycle = 0;
	}
}

// Public functions
bool CAM_Init(void)
{
	cam_state = CAM_STATE_RESET;

    if (!CAM_Config(camera_config, sizeof(camera_config)))
    {
    	return false;
    }

    //Framebuffers config
    cam_frame_buffer = cam_full_frame_buffer + (CAM_HEIGHT - CAM_CONTENTS_HEIGHT) / 2 * CAM_WIDTH * CAM_BYTES_PER_PIXEL;

    //Enable RISAF
    __HAL_RCC_RISAF_CLK_ENABLE();
    RISAF2->REG[0].CFGR = 0x00000000;
    RISAF2->REG[1].CFGR = 0x00000000;
    RISAF2->REG[0].CIDCFGR = 0x000F000F; /* RW for everyone */
    RISAF2->REG[0].ENDR = 0xFFFFFFFF;     /* all-encompassing */
    RISAF2->REG[0].CFGR = 0x00000101;     /* enabled, secure, unprivileged for everyone */
    RISAF2->REG[1].CIDCFGR = 0x00FF00FF; /* RW for everyone */
    RISAF2->REG[1].ENDR = 0xFFFFFFFF;     /* all-encompassing */
    RISAF2->REG[1].CFGR = 0x00000001;     /* enabled, non-secure, unprivileged*/

    RISAF3->REG[0].CFGR = 0x00000000;
    RISAF3->REG[1].CFGR = 0x00000000;
    RISAF3->REG[0].CIDCFGR = 0x000F000F; /* RW for everyone */
    RISAF3->REG[0].ENDR = 0xFFFFFFFF;     /* all-encompassing */
    RISAF3->REG[0].CFGR = 0x00000101;     /* enabled, secure, unprivileged for everyone */
    RISAF3->REG[1].CIDCFGR = 0x00FF00FF; /* RW for everyone */
    RISAF3->REG[1].ENDR = 0xFFFFFFFF;     /* all-encompassing */
    RISAF3->REG[1].CFGR = 0x00000001;     /* enabled, non-secure, unprivileged*/

    //DMA config
    if (HAL_DMA_RegisterCallback(&handle_HPDMA1_Channel15, HAL_DMA_XFER_CPLT_CB_ID, HAL_DMA_CPLT_Callback) != HAL_OK)
    {
    	Log("CAM", "DMA register failed");
    	return false;
    }

    //DCMI-PP config
    if (HAL_DCMIPP_PIPE_EnableRedBlueSwap(&hdcmipp, DCMIPP_PIPE1) != HAL_OK)
    {
    	Log("CAM", "DCMI-PP red-blue swap failed");
    	return false;
    }

    int inputH=400*2;
    int inputV=296*2;
    DCMIPP_DownsizeTypeDef ds = {0};
    ds.HSize = 256; // destination width
    ds.VSize = 190; // destination height

    // Ratios in unsigned 3.13 format: floor(((Src-1) * 8192) / (Dst-1))
    //ds.HRatio = 12817;
    //ds.VRatio = 12785;
    ds.HRatio=(inputH-1)*8192/(ds.HSize-1)-1;
    ds.VRatio=(inputV-1)*8192/(ds.VSize-1)-1;

    // Division factors (see ST ref manual) clamp to <=1023
    // Suggested: floor(min(1024 * Dst / Src, 1023))
    //ds.HDivFactor = 655;
    //ds.VDivFactor = 657;
    ds.HDivFactor=1024*ds.HSize/inputH;
    ds.VDivFactor=1024*ds.VSize/inputV;

    if (HAL_DCMIPP_PIPE_SetDownsizeConfig(&hdcmipp, DCMIPP_PIPE1, &ds) != HAL_OK)
    {
    	Log("CAM", "DCMI-PP set downsize failed");
        return false;
    }

    if (HAL_DCMIPP_PIPE_EnableDownsize(&hdcmipp, DCMIPP_PIPE1) != HAL_OK)
    {
    	Log("CAM", "DCMI-PP enable downsize failed");
        return false;
    }

    cam_state = CAM_STATE_READY;
    Log("CAM", "init done");
    return true;
}

bool CAM_Start(void)
{
    if (cam_state != CAM_STATE_READY)
    {
    	Log("CAM", "cannot start, invalid state: %d", cam_state);
        return false;
    }

    DS_ResetState();
    cam_state = CAM_STATE_WAITING_CAPTURE;
    cam_start_tick = HAL_GetTick();

    if (HAL_DCMIPP_PIPE_Start(&hdcmipp, DCMIPP_PIPE1, (uint32_t)(cam_frame_buffer), DCMIPP_MODE_CONTINUOUS) != HAL_OK)
    {
    	Log("CAM", "cannot start, HAL error");
        return false;
    }

    Log("CAM", "capture started");
    return true;
}

bool CAM_Stop(void)
{
	if (cam_state != CAM_STATE_WAITING_CAPTURE && cam_state != CAM_STATE_WAITING_COPY && cam_state != CAM_STATE_WAITING_INFERENCE)
	{
		Log("CAM", "cannot stop during invalid state: %d.", cam_state);
		return false;
	}

	Log("CAM", "capture stop requested.");
	cam_stop_flag = true;
	return true;
}

CAM_State_Type CAM_GetState(void)
{
	return cam_state;
}

void CAM_Process(void)
{
	CAM_CheckTimeout();

    switch (cam_state)
    {
    case CAM_STATE_WAITING_INFERENCE:
		if (NN_Run() == NN_STATE_READY)
		{
			Log("CAM", "inference done, detected objects: %d.", NN_GetOutput()->nb_detect);
			DS_NetworkOutput(NN_GetOutput());
			cam_state = CAM_STATE_WAITING_RENDERING;
		}
        break;

    case CAM_STATE_WAITING_RENDERING:
    	if (DS_IsReadyToUpdate())
    	{
    		if (!CAM_CheckStop())
    		{
				Log("CAM", "rendering done");
    			cam_state = CAM_STATE_WAITING_CAPTURE;
    			cam_start_tick = HAL_GetTick();
			}
    	}
    	break;

    case CAM_STATE_ERROR:
    	//Error during processing, try to restart or stop
    	BSP_LED_Toggle(LED_RED);
    	if (CAM_CheckStop())
    	{
    		Log("CAM", "error during capture with stop signal, go back to ready state");
    	}
    	else
    	{
			if (HAL_DCMIPP_PIPE_GetState(&hdcmipp, DCMIPP_PIPE1) == HAL_DCMIPP_PIPE_STATE_BUSY)
			{
				if (HAL_DCMIPP_PIPE_Stop(&hdcmipp, DCMIPP_PIPE1) != HAL_OK)
				{
					Log("CAM", "error during capture, cannot stop running capture (HAL error), go back to ready state");
					cam_state = CAM_STATE_READY;
					return;
				}
			}

			Log("CAM", "error during capture, restart capture");
			cam_state = CAM_STATE_READY;
			CAM_Start();
    	}
    	break;

    default:
    	break;
    }
}


//void BSP_PB_Callback(Button_TypeDef)
//{
//    if (cam_state == CAM_STATE_READY)
//    	CAM_Start();
//    else
//    	CAM_Stop();
//}
