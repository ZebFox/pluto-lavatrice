import PySimpleGUI as sg

from .id import Id
from .channel import Channel
from .serialutils import serial_ports
from .messages import ControllerMessage


def main_window(channel: Channel):
    serials = serial_ports()

    layout = [[sg.Combo(serials, key=Id.SERIALS, size=(16, 1)),
               sg.Button('Connetti', key=Id.CONNECT)],
              [sg.Text(key=Id.STATUS, size=(48, 2))]]
    w = sg.Window("Sinottico AirFilter", layout, finalize=True)

    while True:
        if serial_ports() != serials:
            serials = serial_ports()
            w[Id.SERIALS].update(serials)

        event, values = w.read(timeout=0.1)

        if event in [None]:
            break
        elif event == Id.CONNECT:
            channel.send(ControllerMessage.CONNECT_TO(values[Id.SERIALS]))

        if msg := channel.recv(timeout=0):
            msg.match(connected=lambda: w[Id.STATUS].update('Connesso!'),
                      error=lambda e: w[Id.STATUS].update(e))

    w.close()
