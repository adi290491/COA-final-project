/*
 *  cpu.c
 *  Contains APEX cpu pipeline implementation
 *
 *  Author :
 *  
 *  State University of New York, Binghamton
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cpu.h"

/* Set this flag to 1 to enable debug messages */
#define ENABLE_DEBUG_MESSAGES 1
#define LSQ_SIZE 6

/*
 * This function creates and initializes APEX cpu.
 *
 * Note : You are free to edit this function according to your
 * 				implementation
 */
APEX_CPU *
APEX_cpu_init(const char *filename)
{
  if (!filename)
  {
    return NULL;
  }

  APEX_CPU *cpu = malloc(sizeof(*cpu));
  if (!cpu)
  {
    return NULL;
  }

  /* Initialize PC, Registers and all pipeline stages */
  cpu->pc = 4000;
  //memset(cpu->regs, 0, sizeof(int) * 32);
  //memset(cpu->regs_valid, 1, sizeof(int) * 32);
  memset(cpu->stage, 0, sizeof(CPU_Stage) * NUM_STAGES);
  memset(cpu->data_memory, 0, sizeof(int) * 4000);

  /* Parse input file and create code memory */
  cpu->code_memory = create_code_memory(filename, &cpu->code_memory_size);

  if (!cpu->code_memory)
  {
    free(cpu);
    return NULL;
  }

  if (ENABLE_DEBUG_MESSAGES)
  {
    fprintf(stderr,
            "APEX_CPU : Initialized APEX CPU, loaded %d instructions\n",
            cpu->code_memory_size);
    fprintf(stderr, "APEX_CPU : Printing Code Memory\n");
    printf("%-9s %-9s %-9s %-9s %-9s\n", "opcode", "rd", "rs1", "rs2", "rs3", "imm");

    for (int i = 0; i < cpu->code_memory_size; ++i)
    {
      printf("%-9s %-9d %-9d %-9d %-9d %-9d \n",
             cpu->code_memory[i].opcode,
             cpu->code_memory[i].rd,
             cpu->code_memory[i].rs1,
             cpu->code_memory[i].rs2,
             cpu->code_memory[i].rs3,
             cpu->code_memory[i].imm);
    }
  }

  /* Make all stages busy except Fetch stage, initally to start the pipeline */
  for (int i = 1; i < NUM_STAGES; ++i)
  {
    cpu->stage[i].busy = 1;
  }

  return cpu;
}

/*
 * This function de-allocates APEX cpu.
 *
 * Note : You are free to edit this function according to your
 * 				implementation
 */
void APEX_cpu_stop(APEX_CPU *cpu)
{
  free(cpu->code_memory);
  free(cpu);
}

/* Converts the PC(4000 series) into
 * array index for code memory
 *
 * Note : You are not supposed to edit this function
 *
 */
int get_code_index(int pc)
{
  return (pc - 4000) / 4;
}

static void
print_instruction(CPU_Stage *stage)
{
  if (strcmp(stage->opcode, "STORE") == 0)
  {
    printf("%s,R%d,R%d,#%d ", stage->opcode, stage->rs1, stage->rs2, stage->imm);
  }
  if (strcmp(stage->opcode, "LOAD") == 0)
  {
    printf("%s,R%d,R%d,#%d ", stage->opcode, stage->rd, stage->rs1, stage->imm);
  }

  if (strcmp(stage->opcode, "STR") == 0)
  {
    printf("%s,R%d,R%d,#%d ", stage->opcode, stage->rs1, stage->rs2, stage->rs3);
  }

  if (strcmp(stage->opcode, "ADDL") == 0 ||
      strcmp(stage->opcode, "SUBL") == 0)
  {
    printf("%s,R%d,R%d,R%d ", stage->opcode, stage->rd, stage->rs1, stage->imm);
  }

  if (strcmp(stage->opcode, "ADD") == 0 ||
      strcmp(stage->opcode, "SUB") == 0 ||
      strcmp(stage->opcode, "AND") == 0 ||
      strcmp(stage->opcode, "OR") == 0 ||
      strcmp(stage->opcode, "EX-OR") == 0 ||
      strcmp(stage->opcode, "MUL") == 0 ||
      strcmp(stage->opcode, "LDR") == 0)
  {

    printf("%s,R%d,R%d,R%d ", stage->opcode, stage->rd, stage->rs1, stage->rs2);
  }

  if (strcmp(stage->opcode, "MOVC") == 0)
  {
    printf("%s,R%d,#%d ", stage->opcode, stage->rd, stage->imm);
  }
  if (strcmp(stage->opcode, "BZ") == 0 ||
      strcmp(stage->opcode, "BNZ") == 0)
  {
    printf("%s,#%d ", stage->opcode, stage->imm);
  }
  if (strcmp(stage->opcode, "JUMP") == 0)
  {
    printf("%s,R%d,#%d ", stage->opcode, stage->rs1, stage->imm);
  }

  if (strcmp(stage->opcode, "HALT") == 0)
  {
    printf("%s", stage->opcode);
  }

  if (strcmp(stage->opcode, "EMPTY") == 0)
  {
    printf("%s", stage->opcode);
  }
}

/* Debug function which dumps the cpu stage
 * content
 *
 * Note : You are not supposed to edit this function
 *
 */
static void
print_stage_content(char *name, CPU_Stage *stage)
{
  printf("%-15s: pc(%d) ", name, stage->pc);
  print_instruction(stage);
  printf("\n");
}

/*
 *  Fetch Stage of APEX Pipeline
 *
 *  Note : You are free to edit this function according to your
 * 				 implementation
 */
int fetch(APEX_CPU *cpu)
{
  CPU_Stage *stage = &cpu->stage[F];
  if (!stage->busy && !stage->stalled) //&& !cpu->haltflag)
  {
    /* Store current PC in fetch latch */
    stage->pc = cpu->pc;
    /* Index into code memory using this pc and copy all instruction fields into
     * fetch latch
     */
    APEX_Instruction *current_ins = &cpu->code_memory[get_code_index(cpu->pc)];
    strcpy(stage->opcode, current_ins->opcode);
    stage->rd = current_ins->rd;
    stage->rs1 = current_ins->rs1;
    stage->rs2 = current_ins->rs2;
    stage->imm = current_ins->imm;
    stage->rd = current_ins->rd;

    /* Update PC for next instruction */
    cpu->pc += 4;

    /* Copy data from fetch latch to decode latch*/
    //if (!cpu->stage[DRF].stalled)
    //{
    /*if (strcmp(cpu->stage[DRF].opcode, "HALT") != 0)
      {
        cpu->stage[DRF] = cpu->stage[F];
      }*/

    //}
    //else
    ///{
    //  stage->stalled = 1;
    //}
    cpu->stage[DRF] = cpu->stage[F];

    if (ENABLE_DEBUG_MESSAGES)
    {
      print_stage_content("Fetch", stage);
    }
  }
  return 0;
}

/*
 *  Decode Stage of APEX Pipeline
 *
 *  Note : You are free to edit this function according to your
 * 				 implementation
 */
int decode(APEX_CPU *cpu)
{
  CPU_Stage *stage = &cpu->stage[DRF];
  if (!stage->busy && !stage->stalled)
  {
    if (strcmp(stage->opcode, "HALT") == 0)
    {
      cpu->stage[ROB] = cpu->stage[DRF];
      printf("AT DECODE HALT----");
      //cpu->ins_completed++;
    }
    if (strcmp(stage->opcode, "STORE") == 0)
    {
      if (cpu->regs_valid[stage->rs1] && cpu->regs_valid[stage->rs2])
      {
        stage->stalled = 0;
        stage->rs1_value = cpu->regs[stage->rs1];
        stage->rs2_value = cpu->regs[stage->rs2];
      }
      else
      {
        stage->stalled = 0;
      }
    }

    /* STR */
    if (strcmp(stage->opcode, "STR") == 0)
    {
      if (cpu->regs_valid[stage->rs1] && cpu->regs_valid[stage->rs2] && cpu->regs_valid[stage->rs3])
      {
        stage->stalled = 0;
        stage->rs1_value = cpu->regs[stage->rs1];
        stage->rs2_value = cpu->regs[stage->rs2];
        stage->rs3_value = cpu->regs[stage->rs3];
      }
      else
      {
        stage->stalled = 0;
      }
    }

    /* LOAD */
    if (strcmp(stage->opcode, "LOAD") == 0)
    {
      if (cpu->regs_valid[stage->rs1])
      {
        stage->stalled = 0;
        stage->rs1_value = cpu->regs[stage->rs1];
        cpu->regs_valid[stage->rd] = 0; //making register invalid
      }
      else
      {
        stage->stalled = 0;
      }
    }

    /* LDR */
    if (strcmp(stage->opcode, "LDR") == 0)
    {
      if (cpu->regs_valid[stage->rs1] && cpu->regs_valid[stage->rs2])
      {
        stage->stalled = 0;
        stage->rs1_value = cpu->regs[stage->rs1];
        stage->rs1_value = cpu->regs[stage->rs2];
        cpu->regs_valid[stage->rd] = 0;
      }
      else
      {
        stage->stalled = 0;
      }
    }

    /* No Register file read needed for MOVC */
    if (strcmp(stage->opcode, "MOVC") == 0)
    {
      cpu->regs_valid[stage->rd] = 0;
    }

    /* ADD, SUB , MUL*/
    if (strcmp(stage->opcode, "ADD") == 0 ||
        strcmp(stage->opcode, "SUB") == 0 ||
        strcmp(stage->opcode, "MUL") == 0 ||
        strcmp(stage->opcode, "ADDL") == 0 ||
        strcmp(stage->opcode, "SUBL") == 0)
    {
      //cpu->z_flag_set = 0;
      if (cpu->regs_valid[stage->rs1] && cpu->regs_valid[stage->rs2])
      {
        stage->rs1_value = cpu->regs[stage->rs1];
        stage->rs2_value = cpu->regs[stage->rs2];
        cpu->regs_valid[stage->rd] = 0;
        //cpu->z_flag_set = 0;
      }
      else
      {
        stage->stalled = 0;
      }
    }

    /* AND,OR,EX-OR */
    if (strcmp(stage->opcode, "AND") == 0 ||
        strcmp(stage->opcode, "OR") == 0 ||
        strcmp(stage->opcode, "EX-OR") == 0)
    {
      if (cpu->regs_valid[stage->rs1] && cpu->regs_valid[stage->rs2])
      {
        stage->stalled = 0;
        stage->rs1_value = cpu->regs[stage->rs1];
        stage->rs2_value = cpu->regs[stage->rs2];
        cpu->regs_valid[stage->rd] = 0;
      }
      else
      {
        stage->stalled = 0;
      }
    }

    /* JUMP */
    if (strcmp(stage->opcode, "JUMP") == 0)
    {
      if (cpu->regs_valid[stage->rs1])
      {
        stage->stalled = 0;
        stage->rs1_value = cpu->regs[stage->rs1];
      }
      else
      {
        stage->stalled = 0;
      }
    }

    // LSQ Condition
    if (strcmp(stage->opcode, "LOAD") == 0 ||
        strcmp(stage->opcode, "STR") == 0 ||
        strcmp(stage->opcode, "LDR") == 0 ||
        strcmp(stage->opcode, "STORE") == 0)
    {
      cpu->stage[LSQ] = cpu->stage[DRF];
      cpu->stage[IQ] = cpu->stage[DRF];
    }

    /* Copy data from decode latch to execute latch*/
    cpu->stage[IQ] = cpu->stage[DRF];
    cpu->stage[ROB] = cpu->stage[DRF];

    if (ENABLE_DEBUG_MESSAGES)
    {
      print_stage_content("Decode/RF", stage);
    }
  }
  return 0;
}
/*
 *  Memory FU Stage of APEX Pipeline
 *
 *  Note : You are free to edit this function according to your
 * 				 implementation
 */
int fetch_IQ(APEX_CPU *cpu)
{
  for (int i = 0; i <= 7; i++)
  {
    if (cpu->IQ[i].get_data == 0)
    {
      return i;
    }
  }
}

int fetch_LSQ(APEX_CPU *cpu)
{
  for (int i = 0; i <= 5; i++)
  {
    if (cpu->LSQ[i].get_data == 0)
    {
      return i;
    }
  }
}

int fetch_ROB(APEX_CPU *cpu)
{
  for (int i = 0; i <= 12; i++)
  {
    if (cpu->ROB[i].get_data == 0)
    {
      return i;
    }
  }
}

int get_I(APEX_CPU *cpu)
{
  CPU_Stage *stage = &cpu->stage[IQ];

  int getIQ = fetch_IQ(cpu);
  if (IQ_Squash(cpu, stage->pc) == 1)
  {

    cpu->IQ[getIQ].pc = stage->pc;
    // printf("\n \n%d -------- %d",cpu->IQ[getIQ].pc, stage->pc);

    strcpy(cpu->IQ[getIQ].opcode, stage->opcode);
    cpu->IQ[getIQ].rd = stage->rd;
    cpu->IQ[getIQ].rs1 = stage->rs1;
    cpu->IQ[getIQ].rs2 = stage->rs2;
    cpu->IQ[getIQ].get_data = 1;
  }
}

int get_LSQ(APEX_CPU *cpu)
{
  CPU_Stage *stage = &cpu->stage[LSQ];

  int getLSQ = fetch_LSQ(cpu);
  if (LSQ_Squash(cpu, stage->pc) == 1)
  {

    cpu->LSQ[getLSQ].pc = stage->pc;
    // printf("\n \n%d -------- %d",cpu->IQ[getIQ].pc, stage->pc);

    strcpy(cpu->LSQ[getLSQ].opcode, stage->opcode);

    cpu->IQ[getLSQ].rd = stage->rd;
    cpu->IQ[getLSQ].rs1 = stage->rs1;
    cpu->IQ[getLSQ].rs2 = stage->rs2;
    cpu->IQ[getLSQ].get_data = 1;
  }
}

int get_ROB(APEX_CPU *cpu)
{
  CPU_Stage *stage = &cpu->stage[ROB];

  int getROB = fetch_ROB(cpu);

  cpu->ROB[getROB].pc = stage->pc;
  // printf("\n \n%d -------- %d",cpu->IQ[getIQ].pc, stage->pc);

  strcpy(cpu->ROB[getROB].opcode, stage->opcode);
  if (strcmp(cpu->ROB[getROB].opcode, "HALT") == 0)
  {

    cpu->stage[F].stalled = 1;
    cpu->stage[DRF].stalled = 1;
    cpu->haltflag = 0;
  }
  cpu->ROB[getROB].rd = stage->rd;
  cpu->ROB[getROB].rs1 = stage->rs1;
  cpu->ROB[getROB].rs2 = stage->rs2;
  cpu->ROB[getROB].get_data = 1;
  // cpu->ins_completed++;
}

int printI(APEX_CPU *cpu)
{
  for (int i = 0; i < 8; i++)
  {
    if (cpu->IQ[i].get_data == 1)
    {
      if (strcmp(cpu->IQ[i].opcode, "ADD") == 0 || strcmp(cpu->IQ[i].opcode, "SUB") == 0 || strcmp(cpu->IQ[i].opcode, "AND") == 0 || strcmp(cpu->IQ[i].opcode, "OR") == 0 || strcmp(cpu->IQ[i].opcode, "XOR") == 0 || strcmp(cpu->IQ[i].opcode, "MUL") == 0)
      {
        printf("\n IQ %s R%d R%d R%d \n", cpu->IQ[i].opcode, cpu->IQ[i].rd, cpu->IQ[i].rs1, cpu->IQ[i].rs2);
      }

      if (strcmp(cpu->IQ[i].opcode, "ADDL") == 0 || strcmp(cpu->IQ[i].opcode, "SUBL") == 0)
      {
        printf("\n IQ %s R%d R%d \n", cpu->IQ[i].opcode, cpu->IQ[i].rd, cpu->IQ[i].rs1, cpu->IQ[i].imm);
      }

      if (strcmp(cpu->IQ[i].opcode, "LDR") == 0 || strcmp(cpu->IQ[i].opcode, "STR") == 0)
      {
        printf("\n IQ %s R%d R%d R%d \n", cpu->IQ[i].opcode, cpu->IQ[i].rd, cpu->IQ[i].rs1, cpu->IQ[i].rs2);
      }

      if (strcmp(cpu->IQ[i].opcode, "LOAD") == 0 || strcmp(cpu->IQ[i].opcode, "STORE") == 0)
      {
        printf("\n IQ %s R%d R%d R%d \n", cpu->IQ[i].opcode, cpu->IQ[i].rs1, cpu->IQ[i].rs2, cpu->IQ[i].rs3);
      }

      if (strcmp(cpu->IQ[i].opcode, "BZ") == 0 || strcmp(cpu->IQ[i].opcode, "BNZ") == 0)
      {
        printf("\n IQ %s  \n", cpu->IQ[i].opcode, cpu->IQ[i].imm);
      }

      if (strcmp(cpu->IQ[i].opcode, "JUMP") == 0)
      {
        printf("\n IQ %s R%d \n", cpu->IQ[i].opcode, cpu->IQ[i].rs1, cpu->IQ[i].imm);
      }

      if (strcmp(cpu->IQ[i].opcode, "MOVC") == 0)
      {
        printf("\n IQ %s R%d \n", cpu->IQ[i].opcode, cpu->IQ[i].rd, cpu->IQ[i].imm);
      }
    }
  }
}

int printROB(APEX_CPU *cpu)
{
  for (int i = 0; i < 12; i++)
  {

    if (cpu->ROB[i].get_data == 1)
    {
      if (strcmp(cpu->ROB[i].opcode, "ADD") == 0 || strcmp(cpu->ROB[i].opcode, "SUB") == 0 || strcmp(cpu->ROB[i].opcode, "AND") == 0 || strcmp(cpu->ROB[i].opcode, "OR") == 0 || strcmp(cpu->ROB[i].opcode, "XOR") == 0 || strcmp(cpu->ROB[i].opcode, "MUL") == 0)
      {
        printf("\n ROB  %s R%d R%d R%d \n", cpu->ROB[i].opcode, cpu->ROB[i].rd, cpu->ROB[i].rs1, cpu->ROB[i].rs2);
      }

      if (strcmp(cpu->IQ[i].opcode, "ADDL") == 0 || strcmp(cpu->IQ[i].opcode, "SUBL") == 0)
      {
        printf("\n ROB %s R%d R%d \n", cpu->ROB[i].opcode, cpu->ROB[i].rd, cpu->ROB[i].rs1, cpu->ROB[i].imm);
      }

      if (strcmp(cpu->IQ[i].opcode, "LDR") == 0 || strcmp(cpu->IQ[i].opcode, "STR") == 0)
      {
        printf("\n ROB %s R%d R%d R%d \n", cpu->ROB[i].opcode, cpu->ROB[i].rd, cpu->ROB[i].rs1, cpu->ROB[i].rs2);
      }

      if (strcmp(cpu->IQ[i].opcode, "LOAD") == 0 || strcmp(cpu->IQ[i].opcode, "STORE") == 0)
      {
        printf("\n ROB %s R%d R%d R%d \n", cpu->ROB[i].opcode, cpu->ROB[i].rs1, cpu->ROB[i].rs2, cpu->IQ[i].rs3);
      }

      if (strcmp(cpu->IQ[i].opcode, "BZ") == 0 || strcmp(cpu->IQ[i].opcode, "BNZ") == 0)
      {
        printf("\n ROB %s  \n", cpu->ROB[i].opcode, cpu->ROB[i].imm);
      }

      if (strcmp(cpu->IQ[i].opcode, "JUMP") == 0)
      {
        printf("\n ROB %s R%d \n", cpu->ROB[i].opcode, cpu->ROB[i].rs1, cpu->ROB[i].imm);
      }

      if (strcmp(cpu->IQ[i].opcode, "MOVC") == 0)
      {
        printf("\n ROB %s R%d \n", cpu->ROB[i].opcode, cpu->ROB[i].rd, cpu->ROB[i].imm);
      }
    }
  }
}

int printLSQ(APEX_CPU *cpu)
{
  for (int i = 0; i < 6; i++)
  {

    if (cpu->LSQ[i].get_data == 0)
    {
      if (strcmp(cpu->LSQ[i].opcode, "LDR") == 0)
      {
        printf("\n LSQ %s R%d R%d R%d \n", cpu->LSQ[i].opcode, cpu->LSQ[i].rd, cpu->LSQ[i].rs1, cpu->LSQ[i].rs2);
      }

      if (strcmp(cpu->LSQ[i].opcode, "STR") == 0)
      {
        printf("\n LSQ %s R%d R%d R%d \n", cpu->LSQ[i].opcode, cpu->LSQ[i].rs1, cpu->LSQ[i].rs2, cpu->LSQ[i].rs3);
      }

      if (strcmp(cpu->LSQ[i].opcode, "LOAD") == 0)
      {
        printf("\n LSQ %s R%d R%d R%d \n", cpu->LSQ[i].opcode, cpu->LSQ[i].rd, cpu->LSQ[i].rs1, cpu->LSQ[i].imm);
      }

      if (strcmp(cpu->LSQ[i].opcode, "STORE") == 0)
      {
        printf("\n LSQ %s R%d R%d R%d \n", cpu->LSQ[i].opcode, cpu->LSQ[i].rs1, cpu->LSQ[i].rs2, cpu->LSQ[i].imm);
      }
    }
  }
}

int IQ_Squash(APEX_CPU *cpu, int pc)
{
  // int i = 0;

  for (int i = 0; i < 8; i++)
  {
    if (cpu->IQ[i].get_data == 1)
    {

      if (cpu->IQ[i].pc == pc)
      {
        //cpu->stage[IQ].stalled = 1;

        return 0;
      }
    }
    // i += 1;
  }
  return 1;
}

int LSQ_Squash(APEX_CPU *cpu, int pc)
{
  // int i = 0;

  for (int i = 0; i < 6; i++)
  {
    if (cpu->IQ[i].get_data == 1)
    {

      if (cpu->IQ[i].pc == pc)
      {
        //cpu->stage[IQ].stalled = 1;

        return 0;
      }
    }
    // i += 1;
  }
  return 1;
}

int get_int(APEX_CPU *cpu, int i)
{
  CPU_Stage *stage = &cpu->stage[INT1];
  stage->pc = cpu->IQ[i].pc;
  stage->rd = cpu->IQ[i].rd;
  stage->rs1 = cpu->IQ[i].rs1;
  stage->rs2 = cpu->IQ[i].rs2;
  cpu->IQ[i].get_data = 0;
}

int lsqstage(APEX_CPU *cpu)
{
  CPU_Stage *stage = &cpu->stage[LSQ];
  if (!stage->busy && !stage->stalled)
  {
    get_LSQ(cpu);
    cpu->stage[MEM1] = cpu->stage[LSQ];
    if (ENABLE_DEBUG_MESSAGES)
    {
      printLSQ(cpu);
      printf("LSQ Stage");
    }
  }
  return 0;
}

int robstage(APEX_CPU *cpu)
{
  CPU_Stage *stage = &cpu->stage[ROB];
  if (!stage->busy && !stage->stalled)
  {

    get_ROB(cpu);
    if (ENABLE_DEBUG_MESSAGES)
    {
      printROB(cpu);
    }
  }
  return 0;
}

int iqstage(APEX_CPU *cpu)
{
  CPU_Stage *stage = &cpu->stage[IQ];
  if (!stage->busy && !stage->stalled)
  {
    get_I(cpu);

    if (strcmp(stage->opcode, "MUL") == 0)
    {
      cpu->stage[MUL1] = cpu->stage[IQ];
    }
    else
    {
      cpu->stage[INT1] = cpu->stage[IQ];
    }

    if (ENABLE_DEBUG_MESSAGES)
    {
      printI(cpu);
      // print_stage_content("IQ", stage);
    }
  }

  return 0;
}

int memfu1(APEX_CPU *cpu)
{
  CPU_Stage *stage = &cpu->stage[MEM1];
  if (!stage->busy && !stage->stalled)
  {

    /* Store */
    if (strcmp(stage->opcode, "STORE") == 0)
    {
      stage->mem_address = stage->rs2_value + stage->imm;
    }

    /* STR */
    if (strcmp(stage->opcode, "STR") == 0)
    {
      stage->mem_address = stage->rs2_value + stage->rs3_value;
    }

    /* LDR */
    if (strcmp(stage->opcode, "LDR") == 0)
    {
      stage->mem_address = stage->rs1_value + stage->rs2_value;
    }

    /* MOVC */
    if (strcmp(stage->opcode, "LOAD") == 0)
    {
      stage->buffer = cpu->data_memory[stage->mem_address];
    }

    /* Copy data from decode latch to execute latch*/
    cpu->stage[MEM2] = cpu->stage[MEM1];

    if (ENABLE_DEBUG_MESSAGES)
    {
      print_stage_content("Memory FU 1", stage);
    }
  }
  return 0;
}

int memfu2(APEX_CPU *cpu)
{
  CPU_Stage *stage = &cpu->stage[MEM2];
  if (!stage->busy && !stage->stalled)
  {

    /* Copy data from decode latch to execute latch*/
    cpu->stage[MEM3] = cpu->stage[MEM2];

    if (ENABLE_DEBUG_MESSAGES)
    {
      print_stage_content("Memory FU 2", stage);
    }
  }
  return 0;
}

int memfu3(APEX_CPU *cpu)
{
  CPU_Stage *stage = &cpu->stage[MEM3];
  if (!stage->busy && !stage->stalled)
  {
    cpu->stage[RET] = cpu->stage[MEM3];
    if (ENABLE_DEBUG_MESSAGES)
    {
      print_stage_content("Memory FU 3", stage);
    }
  }
  return 0;
}

int intfu1(APEX_CPU *cpu)
{

  CPU_Stage *stage;
  //if (!stage->busy && !stage->stalled)
  //{

  /* Store */
  if (strcmp(stage->opcode, "STORE") == 0)
  {
    stage->mem_address = stage->rs2_value + stage->imm;
  }

  /* STR */
  if (strcmp(stage->opcode, "STR") == 0)
  {
    stage->mem_address = stage->rs2_value + stage->rs3_value;
  }

  /* LOAD */
  if (strcmp(stage->opcode, "LOAD") == 0)
  {
    stage->mem_address = stage->rs1_value + stage->imm;
  }

  /* LDR */
  if (strcmp(stage->opcode, "LDR") == 0)
  {
    stage->mem_address = stage->rs1_value + stage->rs2_value;
  }

  /* MOVC */
  if (strcmp(stage->opcode, "MOVC") == 0)
  {
    stage->buffer = stage->imm + 0;
  }

  /* ADD */
  if (strcmp(stage->opcode, "ADD") == 0)
  {
    stage->buffer = stage->rs1_value + stage->rs2_value;
  }

  /* SUB */
  if (strcmp(stage->opcode, "SUB") == 0)
  {
    stage->buffer = stage->rs1_value - stage->rs2_value;
  }

  /* AND */
  if (strcmp(stage->opcode, "AND") == 0)
  {
    stage->buffer = stage->rs1_value & stage->rs2_value;
  }

  /* OR */
  if (strcmp(stage->opcode, "OR") == 0)
  {
    stage->buffer = stage->rs1_value | stage->rs2_value;
  }

  /* EX-OR */
  if (strcmp(stage->opcode, "EX-OR") == 0)
  {
    stage->buffer = stage->rs1_value ^ stage->rs2_value;
  }

  cpu->stage[INT2] = cpu->stage[INT1];
  //printf("at Int1-----");
  if (ENABLE_DEBUG_MESSAGES)
  {
    print_stage_content("Int FU 1", stage);
  }
  //}
  return 0;
}

int intfu2(APEX_CPU *cpu)
{
  CPU_Stage *stage = &cpu->stage[INT2];
  if (!stage->busy && !stage->stalled)
  {
    cpu->stage[RET] = cpu->stage[INT2];

    if (strcmp(stage->opcode, "MOVC") == 0 ||
        strcmp(stage->opcode, "AND") == 0 ||
        strcmp(stage->opcode, "OR") == 0 ||
        strcmp(stage->opcode, "EX-OR") == 0 ||
        strcmp(stage->opcode, "ADD") == 0 ||
        strcmp(stage->opcode, "ADDL") == 0 ||
        strcmp(stage->opcode, "SUB") == 0 ||
        strcmp(stage->opcode, "SUBL") == 0)
    {
      cpu->regs[stage->rd] = stage->buffer;
      cpu->regs_valid[stage->rd] = 1;
    }
    if (ENABLE_DEBUG_MESSAGES)
    {
      print_stage_content("Int FU 2", stage);
    }
  }
  return 0;
}

int mulfu1(APEX_CPU *cpu)
{
  CPU_Stage *stage = &cpu->stage[MUL1];
  if (!stage->busy && !stage->stalled)
  {
    cpu->stage[MUL2] = cpu->stage[MUL1];

    if (ENABLE_DEBUG_MESSAGES)
    {
      print_stage_content("MUL FU 1", stage);
    }
  }
  return 0;
}

int mulfu2(APEX_CPU *cpu)
{
  CPU_Stage *stage = &cpu->stage[MUL2];
  if (!stage->busy && !stage->stalled)
  {

    cpu->stage[MUL3] = cpu->stage[MUL2];

    if (ENABLE_DEBUG_MESSAGES)
    {
      print_stage_content("MUL FU 2", stage);
    }
  }
  return 0;
}

int mulfu3(APEX_CPU *cpu)
{
  CPU_Stage *stage = &cpu->stage[MUL3];
  if (!stage->busy && !stage->stalled)
  {
    cpu->stage[RET] = cpu->stage[MUL3];
    if (ENABLE_DEBUG_MESSAGES)
    {
      print_stage_content("MUL FU 3", stage);
    }
  }
  return 0;
}

int retire(APEX_CPU *cpu)
{
  CPU_Stage *stage = &cpu->stage[RET];
  if (!stage->busy && !stage->stalled)
  {

    if (ENABLE_DEBUG_MESSAGES)
    {
      print_stage_content("RET", stage);
    }
  }
  return 0;
}

int display(APEX_CPU *cpu)
{
  printf("\n");
    printf("==================REGISTER VALUE==============");
    for (int i = 0; i < 16; i++)
    {
      printf("\n");
      printf(" | Register[%d] | Value=%d | status=%s |", i, cpu->regs[i], (cpu->regs_valid[i]) ? "Valid" : "Invalid");
    }

    printf("\n");
    printf("==================DATA MEMORY ==============");
    printf("\n");
    for (int i = 0; i < 99; i++)
    {
      printf(" | MEM[%d] | Value=%d | \n", i, cpu->data_memory[i]);
    }
}
/*
 *  APEX CPU simulation loop
 *
 *  Note : You are free to edit this function according to your
 * 				 implementation
 */
int APEX_cpu_run(APEX_CPU *cpu, const char *function, const char *totalcycles)
{
  while (cpu->clock <= cpu->code_memory_size)
  {

    if (strcmp(function, "display") == 0)
    {
      ENABLE_DEBUG_MESSAGES;
    }
    int totalcyclecount = atoi(totalcycles);

    /* All the instructions committed, so exit */
    if (cpu->ins_completed == cpu->code_memory_size)
    {
      printf("(apex) >> Simulation Complete");
      break;
    }

    if (ENABLE_DEBUG_MESSAGES)
    {
      printf("--------------------------------\n");
      printf("Clock Cycle #: %d\n", cpu->clock + 1);
      printf("--------------------------------\n");
    }
    retire(cpu);
    memfu3(cpu);
    memfu2(cpu);
    memfu1(cpu);

    mulfu3(cpu);
    mulfu2(cpu);
    mulfu1(cpu);
    intfu2(cpu);

    intfu1(cpu);

    robstage(cpu);

    iqstage(cpu);
    lsqstage(cpu);

    decode(cpu);
    fetch(cpu);
    cpu->clock++;

    if (totalcyclecount == cpu->clock)
    {
      break;
    }

    if (cpu->haltflag == 1)
    {
      break;
    }
  }
  display(cpu);
  //display_reg_file(cpu);
  return 0;
}