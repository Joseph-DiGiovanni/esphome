import esphome.codegen as cg
from esphome.components import switch
import esphome.config_validation as cv
from esphome.const import CONF_TYPE

from .. import (
    CONF_LITTER_ROBOT3_ID,
    LITTER_ROBOT3_DEVICE_SCHEMA,
    LitterRobot3Component,
    litter_robot3_ns,
)

DEPENDENCIES = ["litter_robot3"]

LitterRobot3Switch = litter_robot3_ns.class_(
    "LitterRobot3Switch",
    switch.Switch,
    cg.Component,
    cg.Parented.template(LitterRobot3Component),
)

_TYPES = {
    "power": {
        "icon": "mdi:power",
        "command": "'P'",
        "field": "LR3_FIELD_POWER_STATE",
        "invert": True,
    },
    "night_light": {
        "icon": "mdi:lightbulb-night",
        "command": "'N'",
        "field": "LR3_FIELD_NIGHT_LIGHT",
        "invert": False,
    },
    "panel_lockout": {
        "icon": "mdi:lock",
        "command": "'L'",
        "field": "LR3_FIELD_PANEL_LOCKOUT",
        "invert": False,
    },
}


def _switch_schema(*, icon=None):
    kwargs = {}
    if icon is not None:
        kwargs["icon"] = icon
    return (
        switch.switch_schema(LitterRobot3Switch, **kwargs)
        .extend(LITTER_ROBOT3_DEVICE_SCHEMA)
        .extend(cv.COMPONENT_SCHEMA)
    )


CONFIG_SCHEMA = cv.typed_schema(
    {key: _switch_schema(icon=cfg["icon"]) for key, cfg in _TYPES.items()}
)


async def to_code(config):
    parent = await cg.get_variable(config[CONF_LITTER_ROBOT3_ID])
    var = await switch.new_switch(config)
    await cg.register_component(var, config)
    await cg.register_parented(var, parent)
    cfg = _TYPES[config[CONF_TYPE]]
    cg.add(var.set_command(cg.RawExpression(cfg["command"])))
    cg.add(var.set_field(cg.RawExpression(cfg["field"])))
    cg.add(var.set_invert(cfg["invert"]))
