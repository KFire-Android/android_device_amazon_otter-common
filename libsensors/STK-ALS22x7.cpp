/*
 * Copyright (C) 2008 The Android Open Source Project
 * Copyright (C) 2011 Sorin P. <sorin@hypermagik.com>
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
#include <sys/select.h>

#include <cutils/log.h>

#include "STK-ALS22x7.h"

#define TAG "STK-ALS-22x7"

STK_ALS22x7Sensor::STK_ALS22x7Sensor()
: SensorBase(DEVICE_NAME, "lightsensor-level"),
      mEnabled(0),
      mInputReader(32)
{
    mPendingEvent.version = sizeof(sensors_event_t);
    mPendingEvent.sensor = ID_B;
    mPendingEvent.type = SENSOR_TYPE_LIGHT;
    memset(mPendingEvent.data, 0, sizeof(mPendingEvent.data));
    mEnabled = isEnabled();
}

STK_ALS22x7Sensor::~STK_ALS22x7Sensor() {
}

int STK_ALS22x7Sensor::enable(int32_t handle, int en)
{
    int err = 0;

    int newState = en ? 1 : 0;

    // ALOGD(TAG ": Setting enable: %d", en);

    // don't set enable state if it's already valid
    if (mEnabled == newState) {
        return err;
    }

    int fd = open(STK_ALS22X7_ENABLE_FILE, O_WRONLY);
    if(fd >= 0) {
        char buffer[20];
        int bytes = sprintf(buffer, "%d\n", newState);
        err = write(fd, buffer, bytes);
        err = err < 0 ? -errno : 0;
        close(fd);
    } else {
        err = -errno;
    }

    ALOGE_IF(err < 0, TAG ": Error setting enable of stk-als-22x7 light sensor (%s)", strerror(-err));

    return err;
}


int STK_ALS22x7Sensor::readEvents(sensors_event_t* data, int count)
{
    // ALOGD(TAG ": readEvents: count == %d", count);

    if (count < 1) {
        return -EINVAL;
    }

    ssize_t n = mInputReader.fill(data_fd);
    if (n < 0) {
        return n;
    }

    int numEventReceived = 0;
    input_event const* event;

    while (count && mInputReader.readEvent(&event)) {
        // ALOGD(TAG ": event (type=0x%x, code=0x%x, value=0x%x)", event->type, event->code, event->value);
        switch (event->type) {
            case EV_ABS:
                processEvent(event->code, event->value);
                break;
            case EV_SYN:
                mPendingEvent.timestamp = timevalToNano(event->time);
                *data++ = mPendingEvent;
                count--;
                numEventReceived++;
                break;
            default:
                ALOGW(TAG ": unknown event (type=0x%x, code=0x%x, value=0x%x)",
                        event->type, event->code, event->value);
                break;
        }
        mInputReader.next();
    }

    return numEventReceived;
}

void STK_ALS22x7Sensor::processEvent(int code, int value)
{
    switch (code) {
        case ABS_MISC:
            if (value > 10000)
                value = 0;
            mPendingEvent.light = value;
            break;
        default:
            ALOGW(TAG ": unknown code (code=0x%x, value=0x%x)", code, value);
            break;
    }
}

int STK_ALS22x7Sensor::isEnabled()
{
    int ret = 0;
    int fd = open(STK_ALS22X7_ENABLE_FILE, O_RDONLY);
    if (fd >= 0) {
        char buffer[20];
        int amt = read(fd, buffer, 20);
        close(fd);
        if(amt > 0) {
            ret = (buffer[0] == '1');
        } else {
            ALOGE(TAG ": isEnable failed to read (%s)", strerror(errno));
        }
    } else {
        ALOGE(TAG ": isEnabled failed to open %s", STK_ALS22X7_ENABLE_FILE);
    }
    // ALOGD(TAG ": isEnabled == %d", ret);
    return ret;
}
