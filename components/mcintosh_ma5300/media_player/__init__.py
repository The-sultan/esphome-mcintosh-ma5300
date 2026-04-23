import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import media_player
from .. import mcintosh_ma5300_ns, CONF_MCINTOSH_MA5300_ID, McIntoshMA5300

DEPENDENCIES = ["mcintosh_ma5300"]

McIntoshMediaPlayer = mcintosh_ma5300_ns.class_(
    "McIntoshMediaPlayer", media_player.MediaPlayer, cg.Component
)

CONFIG_SCHEMA = media_player.media_player_schema(McIntoshMediaPlayer).extend(
    {
        cv.GenerateID(CONF_MCINTOSH_MA5300_ID): cv.use_id(McIntoshMA5300),
    }
).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    hub = await cg.get_variable(config[CONF_MCINTOSH_MA5300_ID])
    var = await media_player.new_media_player(config)
    await cg.register_component(var, config)
    cg.add(var.set_hub(hub))
    cg.add(hub.set_media_player(var))
