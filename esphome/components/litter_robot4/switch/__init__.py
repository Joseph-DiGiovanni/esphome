import esphome.codegen as cg
from esphome.components import switch
import esphome.config_validation as cv
from esphome.const import ENTITY_CATEGORY_CONFIG

from .. import CONF_LITTER_ROBOT4_ID, LitterRobot4Component, litter_robot4_ns

DEPENDENCIES = ["litter_robot4"]

ICON_LOCK = "mdi:lock"

LitterRobot4ControlPanelLockoutSwitch = litter_robot4_ns.class_(
    "LitterRobot4ControlPanelLockoutSwitch",
    switch.Switch,
    cg.Component,
    cg.Parented.template(LitterRobot4Component),
)

CONFIG_SCHEMA = cv.All(
    switch.switch_schema(
        LitterRobot4ControlPanelLockoutSwitch,
        icon=ICON_LOCK,
        entity_category=ENTITY_CATEGORY_CONFIG,
        default_restore_mode="DISABLED",
    )
    .extend(
        {
            cv.GenerateID(CONF_LITTER_ROBOT4_ID): cv.use_id(LitterRobot4Component),
        }
    )
    .extend(cv.COMPONENT_SCHEMA)
)


async def to_code(config):
    parent = await cg.get_variable(config[CONF_LITTER_ROBOT4_ID])
    var = await switch.new_switch(config)
    await cg.register_component(var, config)
    await cg.register_parented(var, parent)
