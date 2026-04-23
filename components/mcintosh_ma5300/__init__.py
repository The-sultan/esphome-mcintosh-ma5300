import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import uart
from esphome.const import CONF_ID

DEPENDENCIES = ["uart"]
AUTO_LOAD = ["text_sensor"]

mcintosh_ma5300_ns = cg.esphome_ns.namespace("mcintosh_ma5300")
McIntoshMA5300 = mcintosh_ma5300_ns.class_(
    "McIntoshMA5300", uart.UARTDevice, cg.Component
)

CONF_MCINTOSH_MA5300_ID = "mcintosh_ma5300_id"

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(McIntoshMA5300),
        }
    )
    .extend(uart.UART_DEVICE_SCHEMA)
    .extend(cv.COMPONENT_SCHEMA)
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)
