import esphome.codegen as cg
from esphome.components import select
import esphome.config_validation as cv
from esphome.const import CONF_TYPE, ENTITY_CATEGORY_CONFIG

from .. import CONF_LITTER_ROBOT4_ID, LitterRobot4Component, litter_robot4_ns

DEPENDENCIES = ["litter_robot4"]

LitterRobot4NightLightModeSelect = litter_robot4_ns.class_(
    "LitterRobot4NightLightModeSelect",
    select.Select,
    cg.Component,
    cg.Parented.template(LitterRobot4Component),
)
LitterRobot4NightLightBrightnessSelect = litter_robot4_ns.class_(
    "LitterRobot4NightLightBrightnessSelect",
    select.Select,
    cg.Component,
    cg.Parented.template(LitterRobot4Component),
)
LitterRobot4PanelBrightnessSelect = litter_robot4_ns.class_(
    "LitterRobot4PanelBrightnessSelect",
    select.Select,
    cg.Component,
    cg.Parented.template(LitterRobot4Component),
)

_SELECT_OPTIONS = {
    "night_light_mode": ["Off", "On", "Auto"],
    "night_light_brightness": ["Low", "Medium", "High"],
    "panel_brightness": ["Low", "Medium", "High"],
}


def _select_schema(class_, *, icon=None):
    kwargs = {}
    if icon is not None:
        kwargs["icon"] = icon
    return (
        select.select_schema(class_, entity_category=ENTITY_CATEGORY_CONFIG, **kwargs)
        .extend(
            {
                cv.GenerateID(CONF_LITTER_ROBOT4_ID): cv.use_id(LitterRobot4Component),
            }
        )
        .extend(cv.COMPONENT_SCHEMA)
    )


CONFIG_SCHEMA = cv.typed_schema(
    {
        "night_light_mode": _select_schema(
            LitterRobot4NightLightModeSelect, icon="mdi:lightbulb-night"
        ),
        "night_light_brightness": _select_schema(
            LitterRobot4NightLightBrightnessSelect, icon="mdi:brightness-6"
        ),
        "panel_brightness": _select_schema(
            LitterRobot4PanelBrightnessSelect, icon="mdi:led-on"
        ),
    }
)


async def to_code(config):
    options = _SELECT_OPTIONS[config[CONF_TYPE]]
    parent = await cg.get_variable(config[CONF_LITTER_ROBOT4_ID])
    var = await select.new_select(config, options=options)
    await cg.register_component(var, config)
    await cg.register_parented(var, parent)
