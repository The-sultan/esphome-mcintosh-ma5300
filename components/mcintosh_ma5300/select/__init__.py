import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import select
from esphome.const import CONF_TYPE
from .. import mcintosh_ma5300_ns, CONF_MCINTOSH_MA5300_ID, McIntoshMA5300

DEPENDENCIES = ["mcintosh_ma5300"]

McIntoshSelect = mcintosh_ma5300_ns.class_("McIntoshSelect", select.Select)

CONF_INPUT_LABELS = "input_labels"

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
        # Optional custom labels for input type: map input index (1-13) to display name.
        # Unlabelled inputs fall back to their protocol name (e.g. "COAX 1").
        cv.Optional(CONF_INPUT_LABELS): cv.Schema(
            {cv.int_range(min=1, max=13): cv.string}
        ),
    }
)


async def to_code(config):
    hub = await cg.get_variable(config[CONF_MCINTOSH_MA5300_ID])
    type_key = config[CONF_TYPE]
    cmd_prefix, option_map, setter = TYPES[type_key]

    input_labels = config.get(CONF_INPUT_LABELS, {})
    effective_map = {
        input_labels.get(idx, name): idx for name, idx in option_map.items()
    }

    var = await select.new_select(config, options=list(effective_map.keys()))
    cg.add(var.set_hub(hub))
    cg.add(var.set_command_prefix(cmd_prefix))
    for display_name, idx in effective_map.items():
        cg.add(var.add_option(display_name, idx))
    for idx, label in input_labels.items():
        cg.add(var.set_label_override(idx, label))
    cg.add(getattr(hub, setter)(var))
