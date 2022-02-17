#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "crc16-ccitt.h"
#include "packet.h"
#include <stdio.h>

static const uint8_t preamble[PREAMBLE_LEN] = {0xAA};

static int find_preamble(uint8_t *data, int begin, int end) {
    int i;
    for (i = begin; i < end; i++) {
        if (memcmp(&data[i], preamble, PREAMBLE_LEN) == 0)
            return i;
    }

    return -1;
}

int elaborate_data(uint8_t *data, int len, packet_t *packet, int *forward) {
    int stub  = 0;
    int start = find_preamble(data, 0, len);

    // Se non viene fornito un puntatore si usa una variabile locale che poi viene scartata
    if (!forward)
        forward = &stub;

    *forward = start;

    // Nessun preambolo trovato
    if (start < 0) {
        *forward = len;
        return 1;
    }

    // Preambolo e lunghezza presenti
    if (start + PREAMBLE_LEN + HEADER_LEN < len) {
        int plen = data[start + PREAMBLE_LEN] << 8 | data[start + PREAMBLE_LEN + 1];

        // Il pacchetto non e' ancora completo
        if (start + PREAMBLE_LEN + HEADER_LEN + plen + CRC_SIZE > len)
            return 2;


        if (plen > 0) {
            packet->command     = data[start + PREAMBLE_LEN + HEADER_LEN];
            packet->data_length = plen - 1;
        } else {
            packet->command     = NULL_COMMAND;
            packet->data_length = 0;
        }

        // CRC16 CCITT con polinomio 0x1021 e partenza 0
        uint16_t crccalc = crc16_ccitt((const unsigned char *)&data[start + PREAMBLE_LEN + HEADER_LEN], plen, 0);
        int      crci    = start + PREAMBLE_LEN + HEADER_LEN + plen;
        uint16_t crcread = data[crci] << 8 | data[crci + 1];

        packet->crc_error = (crccalc != crcread);
        if (packet->crc_error) {
            // Se c'e' stato un errore CRC scarta soltanto il primo byte (0xAA);
            // serve ad evitare di finire in loop perche' riconosco dei dati come
            // un preambolo
            *forward = start + 1;
        } else {
            *forward = start + PREAMBLE_LEN + HEADER_LEN + plen + CRC_SIZE;
        }
        // Allocazione dinamica del payload
        if (plen > 0) {
            packet->data = (uint8_t *)malloc(plen);
            memcpy(packet->data, &data[start + PREAMBLE_LEN + HEADER_LEN + 1], plen);
        } else {
            packet->data = NULL;
        }

        return 0;
    } else {
        return 3;
    }
}

int build_packet(uint8_t *buffer, uint8_t comando, uint8_t *data, int len) {
    uint16_t crc;

    buffer[0] = 0xAA;
    buffer[1] = (len + 1) >> 8;
    buffer[2] = (len + 1) & 0xFF;
    buffer[3] = comando;

    if (len > 0)
        memcpy(&buffer[4], data, len);

    crc                 = crc16_ccitt((const uint8_t *)&buffer[3], len + 1, 0);
    buffer[4 + len]     = crc >> 8;
    buffer[4 + len + 1] = crc & 0xFF;

    return len + 6;
}

int build_ack_packet(uint8_t *buffer, uint8_t comando, uint8_t *data, int len) {
    uint16_t tmp = len + 2;
    uint16_t crc;

    buffer[0] = 0xAA;
    buffer[1] = tmp >> 8;
    buffer[2] = tmp & 0xFF;
    buffer[3] = COMANDO_ACK;
    buffer[4] = comando;

    if (len > 0)
        memcpy(&buffer[5], data, len);

    crc                 = crc16_ccitt((const uint8_t *)&buffer[3], len + 2, 0);
    buffer[5 + len]     = crc >> 8;
    buffer[5 + len + 1] = crc & 0xFF;

    return len + 7;
}

int build_nack_packet(uint8_t *buffer, uint8_t comando, uint8_t code) {
    uint16_t crc;

    buffer[0] = 0xAA;
    buffer[1] = 0;
    buffer[2] = 3;
    buffer[3] = COMANDO_NACK;
    buffer[4] = comando;
    buffer[5] = code;

    crc       = crc16_ccitt((const uint8_t *)&buffer[3], 3, 0);
    buffer[6] = crc >> 8;
    buffer[7] = crc & 0xFF;

    return 8;
}

int build_error_packet(uint8_t *buffer, char *msg) {
    uint16_t crc;
    int      len = strlen(msg);

    buffer[0] = 0xAA;
    buffer[1] = len >> 8;
    buffer[2] = len & 0xFF;
    buffer[3] = ERROR;
    if (len > 0)
        memcpy(&buffer[4], msg, len);

    crc                 = crc16_ccitt((const uint8_t *)&buffer[3], len + 2, 0);
    buffer[4 + len]     = crc >> 8;
    buffer[4 + len + 1] = crc & 0xFF;

    return len + 6;
}