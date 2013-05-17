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

#include <fcntl.h>
#include <errno.h>
#include <math.h>
#include <poll.h>
#include <unistd.h>
#include <dirent.h>
#include <stdlib.h>
#include <sys/select.h>

#include <cutils/log.h>

#include "BMA250.h"

/*****************************************************************************/

BMA250Sensor::BMA250Sensor()
: SensorBase(DEVICE_NAME, "bma250"),
      mEnabled(0),
      mInputReader(32)
{
    mPendingEvent.version = sizeof(sensors_event_t);
    mPendingEvent.sensor = ID_A;
    mPendingEvent.type = SENSOR_TYPE_ACCELEROMETER;
    memset(mPendingEvent.data, 0, sizeof(mPendingEvent.data));
    mPendingEvent.acceleration.status = SENSOR_STATUS_ACCURACY_HIGH;
    mEnabled = isEnabled();
}

BMA250Sensor::~BMA250Sensor() {
}

int BMA250Sensor::enable(int32_t handle, int en)
{
    int err = 0;

    unsigned int newState = en ? 1 : 0;

    // don't set enable state if it's already valid
    if (mEnabled == newState) {
        return err;
    }

    int fd = open(BMA250_ENABLE_FILE, O_WRONLY);
    if(fd >= 0) {
        char buffer[20];
        int bytes = sprintf(buffer, "%d\n", newState);
        err = write(fd, buffer, bytes);
        err = err < 0 ? -errno : 0;
    } else {
        err = -errno;
    }

    ALOGE_IF(err < 0, "Error setting enable of bma250 accelerometer (%s)", strerror(-err));

    if (!err) {
        mEnabled = newState;
        setDelay(handle, 40000000); // 40ms by default for faster re-orienting
    }

    return err;
}

int BMA250Sensor::setDelay(int32_t handle, int64_t ns)
{
    int err = 0;

    if (mEnabled) {
        if (ns < 0)
            return -EINVAL;

        unsigned long delay = ns / 1000000;

        int fd = open(BMA250_DELAY_FILE, O_WRONLY);
        if(fd >= 0) {
            char buffer[20];
            int bytes = sprintf(buffer, "%lu\n", delay);
            err = write(fd, buffer, bytes);
            err = err < 0 ? -errno : 0;
        } else {
            err = -errno;
        }

        ALOGE_IF(err < 0, "Error setting delay of bma250 accelerometer (%s)", strerror(-err));
    }

    return err;
}

int BMA250Sensor::readEvents(sensors_event_t* data, int count)
{
    if (count < 1)
        return -EINVAL;

    ssize_t n = mInputReader.fill(data_fd);
    if (n < 0)
        return n;

    int numEventReceived = 0;
    input_event const* event;

    while (count && mInputReader.readEvent(&event)) {
        int type = event->type;
//      ALOGD("BMA250: event (type=%d, code=%d, value=%d)", type, event->code, event->value);
        if ((type == EV_ABS) || (type == EV_REL)) {
            processEvent(event->code, event->value);
        } else if (type == EV_SYN) {
            mPendingEvent.timestamp = timevalToNano(event->time);
            *data++ = mPendingEvent;
            count--;
            numEventReceived++;
        } else {
            ALOGE("BMA250: unknown event (type=%d, code=%d)",
                    type, event->code);
        }
        mInputReader.next();
    }

    return numEventReceived;
}

void BMA250Sensor::processEvent(int code, int value)
{
/*
		temp_x = acc->x;
		acc->x = -acc->y;
		acc->y = temp_x;
		acc->z = acc->z;
*/
    switch (code) {
        case EVENT_TYPE_ACCEL_X:
            mPendingEvent.acceleration.y = value * CONVERT_A_X;
            break;
        case EVENT_TYPE_ACCEL_Y:
            mPendingEvent.acceleration.x = -value * CONVERT_A_Y;
            break;
        case EVENT_TYPE_ACCEL_Z:
            mPendingEvent.acceleration.z = value * CONVERT_A_Z;
            break;
    }
}

int BMA250Sensor::isEnabled()
{
    int fd = open(BMA250_ENABLE_FILE, O_RDONLY);
    if (fd >= 0) {
        char buffer[20];
        int amt = read(fd, buffer, 20);
        close(fd);
        if(amt > 0) {
            return (buffer[0] == '1');
        } else {
            ALOGE("BMA250: isEnable failed to read (%s)", strerror(errno));
            return 0;
        }
    } else {
        ALOGE("BMA250: isEnabled failed to open %s", BMA250_ENABLE_FILE);
        return 0;
    }
}

