import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import select
from esphome.const import CONF_TYPE
from .. import mcintosh_ma5300_ns, CONF_MCINTOSH_MA5300_ID, McIntoshMA5300

DEPENDENCIES = ["mcintosh_ma5300"]

McIntoshSelect = mcintosh_ma5300_ns.class_("McIntoshSelect", select.Select)

INPUT_MAP = {
    "BAL":      1,
    "UNBAL 1":  2,
    "UNBAL 2":  3,
    "UNBAL 3":  4,
    "UNBAL 4":  5,
    "MM PHONO": 6,
    "COAX 1":   7,
    "COAX 2":   8,
    "OPTI 1":   9,
    "OPTI 2":   10,
    "USB":      11,
    "MCT":      12,
    "HDMI":     13,
}

BRIGHTNESS_MAP = {
    "25%":  1,
    "50%":  2,
    "75%":  3,
    "100%": 4,
}

# (cmd_prefix, option_map, hub_setter)
TYPES = {
    "input":              ("INP", INPUT_MAP,      "set_input_select"),
    "display_brightness": ("TDB", BRIGHTNESS_MAP, "set_display_brightness_select"),
}

CONFIG_SCHEMA = select.select_schema(McIntoshSelect).extend(
    {
        cv.GenerateID(CONF_MCINTOSH_MA5300_ID): cv.use_id(McIntoshMA5300),
        cv.Required(CONF_TYPE): cv.one_of(*TYPES, lower=True),
    }
)


async def to_code(config):
    hub = await cg.get_variable(config[CONF_MCINTOSH_MA5300_ID])
    type_key = config[CONF_TYPE]
    cmd_prefix, option_map, setter = TYPES[type_key]
    var = await select.new_select(config, options=list(option_map.keys()))
    cg.add(var.set_hub(hub))
    cg.add(var.set_command_prefix(cmd_prefix))
    for name, idx in option_map.items():
        cg.add(var.add_option(name, idx))
    cg.add(getattr(hub, setter)(var))
