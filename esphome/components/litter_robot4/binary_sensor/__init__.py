import esphome.codegen as cg
from esphome.components import binary_sensor
import esphome.config_validation as cv

from .. import CONF_LITTER_ROBOT4_ID, LitterRobot4Component, litter_robot4_ns

DEPENDENCIES = ["litter_robot4"]

LitterRobot4WasteDrawerFullBinarySensor = litter_robot4_ns.class_(
    "LitterRobot4WasteDrawerFullBinarySensor",
    binary_sensor.BinarySensor,
    cg.Component,
    cg.Parented.template(LitterRobot4Component),
)

CONFIG_SCHEMA = cv.All(
    binary_sensor.binary_sensor_schema(
        LitterRobot4WasteDrawerFullBinarySensor, icon="mdi:inbox-full"
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
    var = await binary_sensor.new_binary_sensor(config)
    await cg.register_component(var, config)
    await cg.register_parented(var, parent)
