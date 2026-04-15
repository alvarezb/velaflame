# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Brian Alvarez

from esphome import automation, pins
from esphome.automation import maybe_simple_id
import esphome.codegen as cg
from esphome.components import fan, output
import esphome.config_validation as cv
from esphome.const import (
    CONF_ID,
    CONF_PIN_A,
    CONF_PIN_B,
    CONF_SPEED_COUNT,
)

from .. import drv8210p_ns

DEPENDENCIES = ["output"]

DRV8210PFan = drv8210p_ns.class_("DRV8210PFan", cg.Component, fan.Fan)
BrakeAction = drv8210p_ns.class_("BrakeAction", automation.Action)

CONF_NSLEEP_PIN = "nsleep_pin"

CONFIG_SCHEMA = (
    fan.fan_schema(DRV8210PFan)
    .extend(
        {
            cv.Required(CONF_PIN_A): cv.use_id(output.FloatOutput),
            cv.Required(CONF_PIN_B): cv.use_id(output.FloatOutput),
            cv.Optional(CONF_NSLEEP_PIN): pins.gpio_output_pin_schema,
            cv.Optional(CONF_SPEED_COUNT, default=100): cv.int_range(min=1),
        }
    )
    .extend(cv.COMPONENT_SCHEMA)
)


@automation.register_action(
    "drv8210p.brake",
    BrakeAction,
    maybe_simple_id({cv.GenerateID(): cv.use_id(DRV8210PFan)}),
)
async def drv8210p_brake_to_code(config, action_id, template_arg, args):
    parent = await cg.get_variable(config[CONF_ID])
    return cg.new_Pvariable(action_id, template_arg, parent)


async def to_code(config):
    var = await fan.new_fan(config, config[CONF_SPEED_COUNT])
    await cg.register_component(var, config)

    pin_a = await cg.get_variable(config[CONF_PIN_A])
    cg.add(var.set_pin_a(pin_a))

    pin_b = await cg.get_variable(config[CONF_PIN_B])
    cg.add(var.set_pin_b(pin_b))

    if CONF_NSLEEP_PIN in config:
        nsleep = await cg.gpio_pin_expression(config[CONF_NSLEEP_PIN])
        cg.add(var.set_nsleep_pin(nsleep))
