#ifndef __PACKET_H__
#define __PACKET_H__

#include <stdint.h>

#define PREAMBLE_LEN         1
#define HEADER_LEN           2
#define CRC_SIZE             2
#define PACKET_SIZE(payload) (payload + PREAMBLE_LEN + HEADER_LEN + CRC_SIZE + 1)

#define NULL_COMMAND               0
#define COMANDO_IMPOSTA_USCITA     0x10
#define IMPOSTA_DAC                0x11
#define COMANDO_IMPOSTA_LED        0x12
#define IMPOSTA_GETTONIERA         0x13
#define ENTRA_IN_TEST              0x14
#define ESCI_DAL_TEST              0x15
#define OFFSET_PRESSIONE           0x16
#define COMANDO_OFFSET_DAC         0x17
#define COMANDO_AZZERA_CREDITO     0x18
#define COMANDO_AZZERA_LITRI       0x19
#define START_LAVAGGIO             0x20
#define ESEGUI_STEP                0x21
#define STOP_LAVAGGIO              0x22
#define COMANDO_APRI_OBLO          0x23
#define COMANDO_CHIUDI_OBLO        0x24
#define PAUSA_LAVAGGIO             0x25
#define COMANDO_MODIFICA_PARAMETRI 0x26
#define CONTROLLO_SAPONE           0x27
#define COLPO_SAPONE               0x28
#define RILASCIA_SAPONI            0x29
#define ESCLUDI_SAPONE             0x2A
#define COMANDO_FORZA_SCARICO      0x2B
#define COLPO_SCARICO              0x2C
#define COMANDO_STATO_PAGAMENTO    0x2D

#define COMANDO_AZZERA_ALLARMI 0x33
#define COMANDO_DEBUG          0x34

#define COMANDO_LEGGI_TEST          0x01
#define COMANDO_PRESENTAZIONI       0x05
#define COMANDO_LEGGI_STATO         0x07
#define COMANDO_IMPOSTA_ORA         0x08
#define COMANDO_LEGGI_NUMERO_EVENTI 0x09
#define COMANDO_LEGGI_EVENTI        0x0A
#define COMANDO_LEGGI_STATISTICHE   0x0B

#define COMANDO_SCRIVI_PARMAC 0x30

#define COMANDO_ACK  0x80
#define COMANDO_NACK 0xF0
#define WRONG_CRC    0xF1
#define ERROR        0xF2

#define UNKNOWN_COMMAND 0x01
#define WRONG_FORMAT    0x02
#define WRONG_VALUE     0x03
#define IMPOSSIBLE      0x04

#define NACK_SCARICO_NECESSARIO 0x10

#define build_command_packet(buffer, comando) build_packet(buffer, comando, NULL, 0)
#define unpack_packet(buffer, packet)         build_packet(buffer, packet.command, packet.data, packet.data_length)

typedef struct {
    uint16_t data_length;
    uint8_t  command;
    uint8_t *data;
    uint8_t  crc_error;
} packet_t;

int elaborate_data(uint8_t *data, int len, packet_t *packet, int *forward);
int build_ack_packet(uint8_t *buffer, uint8_t comando, uint8_t *data, int len);
int build_nack_packet(uint8_t *buffer, uint8_t comando, uint8_t code);
int build_error_packet(uint8_t *buffer, char *msg);
int build_packet(uint8_t *buffer, uint8_t comando, uint8_t *data, int len);



#endif