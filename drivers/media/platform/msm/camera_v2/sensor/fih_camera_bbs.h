
#ifndef _FIH_BBS_CAMERA_H_
#define _FIH_BBS_CAMERA_H_

/****************************************************************************
*                               Include File                                *
****************************************************************************/

/****************************************************************************
*                         Public Constant Definition                        *
****************************************************************************/

enum FIH_BBS_CAMERA_LOCATION
{
    FIH_BBS_CAMERA_LOCATION_MAIN,
    FIH_BBS_CAMERA_LOCATION_FRONT,
    FIH_BBS_CAMERA_LOCATION_SUB,
    FIH_BBS_CAMERA_LOCATION_MAX,
};

enum FIH_BBS_CAMERA_MODULE
{
    FIH_BBS_CAMERA_MODULE_IC,
    FIH_BBS_CAMERA_MODULE_ACTUATOR,
    FIH_BBS_CAMERA_MODULE_EEPROM,
    FIH_BBS_CAMERA_MODULE_OIS,
    FIH_BBS_CAMERA_MODULE_MAX,
};

#define FIH_BBSUEC_MAIN_CAM_ID 9
#define FIH_BBSUEC_FRONT_CAM_ID 44
#define FIH_BBSUEC_AUX_CAM_ID 98 //temp

#endif
