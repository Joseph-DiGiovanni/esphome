import esphome.codegen as cg
from esphome.components import select
import esphome.config_validation as cv
from esphome.const import ENTITY_CATEGORY_CONFIG

from .. import (
    CONF_LITTER_ROBOT3_ID,
    LITTER_ROBOT3_DEVICE_SCHEMA,
    LitterRobot3Component,
    litter_robot3_ns,
)

DEPENDENCIES = ["litter_robot3"]

LitterRobot3CycleDelaySelect = litter_robot3_ns.class_(
    "LitterRobot3CycleDelaySelect",
    select.Select,
    cg.Component,
    cg.Parented.template(LitterRobot3Component),
)

CONFIG_SCHEMA = cv.typed_schema(
    {
        "cycle_delay": (
            select.select_schema(
                LitterRobot3CycleDelaySelect,
                entity_category=ENTITY_CATEGORY_CONFIG,
                icon="mdi:timer-sync",
            )
            .extend(LITTER_ROBOT3_DEVICE_SCHEMA)
            .extend(cv.COMPONENT_SCHEMA)
        ),
    }
)


async def to_code(config):
    parent = await cg.get_variable(config[CONF_LITTER_ROBOT3_ID])
    var = await select.new_select(
        config, options=["3 minutes", "7 minutes", "15 minutes"]
    )
    await cg.register_component(var, config)
    await cg.register_parented(var, parent)
