import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import switch
from esphome.const import CONF_TYPE
from .. import mcintosh_ma5300_ns, CONF_MCINTOSH_MA5300_ID, McIntoshMA5300

DEPENDENCIES = ["mcintosh_ma5300"]

McIntoshSwitch = mcintosh_ma5300_ns.class_("McIntoshSwitch", switch.Switch)

# (cmd_on, cmd_off, hub_setter)
TYPES = {
    "tone":          ("TTN 1", "TTN 0", "set_tone_switch"),
    "mono":          ("TMO 1", "TMO 0", "set_mono_switch"),
    "meter_lights":  ("TML 1", "TML 0", "set_meter_lights_switch"),
    "headphone_hxd": ("THH 1", "THH 0", "set_headphone_hxd_switch"),
}

CONFIG_SCHEMA = switch.switch_schema(McIntoshSwitch).extend(
    {
        cv.GenerateID(CONF_MCINTOSH_MA5300_ID): cv.use_id(McIntoshMA5300),
        cv.Required(CONF_TYPE): cv.one_of(*TYPES, lower=True),
    }
)


async def to_code(config):
    hub = await cg.get_variable(config[CONF_MCINTOSH_MA5300_ID])
    var = await switch.new_switch(config)
    cg.add(var.set_hub(hub))
    type_cfg = TYPES[config[CONF_TYPE]]
    cg.add(var.set_command_on(type_cfg[0]))
    cg.add(var.set_command_off(type_cfg[1]))
    cg.add(getattr(hub, type_cfg[2])(var))
