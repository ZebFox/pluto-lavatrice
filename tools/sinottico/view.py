import PySimpleGUI as sg
from dataclasses import dataclass
from typing import Optional, List, Tuple
import time
import json

from .id import Id
from .channel import Channel
from .serialutils import serial_ports
from .messages import ControllerMessage


@dataclass
class ViewData:
    window: sg.Window
    port: Optional[str] = None
    connected: bool = False

    def avoid_list(self) -> List[str]:
        if self.port:
            return [self.port]
        else:
            return []

    def save_as_json(self, binaries: List[Tuple[int, str]]):
        with open('binaries.json', 'w') as f:
            json.dump(binaries, f)

    def load_json(self):
        try:
            with open('binaries.json', 'r') as f:
                binaries = json.load(f)
                for num, (add, binary) in enumerate(binaries):
                    self.window[(Id.ADDRESS, num)].update(f'0x{add:X}')
                    self.window[(Id.BINARY, num)].update(binary)
        except FileNotFoundError:
            self.window[(Id.ADDRESS, 0)].update('0x1000')
            self.window[(Id.BINARY, 0)].update('bootloader.bin')
            self.window[(Id.ADDRESS, 1)].update('0x8000')
            self.window[(Id.BINARY, 1)].update('partition-table.bin')
            self.window[(Id.ADDRESS, 2)].update('0x10000')
            self.window[(Id.BINARY, 2)].update('esp32-template.bin')
            pass


def main_window(channel: Channel):
    serials = serial_ports()

    layout = [[sg.Combo(serials, key=Id.SERIALS, size=(16, 1)),
               sg.Button('Connetti', key=Id.CONNECT)],
              [sg.TabGroup([[
                  sg.Tab('Collaudo', [[]], key=(Id.TAB, 0)),
                  sg.Tab('Programmazione', [
                      [sg.Input(size=(12, 1), key=(Id.ADDRESS, 0)), sg.Input(size=(
                          56, 1), key=(Id.BINARY, 0)), sg.FileBrowse(file_types=(('Binary files', '*.bin'),))],
                      [sg.Input(size=(12, 1), key=(Id.ADDRESS, 1)), sg.Input(size=(
                          56, 1), key=(Id.BINARY, 1)), sg.FileBrowse(file_types=(('Binary files', '*.bin'),))],
                      [sg.Input(size=(12, 1), key=(Id.ADDRESS, 2)), sg.Input(size=(
                          56, 1), key=(Id.BINARY, 2)), sg.FileBrowse(file_types=(('Binary files', '*.bin'),))],
                      [sg.Input(size=(12, 1), key=(Id.ADDRESS, 3)), sg.Input(size=(
                          56, 1), key=(Id.BINARY, 3)), sg.FileBrowse(file_types=(('Binary files', '*.bin'),))],
                      [sg.Button('Flash', key=Id.ESPTOOL)],
                      [sg.Multiline(size=(80, 20), key=Id.ESPTOOL_OUTPUT,
                                    auto_refresh=True, autoscroll=True)],
                  ], key=(Id.TAB, 1)),
              ]])],
              [sg.Text(key=Id.STATUS, size=(48, 2))]]
    w = sg.Window("Sinottico", layout, finalize=True)
    data = ViewData(window=w)
    data.load_json()

    while True:
        if (newports := serial_ports(avoid=data.avoid_list())) != serials:
            serials = newports
            w[Id.SERIALS].update(serials)

        event, values = w.read(timeout=0.1)

        if event in [None]:
            break
        elif event == Id.CONNECT:
            channel.send(ControllerMessage.CONNECT_TO(values[Id.SERIALS]))
        elif event == Id.ESPTOOL:
            try:
                binaries = [(int(w[fid].get(), base=0), w[sid].get()) for (fid, sid) in [
                    ((Id.ADDRESS, 0), (Id.BINARY, 0)),
                    ((Id.ADDRESS, 1), (Id.BINARY, 1)),
                    ((Id.ADDRESS, 2), (Id.BINARY, 2)),
                    ((Id.ADDRESS, 3), (Id.BINARY, 3)),
                ]]
                channel.send(ControllerMessage.FLASH(binaries))
                data.save_as_json(binaries)
            except ValueError:
                w[Id.STATUS].update('Input non valido!')

        def connected(port: str):
            data.window[Id.STATUS].update(f'Connesso a {port}!')
            data.port = port
            data.connected = True

        def disconnected():
            data.connected = False
            data.port = None
            data.window[Id.STATUS].update('Seriale disconnessa')

        if msg := channel.recv(timeout=0.01):
            msg.match(connected=connected,
                      error=lambda e: w[Id.STATUS].update(e),
                      esptool_output=lambda msg: w[Id.ESPTOOL_OUTPUT].print(
                          msg),
                      flash_done=lambda: w[Id.STATUS].update(
                          'Procedura terminata'),
                      disconnected=disconnected)

        [w[x].update(disabled=not data.connected)
         for x in [(Id.TAB, 0), Id.ESPTOOL]]

    w.close()
