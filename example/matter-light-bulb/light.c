/*
 * Light control — LEDs 1-3 as a dimmable "light bulb" via PWM.
 * LED0 is reserved for Matter status indication.
 * Button 1 (sw0) toggles on/off via callback from Board library.
 */

#include "light.h"

#include <zephyr/drivers/pwm.h>
#include <zephyr/shell/shell.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(light, LOG_LEVEL_INF);

static const struct pwm_dt_spec pwm_leds[] = {
	PWM_DT_SPEC_GET(DT_NODELABEL(pwm_led1)),
	PWM_DT_SPEC_GET(DT_NODELABEL(pwm_led2)),
	PWM_DT_SPEC_GET(DT_NODELABEL(pwm_led3)),
};

#define NUM_LEDS ARRAY_SIZE(pwm_leds)

static bool light_on;
static uint8_t light_level = 254; /* default full brightness */

static void apply_pwm(void)
{
	for (int i = 0; i < NUM_LEDS; i++) {
		uint32_t pulse;

		if (!light_on || light_level == 0) {
			pulse = 0;
		} else {
			/* Map level 1–254 to pulse width (use uint64 to avoid overflow) */
			pulse = (uint32_t)(((uint64_t)pwm_leds[i].period * light_level) / 254);
		}
		pwm_set_pulse_dt(&pwm_leds[i], pulse);
	}
}

/* --- Public API --- */

void light_set(bool on)
{
	light_on = on;
	apply_pwm();
	LOG_INF("Light %s (level %d)", on ? "ON" : "OFF", light_level);
}

bool light_get(void)
{
	return light_on;
}

void light_set_level(uint8_t level)
{
	light_level = level;
	if (level > 0) {
		light_on = true;
	}
	apply_pwm();
	LOG_INF("Light level %d", level);
}

uint8_t light_get_level(void)
{
	return light_level;
}

void light_toggle(void)
{
	light_set(!light_on);
}

void light_init(void)
{
	for (int i = 0; i < NUM_LEDS; i++) {
		if (!pwm_is_ready_dt(&pwm_leds[i])) {
			LOG_ERR("PWM LED %d not ready", i);
		}
	}
	LOG_INF("Light initialized (%d PWM LEDs)", NUM_LEDS);
}

/* --- Shell commands --- */

static int cmd_light_on(const struct shell *sh, size_t argc, char **argv)
{
	light_set(true);
	return 0;
}

static int cmd_light_off(const struct shell *sh, size_t argc, char **argv)
{
	light_set(false);
	return 0;
}

static int cmd_light_toggle(const struct shell *sh, size_t argc, char **argv)
{
	light_toggle();
	return 0;
}

static int cmd_light_level(const struct shell *sh, size_t argc, char **argv)
{
	if (argc < 2) {
		shell_print(sh, "Level: %d", light_get_level());
		return 0;
	}
	uint8_t level = (uint8_t)strtoul(argv[1], NULL, 10);

	light_set_level(level);
	return 0;
}

static int cmd_light_status(const struct shell *sh, size_t argc, char **argv)
{
	shell_print(sh, "Light is %s, level %d",
		    light_get() ? "ON" : "OFF", light_get_level());
	return 0;
}

SHELL_STATIC_SUBCMD_SET_CREATE(light_sub,
	SHELL_CMD(on, NULL, "Turn light on", cmd_light_on),
	SHELL_CMD(off, NULL, "Turn light off", cmd_light_off),
	SHELL_CMD(toggle, NULL, "Toggle light", cmd_light_toggle),
	SHELL_CMD_ARG(level, NULL, "Set/get brightness (0-254)",
		      cmd_light_level, 1, 1),
	SHELL_CMD(status, NULL, "Show light state", cmd_light_status),
	SHELL_SUBCMD_SET_END
);

SHELL_CMD_REGISTER(light, &light_sub, "Light control", NULL);
