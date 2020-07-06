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
from unittest import TestCase
import time

from tidessensorosc.display_animations import DisplayAnimations, State


class TestAnimation(TestCase):

    def test_startup(self):
        """ Ensure start up """
        animations = DisplayAnimations()
        animations.initiate_startup()
        self.assertEqual(animations.state, State.STARTING_UP)

    def test_running(self):
        """ Ensure start up """
        animations = DisplayAnimations()
        animations.initiate_startup()
        time.sleep(15)
        animations.iterate_state()
        self.assertEqual(animations.state, State.RUNNING)

    def test_shutting_down(self):
        """ Ensure start up """
        animations = DisplayAnimations()
        animations.initiate_shutdown()
        self.assertEqual(animations.state, State.SHUTTING_DOWN)

    def test_shutdown(self):
        """ Ensure start up """
        animations = DisplayAnimations()
        animations.initiate_shutdown()
        time.sleep(15)
        animations.iterate_state()
        self.assertEqual(animations.state, State.STOPPED)
