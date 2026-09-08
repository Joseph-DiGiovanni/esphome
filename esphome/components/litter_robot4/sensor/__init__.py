import esphome.codegen as cg
from esphome.components import sensor
import esphome.config_validation as cv
from esphome.const import (
    CONF_TYPE,
    DEVICE_CLASS_DISTANCE,
    DEVICE_CLASS_WEIGHT,
    STATE_CLASS_MEASUREMENT,
    STATE_CLASS_TOTAL_INCREASING,
    UNIT_MILLIMETER,
    UNIT_PERCENT,
    UNIT_POUND,
)

from .. import CONF_LITTER_ROBOT4_ID, LitterRobot4Component, litter_robot4_ns

DEPENDENCIES = ["litter_robot4"]

CONF_MIN_DISTANCE = "min_distance"
CONF_MAX_DISTANCE = "max_distance"

LitterRobot4WasteDrawerSensor = litter_robot4_ns.class_(
    "LitterRobot4WasteDrawerSensor",
    sensor.Sensor,
    cg.Component,
    cg.Parented.template(LitterRobot4Component),
)
LitterRobot4LitterLevelSensor = litter_robot4_ns.class_(
    "LitterRobot4LitterLevelSensor",
    sensor.Sensor,
    cg.Component,
    cg.Parented.template(LitterRobot4Component),
)
CatWeightSensor = litter_robot4_ns.class_(
    "LitterRobot4CatWeightSensor",
    sensor.Sensor,
    cg.Component,
    cg.Parented.template(LitterRobot4Component),
)
RegisterSensor = litter_robot4_ns.class_(
    "LitterRobot4RegisterSensor",
    sensor.Sensor,
    cg.Component,
    cg.Parented.template(LitterRobot4Component),
)

_REGISTER_REGISTERS = {
    "clean_cycle_count": "REG_CLEAN_CYCLE_COUNT",
    "power_cycle_count": "REG_POWER_CYCLE_COUNT",
    "empty_cycle_count": "REG_EMPTY_CYCLE_COUNT",
    "filter_cycle_count": "REG_FILTER_CYCLE_COUNT",
    "litter_level_distance": "REG_LITTER_LEVEL_RAW",
    "calibrated_litter_distance": "REG_CALIBRATED_LITTER_LEVEL_RAW",
}


def _sensor_schema(
    class_,
    *,
    unit=None,
    acc_decimals=0,
    device_class=None,
    state_class=STATE_CLASS_MEASUREMENT,
    icon=None,
):
    kwargs = {
        "accuracy_decimals": acc_decimals,
        "state_class": state_class,
    }
    if unit is not None:
        kwargs["unit_of_measurement"] = unit
    if device_class is not None:
        kwargs["device_class"] = device_class
    if icon is not None:
        kwargs["icon"] = icon
    return (
        sensor.sensor_schema(class_, **kwargs)
        .extend(
            {
                cv.GenerateID(CONF_LITTER_ROBOT4_ID): cv.use_id(LitterRobot4Component),
            }
        )
        .extend(cv.COMPONENT_SCHEMA)
    )


CONFIG_SCHEMA = cv.typed_schema(
    {
        "waste_drawer_level": _sensor_schema(
            LitterRobot4WasteDrawerSensor, unit=UNIT_PERCENT, icon="mdi:inbox"
        ),
        "litter_level": _sensor_schema(
            LitterRobot4LitterLevelSensor, unit=UNIT_PERCENT, icon="mdi:tray-full"
        ).extend(
            {
                cv.Optional(CONF_MIN_DISTANCE, default=435.0): cv.positive_float,
                cv.Optional(CONF_MAX_DISTANCE, default=480.0): cv.positive_float,
            }
        ),
        "litter_level_distance": _sensor_schema(
            RegisterSensor,
            acc_decimals=0,
            unit=UNIT_MILLIMETER,
            device_class=DEVICE_CLASS_DISTANCE,
            icon="mdi:tray-full",
        ),
        "calibrated_litter_distance": _sensor_schema(
            RegisterSensor,
            acc_decimals=0,
            unit=UNIT_MILLIMETER,
            device_class=DEVICE_CLASS_DISTANCE,
            icon="mdi:target",
        ),
        "last_cat_weight": _sensor_schema(
            CatWeightSensor,
            unit=UNIT_POUND,
            acc_decimals=2,
            device_class=DEVICE_CLASS_WEIGHT,
            icon="mdi:weight",
        ),
        "clean_cycle_count": _sensor_schema(
            RegisterSensor,
            acc_decimals=0,
            state_class=STATE_CLASS_TOTAL_INCREASING,
            icon="mdi:counter",
        ),
        "power_cycle_count": _sensor_schema(
            RegisterSensor,
            acc_decimals=0,
            state_class=STATE_CLASS_TOTAL_INCREASING,
            icon="mdi:power-cycle",
        ),
        "empty_cycle_count": _sensor_schema(
            RegisterSensor,
            acc_decimals=0,
            state_class=STATE_CLASS_TOTAL_INCREASING,
            icon="mdi:trash-can",
        ),
        "filter_cycle_count": _sensor_schema(
            RegisterSensor,
            acc_decimals=0,
            state_class=STATE_CLASS_TOTAL_INCREASING,
            icon="mdi:air-filter",
        ),
    }
)


async def to_code(config):
    parent = await cg.get_variable(config[CONF_LITTER_ROBOT4_ID])
    var = await sensor.new_sensor(config)
    await cg.register_component(var, config)
    await cg.register_parented(var, parent)
    if config[CONF_TYPE] in _REGISTER_REGISTERS:
        cg.add(
            var.set_register(cg.RawExpression(_REGISTER_REGISTERS[config[CONF_TYPE]]))
        )
    if config[CONF_TYPE] == "litter_level":
        cg.add(var.set_min_distance(config[CONF_MIN_DISTANCE]))
        cg.add(var.set_max_distance(config[CONF_MAX_DISTANCE]))
