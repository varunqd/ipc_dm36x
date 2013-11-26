
#include "alg_aewb_priv.h"
#include "TI_aewb.h"
#include "drv_ipipe.h"

static int RGB2RGB_stablize(int curr_RGB2RGBIndex, int reset)
{
#define AWB_AVG_BUF_LENGTH 12
    static int history_index[AWB_AVG_BUF_LENGTH];
    static int awb_count = 1;
    static int prev_RGB2RGBIndex = 0;

    int i;
    int max;
    int max_index = 0;
    int history_length;
    int index_histogram[NUM_RGB2RGB_MATRIXES];


    if (reset == 1)
    {
        awb_count = 1;
        prev_RGB2RGBIndex = 0;
    }

    if (awb_count < AWB_AVG_BUF_LENGTH)
    {
        history_length = awb_count;
        awb_count++;
    }
    else
    {
        history_length = AWB_AVG_BUF_LENGTH;
    }

    if (history_length == AWB_AVG_BUF_LENGTH)
    {
        for (i = 0; i < history_length - 1; i++)
        {
            history_index[i] = history_index[i + 1];
        }
    }
    history_index[history_length - 1] = curr_RGB2RGBIndex;

    for (i = 0; i < NUM_RGB2RGB_MATRIXES; i++)
    {
        index_histogram[i] = 0;
    }
    for (i = 0; i < history_length; i++)
    {
        index_histogram[history_index[i]]++;
    }

    max = 0;
    for (i = 0; i < NUM_RGB2RGB_MATRIXES; i++)
    {
        if (index_histogram[i] > max)
        {
            max = index_histogram[i];
            max_index = i;
        }
    }

    if (history_length < AWB_AVG_BUF_LENGTH)
    {
        curr_RGB2RGBIndex = max_index;
    }
    else
    {
        if (max * 10 >= (AWB_AVG_BUF_LENGTH * 8))
        {
            curr_RGB2RGBIndex = max_index;
        }
        else
        {
            curr_RGB2RGBIndex = prev_RGB2RGBIndex;
        }
    }
    prev_RGB2RGBIndex = curr_RGB2RGBIndex;

    return (curr_RGB2RGBIndex);
}

static int switch_rgb2rgb_matrixes(int colorTemp, struct rgb2rgb_index m[], int reset)
{
    int i, diff, next_diff;

    for (i = 0; i < NUM_RGB2RGB_MATRIXES - 1 && m[i+1].color_temp > 0; i++)
    {
        diff = colorTemp - m[i].color_temp;
        next_diff = m[i + 1].color_temp - colorTemp;
        if ((next_diff >= 0 && diff >= 0) || diff < 0)
        {
            if (next_diff < diff)
            {
                i++;
            }
            break;
        }
    }
    i = RGB2RGB_stablize(i, reset);

    static prev_i = -1;
    if (prev_i != i || reset)
    {
        ALG_aewbSetRgb2Rgb(&m[i].rgb2rgbparam);
        ALG_aewbSetRgb2Rgb2(&m[i].rgb2rgb2param);
        prev_i = i;
    }

    return i;
}

struct rgb2rgb_index rgb_matrixes_0[]= {
  {
//U30 
    .color_temp = 2800,
    .rgb2rgbparam = {
      347,  -64, -26,
      -111,  321, 46,
      -49, -242, 547,
        0,    0,   0
    },
    .rgb2rgb2param = {
      251,   -9,  14,
       -1,  256,  0,
       -5,  -4, 265,
        0,    0,   0
    }
  },
  {
//TL84
    .color_temp = 4000,
    .rgb2rgbparam = {
         345,  -106,  17,
         -88,   332,  12,
         -29,  -153, 437,
           0,     0,    0
    },
    .rgb2rgb2param = {
        252,    -8,   12,
          1,   255,    0,
         -2,    -3,  261,
          0,     0,    0
    }
  },
//D50
  {
    .color_temp = 5000,
    .rgb2rgbparam = {
      361, -125,  20,
      -77,  303,  30,
       -9, -142, 408,
        0,    0,   0
    },
    .rgb2rgb2param = {
      255,  -13,   14,
       -2,  256,   1,
       -6,  -1,  262,
        0,    0,    0
    }
  },
//D65
  {
    .color_temp = 6500,
    .rgb2rgbparam = {
      366, -127,  17,
      -69,  284,  41,
       -3, -131, 389,
        0,    0,   0
    },
    .rgb2rgb2param = {
      256,  -11,   11,
        0,  254,    2,
       -4,  -3,   262,
        0,    0,    0
    }
  },
//end
  {
    .color_temp = 0,
  }

};
static struct rgb2rgb_index rgb_matrixes_1[] = {
    {
        .color_temp = 3000,
        .rgb2rgbparam = {
            256, 0, 0,
            0, 256, 0,
            0, 0, 256,
            0, 0, 0},
        .rgb2rgb2param = {
            256, 0, 0,
            0, 256, 0,
            0, 0, 256,
            0, 0, 0}
    },
    //end
    {
        .color_temp = 0,
    }
};
static struct rgb2rgb_index* rgb_matrixes[]= {
    rgb_matrixes_0, 
    rgb_matrixes_1,
};

static CSL_IpipeEdgeEnhanceConfig ee_0 = {
    .enable = 1,
    .haloReduceEnable = 1,
    .mergeMethod = 0,    
    .hpfShift = 15,
    .table = NULL,
    .edgeSharpGain = 64,
    .edgeSharpGainGradient = 96,
    .edgeSharpHpValLowThres = 300,
    .edgeSharpOffsetGradient = 24, 
    .edgeSharpHpValUpLimit = 32,
};
static CSL_IpipeEdgeEnhanceConfig ee_1 = {
    .enable = 1,
    .haloReduceEnable = 1,
    .mergeMethod = 0,    
    .hpfShift = 15,      
    .table = NULL, 
    .edgeSharpGain = 32,
    .edgeSharpGainGradient = 48,
    .edgeSharpHpValLowThres = 900,
    .edgeSharpOffsetGradient = 24, 
    .edgeSharpHpValUpLimit = 32,
}; 
static CSL_IpipeEdgeEnhanceConfig* ee[] = {
    &ee_0,
    &ee_1,
};

static CSL_IpipeNfConfig nf2_0 = {
    .enable = 1,
    .spreadVal = 3,
    .lutAddrShift = 2,
    .greenSampleMethod = 0,
    .lscGainEnable = 0,
    .edgeDetectThresMin = 0,
    .edgeDetectThresMax = 2047,
    .lutThresTable = {15,25,35,45, 55,60,65,70},
    .lutIntensityTable = {20,20,18,18, 16,16,16,16},
};
static CSL_IpipeNfConfig nf2_1 = {
    .enable = 1,
    .spreadVal = 3,
    .lutAddrShift = 2,
    .greenSampleMethod = 0,
    .lscGainEnable = 0,
    .edgeDetectThresMin = 0,
    .edgeDetectThresMax = 2047,
    .lutThresTable = {20,20,20,20, 20,20,20,20},
    .lutIntensityTable = {16,16,16,16, 16,16,16,16},
}; 
static CSL_IpipeNfConfig* nf2[] = {
    &nf2_0,
    &nf2_1,
};


static void ISP_config(int i)
{
    DRV_ipipeSetEdgeEnhance(ee[i]);
    DRV_ipipeSetNf2(nf2[i]);
}

static int getSetIndex(int eTime, int aGain, int dGain, int cTemp)
{
    if (aGain >= 8000)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

int TI_2A_ISP_control(int eTime, int aGain, int dGain, int cTemp)
{
    static int steaySet = -1;
    static int tempSet = -1, tempCnt = 0;
    int tempCountLimit = 10;

    int i = getSetIndex(eTime, aGain, dGain, cTemp);
    int j = 0;

    if (steaySet == i) 
    {
        j = switch_rgb2rgb_matrixes(cTemp, rgb_matrixes[i], 0);
        tempSet = -1;
        tempCnt = 0;
    }
    else if (tempSet == i && tempCnt >= tempCountLimit) 
    {
        steaySet = i; 
        tempSet = -1;
        tempCnt = 0;
        j = switch_rgb2rgb_matrixes(cTemp, rgb_matrixes[i], 1);
        ISP_config(i);
    }
    else if (tempSet == i)
    {
        tempCnt++;
    }
    else
    {
        tempSet = i;
        tempCnt = 0;
    }
    return 100 * steaySet + j;
}



