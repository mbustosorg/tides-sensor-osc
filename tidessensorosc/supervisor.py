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
import datetime
import gc
import json
import logging
import subprocess
from logging.handlers import RotatingFileHandler

from tidessensorosc.display_animations import State
from tidessensorosc.earth_data.earth_data import tide_level, lights_out, current_sunset

try:
    from gpiozero import DigitalOutputDevice, DigitalInputDevice
except ImportError:
    from gpiozero_sim import DigitalOutputDevice, DigitalInputDevice
from pythonosc import osc_message_builder
from pythonosc import udp_client
from pythonosc.dispatcher import Dispatcher
from pythonosc.osc_server import AsyncIOOSCUDPServer
from yoctopuce.yocto_voltage import *
from yoctopuce.yocto_watchdog import *

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

file_handler = RotatingFileHandler('tides_supervisor.log', maxBytes=20000, backupCount=5)
file_handler.setLevel(logging.INFO)
file_handler.setFormatter(log_format)
logger.addHandler(file_handler)

supervision = None
led_play = None
voltage_sensor_name = None
watchdog = None
battery_state_pin = None
charger_pin = None
charger_state_pin = None
power_pin = None
sound_pin = None

BACKGROUND_RUN_INDEX = '/LEDPlay/player/backgroundRunIndex'
BACKGROUND_MODE = '/LEDPlay/player/backgroundMode'
FOREGROUND_RUN_INDEX = '/LEDPlay/player/foregroundRunIndex'

last_external = None


def handle_background_run_index(unused_addr, index, external=True):
    """ Process the BACKGROUND_RUN_INDEX message """
    global last_external

    if not power_pin.value:
        handle_power_on()
        handle_sound_on()
    logger.info('{} {}'.format(BACKGROUND_RUN_INDEX, index))
    msg = osc_message_builder.OscMessageBuilder(address=BACKGROUND_RUN_INDEX)
    msg.add_arg(index)
    led_play.send(msg.build())
    if external:
        last_external = datetime.datetime.now()


def handle_background_mode(unused_addr, index):
    """ Process the BACKGROUND_MODE message """
    logger.info('{} {}'.format(BACKGROUND_MODE, index))
    msg = osc_message_builder.OscMessageBuilder(address=BACKGROUND_MODE)
    msg.add_arg(index)
    led_play.send(msg.build())


def handle_foreground_run_index(unused_addr, index, exernal=True):
    """ Process the FOREGROUND_RUN_INDEX message """
    logger.info('{} {}'.format(FOREGROUND_RUN_INDEX, index))
    msg = osc_message_builder.OscMessageBuilder(address=FOREGROUND_RUN_INDEX)
    if index == 1:
        msg.add_arg(4) #
    elif index == 2:
        msg.add_arg(3) #
    elif index == 3:
        msg.add_arg(2) #
    elif index == 4:
        msg.add_arg(5)
    elif index == 5:
        msg.add_arg(1)
    elif index == 6:
        msg.add_arg(7) #
    elif index == 7:
        msg.add_arg(6) #
    else:
        msg.add_arg(index)
    led_play.send(msg.build())
    #subprocess.Popen(['aplay', '/home/pi/tides-sensor-osc/tidessensorosc/audio_data/piano2.wav', '-Dplug:' + str(index)])


def handle_power_on(unused_addr=None, index=None):
    logger.info('power on')
    power_pin.on()


def handle_sound_on(unused_addr=None, index=None):
    logger.info('sound on')
    sound_pin.on()


def handle_power_off(unused_addr=None, index=None):
    logger.info('power off')
    power_pin.off()


def handle_sound_off(unused_addr=None, index=None):
    logger.info('sound off')
    sound_pin.off()


def handle_charger_on(unused_addr=None, index=None):
    logger.info('charger on')
    power_pin.off()
    charger_pin.off()


def handle_charger_off(unused_addr=None, index=None):
    logger.info('charger off')
    charger_pin.on()


def maintain_background_sound(current_pids, track):
    """ Ensure that the background sound is running """
    try:
        if current_pids:
            pid_0 = current_pids[0]
            if pid_0.poll() is not None:
                pid_0 = subprocess.Popen(['aplay', '/home/pi/tides-sensor-osc/tidessensorosc/audio_data/' + track + '.wav', '-Dplug:outstereo0'])
            pid_1 = current_pids[1]
            if pid_1.poll() is not None:
                pid_1 = subprocess.Popen(['aplay', '/home/pi/tides-sensor-osc/tidessensorosc/audio_data/' + track + '.wav', '-Dplug:outstereo2'])
            gc.collect()
        else:
            pid_0 = subprocess.Popen(['aplay', '/home/pi/tides-sensor-osc/tidessensorosc/audio_data/' + track + '.wav', '-Dplug:outstereo0'])
            pid_1 = subprocess.Popen(['aplay', '/home/pi/tides-sensor-osc/tidessensorosc/audio_data/' + track + '.wav', '-Dplug:outstereo2'])
        return pid_0, pid_1
    except:
        return None, None


async def main_loop(ledplay_startup, disable_sun, disable_sound):
    """ Main execution loop """
    global last_external

    last_voltage = 0.0
    current_tide_level = 0
    current_state = State.STOPPED
    current_pids = []
    handle_background_mode(None, 0)
    handle_power_off()
    handle_sound_off()
    handle_charger_off()

    last_battery_state = battery_state_pin.value
    logger.info('Battery state {}'.format(last_battery_state))
    last_charger_state = charger_state_pin.value
    logger.info('Charger state {}'.format(last_charger_state))

    """ Wait one minute for LED play to start up """
    await asyncio.sleep(ledplay_startup)

    while True:
        """ Health checks """
        if battery_state_pin.value != last_battery_state:
            last_battery_state = battery_state_pin.value
            logger.info('Battery state {}'.format(last_battery_state))
        if charger_state_pin.value != last_charger_state:
            last_charger_state = charger_state_pin.value
            logger.info('Charger state {}'.format(last_charger_state))
        if watchdog:
            watchdog.resetWatchdog()
        if not disable_sound:
            current_pids = maintain_background_sound(current_pids, '1')
        """ Check battery voltage """
        if voltage_sensor_name:
            voltage = int(YVoltage.FindVoltage(voltage_sensor_name).get_currentValue() * 10)
            if abs(int(voltage) - int(last_voltage) > 2):
                logger.info('Voltage: {}'.format(float(voltage) / 10.0))
                last_voltage = voltage
            if voltage < supervision['min_volts']:
                logger.info('Detecting under-volt, starting charger = {}'.format(voltage))
                handle_charger_on()
                for i in range(0, int(1.0 / 3.0 * float(supervision['charge_time']))):
                    await asyncio.sleep(3 * 60)
                    voltage = int(YVoltage.FindVoltage(voltage_sensor_name).get_currentValue() * 10) / 10.0
                    logger.info('Reset watchdog during charging, current voltage: {}'.format(float(voltage) / 10.0))
                    watchdog.resetWatchdog()
            elif charger_pin.value == 0:
                handle_charger_off()
        """ Check on/off timing"""
        if disable_sun:
            off = False
        else:
            off = lights_out(supervision['lights_on'], supervision['lights_off'])
        if last_external is not None:
            if (datetime.datetime.now() - last_external).seconds > 60:
                last_external = None
        elif off:
            if current_state != State.STOPPED:
                logger.info('Shutting down to level 11')
                handle_background_run_index(None, 11)  # Fade to black
                await asyncio.sleep(5)
                handle_background_mode(None, 0)
                handle_power_off()
                handle_sound_off()
                current_state = State.STOPPED
            elif power_pin.value:
                handle_power_off()
                handle_sound_off()
        else:
            level = int(tide_level())
            if current_state == State.STOPPED:
                handle_power_on()
                handle_sound_on()
                await asyncio.sleep(5)
                logger.info('Starting up to level {}'.format(level))
                handle_background_mode(None, 1)
                handle_background_run_index(None, level)
                current_state = State.RUNNING
            elif (current_state == State.RUNNING) and (current_tide_level != level):
                logger.info('Moving to level {}'.format(level))
                handle_background_run_index(None, level)
            current_tide_level = level

        await asyncio.sleep(1)


async def init_main(args, dispatcher, sensor_dispatcher):
    """ Initialization routine """
    loop = asyncio.get_event_loop()
    server = AsyncIOOSCUDPServer((args.ip, args.port), dispatcher, loop)
    transport, _ = await server.create_serve_endpoint()

    sensor_server = AsyncIOOSCUDPServer((args.controller_ip, args.controller_port), sensor_dispatcher, loop)
    server_transport, _ = await sensor_server.create_serve_endpoint()

    await main_loop(args.ledplay_startup, args.disable_sun, args.disable_sound)

    transport.close()


if __name__ == "__main__":

    os.system('hwclock -s')

    parser = argparse.ArgumentParser()
    parser.add_argument('--ip', default='192.168.4.1', help='The ip to listen on')
    parser.add_argument('--port', type=int, default=9999, help='The port to listen on')
    parser.add_argument('--controller_ip', default='192.168.4.1', help='The controller ip address')
    parser.add_argument('--controller_port', type=int, default=9998, help='The port that the controller is listening on')
    parser.add_argument('--ledplay_ip', default='192.168.4.1', help='The LED Play ip address')
    parser.add_argument('--ledplay_port', type=int, default=1234, help='The port that the LED Play application is listening on')
    parser.add_argument('--ledplay_startup', required=False, type=int, default=60, help='Time to wait before LEDPlay starts up')
    parser.add_argument('--config', required=False, type=str, default='tidessensorosc/supervision.json')
    parser.add_argument('--disable_sun', dest='disable_sun', action='store_true')
    parser.add_argument('--disable_sound', dest='disable_sound', action='store_true')
    parser.set_defaults(disable_sun=False, disable_sound=False)
    args = parser.parse_args()

    subprocess = subprocess.Popen(['ps', '-A'], stdout=subprocess.PIPE)
    output, error = subprocess.communicate()
    print(output)
#    target_process = "python"
#    for line in output.splitlines():
#        if target_process in str(line):
#            pid = int(line.split(None, 1)[0])
#            os.kill(pid, 9)

    with open(args.config, 'r') as file:
        supervision = json.load(file)
        battery_state_pin = DigitalInputDevice(supervision['battery_state_pin'], pull_up=True)
        charger_pin = DigitalOutputDevice(supervision['charger_pin'])
        charger_state_pin = DigitalInputDevice(supervision['charger_state_pin'], pull_up=True)
        power_pin = DigitalOutputDevice(supervision['power_pin'])
        sound_pin = DigitalOutputDevice(supervision['sound_pin'])

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
        if watchdog:
            watchdog.resetWatchdog()
        else:
            logger.error('No watchdog connected')

    led_play = udp_client.UDPClient(args.ledplay_ip, args.ledplay_port)

    dispatcher = Dispatcher()
    dispatcher.map('/LEDPlay/player/backgroundRunIndex', handle_background_run_index)
    dispatcher.map('/LEDPlay/player/backgroundMode', handle_background_mode)
    dispatcher.map('/LEDPlay/player/foregroundRunIndex', handle_foreground_run_index)
    dispatcher.map('/poweron', handle_power_on)
    dispatcher.map('/poweroff', handle_power_off)
    dispatcher.map('/chargeron', handle_charger_on)
    dispatcher.map('/chargeroff', handle_charger_off)
    dispatcher.map('/soundon', handle_sound_on)
    dispatcher.map('/soundoff', handle_sound_off)

    sensor_dispatcher = Dispatcher()
    sensor_dispatcher.map('/LEDPlay/player/foregroundRunIndex', handle_foreground_run_index)

    logger.info('Serving on {}:{}'.format(args.ip, args.port))
    logger.info('Current tide level is {}'.format(tide_level()))
    logger.info('Current sunset is {} UTC'.format(current_sunset()))

    loop = asyncio.get_event_loop()
    loop.run_until_complete(init_main(args, dispatcher, sensor_dispatcher))
