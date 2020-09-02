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
import cmd

from pythonosc import osc_message_builder
from pythonosc import udp_client

BACKGROUND_RUN_INDEX = '/LEDPlay/player/backgroundRunIndex'
BACKGROUND_MODE = '/LEDPlay/player/backgroundMode'
FOREGROUND_RUN_INDEX = '/LEDPlay/player/foregroundRunIndex'


class ProcessRunIndex(cmd.Cmd):

    prompt = '(RunIndex) '

    def do_run(self, arg):
        """ Process the BACKGROUND_RUN_INDEX message """
        if arg.isdigit():
            msg = osc_message_builder.OscMessageBuilder(address=BACKGROUND_RUN_INDEX)
            msg.add_arg(int(arg))
            controller.send(msg.build())

    def do_back(self, arg):
        """ Process the BACKGROUND_RUN_INDEX message """
        if arg.isdigit():
            msg = osc_message_builder.OscMessageBuilder(address=BACKGROUND_MODE)
            msg.add_arg(int(arg))
            controller.send(msg.build())


parser = argparse.ArgumentParser()
parser.add_argument('--controller_ip', default='192.168.4.1', help='The controller ip address')
parser.add_argument('--controller_port', type=int, default=9999, help='The port that the controller is listening on')
args = parser.parse_args()

controller = udp_client.UDPClient(args.controller_ip, args.controller_port)

ProcessRunIndex().cmdloop()
