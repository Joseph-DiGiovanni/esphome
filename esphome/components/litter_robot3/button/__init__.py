import esphome.codegen as cg
from esphome.components import button
import esphome.config_validation as cv
from esphome.const import CONF_TYPE

from .. import (
    CONF_LITTER_ROBOT3_ID,
    LITTER_ROBOT3_DEVICE_SCHEMA,
    LitterRobot3Component,
    litter_robot3_ns,
)

DEPENDENCIES = ["litter_robot3"]

LitterRobot3Button = litter_robot3_ns.class_(
    "LitterRobot3Button",
    button.Button,
    cg.Component,
    cg.Parented.template(LitterRobot3Component),
)

_TYPES = {
    "cycle": {"icon": "mdi:restore", "command": "'C'"},
}


def _button_schema(*, icon=None):
    kwargs = {}
    if icon is not None:
        kwargs["icon"] = icon
    return (
        button.button_schema(LitterRobot3Button, **kwargs)
        .extend(LITTER_ROBOT3_DEVICE_SCHEMA)
        .extend(cv.COMPONENT_SCHEMA)
    )


CONFIG_SCHEMA = cv.typed_schema(
    {key: _button_schema(icon=cfg["icon"]) for key, cfg in _TYPES.items()}
)


async def to_code(config):
    parent = await cg.get_variable(config[CONF_LITTER_ROBOT3_ID])
    var = await button.new_button(config)
    await cg.register_component(var, config)
    await cg.register_parented(var, parent)
    cg.add(var.set_command(cg.RawExpression(_TYPES[config[CONF_TYPE]]["command"])))
