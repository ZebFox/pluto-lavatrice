from queue import Queue, Empty
from typing import Any, Tuple


class Channel:
    @staticmethod
    def pipe() -> Tuple[Any, Any]:
        send: Queue = Queue()
        receive: Queue = Queue()
        return (Channel(send, receive), Channel(receive, send))

    def __init__(self, send: Queue, receive: Queue):
        self.sendq = send
        self.receiveq = receive

    def send(self, obj: Any):
        self.sendq.put(obj)

    def recv(self, **kwargs) -> Any:
        try:
            return self.receiveq.get(**kwargs)
        except Empty:
            return None

    def clear(self):
        while not self.sendq.empty():
            self.sendq.get()
        while not self.receiveq.empty():
            self.receiveq.get()