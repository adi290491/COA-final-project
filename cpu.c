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
    printf("%-9s %-9s %-9s %-9s %-9s\n", "opcode", "rd", "rs1", "rs2", "imm");

    for (int i = 0; i < cpu->code_memory_size; ++i)
    {
      printf("%-9s %-9d %-9d %-9d %-9d\n",
             cpu->code_memory[i].opcode,
             cpu->code_memory[i].rd,
             cpu->code_memory[i].rs1,
             cpu->code_memory[i].rs2,
             cpu->code_memory[i].imm);
    }
  }

  /* Make all stages busy except Fetch stage, initally to start the pipeline */
  for (int i = 1; i < NUM_STAGES; ++i)
  {
    cpu->stage[i].busy = 1;
  }

  cpu->haltflag = 0;

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
  if (!stage->busy && !stage->stalled && !cpu->haltflag)
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
    stage->rs3 = current_ins->rs3;
    stage->imm = current_ins->imm;
    stage->rd = current_ins->rd;

    /* Update PC for next instruction */
    cpu->pc += 4;

    /* Copy data from fetch latch to decode latch*/
    if (!cpu->stage[DRF].stalled)
    {
      if (strcmp(cpu->stage[DRF].opcode, "HALT") != 0)
      {
        cpu->stage[DRF] = cpu->stage[F];
        cpu->stage[F].stalled = 0;
      }
      else
      {
        cpu->stage[F].busy = 1;
      }
    }

    else
    {
      stage->stalled = 1;
    }

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
      cpu->haltflag = 1;
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
        stage->stalled = 1;
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
        stage->stalled = 1;
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
        stage->stalled = 1;
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
        stage->stalled = 1;
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
        stage->stalled = 1;
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
        stage->stalled = 1;
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
        stage->stalled = 1;
      }
    }

    // LSQ Condition
    if (strcmp(stage->opcode, "LOAD") == 0 ||
        strcmp(stage->opcode, "STR") == 0 ||
        strcmp(stage->opcode, "LDR") == 0 ||
        strcmp(stage->opcode, "STORE") == 0)
    {
      cpu->stage[LSQ] = cpu->stage[DRF];
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

int lsqstage(APEX_CPU *cpu)
{
  CPU_Stage *stage = &cpu->stage[LSQ];
  if (!stage->busy && !stage->stalled)
  {

    cpu->stage[MEM1] = cpu->stage[LSQ];
    if (ENABLE_DEBUG_MESSAGES)
    {
      print_stage_content("LSQ", stage);
    }
  }
  return 0;
}

int robstage(APEX_CPU *cpu)
{
  CPU_Stage *stage = &cpu->stage[ROB];
  if (!stage->busy && !stage->stalled)
  {

    if (ENABLE_DEBUG_MESSAGES)
    {
      print_stage_content("ROB", stage);
    }
  }
  return 0;
}

int iqstage(APEX_CPU *cpu)
{
  CPU_Stage *stage = &cpu->stage[IQ];
  if (!stage->busy && !stage->stalled)
  {
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
      print_stage_content("IQ", stage);
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
  CPU_Stage *stage = &cpu->stage[INT1];
  if (!stage->busy && !stage->stalled)
  {

    cpu->stage[INT2] = cpu->stage[INT1];
    if (ENABLE_DEBUG_MESSAGES)
    {
      print_stage_content("Int FU 1", stage);
    }
  }
  return 0;
}

int intfu2(APEX_CPU *cpu)
{
  CPU_Stage *stage = &cpu->stage[INT2];
  if (!stage->busy && !stage->stalled)
  {
    cpu->stage[RET] = cpu->stage[INT2];
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
    intfu1(cpu);
    intfu2(cpu);
    mulfu1(cpu);
    mulfu2(cpu);
    mulfu3(cpu);
    iqstage(cpu);
    robstage(cpu);
    lsqstage(cpu);
    decode(cpu);
    fetch(cpu);
    cpu->clock++;

    if (totalcyclecount == cpu->clock)
    {
      break;
    }
  }
  //display_reg_file(cpu);
  return 0;
}