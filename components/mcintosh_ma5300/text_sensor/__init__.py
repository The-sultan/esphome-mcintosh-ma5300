import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import text_sensor
from esphome.const import CONF_TYPE
from .. import mcintosh_ma5300_ns, CONF_MCINTOSH_MA5300_ID, McIntoshMA5300

DEPENDENCIES = ["mcintosh_ma5300"]

# Maps the user-facing 'type' value to the hub setter method name.
TYPES = {
    "serial_number": "set_serial_number_sensor",
    "firmware_version": "set_firmware_version_sensor",
    "da_version": "set_da_version_sensor",
    "last_error": "set_last_error_sensor",
}

CONFIG_SCHEMA = text_sensor.text_sensor_schema().extend(
    {
        cv.GenerateID(CONF_MCINTOSH_MA5300_ID): cv.use_id(McIntoshMA5300),
        cv.Required(CONF_TYPE): cv.one_of(*TYPES, lower=True),
    }
)


async def to_code(config):
    hub = await cg.get_variable(config[CONF_MCINTOSH_MA5300_ID])
    var = await text_sensor.new_text_sensor(config)
    setter = TYPES[config[CONF_TYPE]]
    cg.add(getattr(hub, setter)(var))
