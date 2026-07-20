import esphome.codegen as cg
from esphome.components import uart
import esphome.config_validation as cv
from esphome.const import CONF_ID

CODEOWNERS = ["@Joseph-DiGiovanni"]
DEPENDENCIES = ["uart"]

FINAL_VALIDATE_SCHEMA = uart.final_validate_device_schema(
    "litter_robot4",
    baud_rate=256000,
    require_tx=True,
    require_rx=True,
    data_bits=8,
    parity="NONE",
    stop_bits=1,
)

litter_robot4_ns = cg.esphome_ns.namespace("litter_robot4")
LitterRobot4Component = litter_robot4_ns.class_(
    "LitterRobot4Component", uart.UARTDevice, cg.Component
)

CONF_LITTER_ROBOT4_ID = "litter_robot4_id"

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(LitterRobot4Component),
        }
    )
    .extend(cv.COMPONENT_SCHEMA)
    .extend(uart.UART_DEVICE_SCHEMA)
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)
    cg.add_global(litter_robot4_ns.using)
