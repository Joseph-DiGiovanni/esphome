import esphome.codegen as cg
from esphome.components import number
import esphome.config_validation as cv
from esphome.const import CONF_TYPE, ENTITY_CATEGORY_CONFIG, UNIT_MINUTE, UNIT_POUND

from .. import CONF_LITTER_ROBOT4_ID, LitterRobot4Component, litter_robot4_ns

DEPENDENCIES = ["litter_robot4"]

LitterRobot4CycleDelayNumber = litter_robot4_ns.class_(
    "LitterRobot4CycleDelayNumber",
    number.Number,
    cg.Component,
    cg.Parented.template(LitterRobot4Component),
)
LitterRobot4CatWeightNumber = litter_robot4_ns.class_(
    "LitterRobot4CatWeightNumber",
    number.Number,
    cg.Component,
    cg.Parented.template(LitterRobot4Component),
)


def _number_schema(class_, **kwargs):
    return (
        number.number_schema(class_, **kwargs)
        .extend(
            {
                cv.GenerateID(CONF_LITTER_ROBOT4_ID): cv.use_id(LitterRobot4Component),
            }
        )
        .extend(cv.COMPONENT_SCHEMA)
    )


CONFIG_SCHEMA = cv.typed_schema(
    {
        "clean_cycle_delay": _number_schema(
            LitterRobot4CycleDelayNumber,
            unit_of_measurement=UNIT_MINUTE,
            entity_category=ENTITY_CATEGORY_CONFIG,
            icon="mdi:timer-sync-outline",
        ),
        "cat_weight": _number_schema(
            LitterRobot4CatWeightNumber,
            unit_of_measurement=UNIT_POUND,
            entity_category=ENTITY_CATEGORY_CONFIG,
            icon="mdi:weight",
        ),
    }
)


async def to_code(config):
    parent = await cg.get_variable(config[CONF_LITTER_ROBOT4_ID])
    if config[CONF_TYPE] == "clean_cycle_delay":
        var = await number.new_number(config, min_value=3, max_value=30, step=1)
    else:
        var = await number.new_number(config, min_value=0, max_value=50, step=0.01)
    await cg.register_component(var, config)
    await cg.register_parented(var, parent)
