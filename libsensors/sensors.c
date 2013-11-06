/*
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <hardware/sensors.h>

#include "nusensors.h"

/*****************************************************************************/

/*
 * The SENSORS Module
 */

static const struct sensor_t sSensorList[] = {
        {
		.name		= "BMA250 3-axis Accelerometer",
		.vendor		= "Bosch Sensortec GmbH",
          	.version	= 1,
		.handle		= SENSORS_HANDLE_BASE+ID_A,
		.type		= SENSOR_TYPE_ACCELEROMETER,
		.maxRange	= (16.0f*GRAVITY_EARTH),
		.resolution	= (16.0f*GRAVITY_EARTH)/4096,
		.power		= 0.003f,
		.minDelay	= 0,
		.reserved	= { }
	},
        {
		.name		= "SensorTek 22x7 Ambient Light Sensor",
		.vendor		= "SensorTek",
		.version	= 1,
		.handle		= SENSORS_HANDLE_BASE+ID_B,
		.type		= SENSOR_TYPE_LIGHT,
		.maxRange	= 8192.0f,
		.resolution	= 1.0f,
		.power		= 0.5f,
		.minDelay	= 0,
		.reserved	= { }
	},
};

static int open_sensors(const struct hw_module_t* module, const char* name,
        struct hw_device_t** device);

static int sensors__get_sensors_list(struct sensors_module_t* module,
        struct sensor_t const** list)
{
    *list = sSensorList;
    return ARRAY_SIZE(sSensorList);
}

static struct hw_module_methods_t sensors_module_methods = {
    .open = open_sensors
};

struct sensors_module_t HAL_MODULE_INFO_SYM = {
    .common = {
        .tag = HARDWARE_MODULE_TAG,
        .version_major = 1,
        .version_minor = 0,
        .id = SENSORS_HARDWARE_MODULE_ID,
        .name = "Kindle Fire Sensors Module",
        .author = "Hashcode/Austen Dicken",
        .methods = &sensors_module_methods,
    },
    .get_sensors_list = sensors__get_sensors_list
};

/*****************************************************************************/

static int open_sensors(const struct hw_module_t* module, const char* name,
        struct hw_device_t** device)
{
    return init_nusensors(module, device);
}
