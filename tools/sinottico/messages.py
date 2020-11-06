from adt import adt, Case
from typing import Tuple, List


@adt
class ViewMessage:
    CONNECTED: Case[str]
    DISCONNECTED: Case
    ERROR: Case[str]
    ESPTOOL_OUTPUT: Case[str]
    FLASH_DONE: Case
    

@adt
class ControllerMessage:
    CONNECT_TO: Case[str]
    FLASH: Case[List[Tuple[int, str]]]
