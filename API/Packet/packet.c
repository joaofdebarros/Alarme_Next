/*
 * packet.c
 *
 *  Created on: 24 de fev. de 2025
 *      Author: diego.marinho
 */


#include "packet.h"

#define PACKET_HEADER_LEN       sizeof(uint16_t)
#define PACKET_TAIL_LEN         sizeof(uint16_t)
#define PACKET_IV_LEN           16
#define PACKET_LENGTH_LEN       1
#define PACKET_VERSION_LEN      1
#define PACKET_ENCRYPTED_LEN    1
#define PACKET_ID_USER_LEN      1

cy_stc_cryptolite_aes_state_t aes_state;
cy_stc_cryptolite_aes_buffers_t aesBuffers;
cy_en_cryptolite_status_t status;

static packet_cfg_t packet_cfg;

packet_error_e _packet_build(packet_build_params_t *params, packet_t *pck, uint8_t *serialized, uint8_t *sLen){
//    hAes_error_e aesErr;
//    hRng_error_e rngErr;
    uint16_t oLen;
    uint8_t i, size;
    cy_en_cryptolite_status_t cy_en_cryptolite_status;
    uint32_t randomNum;
    uint8_t iv_run[16];

    if (params == NULL || params->inData == NULL || pck == NULL){
        return PACKET_FAIL_UNKNOWN;
    }

    //pck->header = packet_cfg.Header;
    //pck->tail = packet_cfg.Tail;

    pck->header = 0xAAAA;
    pck->tail = 0xBBBB;
    pck->id = params->puid;

    if(pck->id == 1){
    	memcpy(pck->id_job, params->jobID, 16);
    }


    for (size_t i = 0; i < PACKET_IV_LEN; i += 4) {
        if (Cy_Cryptolite_Trng(CRYPTOLITE, &randomNum) == CY_CRYPTOLITE_SUCCESS) {
            memcpy(&pck->iv[i], &randomNum, 4);
        } else {
            // Tratar erro se necessário
        }
    }

//    CY_ALIGN(4) uint8_t AesCfbIV[] =
//    {
//    	    0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36
//    };
    memcpy(iv_run, pck->iv, 16);


    //memset(params->inData+33, 0, 16); // adicionar um vetor para os dados e zerar depois dos dados


    pck->len = ((params->inLen) <= 16) ? 16 :
    		((params->inLen) <= 32) ? 32 :
    				((params->inLen) <= 48) ? 48 :
    						((params->inLen) <= 64) ? 64 :
    								((params->inLen) <= 80) ? 80 :
    										((params->inLen) <= 96) ? 96 :
    												((params->inLen) <= 112) ? 112 : 0;


    if (params->key != NULL){

    	Cy_Cryptolite_Aes_Init(CRYPTOLITE, params->key, &aes_state, &aesBuffers);
    	cy_en_cryptolite_status = Cy_Cryptolite_Aes_Cfb(CRYPTOLITE,
    			CY_CRYPTOLITE_ENCRYPT, pck->len, iv_run, pck->data, params->inData, &aes_state);
        /* ... check for errors... */
        Cy_Cryptolite_Aes_Free(CRYPTOLITE, &aes_state);

        if(cy_en_cryptolite_status != CY_CRYPTOLITE_SUCCESS){
        	return PACKET_FAIL_DECRYPT;
        }

    }
    else{
        memcpy(pck->data, params->inData, params->inLen);
        pck->len = params->inLen;

    }


    if (serialized != NULL){
        size = 0;
        for (i=0; i<PACKET_HEADER_LEN; i++){
            serialized[size++] = ((pck->header >> (8*i))&0xFF);
        }

        for (i=0; i<PACKET_LENGTH_LEN; i++){
        	if(params->version == 0){
        		serialized[size++] = (((params->inLen-6) >> (8*i))&0xFF);
        	}
        	else if(params->version == 1){
        		serialized[size++] = (((params->inLen-2) >> (8*i))&0xFF);
        	}

        }

        for (i=0; i<PACKET_ID_USER_LEN; i++){
            serialized[size++] = ((pck->id >> (8*i))&0xFF);
        }

        if(pck->id == 1){
        	for (i=0; i<PACKET_IV_LEN; i++){
        		serialized[size++] = pck->id_job[i];
        	}
        }

        for (i=0; i<PACKET_IV_LEN; i++){
            serialized[size++] = pck->iv[i];
        }
        memcpy((void*)&serialized[size], pck->data, pck->len);
        size += pck->len;
        for (i=0; i<PACKET_TAIL_LEN; i++){
            serialized[size++] = ((pck->tail >> (8*i))&0xFF);
        }
        if (sLen != NULL){
            *sLen = size;
        }
    }

    return PACKET_OK;
}

packet_error_e packet_data_mount(packet_build_params_t *params, packet_t *pck, uint8_t* serial, uint8_t *len){
    packet_error_e e;

//    if (inData == NULL || outPacket == NULL || outLen == NULL){
//        return PACKET_FAIL_UNKNOWN;
//    }

    e = _packet_build(params, pck, serial, len);

    return e;
}


packet_error_e packet_data_demount(uint8_t *datain, uint16_t len, packet_t *packet){
	uint16_t i, size, lHold;

    if (datain == NULL || packet == NULL){
        return PACKET_FAIL_UNKNOWN;
    }

    // Transport all bytes to the struct
    size = 0;
    memset(packet, 0, sizeof(packet_t));

    for (i=0; i<PACKET_LENGTH_LEN; i++){
        packet->len += (datain[size++]);
    }
    lHold = len;

    for (i=0; i<PACKET_ID_USER_LEN; i++){
        packet->id += (datain[size++]);
    }

    if( packet->id == 1){
        for (i=0; i<PACKET_IV_LEN; i++){
            packet->id_job[i] = datain[size++];
        }
    }

    for (i=0; i<PACKET_IV_LEN; i++){
        packet->iv[i] = datain[size++];
    }

    memcpy(packet->data, &datain[size], len);
    size += lHold;

    for (i=0; i<PACKET_TAIL_LEN; i++){
        packet->tail += (datain[size++] << (8*i));
    }

    if (packet->tail != 0xBBBB){
    	return PACKET_FAIL_TAIL;
    }

    return PACKET_OK;
}

packet_error_e packet_decrypt_and_get(packet_t *in, uint8_t *key, packet_void_t *out){
    uint8_t dec[256];
    uint16_t len;
    packet_struct_t pck_str;
    packet_error_t r;
    cy_en_cryptolite_status_t cy_en_cryptolite_status;

    if (in == NULL || key == NULL || out == NULL){
        return PACKET_FAIL_UNKNOWN;
    }

	Cy_Cryptolite_Aes_Init(CRYPTOLITE, key, &aes_state, &aesBuffers);
	cy_en_cryptolite_status = Cy_Cryptolite_Aes_Cfb(CRYPTOLITE,
    		CY_CRYPTOLITE_DECRYPT, 144, in->iv, dec, in->data, &aes_state);
    /* ... check for errors... */
    Cy_Cryptolite_Aes_Free(CRYPTOLITE, &aes_state);

    if(cy_en_cryptolite_status != CY_CRYPTOLITE_SUCCESS){
    	return PACKET_FAIL_DECRYPT;
    }

    len = in->len;

    r = packet_parse_data(dec, len, &pck_str);

    out->cmd = pck_str.Command;
    out->timestamp = pck_str.timestamp;
    memcpy(out->data.raw, pck_str.data, pck_str.dLen);
    out->data.len = pck_str.dLen;

    return (packet_error_e)r;
}

uint8_t packet_buidPacket(packet_void_t *pck, uint8_t *data){
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

packet_error_e packet_get_and_encrypt(packet_void_t *in, uint8_t version, uint16_t puid, uint8_t *key, packet_t *out, uint8_t *serialized, uint8_t *sLen, uint8_t *jobID){
    uint8_t enc[256], len;
    packet_build_params_t params;

    if (in == NULL || key == NULL || out == NULL){
        return PACKET_FAIL_UNKNOWN;
    }

    len = packet_buidPacket(in, enc);
    params.inData = enc;
    params.inLen = len;
    params.key = key;
    params.puid = puid;
    params.version = version;
    params.jobID = jobID;

    return packet_data_mount(&params, out, serialized, sLen);
}

packet_error_t packet_parse_data(uint8_t *data, uint16_t len, packet_struct_t *pck){
	uint16_t data_len, i;

    //sanity check
    if (data == NULL || pck == NULL)
        return PACKET_ERR_UNKNOWN;

    data_len = len;
    i=0;
    pck->Command = (AlarmCmd_e)data[i];
    i += COMMAND_LEN;

    if(pck->Command != ALARMCMD_NOTIFY){
        memcpy(&pck->timestamp, &data[i], 4);
        i += TIMESTAMP_LEN;
    }


    if (data_len > 0){
        memcpy(pck->data, &(data[i]), data_len);
        i += data_len;
    }

    pck->crc = data[i];
    pck->dLen = data_len;

    if(crc8(data, (len+NON_DATA_SIZE)) != MAGIC_CHECK)
    		return PACKET_ERR_FAIL;

    return PACKET_ERR_OK;
}


//////////////////////////////////////////////////packet relaciona ao Long Range///////////////////////////////////
packet_error_e packet_decrypt_and_get_LORA(packet_t *in, uint8_t *key, packet_void_t *out){
    uint8_t dec[256];
    uint16_t len;
    packet_struct_t pck_str;
    packet_error_t r;
    cy_en_cryptolite_status_t cy_en_cryptolite_status;

    if (in == NULL || out == NULL){
        return PACKET_FAIL_UNKNOWN;
    }

    // adicionar uma criprografia dedicada para o LORA
    if(key != NULL){
    	Cy_Cryptolite_Aes_Init(CRYPTOLITE, key, &aes_state, &aesBuffers);
    	cy_en_cryptolite_status = Cy_Cryptolite_Aes_Cfb(CRYPTOLITE,
        		CY_CRYPTOLITE_DECRYPT, 144, in->iv, dec, in->data, &aes_state);
        /* ... check for errors... */
        Cy_Cryptolite_Aes_Free(CRYPTOLITE, &aes_state);

        if(cy_en_cryptolite_status != CY_CRYPTOLITE_SUCCESS){
        	return PACKET_FAIL_DECRYPT;
        }
    }
    else{
    	memcpy(dec,in->data,in->len+2);
    }


    len = in->len;

    r = packet_parse_data_LORA(dec, len, &pck_str);

    out->cmd = pck_str.Command;
    out->timestamp = 0;
    memcpy(out->data.raw, pck_str.data, pck_str.dLen);
    out->data.len = pck_str.dLen;

    return (packet_error_e)r;
}

packet_error_t packet_parse_data_LORA(uint8_t *data, uint16_t len, packet_struct_t *pck){
	uint16_t data_len, i;

    //sanity check
    if (data == NULL || pck == NULL)
        return PACKET_ERR_UNKNOWN;

    data_len = len;
    i=0;
    pck->Command = (AlarmCmd_e)data[i];
    i += COMMAND_LEN;

//    memcpy(&pck->timestamp, &data[i], 4);
//    i += TIMESTAMP_LEN;

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

packet_error_e packet_data_demount_LORA(uint8_t *datain, uint16_t len, packet_t *packet){
	uint16_t i, size, lHold;

    if (datain == NULL || packet == NULL){
        return PACKET_FAIL_UNKNOWN;
    }

    // Transport all bytes to the struct
    size = 0;
    memset(packet, 0, sizeof(packet_t));

    for (i=0; i<PACKET_LENGTH_LEN; i++){
        packet->len += (datain[size++]);
    }
    lHold = len;

    for (i=0; i<PACKET_ID_USER_LEN; i++){
        packet->id += (datain[size++]);
    }

    memcpy(packet->data, &datain[size], len);
    size += lHold;

    for (i=0; i<PACKET_TAIL_LEN; i++){
        packet->tail += (datain[size++] << (8*i));
    }

    if (packet->tail != 0xBBBB){
    	return PACKET_FAIL_TAIL;
    }

    return PACKET_OK;
}

packet_error_e _packet_build_LORA(packet_build_params_t *params, packet_t *pck, uint8_t *serialized, uint8_t *sLen){
//    hAes_error_e aesErr;
//    hRng_error_e rngErr;
    uint8_t i, size;
    cy_en_cryptolite_status_t cy_en_cryptolite_status;
    uint8_t iv_run[16];

    if (params == NULL || params->inData == NULL || pck == NULL){
        return PACKET_FAIL_UNKNOWN;
    }

    //pck->header = packet_cfg.Header;
    //pck->tail = packet_cfg.Tail;

    pck->header = 0xAAAA;
    pck->tail = 0xBBBB;
    //pck->id = 0;


    memset(params->inData+33, 0, 16); // adicionar um vetor para os dados e zerar depois dos dados


    pck->len = ((params->inLen) <= 16) ? 16 :
	                                ((params->inLen) <= 32) ? 32 :
	                                ((params->inLen) <= 48) ? 48 : 0;


    if (params->key != NULL){//adicionar uma criptografia dedicada para o LORA

    	Cy_Cryptolite_Aes_Init(CRYPTOLITE, params->key, &aes_state, &aesBuffers);
    	cy_en_cryptolite_status = Cy_Cryptolite_Aes_Cfb(CRYPTOLITE,
    			CY_CRYPTOLITE_ENCRYPT, pck->len, iv_run, pck->data, params->inData, &aes_state);
        /* ... check for errors... */
        Cy_Cryptolite_Aes_Free(CRYPTOLITE, &aes_state);

        if(cy_en_cryptolite_status != CY_CRYPTOLITE_SUCCESS){
        	return PACKET_FAIL_DECRYPT;
        }

    }
    else{
        memcpy(pck->data, params->inData, params->inLen);
        pck->len = params->inLen;

    }


    if (serialized != NULL){
        size = 0;
        for (i=0; i<PACKET_HEADER_LEN; i++){
            serialized[size++] = ((pck->header >> (8*i))&0xFF);
        }

        for (i=0; i<PACKET_LENGTH_LEN; i++){
        	if(params->version == 0){
        		serialized[size++] = (((params->inLen-6) >> (8*i))&0xFF);
        	}
        	else if(params->version == 1){
        		serialized[size++] = (((params->inLen-2) >> (8*i))&0xFF);
        	}

        }

        for (i=0; i<PACKET_ID_USER_LEN; i++){
            serialized[size++] = ((pck->id >> (8*i))&0xFF);
        }

        memcpy((void*)&serialized[size], pck->data, pck->len);
        size += pck->len;
        for (i=0; i<PACKET_TAIL_LEN; i++){
            serialized[size++] = ((pck->tail >> (8*i))&0xFF);
        }
        if (sLen != NULL){
            *sLen = size;
        }
    }

    return PACKET_OK;
}

packet_error_e packet_data_mount_LORA(packet_build_params_t *params, packet_t *pck, uint8_t* serial, uint8_t *len){
    packet_error_e e;

//    if (inData == NULL || outPacket == NULL || outLen == NULL){
//        return PACKET_FAIL_UNKNOWN;
//    }

    e = _packet_build_LORA(params, pck, serial, len);

    return e;
}

packet_error_e packet_get_and_encrypt_LORA(packet_void_t *in, uint8_t version, uint8_t *key, packet_t *out, uint8_t *serialized, uint8_t *sLen){
    uint8_t enc[256], len;
    packet_build_params_t params;

    if (in == NULL || out == NULL){
        return PACKET_FAIL_UNKNOWN;
    }

    len = packet_buidPacket(in, enc);
    params.inData = enc;
    params.inLen = len;
    params.key = key;
    params.version = version;

    return packet_data_mount_LORA(&params, out, serialized, sLen);
}
