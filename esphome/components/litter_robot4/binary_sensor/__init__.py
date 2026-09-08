import esphome.codegen as cg
from esphome.components import binary_sensor
import esphome.config_validation as cv
from esphome.const import CONF_TYPE

from .. import CONF_LITTER_ROBOT4_ID, LitterRobot4Component, litter_robot4_ns

DEPENDENCIES = ["litter_robot4"]

LitterRobot4BoolBinarySensor = litter_robot4_ns.class_(
    "LitterRobot4BoolBinarySensor",
    binary_sensor.BinarySensor,
    cg.Component,
    cg.Parented.template(LitterRobot4Component),
)
LitterRobot4DetectionBinarySensor = litter_robot4_ns.class_(
    "LitterRobot4DetectionBinarySensor",
    binary_sensor.BinarySensor,
    cg.Component,
    cg.Parented.template(LitterRobot4Component),
)
LitterRobot4HopperMotorBinarySensor = litter_robot4_ns.class_(
    "LitterRobot4HopperMotorBinarySensor",
    binary_sensor.BinarySensor,
    cg.Component,
    cg.Parented.template(LitterRobot4Component),
)


def _bs_schema(class_, *, icon=None, device_class=None):
    kwargs = {}
    if icon is not None:
        kwargs["icon"] = icon
    if device_class is not None:
        kwargs["device_class"] = device_class
    return (
        binary_sensor.binary_sensor_schema(class_, **kwargs)
        .extend(
            {
                cv.GenerateID(CONF_LITTER_ROBOT4_ID): cv.use_id(LitterRobot4Component),
            }
        )
        .extend(cv.COMPONENT_SCHEMA)
    )


_BOOL_REGISTERS = {
    "waste_drawer_full": "REG_WASTE_DRAWER_FULL",
    "night_light": "REG_NIGHT_LIGHT",
    "sleeping": "REG_SLEEPING",
}

CONFIG_SCHEMA = cv.typed_schema(
    {
        "waste_drawer_full": _bs_schema(
            LitterRobot4BoolBinarySensor, icon="mdi:inbox-full"
        ),
        "bonnet": _bs_schema(
            LitterRobot4BoolBinarySensor, icon="mdi:circle-off-outline"
        ),
        "night_light": _bs_schema(
            LitterRobot4BoolBinarySensor, icon="mdi:lightbulb-night"
        ),
        "sleeping": _bs_schema(LitterRobot4BoolBinarySensor, icon="mdi:sleep"),
        "laser_detect": _bs_schema(
            LitterRobot4DetectionBinarySensor,
            icon="mdi:signal-variant",
            device_class="motion",
        ),
        "weight_detect": _bs_schema(
            LitterRobot4DetectionBinarySensor,
            icon="mdi:scale",
            device_class="motion",
        ),
        "hopper_motor": _bs_schema(
            LitterRobot4HopperMotorBinarySensor,
            icon="mdi:basket-fill",
            device_class="running",
        ),
    }
)


async def to_code(config):
    parent = await cg.get_variable(config[CONF_LITTER_ROBOT4_ID])
    var = await binary_sensor.new_binary_sensor(config)
    await cg.register_component(var, config)
    await cg.register_parented(var, parent)
    if config[CONF_TYPE] in _BOOL_REGISTERS:
        cg.add(var.set_register(cg.RawExpression(_BOOL_REGISTERS[config[CONF_TYPE]])))
    if config[CONF_TYPE] == "bonnet":
        cg.add(var.set_register(cg.RawExpression("REG_BONNET_REMOVED")))
        cg.add(var.set_invert(True))
    if config[CONF_TYPE] == "weight_detect":
        cg.add(var.set_weight(True))
