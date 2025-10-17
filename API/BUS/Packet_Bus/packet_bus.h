/*
 * packet_bus.h
 *
 *  Created on: 8 de set. de 2025
 *      Author: diego.marinho
 */

#ifndef API_BUS_PACKET_BUS_PACKET_BUS_H_
#define API_BUS_PACKET_BUS_PACKET_BUS_H_

#include "cy_pdl.h"
#include "packet.h"

typedef enum {
	TECLADO_WORK = 0x00,
	TECLADO_REGISTER = 0x01,
} function_TECLADO_t;


typedef enum TecladoCmd_e{
	TECLADOCMD_REGISTER = 1,
	TECLADOCMD_COMMIT,
	TECLADOCMD_ADD_INST,
	TECLADOCMD_ADD_COMUM,
	TECLADOCMD_AUTHENTICATE,
	TECLADOCMD_ARM_DISARM,
	TECLADOCMD_GET_NOME,
	TECLADOCMD_UPDATE_NOME,
	TECLADOCMD_ADD_CONTROL,
	TECLADOCMD_REGISTER_CONTROL,
	TECLADOCMD_ADD_PARTITION,
	TECLADOCMD_GET_PARTITION,

	TECLADOCMD_UNKNOWN = 0xFF,

}TecladoCmd_e;

typedef struct __attribute__ ((__packed__)){
	uint8_t id;
}register_teclado_c;

typedef struct __attribute__ ((__packed__)){
	uint32_t code;
}add_instalador_teclado_c;

typedef struct __attribute__ ((__packed__)){
	uint32_t code_requester;
	uint32_t code;
}add_user_teclado_c;

typedef struct __attribute__ ((__packed__)){
	uint32_t code_requester;
	uint8_t particao;
	uint8_t arm_disarm;
}arm_user_teclado_c;

typedef struct __attribute__ ((__packed__)){
	uint8_t indice_user;
}get_name_teclado_c;

typedef struct __attribute__ ((__packed__)){
	uint8_t indice_user;
	uint8_t name[16];
}update_name_teclado_c;

// Monster unions
typedef union{
	register_teclado_c registerTeclado_c;
	add_instalador_teclado_c add_instaladorTeclado_c;
	add_user_teclado_c add_userTeclado_c;
	arm_user_teclado_c arm_userTeclado_c;
	get_name_teclado_c get_nameTeclado_c;
	update_name_teclado_c update_nameTeclado_c;

	struct{
		uint8_t raw[220];
		uint16_t len;
	};
}pck_teclado_cmd_u;



typedef struct{
	    TecladoCmd_e Command;
        uint32_t timestamp;
        uint8_t data[250];
        uint8_t crc;
        uint16_t dLen;
}packet_struct_teclado_t;

typedef struct{
	TecladoCmd_e cmd;
	uint32_t timestamp;
	pck_teclado_cmd_u data;
	uint8_t properties;
}packet_teclado_void_t;

typedef struct {
    uint8_t start;
    uint8_t size;
    uint8_t id;
    uint8_t address;
    uint8_t function;
    uint8_t iv[16];
    uint8_t data[256];
    uint8_t cks;
    uint8_t stop;
} packet_teclado_t;

typedef struct{
	uint8_t id;
	uint8_t addr;
	uint8_t function;
    uint8_t* key;
    uint8_t *inData;
    uint16_t inLen;
}packet_build_params_teclado_t;



packet_error_e packet_get_and_encrypt_teclado(packet_teclado_void_t *in, uint8_t *key, uint8_t id, uint8_t addr, uint8_t function, packet_teclado_t *out, uint8_t *serialized, uint8_t *sLen);
uint8_t packet_buidPacket_teclado(packet_teclado_void_t *pck, uint8_t *data);
packet_error_e packet_data_mount_teclado(packet_build_params_teclado_t *params, packet_teclado_t *pck, uint8_t* serial, uint8_t *len);
packet_error_e packet_data_demount_teclado(uint8_t *datain, uint16_t len, packet_teclado_t *packet);
packet_error_e packet_decrypt_and_get_teclado(packet_teclado_t *in, uint8_t *key, packet_teclado_void_t *out);
packet_error_t packet_parse_data_teclado(uint8_t *data, uint16_t len, packet_struct_teclado_t *pck);

#endif /* API_BUS_PACKET_BUS_PACKET_BUS_H_ */
