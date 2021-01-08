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
import subprocess
import time


def run_background_audio():
    """ Start and maintain background audio """
    proc = subprocess.Popen(['aplay', 'audio_data/RipplingWater.wav', '-Dchannel1'])
    return_code = proc.returncode
    while True:
        if return_code == 0:
            proc = subprocess.Popen(['aplay', 'audio_data/RipplingWater.wav', '-Dchannel1'])
        proc.poll()
        return_code = proc.returncode
        time.sleep(1)
