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
LitterRobot4BonnetRemovedBinarySensor = litter_robot4_ns.class_(
    "LitterRobot4BonnetRemovedBinarySensor",
    binary_sensor.BinarySensor,
    cg.Component,
    cg.Parented.template(LitterRobot4Component),
)
LitterRobot4NightLightBinarySensor = litter_robot4_ns.class_(
    "LitterRobot4NightLightBinarySensor",
    binary_sensor.BinarySensor,
    cg.Component,
    cg.Parented.template(LitterRobot4Component),
)
LitterRobot4SleepStatusBinarySensor = litter_robot4_ns.class_(
    "LitterRobot4SleepStatusBinarySensor",
    binary_sensor.BinarySensor,
    cg.Component,
    cg.Parented.template(LitterRobot4Component),
)
LitterRobot4FaultStatusBinarySensor = litter_robot4_ns.class_(
    "LitterRobot4FaultStatusBinarySensor",
    binary_sensor.BinarySensor,
    cg.Component,
    cg.Parented.template(LitterRobot4Component),
)
LitterRobot4LaserDetectBinarySensor = litter_robot4_ns.class_(
    "LitterRobot4LaserDetectBinarySensor",
    binary_sensor.BinarySensor,
    cg.Component,
    cg.Parented.template(LitterRobot4Component),
)
LitterRobot4WeightDetectBinarySensor = litter_robot4_ns.class_(
    "LitterRobot4WeightDetectBinarySensor",
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


CONFIG_SCHEMA = cv.typed_schema(
    {
        "waste_drawer_full": _bs_schema(
            LitterRobot4WasteDrawerFullBinarySensor, icon="mdi:inbox-full"
        ),
        "bonnet_removed": _bs_schema(
            LitterRobot4BonnetRemovedBinarySensor, icon="mdi:alert-circle"
        ),
        "night_light": _bs_schema(
            LitterRobot4NightLightBinarySensor, icon="mdi:lightbulb-night"
        ),
        "sleep_status": _bs_schema(
            LitterRobot4SleepStatusBinarySensor, icon="mdi:sleep"
        ),
        "fault_status": _bs_schema(
            LitterRobot4FaultStatusBinarySensor,
            icon="mdi:alert-octagon",
            device_class="problem",
        ),
        "laser_detect": _bs_schema(
            LitterRobot4LaserDetectBinarySensor,
            icon="mdi:signal-variant",
            device_class="motion",
        ),
        "weight_detect": _bs_schema(
            LitterRobot4WeightDetectBinarySensor,
            icon="mdi:scale",
            device_class="motion",
        ),
    }
)


async def to_code(config):
    parent = await cg.get_variable(config[CONF_LITTER_ROBOT4_ID])
    var = await binary_sensor.new_binary_sensor(config)
    await cg.register_component(var, config)
    await cg.register_parented(var, parent)
