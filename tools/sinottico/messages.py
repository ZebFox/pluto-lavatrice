from adt import adt, Case


@adt
class ViewMessage:
    CONNECTED: Case
    ERROR: Case[str]


@adt
class ControllerMessage:
    CONNECT_TO: Case[str]
