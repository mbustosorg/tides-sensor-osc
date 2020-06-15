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
from enum import Enum
import datetime
import logging


class State(Enum):
    STARTING_UP = 1
    RUNNING = 2
    SHUTTING_DOWN = 3
    STOPPED = 4


LOGGER = logging.getLogger(__name__)


class DisplayAnimations:

    def __init__(self):
        self.controller = None
        self.transition_time = None
        self.state = State.STOPPED
        self.tide_level = 0

    def initiate_startup(self):
        """ Enable the startup sequence """
        LOGGER.info('Initiate startup')
        self.transition_time = datetime.datetime.now()
        self.state = State.STARTING_UP

    def initiate_shutdown(self):
        """ Enable the shutdown sequence """
        LOGGER.info('Initiate shutdown')
        self.transition_time = datetime.datetime.now()
        self.state = State.SHUTTING_DOWN

    def iterate_state(self):
        """ Iterate the states that each sequence goes through """
        if self.state == State.STARTING_UP:
            if (datetime.datetime.now() - self.transition_time).seconds > 10:
                LOGGER.info('Transitioning to running')
                self.state = State.RUNNING
        elif self.state == State.SHUTTING_DOWN:
            if (datetime.datetime.now() - self.transition_time).seconds > 10:
                LOGGER.info('Transitioning to stopping')
                self.state = State.STOPPED
