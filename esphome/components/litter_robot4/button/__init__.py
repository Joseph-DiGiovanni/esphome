import esphome.codegen as cg
from esphome.components import button
import esphome.config_validation as cv
from esphome.const import CONF_TYPE

from .. import (
    CONF_LITTER_ROBOT4_ID,
    LITTER_ROBOT4_DEVICE_SCHEMA,
    LitterRobot4Component,
    litter_robot4_ns,
)

DEPENDENCIES = ["litter_robot4"]

LitterRobot4CommandButton = litter_robot4_ns.class_(
    "LitterRobot4CommandButton",
    button.Button,
    cg.Component,
    cg.Parented.template(LitterRobot4Component),
)

_COMMANDS = {
    "cycle": ("REG_ROBOT_STATUS", "CMD_START_CLEAN"),
    "empty": ("REG_ROBOT_STATUS", "CMD_EMPTY_LITTER"),
    "replace_filter": ("REG_ROBOT_STATUS", "CMD_REPLACE_FILTER"),
    "factory_reset": ("REG_FACTORY_RESET", "CMD_FACTORY_RESET"),
    "reset": ("REG_KEYPAD", "CMD_KEYPAD_RESET"),
}


def _button_schema(class_, *, icon=None):
    kwargs = {}
    if icon is not None:
        kwargs["icon"] = icon
    return (
        button.button_schema(class_, **kwargs)
        .extend(LITTER_ROBOT4_DEVICE_SCHEMA)
        .extend(cv.COMPONENT_SCHEMA)
    )


CONFIG_SCHEMA = cv.typed_schema(
    {
        "cycle": _button_schema(LitterRobot4CommandButton, icon="mdi:restore"),
        "empty": _button_schema(LitterRobot4CommandButton, icon="mdi:trash-can"),
        "replace_filter": _button_schema(
            LitterRobot4CommandButton, icon="mdi:air-filter"
        ),
        "factory_reset": _button_schema(
            LitterRobot4CommandButton, icon="mdi:cog-refresh"
        ),
        "reset": _button_schema(LitterRobot4CommandButton, icon="mdi:reload"),
    }
)


async def to_code(config):
    parent = await cg.get_variable(config[CONF_LITTER_ROBOT4_ID])
    var = await button.new_button(config)
    await cg.register_component(var, config)
    await cg.register_parented(var, parent)
    reg, cmd = _COMMANDS[config[CONF_TYPE]]
    cg.add(var.set_register(cg.RawExpression(reg)))
    cg.add(var.set_command(cg.RawExpression(cmd)))
