/*
 * Copyright (c) 2011-2013, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/kernel.h>
#include <linux/debugfs.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/string.h>
#include <linux/sysfs.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/delay.h>
#include "msm_sensor.h"
#include "msm_sd.h"
#include "camera.h"
#include "msm_cci.h"
#include "msm_camera_dt_util.h"
#include "fih_camera_bbs.h"

#undef SERR
#define SERR(fmt, args...)  pr_err("%s:%d " fmt"\n", __func__, __LINE__, ##args)

#undef SLOW
#undef SINFO
#define SLOW(fmt, args...)  pr_debug("%s:%d " fmt"\n", __func__, __LINE__, ##args)
#define SINFO(fmt, args...)  pr_info("%s:%d " fmt"\n", __func__, __LINE__, ##args)

#define FIH_MAX_NAME_LEN MAX_SENSOR_NAME

const char *FIH_BBS_CAMERA_MODULE_NAME[FIH_BBS_CAMERA_MODULE_MAX]=
{
    "sensor",
    "actuator",
    "eeprom",
    "ois",
};

struct fih_camera_sub_module_info{
    int subdev_id;
    char name[FIH_MAX_NAME_LEN];
    enum cci_i2c_master_t cci_master;
    unsigned short sid;
    int save_done;
};

struct fih_camera_sensor_info {
    enum camb_position_t position;
    struct fih_camera_sub_module_info module_info[FIH_BBS_CAMERA_MODULE_MAX];
} *fih_cam_info;

void fih_camera_sensor_info_init(struct fih_camera_sensor_info* info)
{
    int i = 0, id = 0;
    for (id=0;id<FIH_BBS_CAMERA_LOCATION_MAX;id++)
    {
        for (i=0;i<FIH_BBS_CAMERA_MODULE_MAX;i++)
        {
           info[id].module_info[i].subdev_id=-1;
           info[id].module_info[i].cci_master=-1;
           info[id].module_info[i].sid=-1;
           info[id].module_info[i].save_done=0;
        }
    }
}

char* fih_camera_bbs_get_name_by_cci(int master,int sid)
{
    int id=0,i=0;

    if(!fih_cam_info)
        return NULL;

    for (id=0;id<FIH_BBS_CAMERA_LOCATION_MAX;id++)
    {
        for (i=0;i<FIH_BBS_CAMERA_MODULE_MAX;i++)
        {
            if((fih_cam_info[id].module_info[i].cci_master == master) &&
                    (fih_cam_info[id].module_info[i].sid)==sid)
            {
                goto GetName;
            }
        }
    }
    if(id==FIH_BBS_CAMERA_LOCATION_MAX)
    {
        SERR("cannot match cci master and slave address.");
        return NULL;
    }
GetName:
    return fih_cam_info[id].module_info[i].name;
}
EXPORT_SYMBOL(fih_camera_bbs_get_name_by_cci);

//save actuator info by subid
int fih_camera_bbs_set(int subid,int master,unsigned short sid,int module)
{
    int i=0;

    if(module>=FIH_BBS_CAMERA_MODULE_MAX)
        SERR("error! module %d not exist",module);

    SLOW(" subid=%d,master=%d,sid=0x%x,module=%d",subid,master,sid,module);

    for (i=0;i<FIH_BBS_CAMERA_LOCATION_MAX;i++)
    {
        if(fih_cam_info[i].module_info[module].subdev_id == subid)
            break;
    }
    if(i==FIH_BBS_CAMERA_LOCATION_MAX)
    {
        SERR("cannot match subdev_id=%d",subid);
        return -1;
    }
    if(fih_cam_info[i].module_info[module].save_done)
    {
        SLOW("already save done,bypass");
        return 0;
    }

    fih_cam_info[i].module_info[module].cci_master = master;
    fih_cam_info[i].module_info[module].sid = sid;
    fih_cam_info[i].module_info[module].save_done=1;
    SINFO("set %s parameter for camera%d,master=%d,sid=0x%x",FIH_BBS_CAMERA_MODULE_NAME[module],fih_cam_info[i].position,master,sid);

    return 0;
}
EXPORT_SYMBOL(fih_camera_bbs_set);

int fih_camera_bbs_init(struct msm_sensor_ctrl_t * ctrl)
{
    struct msm_camera_sensor_slave_info *slave_info=NULL;
    int position=-1,id=-1,temp=0;

    if (!ctrl->sensordata->cam_slave_info)
    {
        SERR("cam_slave_info is null pointer");
        return -1;
    }

    slave_info = ctrl->sensordata->cam_slave_info;
    position = slave_info->sensor_init_params.position;
    SLOW("position %d", position);

    switch(position){
        case BACK_CAMERA_B:id = FIH_BBS_CAMERA_LOCATION_MAIN;break;
        case FRONT_CAMERA_B:id = FIH_BBS_CAMERA_LOCATION_FRONT;break;
        case AUX_CAMERA_B:id = FIH_BBS_CAMERA_LOCATION_SUB;break;
        default:SERR("invalid position %d", position);break;
    }

    fih_cam_info[id].position = position;
    //set sensor info
    fih_cam_info[id].module_info[FIH_BBS_CAMERA_MODULE_IC].subdev_id=(int)ctrl->sensordata->sensor_info->subdev_id[SUB_MODULE_SENSOR];
    snprintf(fih_cam_info[id].module_info[FIH_BBS_CAMERA_MODULE_IC].name,FIH_MAX_NAME_LEN,"%s",slave_info->sensor_name);
    fih_cam_info[id].module_info[FIH_BBS_CAMERA_MODULE_IC].cci_master = ctrl->cci_i2c_master;
    fih_cam_info[id].module_info[FIH_BBS_CAMERA_MODULE_IC].sid = slave_info->slave_addr >> 1;
    fih_cam_info[id].module_info[FIH_BBS_CAMERA_MODULE_IC].save_done = 1;

    //get actuator subdev id
    if (ctrl->sensordata->sensor_info->subdev_id[SUB_MODULE_ACTUATOR]!=-1)
    {
        temp=FIH_BBS_CAMERA_MODULE_ACTUATOR;
        snprintf(fih_cam_info[id].module_info[temp].name,FIH_MAX_NAME_LEN,"%s",slave_info->actuator_name);
        fih_cam_info[id].module_info[temp].subdev_id=(int)ctrl->sensordata->sensor_info->subdev_id[SUB_MODULE_ACTUATOR];
        SINFO(" [cam%d][%s]subdev_id=%d",id,FIH_BBS_CAMERA_MODULE_NAME[temp],fih_cam_info[id].module_info[temp].subdev_id);
    }else
        SLOW("no %s module in cam%d",FIH_BBS_CAMERA_MODULE_NAME[temp],id);

    //get eeprom subdev id
    if (ctrl->sensordata->sensor_info->subdev_id[SUB_MODULE_EEPROM]!=-1)
    {
        temp=FIH_BBS_CAMERA_MODULE_EEPROM;
        snprintf(fih_cam_info[id].module_info[temp].name,FIH_MAX_NAME_LEN,"%s",slave_info->eeprom_name);
        fih_cam_info[id].module_info[temp].subdev_id=(int)ctrl->sensordata->sensor_info->subdev_id[SUB_MODULE_EEPROM];
        SINFO(" [cam%d][%s]subdev_id=%d",id,FIH_BBS_CAMERA_MODULE_NAME[temp],fih_cam_info[id].module_info[temp].subdev_id);
    }else
        SLOW("no %s module in cam%d",FIH_BBS_CAMERA_MODULE_NAME[temp],id);

    //get ois subdev id
    if (ctrl->sensordata->sensor_info->subdev_id[SUB_MODULE_OIS]!=-1)
    {
        temp=FIH_BBS_CAMERA_MODULE_OIS;
        snprintf(fih_cam_info[id].module_info[temp].name,FIH_MAX_NAME_LEN,"%s",slave_info->ois_name);
        fih_cam_info[id].module_info[temp].subdev_id=(int)ctrl->sensordata->sensor_info->subdev_id[SUB_MODULE_OIS];
        SINFO(" [cam%d][%s]subdev_id=%d",id,FIH_BBS_CAMERA_MODULE_NAME[temp],fih_cam_info[id].module_info[temp].subdev_id);
    }else
        SLOW("no %s module in cam%d",FIH_BBS_CAMERA_MODULE_NAME[temp],id);

    return 0;
}
EXPORT_SYMBOL(fih_camera_bbs_init);

static int __init fih_camera_dev_probe(void)
{
    int fsize = 0;

    fsize = sizeof(struct fih_camera_sensor_info)*FIH_BBS_CAMERA_LOCATION_MAX;
    fih_cam_info = kzalloc(fsize, GFP_KERNEL);
    if (!fih_cam_info) {
        SERR("failed no memory\n");
        return -ENOMEM;
    }

    fih_camera_sensor_info_init(fih_cam_info);

    SINFO("fih_camera_dev_probe alloc success (%d)",fsize);
    return (0);
}

module_init(fih_camera_dev_probe);

