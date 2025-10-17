/*
 * registerUtils.c
 *
 *  Created on: 10 de mar. de 2025
 *      Author: diego.marinho
 */

#include "registerUtils.h"


uint8_t seg_fill_data(seg_data_t* pSeg, data_add_t* data)
{
	if (data != NULL) { // eh uma solicitacao por app
		memcpy(pSeg->uuid, data->uuid, 16);
		//memcpy(pSeg->aes, data->aes, 16);
		pSeg->uts_inic = data->uts_inic;
		pSeg->uts_delta = data->uts_delta;
		pSeg->min_inic = data->min_inic;
		pSeg->min_delta = data->min_delta;
		pSeg->d_acesso.u8 = data->days;
		pSeg->f_unico = 0;
		pSeg->f_inval = 0;

		return 1;
	}
	else {
		memset(pSeg->uuid, 0, 16);
		pSeg->uts_inic = 0;
		pSeg->uts_delta = 0;
		pSeg->min_inic = 0;
		pSeg->min_delta = 0;
		pSeg->d_acesso.u8 = 0;
		pSeg->f_unico = 0;
		pSeg->f_inval = 0;

		return 0;
	}
}

uint8_t seg_fill_data_remote(seg_data_t* pSeg, data_add_remote_t* data)
{
	if (data != NULL) { // eh uma solicitacao por app
		memcpy(pSeg->ID, data->uuid, 16);
		memcpy(pSeg->nome, data->name, 16);
//		memcpy(pSeg->aes, data->aes, 16);
//		pSeg->uts_inic = data->uts_inic;
//		pSeg->uts_delta = data->uts_delta;
//		pSeg->min_inic = data->min_inic;
//		pSeg->min_delta = data->min_delta;
//		pSeg->d_acesso.u8 = data->days;
		pSeg->f_unico = 0;
		pSeg->f_inval = 0;

		return 1;
	}

}

uint8_t seg_fill_data_user(seg_data_t* pSeg, data_add_user_t* data)
{
	if (data != NULL) { // eh uma solicitacao por app

		pSeg->Access_code = data->perm_user;
		pSeg->uts_inic = data->uts_inic;
		pSeg->uts_delta = data->uts_delta;
		pSeg->min_inic = data->min_inic;
		pSeg->min_delta = data->min_delta;
		pSeg->d_acesso.u8 = data->days;
		pSeg->resource = data->resource;
		//pSeg->Tecl_Code = data->senha_tecl;
		pSeg->f_unico = 0;
		pSeg->f_inval = 0;
		pSeg->resource_Button1 = data->resource_button1;
		pSeg->resource_Button2 = data->resource_button2;
		pSeg->resource_Button3 = data->resource_button3;
		pSeg->resource_Button4 = data->resource_button4;
		pSeg->type_resource = data->type_resource;
		pSeg->type_resource_Button1 = data->type_resource_Button1;
		pSeg->type_resource_Button2 = data->type_resource_Button2;
		pSeg->type_resource_Button3 = data->type_resource_Button3;
		pSeg->type_resource_Button4 = data->type_resource_Button4;

		return 1;
	}

}

uint8_t seg_fill_data_setor(seg_data_setor_t* pSeg, data_add_setor_t* data){
	if (data != NULL) {
		memcpy(pSeg->nome, data->nome, 16);
		pSeg->ID_setor = data->ID_Setor;
		pSeg->ID_partition = data->ID_partition;
		pSeg->trigger_type = data->trigger_type;
		pSeg->attributes_setor = data->attributes_setor;
		pSeg->type_setor = data->type_setor;
		pSeg->ID_PGM = data->ID_PGM;
	}

	return 1;
}

uint8_t seg_fill_data_partition(seg_data_partition_t* pSeg, data_add_partition_t* data){
	if (data != NULL) {
		memcpy(pSeg->nome, data->nome, 16);
		pSeg->ID_partition = data->ID_partition;
		pSeg->tempo_entrada = data->tempo_entrada;
		pSeg->tempo_saida = data->tempo_saida;
		pSeg->tempo_auto_arme = data->tempo_autoarme;
		pSeg->dias_arme_auto = data->dias_arme_auto;
		pSeg->dias_desarme_auto = data->dias_desarme_auto;
		pSeg->hora_arme_auto = data->hora_arme_auto;
		pSeg->hora_desarme_auto = data->hora_dearme_auto;
		pSeg->numero_auto_anular = data->numero_auto_anular;
		pSeg->tempo_setor_inteligente = data->tempo_setor_inteligente;
		pSeg->ID_PGM = data->ID_PGM;
		pSeg->numero_conta = data->numero_conta;
		pSeg->binario_flags = data->binario_flags;
	}

	return 1;
}

uint32_t perm_nova_get(uint32_t root_perm)
{
	uint32_t aux = 0xFFFFFFFF;
	int16_t idx = -1;
	uint32_t randomNum;

	do{
	    //hRng_random((uint8_t*)&aux, 4);
	    Cy_Cryptolite_Trng(CRYPTOLITE, &aux);



	    if (aux != root_perm || aux != 0){
	        uint64_t temp = aux;
	        _seg_fgroup_seek(MEM_CLASS_REMOTE, temp, &idx, NULL);
	    }
	    else{
	        idx = -1;
	    }
	}while (idx != -1);

	return aux;
}
