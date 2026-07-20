import esphome.codegen as cg
from esphome.components import datetime as datetime_
import esphome.config_validation as cv
from esphome.const import CONF_ICON, CONF_TYPE

from .. import CONF_LITTER_ROBOT4_ID, LitterRobot4Component, litter_robot4_ns

DEPENDENCIES = ["litter_robot4"]

LitterRobot4Time = litter_robot4_ns.class_(
    "LitterRobot4Time",
    datetime_.TimeEntity,
    cg.Component,
    cg.Parented.template(LitterRobot4Component),
)

_DAY_TYPES = {
    "sleep_sun": ("REG_SLEEP_SUN", "mdi:bed-clock"),
    "sleep_mon": ("REG_SLEEP_MON", "mdi:bed-clock"),
    "sleep_tue": ("REG_SLEEP_TUE", "mdi:bed-clock"),
    "sleep_wed": ("REG_SLEEP_WED", "mdi:bed-clock"),
    "sleep_thu": ("REG_SLEEP_THU", "mdi:bed-clock"),
    "sleep_fri": ("REG_SLEEP_FRI", "mdi:bed-clock"),
    "sleep_sat": ("REG_SLEEP_SAT", "mdi:bed-clock"),
    "wake_sun": ("REG_WAKE_SUN", "mdi:sun-clock"),
    "wake_mon": ("REG_WAKE_MON", "mdi:sun-clock"),
    "wake_tue": ("REG_WAKE_TUE", "mdi:sun-clock"),
    "wake_wed": ("REG_WAKE_WED", "mdi:sun-clock"),
    "wake_thu": ("REG_WAKE_THU", "mdi:sun-clock"),
    "wake_fri": ("REG_WAKE_FRI", "mdi:sun-clock"),
    "wake_sat": ("REG_WAKE_SAT", "mdi:sun-clock"),
}


def _time_schema(*, icon=None):
    ext = {
        cv.GenerateID(CONF_LITTER_ROBOT4_ID): cv.use_id(LitterRobot4Component),
    }
    if icon is not None:
        ext[cv.Optional(CONF_ICON, default=icon)] = cv.icon
    return (
        datetime_.time_schema(LitterRobot4Time).extend(ext).extend(cv.COMPONENT_SCHEMA)
    )


CONFIG_SCHEMA = cv.typed_schema(
    {key: _time_schema(icon=icon) for key, (_, icon) in _DAY_TYPES.items()}
)


async def to_code(config):
    reg_name = _DAY_TYPES[config[CONF_TYPE]][0]
    reg_expr = cg.RawExpression(reg_name)
    config[CONF_TYPE] = "TIME"
    parent = await cg.get_variable(config[CONF_LITTER_ROBOT4_ID])
    var = await datetime_.new_datetime(config)
    await cg.register_component(var, config)
    await cg.register_parented(var, parent)
    cg.add(var.set_register(reg_expr))
