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

// global variables for custom BP

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
	case TOURNAMENT:
	  break;
	case CUSTOM:
	  break;
	default:
	  break;
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
	  gsLow_pc = pc & gshare_mask; // get lower bits of the pc address
	  gsPHT_idx = gsLow_pc ^ gshare_history;
	  prediction = gshare_PHT[gsPHT_idx];
	  if ((prediction > 3) || (prediction < 0))
		return -1;
	  else if (prediction > 1)
		return TAKEN;
	  else
		return NOTTAKEN;
	
    case TOURNAMENT:
	  break;
    case CUSTOM:
	  break;
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
	case TOURNAMENT:
	  break;
    case CUSTOM:
	  break;
	default:
	  break;
  }
  
  
}
