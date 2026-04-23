import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import button
from esphome.const import CONF_TYPE
from .. import mcintosh_ma5300_ns, CONF_MCINTOSH_MA5300_ID, McIntoshMA5300

DEPENDENCIES = ["mcintosh_ma5300"]

McIntoshButton = mcintosh_ma5300_ns.class_("McIntoshButton", button.Button)

# (hub_action_method,)
TYPES = {
    "query":            "set_query_button",
    "discover_inputs":  "set_discover_inputs_button",
    "volume_up":        "set_volume_up_button",
    "volume_down":      "set_volume_down_button",
}

CONFIG_SCHEMA = button.button_schema(McIntoshButton).extend(
    {
        cv.GenerateID(CONF_MCINTOSH_MA5300_ID): cv.use_id(McIntoshMA5300),
        cv.Required(CONF_TYPE): cv.one_of(*TYPES, lower=True),
    }
)


async def to_code(config):
    hub = await cg.get_variable(config[CONF_MCINTOSH_MA5300_ID])
    var = await button.new_button(config)
    cg.add(var.set_hub(hub))
    cg.add(getattr(hub, TYPES[config[CONF_TYPE]])(var))
