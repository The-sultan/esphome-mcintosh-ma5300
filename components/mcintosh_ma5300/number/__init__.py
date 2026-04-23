import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import number
from esphome.const import CONF_TYPE
from .. import mcintosh_ma5300_ns, CONF_MCINTOSH_MA5300_ID, McIntoshMA5300

DEPENDENCIES = ["mcintosh_ma5300"]

McIntoshNumber = mcintosh_ma5300_ns.class_("McIntoshNumber", number.Number)

# (min, max, step, cmd_prefix, protocol_multiplier, hub_setter)
# protocol_multiplier: value sent to amp = user_value * multiplier (rounded to int)
# Input Trim: user sees -6.0..+6.0 dB; amp expects -12..+12 (half-dB steps) → ×2
TYPES = {
    "bass":       (-12.0, 12.0, 1.0, "TTB", 1.0, "set_bass_number"),
    "treble":     (-12.0, 12.0, 1.0, "TTT", 1.0, "set_treble_number"),
    "balance":    (-50.0, 50.0, 1.0, "TBA", 1.0, "set_balance_number"),
    "input_trim": (-6.0,   6.0, 0.5, "TIN", 2.0, "set_input_trim_number"),
}

CONFIG_SCHEMA = number.number_schema(McIntoshNumber).extend(
    {
        cv.GenerateID(CONF_MCINTOSH_MA5300_ID): cv.use_id(McIntoshMA5300),
        cv.Required(CONF_TYPE): cv.one_of(*TYPES, lower=True),
    }
)


async def to_code(config):
    hub = await cg.get_variable(config[CONF_MCINTOSH_MA5300_ID])
    type_cfg = TYPES[config[CONF_TYPE]]
    var = await number.new_number(
        config,
        min_value=type_cfg[0],
        max_value=type_cfg[1],
        step=type_cfg[2],
    )
    cg.add(var.set_hub(hub))
    cg.add(var.set_command_prefix(type_cfg[3]))
    if type_cfg[4] != 1.0:
        cg.add(var.set_multiplier(type_cfg[4]))
    cg.add(getattr(hub, type_cfg[5])(var))
