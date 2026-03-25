import esphome.codegen as cg
from esphome.components import output
import esphome.config_validation as cv
from esphome.const import CONF_CHANNEL, CONF_ID

from .. import TLC59116Output, tlc59116_ns

DEPENDENCIES = ["tlc59116"]

TLC59116Channel = tlc59116_ns.class_(
    "TLC59116Channel", output.FloatOutput, cg.Component
)

CONF_TLC59116_ID = "tlc59116_id"

CONFIG_SCHEMA = output.FLOAT_OUTPUT_SCHEMA.extend(
    {
        cv.GenerateID(): cv.declare_id(TLC59116Channel),
        cv.GenerateID(CONF_TLC59116_ID): cv.use_id(TLC59116Output),
        cv.Required(CONF_CHANNEL): cv.int_range(min=0, max=15),
    }
).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await output.register_output(var, config)
    await cg.register_component(var, config)

    parent = await cg.get_variable(config[CONF_TLC59116_ID])
    cg.add(var.set_parent(parent))
    cg.add(var.set_channel(config[CONF_CHANNEL]))
