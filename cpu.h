#ifndef _APEX_CPU_H_
#define _APEX_CPU_H_
/**
 *  cpu.h
 *  Contains various CPU and Pipeline Data structures
 *
 *  Author :
 *  
 *  State University of New York, Binghamton
 */

enum
{
  F,
  DRF,
  IQ,
  LSQ,
  ROB,
  MEM1,
  MEM2,
  MEM3,
  INT1,
  INT2,
  MUL1,
  MUL2,
  MUL3,
  RET,
  NUM_STAGES
};

/* Format of an APEX instruction  */
typedef struct APEX_Instruction
{
  char opcode[128]; // Operation Code
  int rd;           // Destination Register Address
  int rs1;          // Source-1 Register Address
  int rs2;          // Source-2 Register Address
  int rs3;          // Source-3 Regsiter Address
  int imm;          // Literal Value
} APEX_Instruction;

/* Model of CPU stage latch */
typedef struct CPU_Stage
{
  int pc;           // Program Counter
  char opcode[128]; // Operation Code
  int rs1;          // Source-1 Register Address
  int rs2;          // Source-2 Register Address
  int rs3;          // Source-3 Regsiter Address
  int rd;           // Destination Register Address
  int imm;          // Literal Value
  int rs1_value;    // Source-1 Register Value
  int rs2_value;    // Source-2 Register Value
  int rs3_value;    // Source-3 Register Value
  int buffer;       // Latch to hold some value
  int mem_address;  // Computed Memory Address
  int busy;         // Flag to indicate, stage is performing some action
  int stalled;      // Flag to indicate, stage is stalled
} CPU_Stage;

struct LSQ_Entry
{
  int memory_address;
  char opcode[128];
  int pc; // Program counter
  int rs1;
  int rs2;
  int rs3;
  int rd;
  int imm;
  int rs1_value;
} LSQ_Entry;

typedef struct l1
{
  int pc;
  char opcode[128];
  int rd;
  int rs1;
  int rs2;
  int rs3;
  int imm;
} l1;

typedef struct iq
{
  int pc;
  char opcode[128];
  int rd;
  int rs1;
  int rs2;
  int rs3;
  int imm;
  int get_data;
} iq;

typedef struct rob
{
  int pc;
  char opcode[128];
  int rd;
  int rs1;
  int rs2;
  int imm;
  int get_data;
}rob;

typedef struct lsq
{
  int pc;
  char opcode[128];
  int rd;
  int rs1;
  int rs2;
  int rs3;
  int imm;
  int get_data;
} lsq;

/* Model of APEX CPU */
typedef struct APEX_CPU
{
  /* Clock cycles elasped */
  int clock;

  /* Current program counter */
  int pc;

  /* HALT flag */
  int haltflag;

  int LSQ_Instruction_flag;

  /* Integer register file */
  int regs[32];
  int regs_valid[32];

  //int rob[12];
  CPU_Stage stage[14];

  iq IQ[8];
  lsq LSQ[6];
  rob ROB[12];
  /* Code Memory where instructions are stored */
  APEX_Instruction *code_memory;
  int code_memory_size;

  /* Data Memory */
  int data_memory[4096];

  /* Some stats */
  int ins_completed;

} APEX_CPU;

APEX_Instruction *
create_code_memory(const char *filename, int *size);

APEX_CPU *
APEX_cpu_init(const char *filename);

int APEX_cpu_run(APEX_CPU *cpu, const char *function, const char *totalcycles);

void APEX_cpu_stop(APEX_CPU *cpu);

int fetch(APEX_CPU *cpu);

int decode(APEX_CPU *cpu);

int intfu1(APEX_CPU *cpu);

int intfu2(APEX_CPU *cpu);

int mulfu1(APEX_CPU *cpu);

int mulfu2(APEX_CPU *cpu);

int mulfu3(APEX_CPU *cpu);

int mulfu1(APEX_CPU *cpu);

int memfu1(APEX_CPU *cpu);

int memfu2(APEX_CPU *cpu);

int memfu3(APEX_CPU *cpu);

int retire(APEX_CPU *cpu);

#endif
