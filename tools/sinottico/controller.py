from serial import Serial
from serial.serialutil import SerialException
from dataclasses import dataclass
from typing import Optional

from .channel import Channel
from .messages import ViewMessage


@dataclass
class ControllerData:
    port: Optional[Serial]
    channel: Channel


def controller_task(channel: Channel):
    data = ControllerData(port=None, channel=channel)

    while True:
        def connect_to(data: ControllerData, portName: str):
            if data.port:
                data.port.close()
            try:
                data.port = Serial(port=portName, baudrate=115200)
                data.channel.send(ViewMessage.CONNECTED())
            except SerialException:
                data.channel.send(ViewMessage.ERROR('Connessione fallita!'))

        if msg := channel.recv():
            msg.match(connect_to=lambda x: connect_to(data, x))
