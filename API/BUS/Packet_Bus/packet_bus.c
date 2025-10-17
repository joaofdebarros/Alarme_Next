/*
 * packet_bus.c
 *
 *  Created on: 8 de set. de 2025
 *      Author: diego.marinho
 */

#include "packet_bus.h"

cy_stc_cryptolite_aes_state_t aes_state_teclado;
cy_stc_cryptolite_aes_buffers_t aesBuffers_teclado;
cy_en_cryptolite_status_t status_teclado;

////////////////////////////////Criptografia e enpacotamento//////////////////////

packet_error_e packet_get_and_encrypt_teclado(packet_teclado_void_t *in, uint8_t *key, uint8_t id, uint8_t addr, uint8_t function, packet_teclado_t *out, uint8_t *serialized, uint8_t *sLen){
    uint8_t enc[256], len;
    packet_build_params_teclado_t params;

    if (in == NULL || key == NULL || out == NULL){
        return false;
    }

    len = packet_buidPacket_teclado(in, enc);

    params.inData = enc;
    params.inLen = len;
    params.key = key;
    params.id = id;
    params.addr = addr;
    params.function = function;

    return packet_data_mount_teclado(&params, out, serialized, sLen);
}

uint8_t packet_buidPacket_teclado(packet_teclado_void_t *pck, uint8_t *data){
    uint8_t i;

    if (pck == NULL || data == NULL){
        return 0;
    }
    i = 0;
    data[i] = (uint8_t)pck->cmd;
    i += COMMAND_LEN;

    if (pck->timestamp > 0){
        memcpy(&(data[i]), &(pck->timestamp), TIMESTAMP_LEN);
        i += TIMESTAMP_LEN;
    }
    memcpy(&(data[i]), pck->data.raw, pck->data.len);
    i += pck->data.len;

    data[i] = crc8((uint8_t*)data, i);
    i += CRC_LEN;

    return i;
}

packet_error_e _packet_build_teclado(packet_build_params_teclado_t *params, packet_teclado_t *pck, uint8_t *serialized, uint8_t *sLen){
    uint16_t oLen;
    uint8_t i, size;
    cy_en_cryptolite_status_t cy_en_cryptolite_status;
    uint32_t randomNum;
    uint8_t iv_run[16];

    if (params == NULL || params->inData == NULL || pck == NULL){
        return PACKET_FAIL_UNKNOWN;
    }

    pck->start = 0x7E;
    pck->stop = 0x81;
    pck->id = params->id;
    pck->address = params->addr;
    pck->function = params->function;


    for (size_t i = 0; i < 16; i += 4) {
        if (Cy_Cryptolite_Trng(CRYPTOLITE, &randomNum) == CY_CRYPTOLITE_SUCCESS) {
            memcpy(&pck->iv[i], &randomNum, 4);
        } else {
            // Tratar erro se necessário
        }
    }


    memcpy(iv_run, pck->iv, 16);



    pck->size = ((params->inLen) <= 16) ? 16 :
	                                ((params->inLen) <= 32) ? 32 :
	                                ((params->inLen) <= 48) ? 48 : 0;


    if (params->key != NULL){

    	Cy_Cryptolite_Aes_Init(CRYPTOLITE, params->key, &aes_state_teclado, &aesBuffers_teclado);
    	cy_en_cryptolite_status = Cy_Cryptolite_Aes_Cfb(CRYPTOLITE,
    			CY_CRYPTOLITE_ENCRYPT, pck->size, iv_run, pck->data, params->inData, &aes_state_teclado);
        /* ... check for errors... */
        Cy_Cryptolite_Aes_Free(CRYPTOLITE, &aes_state_teclado);

        if(cy_en_cryptolite_status != CY_CRYPTOLITE_SUCCESS){
        	return PACKET_FAIL_DECRYPT;
        }

    }
    else{
        memcpy(pck->data, params->inData, params->inLen);
        pck->size = params->inLen;

    }

    //pck->size = params->inLen-6;


    if (serialized != NULL){
        size = 0;
        serialized[size++] = pck->start;

        serialized[size++] = params->inLen-6;

        serialized[size++] = pck->id;

        serialized[size++] = pck->address;

        serialized[size++] = pck->function;

        for (i=0; i<16; i++){
            serialized[size++] = pck->iv[i];
        }

        memcpy((void*)&serialized[size], pck->data, pck->size);
        size += pck->size;

        pck->cks = calculate_checksum(serialized, (size-5));
        serialized[size++] = pck->cks;

        serialized[size++] = pck->stop;

        if (sLen != NULL){
            *sLen = size;
        }
    }

    pck->size = params->inLen-6;

    return PACKET_OK;
}

packet_error_e packet_data_mount_teclado(packet_build_params_teclado_t *params, packet_teclado_t *pck, uint8_t* serial, uint8_t *len){
    packet_error_e e;

//    if (inData == NULL || outPacket == NULL || outLen == NULL){
//        return PACKET_FAIL_UNKNOWN;
//    }

    e = _packet_build_teclado(params, pck, serial, len);

    return e;
}

////////////////////////////////Decriptografia e desenpacotamento//////////////////////
packet_error_e packet_data_demount_teclado(uint8_t *datain, uint16_t len, packet_teclado_t *packet){
	uint16_t i, size, lHold;
	uint8_t checksum_validate;

    if (datain == NULL || packet == NULL){
        return PACKET_FAIL_UNKNOWN;
    }

    checksum_validate = 0x7E;

    // Transport all bytes to the struct
    size = 0;
    memset(packet, 0, sizeof(packet_teclado_t));

    packet->size = (datain[size++]);
    checksum_validate ^= packet->size;

    lHold = len;

    packet->id = (datain[size++]);
    checksum_validate ^= packet->id;

    packet->address = (datain[size++]);
    checksum_validate ^= packet->address;

    packet->function = (datain[size++]);
    checksum_validate ^= packet->function;


    for (i=0; i<16; i++){
        packet->iv[i] = datain[size++];
        checksum_validate ^= packet->iv[i];
    }

    memcpy(packet->data, &datain[size], len);
    size += lHold;

    for (i = 0; i < lHold; i++) {
      checksum_validate ^= packet->data[i];
    }

    packet->cks = (datain[size++]);
    checksum_validate = ~checksum_validate;

    packet->stop = (datain[size++]);

    if (packet->stop != 0x81){
    	return PACKET_FAIL_TAIL;
    }

    if (checksum_validate != packet->cks) {
      return PACKET_FAIL_UNKNOWN;
    }

    return PACKET_OK;
}

packet_error_e packet_decrypt_and_get_teclado(packet_teclado_t *in, uint8_t *key, packet_teclado_void_t *out){
    uint8_t dec[256];
    uint16_t len;
    packet_struct_teclado_t pck_str;
    packet_error_t r;
    cy_en_cryptolite_status_t cy_en_cryptolite_status;

    if (in == NULL || key == NULL || out == NULL){
        return PACKET_FAIL_UNKNOWN;
    }

	Cy_Cryptolite_Aes_Init(CRYPTOLITE, key, &aes_state_teclado, &aesBuffers_teclado);
	cy_en_cryptolite_status = Cy_Cryptolite_Aes_Cfb(CRYPTOLITE,
    		CY_CRYPTOLITE_DECRYPT, 144, in->iv, dec, in->data, &aes_state_teclado);
    /* ... check for errors... */
    Cy_Cryptolite_Aes_Free(CRYPTOLITE, &aes_state_teclado);

    if(cy_en_cryptolite_status != CY_CRYPTOLITE_SUCCESS){
    	return PACKET_FAIL_DECRYPT;
    }

    len = in->size;

    r = packet_parse_data_teclado(dec, len, &pck_str);

    out->cmd = pck_str.Command;
    out->timestamp = pck_str.timestamp;
    memcpy(out->data.raw, pck_str.data, pck_str.dLen);
    out->data.len = pck_str.dLen;

    return (packet_error_e)r;
}

packet_error_t packet_parse_data_teclado(uint8_t *data, uint16_t len, packet_struct_teclado_t *pck){
	uint16_t data_len, i;

    //sanity check
    if (data == NULL || pck == NULL)
        return PACKET_ERR_UNKNOWN;

    data_len = len;
    i=0;
    pck->Command = (TecladoCmd_e)data[i];
    i += COMMAND_LEN;


    if (data_len > 0){
        memcpy(pck->data, &(data[i]), data_len);
        i += data_len;
    }

    pck->crc = data[i];
    pck->dLen = data_len;

    if(crc8(data, (len+2)) != MAGIC_CHECK)
    		return PACKET_ERR_FAIL;

    return PACKET_ERR_OK;
}


