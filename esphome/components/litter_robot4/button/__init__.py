import esphome.codegen as cg
from esphome.components import button
import esphome.config_validation as cv

from .. import CONF_LITTER_ROBOT4_ID, LitterRobot4Component, litter_robot4_ns

DEPENDENCIES = ["litter_robot4"]

LitterRobot4CycleButton = litter_robot4_ns.class_(
    "LitterRobot4CycleButton",
    button.Button,
    cg.Component,
    cg.Parented.template(LitterRobot4Component),
)
LitterRobot4EmptyButton = litter_robot4_ns.class_(
    "LitterRobot4EmptyButton",
    button.Button,
    cg.Component,
    cg.Parented.template(LitterRobot4Component),
)
LitterRobot4ReplaceFilterButton = litter_robot4_ns.class_(
    "LitterRobot4ReplaceFilterButton",
    button.Button,
    cg.Component,
    cg.Parented.template(LitterRobot4Component),
)
LitterRobot4FactoryResetButton = litter_robot4_ns.class_(
    "LitterRobot4FactoryResetButton",
    button.Button,
    cg.Component,
    cg.Parented.template(LitterRobot4Component),
)
LitterRobot4PowerButton = litter_robot4_ns.class_(
    "LitterRobot4PowerButton",
    button.Button,
    cg.Component,
    cg.Parented.template(LitterRobot4Component),
)
LitterRobot4ResetButton = litter_robot4_ns.class_(
    "LitterRobot4ResetButton",
    button.Button,
    cg.Component,
    cg.Parented.template(LitterRobot4Component),
)


def _button_schema(class_, *, icon=None):
    kwargs = {}
    if icon is not None:
        kwargs["icon"] = icon
    return (
        button.button_schema(class_, **kwargs)
        .extend(
            {
                cv.GenerateID(CONF_LITTER_ROBOT4_ID): cv.use_id(LitterRobot4Component),
            }
        )
        .extend(cv.COMPONENT_SCHEMA)
    )


CONFIG_SCHEMA = cv.typed_schema(
    {
        "cycle": _button_schema(LitterRobot4CycleButton, icon="mdi:restore"),
        "empty": _button_schema(LitterRobot4EmptyButton, icon="mdi:delete-outline"),
        "replace_filter": _button_schema(
            LitterRobot4ReplaceFilterButton, icon="mdi:air-filter"
        ),
        "factory_reset": _button_schema(
            LitterRobot4FactoryResetButton, icon="mdi:cog-refresh"
        ),
        "power": _button_schema(LitterRobot4PowerButton, icon="mdi:power"),
        "reset": _button_schema(LitterRobot4ResetButton, icon="mdi:reload"),
    }
)


async def to_code(config):
    parent = await cg.get_variable(config[CONF_LITTER_ROBOT4_ID])
    var = await button.new_button(config)
    await cg.register_component(var, config)
    await cg.register_parented(var, parent)
