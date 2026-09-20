import esphome.codegen as cg
from esphome.components import uart
import esphome.config_validation as cv
from esphome.const import CONF_ID

CODEOWNERS = ["@Joseph-DiGiovanni"]
DEPENDENCIES = ["uart"]

FINAL_VALIDATE_SCHEMA = uart.final_validate_device_schema(
    "litter_robot3",
    baud_rate=19200,
    require_tx=True,
    require_rx=True,
    data_bits=8,
    parity="NONE",
    stop_bits=1,
)

litter_robot3_ns = cg.esphome_ns.namespace("litter_robot3")
LitterRobot3Component = litter_robot3_ns.class_(
    "LitterRobot3Component", uart.UARTDevice, cg.Component
)

CONF_LITTER_ROBOT3_ID = "litter_robot3_id"

LITTER_ROBOT3_DEVICE_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_LITTER_ROBOT3_ID): cv.use_id(LitterRobot3Component),
    }
)

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(LitterRobot3Component),
        }
    )
    .extend(cv.COMPONENT_SCHEMA)
    .extend(uart.UART_DEVICE_SCHEMA)
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)
    cg.add_global(litter_robot3_ns.using)
