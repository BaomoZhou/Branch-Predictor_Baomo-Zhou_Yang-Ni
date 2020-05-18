//========================================================//
//  predictor.c                                           //
//  Source file for the Branch Predictor                  //
//                                                        //
//  Implement the various branch predictors below as      //
//  described in the README                               //
//========================================================//
#include <stdio.h>
#include <math.h>
#include "predictor.h"

//
// TODO:Student Information
//
const char *studentName = "Yang Ni";
const char *studentID   = "A53295687";
const char *email       = "y1ni@eng.ucsd.edu";

const char *studentName_2 = "Baomo Zhou";
const char *studentID_2   = "A53311173";
const char *email_2       = "bazhou@eng.ucsd.edu";

//------------------------------------//
//      Predictor Configuration       //
//------------------------------------//

// Handy Global for use in output routines
const char *bpName[4] = { "Static", "Gshare",
                          "Tournament", "Custom" };

int ghistoryBits; // Number of bits used for Global History
int lhistoryBits; // Number of bits used for Local History
int pcIndexBits;  // Number of bits used for PC index
int bpType;       // Branch Prediction Type
int verbose;

//------------------------------------//
//      Predictor Data Structures     //
//------------------------------------//

//
//TODO: Add your own Branch Predictor data structures here
//

// global variables for gshare
uint32_t *gshare_PHT;
uint32_t gshare_history;
uint32_t gshare_mask;
uint32_t gsPHT_idx;
uint32_t gsLow_pc;

// global variables for tournament
uint32_t local_mask;
uint32_t pc_mask;
uint32_t tnm_mask;
uint32_t local_his;
uint32_t global_his;
uint32_t choice;
uint32_t pc_index;
uint32_t *tnm_lBHT;
uint32_t *tnm_lPHT;
uint32_t *tnm_gPHT;
uint32_t *tnm_selector;

uint32_t local_pRes;
uint32_t global_pRes;

// global variables for custom BP
uint32_t pct_int_his;
uint32_t pct_mask;
int **pct_weights;
int *pct_array_his;
float pct_output;


//------------------------------------//
//        Predictor Functions         //
//------------------------------------//

// Initialize the predictor
//
void init_predictor() {
  //
  //TODO: Initialize Branch Predictor Data Structures
  //
switch (bpType) {
    case GSHARE:
	{
	  gshare_mask = 0;
	  for(int bit_loc=0;bit_loc<ghistoryBits;bit_loc++){
		  gshare_mask = 1 << bit_loc | gshare_mask;
	  }

	  gshare_history = 0; // initialize the global history to 0
	  int size = pow(2,ghistoryBits); // the number of entries in the PHT
	  gshare_PHT = (uint32_t*) malloc(sizeof(uint32_t) * size);
	  for(int idx=0;idx<size;idx++){
	    gshare_PHT[idx]=1; // initialize to weak not taken (1)
	  }
	  break;
	}
	case TOURNAMENT:
	{
	  global_his = 0;
	  local_his = 0;
	  tnm_mask = 0;
	  for(int bit_loc=0;bit_loc<ghistoryBits;bit_loc++){
		  tnm_mask = 1 << bit_loc | tnm_mask;
	  }
	  pc_mask = 0;
	  for(int bit_loc=0;bit_loc<pcIndexBits;bit_loc++){
		  pc_mask = 1 << bit_loc | pc_mask;
	  }
      local_mask = 0;
	  for(int bit_loc=0;bit_loc<lhistoryBits;bit_loc++){
		  local_mask = 1 << bit_loc | local_mask;
	  }

	  int size = pow(2,pcIndexBits);
      tnm_lBHT = (uint32_t*) malloc(sizeof(uint32_t) * size);
      for(int idx=0;idx<size;idx++){
        tnm_lBHT[idx] = 0;
      }

      size = pow(2,lhistoryBits);
      tnm_lPHT = (uint32_t*) malloc(sizeof(uint32_t) * size);
      for(int idx=0;idx<size;idx++){
        tnm_lPHT[idx] = 1;
      }

      size = pow(2,ghistoryBits);
      tnm_gPHT = (uint32_t*) malloc(sizeof(uint32_t) * size);
      for(int idx=0;idx<size;idx++){
        tnm_gPHT[idx] = 1;
      }

      size = pow(2,ghistoryBits);
      tnm_selector = (uint32_t*) malloc(sizeof(uint32_t) * size);
      for(int idx=0;idx<size;idx++){
        tnm_selector[idx] = 1;
      }
	  break;
	}
	case CUSTOM:
	{
	  ghistoryBits = 12;
      pcIndexBits = 1;
	  pct_mask = 0;

	  for(int bit_loc=0;bit_loc<pcIndexBits;bit_loc++){
		  pct_mask = (1 << bit_loc) | pct_mask;
	  }
	  int size = pow(2,pcIndexBits);
	  pct_weights = (int**)malloc(sizeof(int*) * size);
	  int node_num = ghistoryBits + 1;
	  for (int idx_1=0;idx_1<size;idx_1++) {
		  pct_weights[idx_1] = (int*)malloc(sizeof(int) * node_num);
		  for (int idx_2=0;idx_2<node_num;idx_2++) {
			  pct_weights[idx_1][idx_2] = 0;
		  }
	  }

	  pct_array_his = (int*)malloc(sizeof(int) * ghistoryBits);
	  for (int idx=0;idx<ghistoryBits;idx++) {
		  pct_array_his[idx] = -1;
	  }
	  break;
	}
	default:
	  break;
  }
}

void arr_to_int() {
	pct_int_his = 0;
	for (int idx=0;idx<ghistoryBits;idx++) {
		pct_int_his = pct_int_his << 1;
		if (pct_array_his[idx] == 1) {
			pct_int_his += 1;
		}
	}
}

// Make a prediction for conditional branch instruction at PC 'pc'
// Returning TAKEN indicates a prediction of taken; returning NOTTAKEN
// indicates a prediction of not taken
//
uint8_t make_prediction(uint32_t pc){
  //
  //TODO: Implement prediction scheme
  //
  uint32_t prediction;
  // Make a prediction based on the bpType
  switch (bpType) {
    case STATIC:
      return TAKEN;
    case GSHARE:
	{
	  gsLow_pc = pc & gshare_mask; // get lower bits of the pc address
	  gsPHT_idx = gsLow_pc ^ gshare_history;
	  prediction = gshare_PHT[gsPHT_idx];
	  if ((prediction > 3) || (prediction < 0))
		return -1;
	  else if (prediction > 1)
		return TAKEN;
	  else
		return NOTTAKEN;
	}
    case TOURNAMENT:
	{
      pc_index = pc_mask & pc;
      local_his = tnm_lBHT[pc_index];
      choice = tnm_selector[global_his];
      if(choice < 2){
        prediction = tnm_gPHT[global_his];
      }
      else{
        prediction = tnm_lPHT[local_his];
      }
      if(prediction < 2){
        return NOTTAKEN;
      }
      else{
        return TAKEN;
      }
	}
    case CUSTOM:
	{
	  arr_to_int();
	  int masked_pc = pc & pct_mask;
	  int masked_history = pct_int_his & pct_mask;
	  int weight_idx = masked_pc ^ masked_history;
      printf("The first index is: %d\n", weight_idx);
	  pct_output = pct_weights[weight_idx][0];

	  for (int idx=0;idx<ghistoryBits;idx++) {
		  //printf("The first index is: %d, the second one is: %d\n", weight_idx, idx);
		  pct_output += pct_weights[weight_idx][idx+1] * pct_array_his[idx];
	  }

	  if (pct_output < 0) {
		  return NOTTAKEN;
	  }
	  else return TAKEN;
	  break;
	}
    default:
      break;
  }

  // If there is not a compatable bpType then return NOTTAKEN
  return NOTTAKEN;
}

// Train the predictor the last executed branch at PC 'pc' and with
// outcome 'outcome' (true indicates that the branch was taken, false
// indicates that the branch was not taken)
//
void train_predictor(uint32_t pc, uint8_t outcome) {
  //
  //TODO: Implement Predictor training
  //
  switch(bpType){
    case GSHARE:
	  if(outcome == TAKEN){
		if(gshare_PHT[gsPHT_idx]<3)
	      gshare_PHT[gsPHT_idx]++;
	  } else {
		if(gshare_PHT[gsPHT_idx]>0)
	      gshare_PHT[gsPHT_idx]--;
	  }
	  gshare_history = (gshare_history<<1) | outcome;
	  gshare_history &= gshare_mask;
	  break;

	case TOURNAMENT:
	  pc_index = pc_mask & pc;
      local_his = tnm_lBHT[pc_index];
      choice = tnm_selector[global_his];

	  local_pRes = tnm_lPHT[local_his];
      if(local_pRes<2){
        local_pRes = NOTTAKEN;
      }
      else{
        local_pRes = TAKEN;
      }

      global_pRes = tnm_gPHT[global_his];
      if(global_pRes<2){
        global_pRes = NOTTAKEN;
      }
      else{
        global_pRes = TAKEN;
      }

      if(choice < 3 && local_pRes == outcome && global_pRes != outcome){
        tnm_selector[global_his]++;
      }
      else if(choice > 0 && global_pRes == outcome && local_pRes != outcome){
        tnm_selector[global_his]--;
      }

      if(outcome == TAKEN){
        if(tnm_gPHT[global_his] < 3){
          tnm_gPHT[global_his]++;
        }
        if(tnm_lPHT[global_his] < 3){
          tnm_lPHT[local_his]++;
        }
      }
      else{
        if(tnm_gPHT[global_his] > 0){
          tnm_gPHT[global_his]--;
        }
        if(tnm_lPHT[global_his] > 0){
          tnm_lPHT[local_his]--;
        }
      }
      tnm_lBHT[pc_index] = (tnm_lBHT[pc_index] << 1) | outcome;
      tnm_lBHT[pc_index] &= local_mask;
      global_his = (global_his << 1 )| outcome;
      global_his &= tnm_mask;
	  break;

    case CUSTOM: {
      uint32_t addr_low = pc & pct_mask;
      uint32_t his_low = pct_int_his & pct_mask;
      uint32_t pct_index = addr_low ^ his_low;
      int t;
      if(outcome == TAKEN){
        t = 1;
      }
      else{
        t = -1;
      }
      int upper_bound = ceil(1.93*ghistoryBits+14);
      int lower_bound = -upper_bound;
      uint8_t prediction;
      if (pct_output < 0) {
        prediction = NOTTAKEN; //赋初始值的问题值得关注
      }
      else{
        prediction = TAKEN;
      }
      if((prediction != outcome) || ((pct_output >= lower_bound) && (pct_output <= upper_bound))){
        pct_weights[pct_index][0] += 1*t;
        for(int i = 1; i <= ghistoryBits; i++){
          pct_weights[pct_index][i] += t*pct_array_his[i-1];
        }
      }
      for(int i = ghistoryBits - 1; i>0; i--){
          pct_array_his[i] = pct_array_his[i-1];
      }
      pct_array_his[0] = t;
    }
	//printf("finished training");
	  break;
	default:
	  break;
  }




}
