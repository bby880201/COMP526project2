#include "global.h"

extern int showRS(int);
extern char * bool(int);
extern char * map(int);

extern struct RSEntry  RS[ ];
extern unsigned REG_FILE[ ];
extern unsigned REG_TAG[];


// Input from FETCH stage
extern unsigned INSTRUCTION;
extern unsigned PC4;

// Output to FETCH stage
extern unsigned stallIF, branchFlag;
extern unsigned nextPC;


// Statistics
extern int numInstrComplete;
extern int  numBranchStallCycles, numHaltStallCycles;
extern int  numBranchDataStallCycles, numRSFullStallCycles;
extern int isHALT;  



/*******************************************************************************/
void issuestage()
{
        int job_num;
	job_num = ActivityArgSize(ME) - 1;
	
	while(1){
	  if (TRACE)
	    printf("In ISSUE  Stage at time %2.0f\n", GetSimTime());
	do_issue();
	ProcessDelay(1.0);	
	}
}


decode(int *opCode, int *srcReg1, int *srcReg2, int *destReg, int *offset) {
  unsigned instr;

  instr = INSTRUCTION;
  *opCode = (instr >> 26) & 0x3F; // INSTRUCTION[31:26]
  *srcReg1 = (instr >> 21) & 0x1F; // INSTRUCTION[25:21];
  *srcReg2 = (instr >> 16) & 0x1F; //INSTRUCTION[20:16];
  *destReg = (instr >> 11) & 0x1F; //INSTRUCTION[15:11];
  *offset =  instr & 0xFFFF; // //INSTRUCTION[15:0];
  if ( (instr >> 15) & 0x1)  //  sign extend offset
    *offset =0xFFFF0000 | *offset;   // 
  nextPC = PC4 + *offset;  
}
 
getFU(int op) {  // Returns the index of the next FU for that op 
                 // Does not check for free/busy status
  int i, fu;
  static int nextFU[NUM_FU] = {0};

  nextFU[op] = (nextFU[op]+1)%NUM_COPIES;
  fu =  nextFU[op] * NUM_FU + op;
  if (DEBUG2)
    printf("Selected FU: Copy %d  opcode: %d  FU Id: %d\n", nextFU[op], op, fu); 
  return(fu);
}


getFreeRS() {
  int i;
  
  for (i=0; i < NUM_RESERVATION_STATIONS; i++) 
    if (RS[i].free == TRUE)
      return(i);

  return(-1);
}



do_issue() {
  int opCode, srcReg1, srcReg2,destReg,offset;
  int rsindex;
  int fu;
    stallIF=FALSE;
    isHALT=FALSE;
    branchFlag=FALSE;
  


  decode(&opCode, &srcReg1, &srcReg2, &destReg, &offset); // Decode instruction
    
  // Handle NOP, HALT, BRANCH and BNEZ
    switch (opCode) {
   /********************  Handle NOP  ********************/
        case NOP:
            if (DEBUG)
                printf("\tInstruction is NOP. Will not issue into RS. Time: %5.2f\n", GetSimTime());
            return;

  /********************  Handle HALT  ********************/
        case HALT:
            stallIF=TRUE;
            isHALT=TRUE;
            numHaltStallCycles++;
            if (DEBUG)
                printf("\tInstruction is HALT. Will not issue into RS. Will assert stallIF. Time: %5.2f\n", GetSimTime());
            return;
  /********************  Handle BRANCH  ********************/
        case BRANCH:
            
            branchFlag=TRUE;
            numInstrComplete++;  // Branch Instruction completes in the ISSUE stage
            numBranchStallCycles++;
            if (DEBUG)
                printf("\tCompleted Instruction: %s. Number Instructions Completed: %d Time: %5.2f\n", "BRANCH", numInstrComplete, GetSimTime());
     
            return;
  /********************  Handle BNEZ  ********************/
        case BNEZ:
    // if BNEZ register  value not available stall for a cycle. Set stallIF and return
            if (REG_TAG[srcReg1]!=-1) {
                stallIF=TRUE;
                numBranchDataStallCycles++;  // Increment if  BNEZ  register is not available
                if (DEBUG)
                    printf("\tBNEZ source register %d not READY ! Setting IFstall. Time: %5.2f\n", srcReg1,  bool(stallIF), GetSimTime());
                return;
            }
            
    // if Branch is TAKEN
            numInstrComplete++;   // Taken BNEZ completes in the ISSUE stage
            if (REG_FILE[srcReg1]!=0) {
                branchFlag=TRUE;
                numBranchStallCycles++;
            }
       // if Branch is NOT TAKEN continue execution with next fetched instruction
            if (DEBUG)
                printf("\tCompleted Instruction: %s.  Time: %5.2f Number Instructions Completed: %d\n", "BNEZ", GetSimTime(), numInstrComplete);
            return;
  /*******************************************************************/
            
        default:
            
   
	  // Get index of free RS 
            if ( ( rsindex = getFreeRS()) == -1) {
                stallIF = TRUE;
                numRSFullStallCycles++;
                if (DEBUG)
                    printf("\tNo free Reservation Stations! Setting IFstall. Time: %5.2f\n", GetSimTime());
                return;
            }
             // Get Functional Unit
            fu = getFU(opCode);
            
  if (TRACE)
    printf("\tOPCODE: %s  Adding to RS index %d\n", map(fu),rsindex); 
 
  // Update fields of RS Entry

            RS[rsindex].fu=fu;
            RS[rsindex].busy=FALSE;
            RS[rsindex].free=FALSE;
            
            if (REG_TAG[srcReg1] != -1) {
                RS[rsindex].op1RDY = FALSE;
            } else {
                RS[rsindex].op1RDY=TRUE;
            }
            RS[rsindex].operand1=REG_FILE[srcReg1];
            RS[rsindex].tag1=REG_TAG[srcReg1];

            if (REG_TAG[srcReg2] != -1) {
                RS[rsindex].op2RDY = FALSE;
            } else {
                RS[rsindex].op2RDY=TRUE;
            }
            RS[rsindex].operand2=REG_FILE[srcReg2];
            
            if (opCode==LOADFP) {
                RS[rsindex].tag2=-1;
            } else {
                RS[rsindex].tag2=REG_TAG[srcReg2];
            }
            
            if (opCode!=STOREFP) {
                RS[rsindex].destReg=destReg;
            } else {
                RS[rsindex].destReg=0;
            }
            
  // Update Register File Tags
            if (opCode!=STOREFP) REG_TAG[destReg]=rsindex;
            return;
    }
 }
