import esphome.codegen as cg
from esphome.components import number
import esphome.config_validation as cv
from esphome.const import ENTITY_CATEGORY_CONFIG, UNIT_MINUTE

from .. import (
    CONF_LITTER_ROBOT3_ID,
    LITTER_ROBOT3_DEVICE_SCHEMA,
    LitterRobot3Component,
    litter_robot3_ns,
)

DEPENDENCIES = ["litter_robot3"]

LitterRobot3CycleDelayNumber = litter_robot3_ns.class_(
    "LitterRobot3CycleDelayNumber",
    number.Number,
    cg.Component,
    cg.Parented.template(LitterRobot3Component),
)


def _number_schema(class_, **kwargs):
    return (
        number.number_schema(class_, **kwargs)
        .extend(LITTER_ROBOT3_DEVICE_SCHEMA)
        .extend(cv.COMPONENT_SCHEMA)
    )


CONFIG_SCHEMA = cv.typed_schema(
    {
        "cycle_delay": _number_schema(
            LitterRobot3CycleDelayNumber,
            unit_of_measurement=UNIT_MINUTE,
            entity_category=ENTITY_CATEGORY_CONFIG,
            icon="mdi:timer-sync",
        ),
    }
)


async def to_code(config):
    parent = await cg.get_variable(config[CONF_LITTER_ROBOT3_ID])
    var = await number.new_number(config, min_value=3, max_value=15, step=1)
    await cg.register_component(var, config)
    await cg.register_parented(var, parent)
