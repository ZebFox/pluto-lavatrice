from serial import Serial
from serial.serialutil import SerialException
import termios
from dataclasses import dataclass
from typing import Optional, Callable, List, Tuple
from minimalmodbus import Instrument, NoResponseError, InvalidResponseError
import threading
import time
import sys

from .channel import Channel
from .messages import ViewMessage
from .esptool import main as esptool_main


@dataclass
class ControllerData:
    modbus: Optional[Instrument]
    channel: Channel

    def send_command(self, fun: Callable[[Instrument], None]) -> bool:
        if not self.modbus:
            return False
        else:
            try:
                fun(self.modbus)
                time.sleep(0.05)
                return True
            except (NoResponseError, InvalidResponseError) as e:
                print('Errore di comunicazione:', e)
                return False
            except (termios.error, SerialException):
                self.modbus = None
                self.channel.send(ViewMessage.DISCONNECTED())


class SysRedirect:
    def __init__(self, channel: Channel):
        self.channel = channel

    def write(self, msg):
        msg = msg.replace('\r', '\n')
        self.channel.send(ViewMessage.ESPTOOL_OUTPUT(msg))

    def flush(self):
        pass

    def isatty(self):
        return True


def controller_task(channel: Channel):
    data = ControllerData(modbus=None, channel=channel)

    while True:

        def connect_to(portName: str):
            print('connecting to ', portName)
            if data.modbus:
                data.modbus.serial.close()
            try:
                data.modbus = Instrument(portName, 1)
                data.modbus.serial.baudrate = 115200
                data.modbus.serial.timeout = 0.1
                data.modbus.serial.rts = 0
                data.modbus.serial.dtr = 0
                print(data.modbus.serial.rts, data.modbus.serial.dtr)
                data.modbus.clear_buffers_before_each_transaction = True

                data.channel.send(ViewMessage.CONNECTED(portName))
            except SerialException:
                data.modbus = None
                data.channel.send(ViewMessage.ERROR('Connessione fallita!'))

        def flash(binaries: List[Tuple[int, str]]):
            def task():
                command = f'-p {data.modbus.serial.name} -b 460800 --before default_reset --after hard_reset write_flash --flash_mode dio --flash_size detect --flash_freq 40m '

                for (add, binary) in binaries:
                    command += f'0x{add:0X} {binary} '

                command = [x for x in command.split(' ') if len(x) > 0]
                esptool_main(command)

            old = sys.stdout
            t = threading.Thread(target=task)
            t.daemon = True

            sys.stdout = SysRedirect(data.channel)
            if data.modbus:
                data.modbus.serial.close()

            t.start()
            t.join()

            if data.modbus:
                data.modbus.serial.open()
            sys.stdout = old
            data.channel.send(ViewMessage.FLASH_DONE())

        if msg := channel.recv(timeout=0.1):
            msg.match(connect_to=connect_to, flash=flash)
