import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import binary_sensor
from esphome.const import CONF_TYPE
from .. import mcintosh_ma5300_ns, CONF_MCINTOSH_MA5300_ID, McIntoshMA5300

DEPENDENCIES = ["mcintosh_ma5300"]

McIntoshBinarySensor = mcintosh_ma5300_ns.class_(
    "McIntoshBinarySensor", binary_sensor.BinarySensor
)

TYPES = {
    "headphones": "set_headphones_sensor",
}

CONFIG_SCHEMA = binary_sensor.binary_sensor_schema(McIntoshBinarySensor).extend(
    {
        cv.GenerateID(CONF_MCINTOSH_MA5300_ID): cv.use_id(McIntoshMA5300),
        cv.Required(CONF_TYPE): cv.one_of(*TYPES, lower=True),
    }
)


async def to_code(config):
    hub = await cg.get_variable(config[CONF_MCINTOSH_MA5300_ID])
    var = await binary_sensor.new_binary_sensor(config)
    cg.add(getattr(hub, TYPES[config[CONF_TYPE]])(var))
