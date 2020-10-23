import threading

from .controller import controller_task
from .view import main_window
from .channel import Channel


def main():
    v, c = Channel.pipe()

    t = threading.Thread(target=controller_task, args=(c,))
    t.daemon = True
    t.start()

    main_window(v)


if __name__ == '__main__':
    main()
