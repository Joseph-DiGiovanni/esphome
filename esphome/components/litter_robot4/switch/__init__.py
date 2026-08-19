import esphome.codegen as cg
from esphome.components import switch
import esphome.config_validation as cv
from esphome.const import CONF_TYPE, ENTITY_CATEGORY_CONFIG

from .. import CONF_LITTER_ROBOT4_ID, LitterRobot4Component, litter_robot4_ns

DEPENDENCIES = ["litter_robot4"]

ICON_LOCK = "mdi:lock"

LitterRobot4ControlPanelLockoutSwitch = litter_robot4_ns.class_(
    "LitterRobot4ControlPanelLockoutSwitch",
    switch.Switch,
    cg.Component,
    cg.Parented.template(LitterRobot4Component),
)
LitterRobot4SleepDayEnabledSwitch = litter_robot4_ns.class_(
    "LitterRobot4SleepDayEnabledSwitch",
    switch.Switch,
    cg.Component,
    cg.Parented.template(LitterRobot4Component),
)
LitterRobot4PowerSwitch = litter_robot4_ns.class_(
    "LitterRobot4PowerSwitch",
    switch.Switch,
    cg.Component,
    cg.Parented.template(LitterRobot4Component),
)

DAY_TYPES = {
    "sleep_schedule_sun": "DAY_SUN",
    "sleep_schedule_mon": "DAY_MON",
    "sleep_schedule_tue": "DAY_TUE",
    "sleep_schedule_wed": "DAY_WED",
    "sleep_schedule_thu": "DAY_THU",
    "sleep_schedule_fri": "DAY_FRI",
    "sleep_schedule_sat": "DAY_SAT",
}


def _lockout_schema():
    return (
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


def _day_switch_schema():
    return (
        switch.switch_schema(
            LitterRobot4SleepDayEnabledSwitch,
            entity_category=ENTITY_CATEGORY_CONFIG,
            icon="mdi:calendar-clock",
        )
        .extend(
            {
                cv.GenerateID(CONF_LITTER_ROBOT4_ID): cv.use_id(LitterRobot4Component),
            }
        )
        .extend(cv.COMPONENT_SCHEMA)
    )


def _power_schema():
    return (
        switch.switch_schema(
            LitterRobot4PowerSwitch,
            icon="mdi:power",
            default_restore_mode="DISABLED",
        )
        .extend(
            {
                cv.GenerateID(CONF_LITTER_ROBOT4_ID): cv.use_id(LitterRobot4Component),
            }
        )
        .extend(cv.COMPONENT_SCHEMA)
    )


CONFIG_SCHEMA = cv.typed_schema(
    {
        "control_panel_lockout": _lockout_schema(),
        **{key: _day_switch_schema() for key in DAY_TYPES},
        "power": _power_schema(),
    }
)


async def to_code(config):
    parent = await cg.get_variable(config[CONF_LITTER_ROBOT4_ID])
    var = await switch.new_switch(config)
    await cg.register_component(var, config)
    await cg.register_parented(var, parent)
    if config[CONF_TYPE] in DAY_TYPES:
        cg.add(var.set_day(cg.RawExpression(DAY_TYPES[config[CONF_TYPE]])))
