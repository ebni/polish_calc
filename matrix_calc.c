#include <stdio.h>
#include <string.h>
#include <gsl/gsl_matrix.h>

// SINGLEQUOTED_? are needed to define CALC_OPTION
// smarter solutions are welcome
#define SINGLEQUOTED_a 'a'
#define SINGLEQUOTED_b 'b'
#define SINGLEQUOTED_c 'c'
#define SINGLEQUOTED_d 'd'
#define SINGLEQUOTED_e 'e'
#define SINGLEQUOTED_f 'f'
#define SINGLEQUOTED_g 'g'
#define SINGLEQUOTED_h 'h'
#define SINGLEQUOTED_i 'i'
#define SINGLEQUOTED_j 'j'
#define SINGLEQUOTED_k 'k'
#define SINGLEQUOTED_l 'l'
#define SINGLEQUOTED_m 'm'
#define SINGLEQUOTED_n 'n'
#define SINGLEQUOTED_o 'o'
#define SINGLEQUOTED_p 'p'
#define SINGLEQUOTED_q 'q'
#define SINGLEQUOTED_r 'r'
#define SINGLEQUOTED_s 's'
#define SINGLEQUOTED_t 't'
#define SINGLEQUOTED_u 'u'
#define SINGLEQUOTED_v 'v'
#define SINGLEQUOTED_w 'w'
#define SINGLEQUOTED_x 'x'
#define SINGLEQUOTED_y 'y'
#define SINGLEQUOTED_z 'z'

#define MAX_VARS 256      // number of preallocated variables
#define MAX_LEN 256       // maximum length of strings


// list of data types
#define TYPE_gsl_matrix 1
#define gsl_matrix_pretty_print(p)					\
  {									\
    gsl_matrix * cur_m;							\
    int i,j; 								\
    cur_m = (gsl_matrix *)p;						\
    printf("Variable type: \tmatrix of size: %lu rows, %lu columns\n",	\
	   cur_m->size1,cur_m->size2);					\
    printf("Variable data:\n");						\
    for (i=0; i<cur_m->size1; i++) {					\
      for (j=0; j<cur_m->size2; j++) {					\
	printf("%9f ", gsl_matrix_get(cur_m, i, j));			\
      }									\
      printf("\n");							\
    }									\
  }
// #define gsl_matrix_free  // already defined in GSL library
#define gsl_matrix_duplicate(p)				\
  {							\
    gsl_matrix * m;					\
    gsl_matrix * new_m;					\
							\
    m = (gsl_matrix *)(p);				\
    p = new_m = gsl_matrix_alloc(m->size1, m->size2);	\
    gsl_matrix_memcpy(new_m,m);				\
  }



// struct for a single variable
typedef struct {
  void * data;
  char * name;
  int type;
} var_t;

// struct for the list of variables
typedef struct {
  // circular list of variables
  var_t var_list[MAX_VARS];
  //  unsigned int num_vars;
  int first_empty;
} state_t;


#define STRING_DESCR_f "Read from (f)ile"
#define operation_f(...)  read_matrix_from_file(__VA_ARGS__)
int read_matrix_from_file(state_t * state)
{
  gsl_matrix * m;
  unsigned int row, col;
  char buf[MAX_LEN];
  char * var_name;
  FILE *f;

  if (state->first_empty < 0) {
    printf("Exceeding pre-allocated variables. Increase the value %i of MAX_VARS\n", MAX_VARS);
    return 1;
  }
  printf("Expected: number n of rows, number m of columns\n");
  scanf("%i %i", &row, &col);

  printf("Expected: file name to read matrix from\n");
  scanf("%s", buf);
  if ((f=fopen(buf,"r")) == NULL) {
    printf("Unable to open file %s\n", buf);
    return 1;
  }

  m = gsl_matrix_alloc(row, col);
  if (gsl_matrix_fscanf(f,m)) {
    printf("Unable to read %i rows and %i columns from file \"%s\"\n",
	   row, col, buf);
    gsl_matrix_free(m);
    return 1;
  }
  fclose(f);

  printf("Expected: variable name\n");
  scanf("%s", buf);
  var_name = malloc(sizeof(*var_name)*(strlen(buf)+1));   // +1 to allocate 0
  strcpy(var_name, buf);

  // storing the matrix in the variable list and update the first_empty
  state->var_list[state->first_empty].data = m;
  state->var_list[state->first_empty].name = var_name;
  state->var_list[state->first_empty].type = TYPE_gsl_matrix;
  //  state->num_vars++;
  state->first_empty--;

  return 0;
}


#define STRING_DESCR_d "(D)uplicate variable"
#define CASE_DUP_VAR(type, p)	  \
  case TYPE_##type:		  \
  type##_duplicate(p);		  \
  break
#define operation_d(...) duplicate_var(__VA_ARGS__)
int duplicate_var(state_t * state)
{
  var_t new_var;

  gsl_matrix * old_m;
  gsl_matrix * new_m;
  char buf[MAX_LEN];
  char * new_name;

  if (state->first_empty >= MAX_VARS-1) {
    printf("Empty stack: unable to duplicate first variable\n");
    return 1;
  }

  if (state->first_empty < 0) {
    printf("Exceeding pre-allocated variables. Increase the value %i of MAX_VARS\n", MAX_VARS);
    return 1;
  }

  //read the variable to be duplicated
  new_var = state->var_list[(state->first_empty+1) % MAX_VARS];
  switch (new_var.type) {
    CASE_DUP_VAR(gsl_matrix,new_var.data);
  default:
    printf("Variable type: \tUNKNOWN\n");
    printf("Duplicating a variable of type %i must be implemented\n", new_var.type);
    return 1;
  }
  
  printf("Expected: duplicate variable name\n");
  scanf("%s", buf);
  new_var.name = malloc(sizeof(*new_var.name)*(strlen(buf)+1));   // +1 to allocate 0
  strcpy(new_var.name, buf);

  // storing the matrix in the variable list and update the first_empty
  state->var_list[state->first_empty] = new_var;
  //  state->num_vars++;
  state->first_empty--;

  return 0;
}


#define STRING_DESCR_p "(P)rint variable"
// type-dependent pretty print operation
#define CASE_PRINT_DATA(type)						\
  case TYPE_##type:							\
  type##_pretty_print(state->var_list[state->first_empty+1].data);	\
  break
#define operation_p(...)  print_1st_var(__VA_ARGS__)
int print_1st_var(const state_t * state)
{
  var_t cur_var;

  if (state->first_empty >= MAX_VARS-1) {
    printf("Empty stack: unable to print first variable\n");
    return 1;
  }

  //read the variable to be printed
  cur_var = state->var_list[(state->first_empty+1) % MAX_VARS];
  printf("Variable name: \t%s\n", cur_var.name);
  switch (cur_var.type) {
    CASE_PRINT_DATA(gsl_matrix);
  default:
    printf("Variable type: \tUNKNOWN\n");
    printf("Printing a variable of type %i must be implemented\n", cur_var.type);
  }
}


#define STRING_DESCR_l "Roll (l)eft"
#define operation_l(...) roll_left_list(__VA_ARGS__)
int roll_left_list(state_t * state)
{
  int i;
  var_t aux;

  if (state->first_empty >= MAX_VARS-2) {
    printf("Too few variables to be rotated\n");
    return 0;
  }
  aux=state->var_list[state->first_empty+1];
  for (i=state->first_empty+1; i<=MAX_VARS-2; i++)
    state->var_list[i]=state->var_list[i+1];
  state->var_list[MAX_VARS-1] = aux;
  return 0;
}

#define STRING_DESCR_r "Roll (r)ight"
#define operation_r(...) roll_right_list(__VA_ARGS__)
int roll_right_list(state_t * state)
{
  int i;
  var_t aux;

  if (state->first_empty >= MAX_VARS-2) {
    printf("Too few variables to be rotated\n");
    return 0;
  }
  aux=state->var_list[MAX_VARS-1];
  for (i=MAX_VARS-1; i >= state->first_empty+2; i--)
    state->var_list[i]=state->var_list[i-1];
  state->var_list[state->first_empty+1] = aux;
  return 0;
}

#define STRING_DESCR_e "(E)rase"
// type-dependent free operation
#define CASE_FREE_VAR_DATA(type)				 \
  case TYPE_##type:						 \
  type##_free(state->var_list[state->first_empty+1].data);	 \
  break
#define operation_e(...) erase_var(__VA_ARGS__)
void erase_var(state_t * state)
{
  switch (state->var_list[state->first_empty+1].type) {
    CASE_FREE_VAR_DATA(gsl_matrix);
  default:
    printf("Unknown type of variable \"%s\"\n",
	   state->var_list[state->first_empty+1].name);
    printf("Freeing a variable of type %i must be implemented\n",
	   state->var_list[state->first_empty+1].type);
  }
  free(state->var_list[state->first_empty+1].name);
  state->first_empty++;
  //  state->num_vars--;
}

#define EXIT_COMMAND 'x'
#define STRING_DESCR_x "E(x)it"
#define operation_x(...) free_all(__VA_ARGS__)
void free_all(state_t * state)
{
  while (state->first_empty < MAX_VARS-1)
    erase_var(state);
}

void print_status(const state_t * state)
{
  int i;

  printf("STATUS\n");
  if (state->first_empty >= MAX_VARS-1)
    printf("--- EMPTY STACK ---");
  for (i=1; i <= MAX_VARS-1-state->first_empty; i++)
    printf("%s ", state->var_list[(state->first_empty+i) % MAX_VARS].name);
  printf("\n");
}

char print_menu(const state_t * state)
/*
 * - print the status
 * - print the menu of available options
 * - return the selected option
 */
{
  char buf[MAX_LEN];
  
  print_status(state);
  printf("AVAILABLE OPERATIONS\n");
  printf("  %s\n", STRING_DESCR_f);
  printf("  %s\n", STRING_DESCR_d);
  printf("  %s\n", STRING_DESCR_p);
  printf("  %s\n", STRING_DESCR_l);
  printf("  %s\n", STRING_DESCR_r);
  printf("  %s\n", STRING_DESCR_e);
  printf("  %s\n", STRING_DESCR_x);
  do {
    printf("Enter your command: ");
    scanf("%s", buf);
    // check if alphabetic character
    if ((buf[0] & 0xDF) >= 'A' && (buf[0] & 0xDF) <= 'Z') {
      // make it lowcase
      buf[0] |= ' ';  //  buf[0] |= 32;  // buf[0] = buf[0] | 0x20;
      break;
    } else
      printf("Command must be an alphabetic character\n");
  } while (1);
  return buf[0];
}

int main()
{
  state_t state_vars; // state of all variables

  char command_cur;
  
  //  state_vars.num_vars = 0;
  state_vars.first_empty = MAX_VARS-1; //start from the last element
  do {
    command_cur = print_menu(&state_vars);
    switch (command_cur) {
#define CALC_OPTION(com,...)    \
  case SINGLEQUOTED_##com:      \
  operation_##com(__VA_ARGS__); \
  break
      CALC_OPTION(f, &state_vars);
      CALC_OPTION(p, &state_vars);
      CALC_OPTION(l, &state_vars);
      CALC_OPTION(r, &state_vars);
      CALC_OPTION(d, &state_vars);
      CALC_OPTION(e, &state_vars);
      CALC_OPTION(x, &state_vars);
    default:
      printf("Warning: unknown command\n");
    }
  } while (command_cur != EXIT_COMMAND);

  return 0;
}
