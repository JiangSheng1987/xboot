/*
 * drivers/led/led.c
 *
 * Copyright(c) 2007-2013 jianjun jiang <jerryjianjun@gmail.com>
 * official site: http://xboot.org
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <xboot.h>
#include <led/led.h>

static void led_suspend(struct device_t * dev)
{
	struct led_t * led;

	if(!dev || dev->type != DEVICE_TYPE_LED)
		return;

	led = (struct led_t *)(dev->driver);
	if(!led)
		return;

	if(led->suspend)
		led->suspend(led);
}

static void led_resume(struct device_t * dev)
{
	struct led_t * led;

	if(!dev || dev->type != DEVICE_TYPE_LED)
		return;

	led = (struct led_t *)(dev->driver);
	if(!led)
		return;

	if(led->resume)
		led->resume(led);
}

static ssize_t led_read_brightness(struct kobj_t * kobj, void * buf, size_t size)
{
	struct led_t * led = (struct led_t *)kobj->priv;
	int brightness;

	brightness = led_get_brightness(led);
	return sprintf(buf, "%d", brightness);
}

static ssize_t led_write_brightness(struct kobj_t * kobj, void * buf, size_t size)
{
	struct led_t * led = (struct led_t *)kobj->priv;
	int brightness = strtol(buf, NULL, 0);

	led_set_brightness(led, brightness);
	return size;
}

struct led_t * search_led(const char * name)
{
	struct device_t * dev;

	dev = search_device_with_type(name, DEVICE_TYPE_LED);
	if(!dev)
		return NULL;

	return (struct led_t *)dev->driver;
}

bool_t register_led(struct led_t * led)
{
	struct device_t * dev;

	if(!led || !led->name)
		return FALSE;

	dev = malloc(sizeof(struct device_t));
	if(!dev)
		return FALSE;

	dev->name = strdup(led->name);
	dev->type = DEVICE_TYPE_LED;
	dev->suspend = led_suspend;
	dev->resume = led_resume;
	dev->driver = led;
	dev->kobj = kobj_alloc_directory(dev->name);
	kobj_add_regular(dev->kobj, "brightness", led_read_brightness, led_write_brightness, led);

	if(!register_device(dev))
	{
		kobj_remove_self(dev->kobj);
		free(dev->name);
		free(dev);
		return FALSE;
	}

	if(led->init)
		(led->init)(led);

	return TRUE;
}

bool_t unregister_led(struct led_t * led)
{
	struct device_t * dev;
	struct led_t * driver;

	if(!led || !led->name)
		return FALSE;

	dev = search_device_with_type(led->name, DEVICE_TYPE_LED);
	if(!dev)
		return FALSE;

	driver = (struct led_t *)(dev->driver);
	if(driver && driver->exit)
		(driver->exit)(led);

	if(!unregister_device(dev))
		return FALSE;

	kobj_remove_self(dev->kobj);
	free(dev->name);
	free(dev);
	return TRUE;
}

void led_set_brightness(struct led_t * led, int brightness)
{
	if(led && led->set)
		led->set(led, brightness);
}

int led_get_brightness(struct led_t * led)
{
	if(led && led->get)
		return led->get(led);
	return 0;
}
