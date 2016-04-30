/*
 * Copyright (C) 2016 http://www.brobwind.com
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

//#define LOG_NDEBUG 0

#include <cutils/log.h>

#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <stdlib.h>

#include <hardware/lights.h>

/******************************************************************************/
#define SET_LIGHT(color)    (color & 0xffffff ? 0x28 : 0)

static pthread_once_t g_init = PTHREAD_ONCE_INIT;
static pthread_mutex_t g_lock = PTHREAD_MUTEX_INITIALIZER;

char const* const LED1_LED_FILE
        = "/sys/class/leds/verbose/brightness";

char const* const LED2_LED_FILE
        = "/sys/class/leds/debug/brightness";

char const* const LED3_LED_FILE
        = "/sys/class/leds/info/brightness";

char const* const BLUETOOTH_LED_FILE
        = "/sys/class/leds/warn/brightness";

char const* const WIFI_LED_FILE
        = "/sys/class/leds/led0/brightness";

/**
 * device methods
 */

void init_globals(void)
{
    // init the mutex
    pthread_mutex_init(&g_lock, NULL);
}

static int write_int(char const* path, int value)
{
#if 0
	ALOGI(" -> write: %s value=0x%08x(%d)", path, value, value);
	return 0;
#else
    FILE *fd;

    fd = fopen(path, "w+");
    if (fd) {
        int bytes = fprintf(fd, "%d", value);
        fclose(fd);
        return (bytes < 0 ? bytes : 0);
    } else {
        ALOGE("write_int failed to open %s\n", path);
        return -errno;
    }
#endif
}

static int set_light_notifications(struct light_device_t* dev,
        struct light_state_t const* state)
{
    int err = 0;
    if(!dev) {
        return -1;
    }
    pthread_mutex_lock(&g_lock);
    err = write_int(LED1_LED_FILE, SET_LIGHT(state->color));
    pthread_mutex_unlock(&g_lock);
    return err;
}

static int set_light_attention(struct light_device_t* dev,
        struct light_state_t const* state)
{
    int err = 0;
    if(!dev) {
        return -1;
    }
    pthread_mutex_lock(&g_lock);
    err = write_int(LED2_LED_FILE, SET_LIGHT(state->color));
    pthread_mutex_unlock(&g_lock);
    return err;
}

static int set_light_buttons(struct light_device_t* dev,
        struct light_state_t const* state)
{
    int err = 0;
    if(!dev) {
        return -1;
    }
    pthread_mutex_lock(&g_lock);
    err = write_int(LED3_LED_FILE, SET_LIGHT(state->color));
    pthread_mutex_unlock(&g_lock);
    return err;
}

static int set_light_bluetooth(struct light_device_t* dev,
        struct light_state_t const* state)
{
    int err = 0;
    if(!dev) {
        return -1;
    }
    pthread_mutex_lock(&g_lock);
    err = write_int(BLUETOOTH_LED_FILE, SET_LIGHT(state->color));
    pthread_mutex_unlock(&g_lock);
    return err;
}

static int set_light_wifi(struct light_device_t* dev,
        struct light_state_t const* state)
{
    int err = 0;
    if(!dev) {
        return -1;
    }
    pthread_mutex_lock(&g_lock);
    err = write_int(WIFI_LED_FILE, SET_LIGHT(state->color));
    pthread_mutex_unlock(&g_lock);
    return err;
}

/** Close the lights device */
static int close_lights(struct light_device_t *dev)
{
    if (dev) {
        free(dev);
    }
    return 0;
}


/******************************************************************************/

/**
 * module methods
 */

/** Open a new instance of a lights device using name */
static int open_lights(const struct hw_module_t* module, char const* name,
        struct hw_device_t** device)
{
    int (*set_light)(struct light_device_t* dev,
            struct light_state_t const* state);

    if (0 == strcmp(LIGHT_ID_NOTIFICATIONS, name))
        set_light = set_light_notifications;
    else if (0 == strcmp(LIGHT_ID_ATTENTION, name))
        set_light = set_light_attention;
    else if (0 == strcmp(LIGHT_ID_BUTTONS, name))
        set_light = set_light_buttons;
    else if (0 == strcmp(LIGHT_ID_BLUETOOTH, name))
        set_light = set_light_bluetooth;
    else if (0 == strcmp(LIGHT_ID_WIFI, name))
        set_light = set_light_wifi;
    else
        return -EINVAL;

    pthread_once(&g_init, init_globals);

    struct light_device_t *dev = calloc( 1, sizeof(struct light_device_t));

    if(!dev)
        return -ENOMEM;

    dev->common.tag = HARDWARE_DEVICE_TAG;
    dev->common.version = 0;
    dev->common.module = (struct hw_module_t*)module;
    dev->common.close = (int (*)(struct hw_device_t*))close_lights;
    dev->set_light = set_light;

    *device = (struct hw_device_t*)dev;
    return 0;
}

static struct hw_module_methods_t lights_module_methods = {
    .open =  open_lights,
};

/*
 * The lights Module
 */
struct hw_module_t HAL_MODULE_INFO_SYM = {
    .tag = HARDWARE_MODULE_TAG,
    .version_major = 1,
    .version_minor = 0,
    .id = LIGHTS_HARDWARE_MODULE_ID,
    .name = "Lights Module / RPi 3B module",
    .author = "hzak",
    .methods = &lights_module_methods,
};
