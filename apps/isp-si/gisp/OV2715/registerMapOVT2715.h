// ============================================================================
// DxO Labs proprietary and confidential information
// Copyright (C) DxO Labs 1999-2014 - (All rights reserved)
// ============================================================================


/*********************************************************************************/
/* sensor  OV8820 register mapping                                               */
/*********************************************************************************/


#define Offset_mode_select                                    0x3008
#define Offset_software_reset                                 0x0103
#define Offset_group_access                                   0x3208
#define Offset_group_switch                                   0x320b
#define Offset_timing_tc_reg20                                0x3820		//p no use in OV2715
#define Offset_timing_tc_reg21                                0x3821		//p no use in OV2715
#define Offset_psram_ctrl0                                    0x3f00
#define Offset_ExposureMSB                                    0x3500
#define Offset_Exposure                                       0x3501
#define Offset_AG                                             0x350a 
#define Offset_DG_G                                           0x3402
#define Offset_DG_R                                           0x3400
#define Offset_DG_B                                           0x3404
#define Offset_Af                                             0x3618
#define Offset_frame_length_lines                             0x380e 
#define Offset_line_length_pck                                0x380c
#define Offset_x_addr_start                                   0x3800
#define Offset_y_addr_start                                   0x3802
#define Offset_x_addr_end                                     0x3804
#define Offset_y_addr_end                                     0x3806
#define Offset_x_output_size                                  0x3808
#define Offset_y_output_size                                  0x380a
#define Offset_x_odd_even_inc                                 0x3814
#define Offset_y_odd_even_inc                                 0x3815
#define Offset_offset_h                                       0x3810
#define Offset_offset_v                                       0x3812


#define Size_Offset_mode_select                              1
#define Size_Offset_software_reset                           1
#define Size_Offset_group_access                             1
#define Size_Offset_group_switch                             1
#define Size_Offset_timing_tc_reg20                          1
#define Size_Offset_timing_tc_reg21                          1
#define Size_Offset_psram_ctrl0                              1
#define Size_Offset_ExposureMSB                              1
#define Size_Offset_Exposure                                 2
#define Size_Offset_AG                                       2
#define Size_Offset_DG_G                                     2
#define Size_Offset_DG_R                                     2
#define Size_Offset_DG_B                                     2
#define Size_Offset_Af                                       2     
#define Size_Offset_frame_length_lines                       2
#define Size_Offset_line_length_pck                          2
#define Size_Offset_x_addr_start                             2
#define Size_Offset_y_addr_start                             2
#define Size_Offset_x_addr_end                               2
#define Size_Offset_y_addr_end                               2
#define Size_Offset_x_output_size                            2
#define Size_Offset_y_output_size                            2
#define Size_Offset_x_odd_even_inc                           1
#define Size_Offset_y_odd_even_inc                           1
#define Size_Offset_offset_h                                 2
#define Size_Offset_offset_v                                 2

/*********************************************************************************/
/* special register values                                                       */
/*********************************************************************************/
#define HORIZONTAL_OFFSET                   0x04
#define VERTICAL_OFFSET                     0x02

#define DIGITAL_GAIN_MIN                    0x0100
#define DIGITAL_GAIN_MAX                    0x01F8
#define ANALOG_CODE_GAIN_MAX                0x00F0

#define MIN_FRAME_LENGTH_LINE               28 			//p
#define MAX_FRAME_LENGTH_LINE               0xFFFE
#define MIN_LINE_LENGTH_PCK                 20			//p 0714
#define MAX_LINE_LENGTH_PCK                 0x3FFF

#define X_ADDR_MIN                          0x0000
#define Y_ADDR_MIN                          0x0000
#define X_ADDR_MAX                          0x0788
#define Y_ADDR_MAX                          0x0440
#define X_OUTPUT_SIZE_MIN                   0x0100
#define Y_OUTPUT_SIZE_MIN                   0x0004
//#define X_OUTPUT_SIZE_MAX                   640//0x0780	//p
//#define Y_OUTPUT_SIZE_MAX                   480//0x0438	//p
#define X_OUTPUT_SIZE_MAX                   0x0780	//p
#define Y_OUTPUT_SIZE_MAX                   0x0438	//p
#define MAX_ODD_INC                         0x0005
#define MIN_ODD_INC                         0x0001
#define MAX_EVEN_INC                        0x0005
#define MIN_EVEN_INC                        0x0001

#define MIN_LINE_BLANKING_PCK               0x0100	//p
#define MIN_FRAME_BLANKING_LINE                 32	//p
#define MIN_INTEGRATION_TIME                (1<<4)

#define MIN_AF_POSITION                        0
#define MAX_AF_POSITION                        255


#define INTEGRATION_TIME_REG                      2
#define cache_Offset_ExposureMSB                  0
#define cache_Offset_Exposure                     1

#define ANALOG_GAIN_REG                           1
#define cache_Offset_AG                           2

#define DIGITAL_GAIN_REG                          3
#define cache_Offset_DG_G                         3
#define cache_Offset_DG_R                         4
#define cache_Offset_DG_B                         5

#define CROP_REG                                  4
#define cache_Offset_x_addr_start                 6
#define cache_Offset_y_addr_start                 7
#define cache_Offset_x_addr_end                   8
#define cache_Offset_y_addr_end                   9

#define GEOMETRY_REG                              6
#define cache_Offset_x_output_size               10
#define cache_Offset_y_output_size               11
#define cache_Offset_x_odd_even_inc              12
#define cache_Offset_y_odd_even_inc              13
#define cache_Offset_offset_h                    14
#define cache_Offset_offset_v                    15

#define FRAME_RATE_REG                            2
#define cache_Offset_line_length_pck             16
#define cache_Offset_frame_length_lines          17

#define MODE_SELECT_REG                           1
#define cache_Offset_mode_select                 18

#define TIMING_REG                                3
#define cache_Offset_timing_tc_reg20             19
#define cache_Offset_timing_tc_reg21             20
#define cache_Offset_psram_ctrl0                 21

#define AF_REG                                    1
#define cache_Offset_Af                          22

/*****************************************************************************/
/*****************************************************************************/
/* END OF FILE                                                               */
/*****************************************************************************/
/*****************************************************************************/

