"""
    Copyright (C) 2020 Mauricio Bustos (m@bustos.org)
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
"""

import argparse
import asyncio
import json
import logging

from gpiozero import DigitalOutputDevice, DigitalInputDevice
from pythonosc import osc_message_builder
from pythonosc import udp_client
from pythonosc.dispatcher import Dispatcher
from pythonosc.osc_server import AsyncIOOSCUDPServer
from yoctopuce.yocto_voltage import *
from yoctopuce.yocto_watchdog import *

from earth_data import tide_level, lights_out
from display_animations import DisplayAnimations, State


from logging.handlers import RotatingFileHandler


try:
    import sys
    sys.path.append("pydevd-pycharm.egg")
    import pydevd_pycharm

    pydevd_pycharm.settrace('10.0.1.22', port=12345, stdoutToServer=True, stderrToServer=True)
except:
    pass

FORMAT = '%(asctime)-15s %(message)s'
logging.basicConfig(format=FORMAT)
logger = logging.getLogger('supervisor')
logger.setLevel(logging.INFO)
log_format = logging.Formatter(FORMAT)

file_handler = RotatingFileHandler('supervisor.log', maxBytes=20000, backupCount=10)
file_handler.setLevel(logging.INFO)
file_handler.setFormatter(log_format)
logger.addHandler(file_handler)

supervision = None
controller = None
voltage_sensor_name = None
watchdog = None
battery_state_pin = None
charger_pin = None
charger_state_pin = None
power_pin = None
animations = DisplayAnimations()

BACKGROUND_RUN_INDEX = '/LEDPlay/player/backgroundRunIndex'
BACKGROUND_MODE = '/LEDPlay/player/backgroundMode'
FOREGROUND_RUN_INDEX = '/LEDPlay/player/foregroundRunIndex'


def handle_background_run_index(unused_addr, index):
    """ Process the BACKGROUND_RUN_INDEX message """
    logger.info(f'{BACKGROUND_RUN_INDEX} {index}')
    msg = osc_message_builder.OscMessageBuilder(address=BACKGROUND_RUN_INDEX)
    msg.add_arg(index)
    controller.send(msg.build())


def handle_background_mode(unused_addr, index):
    """ Process the BACKGROUND_MODE message """
    logger.info(f'{BACKGROUND_MODE} {index}')
    msg = osc_message_builder.OscMessageBuilder(address=BACKGROUND_MODE)
    msg.add_arg(index)
    controller.send(msg.build())


def handle_foreground_run_index(unused_addr, index):
    """ Process the FOREGROUND_RUN_INDEX message """
    logger.info(f'{FOREGROUND_RUN_INDEX} {index}')
    msg = osc_message_builder.OscMessageBuilder(address=FOREGROUND_RUN_INDEX)
    msg.add_arg(index)
    controller.send(msg.build())


def handle_power_on():
    logger.info(f'power on')
    power_pin.on()
    charger_pin.off()


def handle_power_off():
    logger.info(f'power off')
    power_pin.off()
    charger_pin.off()


def handle_charger_on():
    logger.info(f'charger on')
    power_pin.off()
    charger_pin.on()


def handle_charger_off():
    logger.info(f'charger off')
    power_pin.off()
    charger_pin.off()


async def main_loop():
    """ Main execution loop """
    last_battery_state: int = 0
    last_charger_state: int = 0

    while True:
        """ Health checks """
        if battery_state_pin.value != last_battery_state:
            last_battery_state = battery_state_pin.value
            logger.info(f'Battery state {last_battery_state}')
        if charger_state_pin.value != last_charger_state:
            last_charger_state = charger_state_pin.value
            logger.info(f'Charger state {last_charger_state}')
        if watchdog:
            watchdog.resetWatchdog()
        """ Check battery voltage """
        if voltage_sensor_name:
            voltage = YVoltage.FindVoltage(voltage_sensor_name).get_currentValue()
            if voltage < supervision['min_volts']:
                logger.info(f'Detecting under-volt, starting charger = {voltage}')
                handle_charger_on()
                await asyncio.sleep(supervision['charge_time'] * 1)
            else:
                if charger_pin.value == 1:
                    logger.info(f'Voltage = {voltage}')
                    handle_charger_off()
        """ Check on/off timing"""
        if lights_out(supervision['lights_on'], supervision['lights_off']) and \
                (animations.state != State.STOPPED) and \
                (animations.state != State.SHUTTING_DOWN):
            animations.initiate_shutdown()
        else:
            current_tide_level = int(tide_level())
            if (animations.state == State.STOPPED) and (animations.state != State.RUNNING) and (animations.state != State.STARTING_UP):
                handle_power_on()
                await asyncio.sleep(5)
                animations.initiate_startup()
            elif (animations.state == State.RUNNING) and (current_tide_level != animations.tide_level):
                logger.info(f'Moving to level {current_tide_level}')
                animations.tide_level = current_tide_level
                handle_background_run_index(None, current_tide_level)
        animations.iterate_state()

        await asyncio.sleep(1)


async def init_main(args, dispatcher):
    """ Initialization routine """
    loop = asyncio.get_event_loop()
    server = AsyncIOOSCUDPServer((args.ip, args.port), dispatcher, loop)
    transport, protocol = await server.create_serve_endpoint()

    await main_loop()

    transport.close()


if __name__ == "__main__":

    parser = argparse.ArgumentParser()
    #parser.add_argument('--ip', default='127.0.0.1', help='The ip to listen on')
    parser.add_argument('--ip', default='10.0.1.27', help='The ip to listen on')
    parser.add_argument('--port', type=int, default=9999, help='The port to listen on')
    parser.add_argument('--controller_ip', default='127.0.0.1', help='The controller ip address')
    parser.add_argument('--controller_port', type=int, default=9998, help='The port that the controller is listening on')
    args = parser.parse_args()

    with open('supervision.json', 'r') as file:
        supervision = json.load(file)
        battery_state_pin: DigitalInputDevice = DigitalInputDevice(supervision['battery_state_pin'])
        charger_pin: DigitalOutputDevice = DigitalOutputDevice(supervision['charger_pin'])
        charger_state_pin: DigitalInputDevice = DigitalInputDevice(supervision['charger_state_pin'])
        power_pin: DigitalOutputDevice = DigitalOutputDevice(supervision['power_pin'])

    errmsg = YRefParam()
    if YAPI.RegisterHub('usb', errmsg) != YAPI.SUCCESS:
        logger.error('YAPI init error ' + errmsg.value)
    else:
        voltage_sensor = YVoltage.FirstVoltage()
        if voltage_sensor is None:
            logger.error('No voltage sensor connected')
        else:
            voltage_sensor_name = voltage_sensor.get_module().get_serialNumber() + '.voltage1'
        watchdog = YWatchdog.FirstWatchdog()

    controller = udp_client.UDPClient(args.controller_ip, args.controller_port)
    animations.controller = controller

    dispatcher = Dispatcher()
    dispatcher.map('/LEDPlay/player/backgroundRunIndex', handle_background_run_index)
    dispatcher.map('/LEDPlay/player/backgroundMode', handle_background_mode)
    dispatcher.map('/LEDPlay/player/foregroundRunIndex', handle_foreground_run_index)
    dispatcher.map('/poweron', handle_power_on)
    dispatcher.map('/poweroff', handle_power_off)
    dispatcher.map('/chargeron', handle_charger_on)
    dispatcher.map('/chargeroff', handle_charger_off)

    logger.info(f'Serving on {args.ip}:{args.port}')

    asyncio.run(init_main(args, dispatcher))
