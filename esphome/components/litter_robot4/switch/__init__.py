import esphome.codegen as cg
from esphome.components import switch
import esphome.config_validation as cv
from esphome.const import CONF_TYPE, ENTITY_CATEGORY_CONFIG, ENTITY_CATEGORY_DIAGNOSTIC
from esphome.core import CORE, ID

from .. import (
    CONF_LITTER_ROBOT4_ID,
    LITTER_ROBOT4_DEVICE_SCHEMA,
    LitterRobot4Component,
    litter_robot4_ns,
)

DEPENDENCIES = ["litter_robot4"]

ICON_LOCK = "mdi:lock"

_SLEEP_SCHEDULES_KEY = "litter_robot4_sleep_schedules"

LitterRobot4SleepSchedule = litter_robot4_ns.class_("LitterRobot4SleepSchedule")

LitterRobot4BoolSwitch = litter_robot4_ns.class_(
    "LitterRobot4BoolSwitch",
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
_BOOL_REGISTERS = {
    "control_panel_lockout": "REG_PANEL_LOCKOUT",
    "litter_hopper": "REG_LITTER_HOPPER",
    "debug_mode": "REG_DEBUG",
}

DAY_TYPES = {
    "sleep_schedule_sun": "DAY_SUN",
    "sleep_schedule_mon": "DAY_MON",
    "sleep_schedule_tue": "DAY_TUE",
    "sleep_schedule_wed": "DAY_WED",
    "sleep_schedule_thu": "DAY_THU",
    "sleep_schedule_fri": "DAY_FRI",
    "sleep_schedule_sat": "DAY_SAT",
}


def _switch_schema(
    class_,
    *,
    icon=None,
    entity_category=None,
    default_restore_mode=None,
):
    kwargs = {}
    if icon is not None:
        kwargs["icon"] = icon
    if entity_category is not None:
        kwargs["entity_category"] = entity_category
    if default_restore_mode is not None:
        kwargs["default_restore_mode"] = default_restore_mode
    return (
        switch.switch_schema(class_, **kwargs)
        .extend(LITTER_ROBOT4_DEVICE_SCHEMA)
        .extend(cv.COMPONENT_SCHEMA)
    )


CONFIG_SCHEMA = cv.typed_schema(
    {
        "control_panel_lockout": _switch_schema(
            LitterRobot4BoolSwitch,
            icon=ICON_LOCK,
            entity_category=ENTITY_CATEGORY_CONFIG,
            default_restore_mode="DISABLED",
        ),
        **{
            key: _switch_schema(
                LitterRobot4SleepDayEnabledSwitch,
                icon="mdi:calendar-clock",
                entity_category=ENTITY_CATEGORY_CONFIG,
            )
            for key in DAY_TYPES
        },
        "power": _switch_schema(
            LitterRobot4PowerSwitch,
            icon="mdi:power",
            default_restore_mode="DISABLED",
        ),
        "litter_hopper": _switch_schema(
            LitterRobot4BoolSwitch,
            icon="mdi:cup",
            entity_category=ENTITY_CATEGORY_CONFIG,
            default_restore_mode="DISABLED",
        ),
        "debug_mode": _switch_schema(
            LitterRobot4BoolSwitch,
            icon="mdi:bug",
            entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
            default_restore_mode="DISABLED",
        ),
    }
)


async def _get_sleep_schedule(parent_id: str, parent):
    """Return the single shared sleep-schedule coordinator for this hub."""
    schedules = CORE.data.setdefault(_SLEEP_SCHEDULES_KEY, {})
    if (schedule := schedules.get(parent_id)) is not None:
        return schedule
    schedule_id = ID(f"{parent_id}_sleep_schedule", type=LitterRobot4SleepSchedule)
    schedule = cg.new_Pvariable(schedule_id)
    await cg.register_parented(schedule, parent)
    schedules[parent_id] = schedule
    return schedule


async def to_code(config):
    parent = await cg.get_variable(config[CONF_LITTER_ROBOT4_ID])
    var = await switch.new_switch(config)
    await cg.register_component(var, config)
    await cg.register_parented(var, parent)
    if config[CONF_TYPE] in DAY_TYPES:
        cg.add(var.set_day(cg.RawExpression(DAY_TYPES[config[CONF_TYPE]])))
        schedule = await _get_sleep_schedule(str(config[CONF_LITTER_ROBOT4_ID]), parent)
        cg.add(var.set_schedule(schedule))
    if config[CONF_TYPE] in _BOOL_REGISTERS:
        cg.add(var.set_register(cg.RawExpression(_BOOL_REGISTERS[config[CONF_TYPE]])))
