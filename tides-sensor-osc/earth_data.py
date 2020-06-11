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
import pandas as pd
import os
import datetime
from dateutil.relativedelta import relativedelta

tides_data = pd.read_csv('earth_data/tidelevels_9414863.csv', parse_dates=['DateTime']).set_index('DateTime')
tides_data['decile'] = pd.cut(tides_data['Height'], 10, labels=False).apply(int)

seven_hours = relativedelta(hours=7)
sun_data = pd.read_csv('earth_data/sunriseSunset.csv', parse_dates=['sunrise', 'sunset'])
sun_data['sunrise'] = sun_data['sunrise'].apply(lambda x: x - seven_hours)
sun_data['sunset'] = sun_data['sunset'].apply(lambda x: x - seven_hours)
sun_data['date'] = sun_data['sunrise'].dt.date
sun_data = sun_data.set_index('date')


def tide_level():
    """ Current tide level decile """
    return tides_data[tides_data.index > datetime.datetime.now()].iloc[0]['decile']


def lights_out(on_offset: int, hard_off: str):
    """ Are we off now? """
    now = datetime.datetime.now().replace(year=2000)
    off_time = datetime.datetime.strptime(hard_off, '%H:%M')
    if now.time() > off_time.time():
        return True
    on_delta = relativedelta(minutes=on_offset)
    sunset = sun_data.loc[now.date()]
    if now < sunset['sunset'] + on_delta:
        return True
    return False
