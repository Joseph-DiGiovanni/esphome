import esphome.codegen as cg
from esphome.components import text_sensor
import esphome.config_validation as cv

from .. import (
    CONF_LITTER_ROBOT3_ID,
    LITTER_ROBOT3_DEVICE_SCHEMA,
    LitterRobot3Component,
    litter_robot3_ns,
)

DEPENDENCIES = ["litter_robot3"]

LitterRobot3StatusTextSensor = litter_robot3_ns.class_(
    "LitterRobot3StatusTextSensor",
    text_sensor.TextSensor,
    cg.Component,
    cg.Parented.template(LitterRobot3Component),
)

CONFIG_SCHEMA = cv.typed_schema(
    {
        "status": (
            text_sensor.text_sensor_schema(
                LitterRobot3StatusTextSensor, icon="mdi:information"
            )
            .extend(LITTER_ROBOT3_DEVICE_SCHEMA)
            .extend(cv.COMPONENT_SCHEMA)
        ),
    }
)


async def to_code(config):
    parent = await cg.get_variable(config[CONF_LITTER_ROBOT3_ID])
    var = await text_sensor.new_text_sensor(config)
    await cg.register_component(var, config)
    await cg.register_parented(var, parent)
