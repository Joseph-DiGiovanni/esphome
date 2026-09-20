import esphome.codegen as cg
from esphome.components import sensor
import esphome.config_validation as cv
from esphome.const import STATE_CLASS_TOTAL

from .. import (
    CONF_LITTER_ROBOT3_ID,
    LITTER_ROBOT3_DEVICE_SCHEMA,
    LitterRobot3Component,
    litter_robot3_ns,
)

DEPENDENCIES = ["litter_robot3"]

LitterRobot3CycleCountSensor = litter_robot3_ns.class_(
    "LitterRobot3CycleCountSensor",
    sensor.Sensor,
    cg.Component,
    cg.Parented.template(LitterRobot3Component),
)

CONFIG_SCHEMA = cv.typed_schema(
    {
        "waste_drawer_cycle_count": (
            sensor.sensor_schema(
                LitterRobot3CycleCountSensor,
                accuracy_decimals=0,
                state_class=STATE_CLASS_TOTAL,
                icon="mdi:trash-can",
            )
            .extend(LITTER_ROBOT3_DEVICE_SCHEMA)
            .extend(cv.COMPONENT_SCHEMA)
        ),
    }
)


async def to_code(config):
    parent = await cg.get_variable(config[CONF_LITTER_ROBOT3_ID])
    var = await sensor.new_sensor(config)
    await cg.register_component(var, config)
    await cg.register_parented(var, parent)
