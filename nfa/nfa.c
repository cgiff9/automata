#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <signal.h>

// MACHINE CONSTANT(S)
// Change the following values as you see fit.
// Just remember to re-compile this dfa
// program when you change these constants:

#define STATE_NAME_MAX 50			// undefined behavior for less than 2 characters
#define SLEEP_ACCEPT_MSEC 1000		// default when argument '-a 0'
#define SLEEP_REJECT_MSEC 100		// default when argument '-r 0'

int flag_verbose = 1;
int flag_string = 0;
int sleep_accept_msec = -1;
int sleep_reject_msec = -1;
char *input_string=NULL;
char *machine_file=NULL;

char datebuf[100];

void INThandler(int);

int nsleep(long miliseconds)
{
	struct timespec req, rem;
	if(miliseconds > 999)
	{   
		/* Must be Non-Negative */
		req.tv_sec = (int)(miliseconds / 1000); 
		/* Must be in range of 0 to 999999999 */
		req.tv_nsec = (miliseconds - 
				((long)req.tv_sec * 1000)) * 1000000; 
	}   
	else
	{   
		req.tv_sec = 0;							/* Must be Non-Negative */
		req.tv_nsec = miliseconds * 1000000;    /* Must be in range of 0 to 999999999 */
	}   
	return nanosleep(&req , &rem);
}

char* print_date()
{
	time_t rawtime;
	struct tm *info;
	//char datebuf[80];

	time( &rawtime );
	info = localtime( &rawtime );
	strftime(datebuf,80,"%x %H:%M:%S", info);
	(flag_verbose) && printf("         %s\n", datebuf);
	return datebuf;
}

char* string_to_bin(char* s) {
	if(s == NULL) return 0; /* no input string */
	size_t len = strlen(s);
	char *binary = malloc(len*8 + 1); // each char is one byte (8 bits) and + 1 at the end for null terminator
	binary[0] = '\0';
	for(size_t i = 0; i < len; ++i) {
		char ch = s[i];
		for(int j = 7; j >= 0; --j){
			if(ch & (1 << j)) {
				strcat(binary,"1");
			} else {
				strcat(binary,"0");
			}
		}
	}

	// trim leading zeroes
	int n;
	if ((n = strspn(binary, "0")) != 0 && binary[n] != '\0') {        
		return &binary[n];
	}
	return binary;
}

struct State{
	char *name;
	struct State **zero;
	struct State **one;
	struct State **empty;
	int zero_len, one_len, empty_len;
	int start;
	int final;
};

struct State *State_create(char *name, int start, int final)
{
	struct State *state = malloc(sizeof(struct State));
	assert(state != NULL);

	state->name = name;
	state->start = start;
	state->final = final;

	// Every state defaults to returning to itself
	// upon a '0' and a '1'
	state->zero = malloc(sizeof(struct State *));
	state->one = malloc(sizeof(struct State *));
	state->empty = malloc(sizeof(struct State *));

	state->zero_len = 0;
	state->one_len = 0;
	state->empty_len = 0;

	state->zero[0] = NULL;
	state->one[0] = NULL;
	state->empty[0] = NULL;

	return state;
}

void State_transition_add(int symbol, struct State *from, struct State *to)
{
	if (symbol == 0) {
		if (from->zero_len == 0) {
			from->zero[0] = to;
			from->zero_len++;
		} else {
			from->zero_len++;
			from->zero = realloc(from->zero, sizeof(struct State *) * from->zero_len);
			from->zero[from->zero_len-1]=to;
		}
	}
	if (symbol == 1) {
		if (from->one_len == 0) {
			from->one[0] = to;
			from->one_len++;
		} else {
			from->one_len++;
			from->one = realloc(from->one, sizeof(struct State *) * from->one_len);
			from->one[from->one_len-1]=to;
		}
	}
	if (symbol == 2) {
		if (from->empty_len == 0) {
			from->empty[0] = to;
			from->empty_len++;
		} else {
			from->empty_len++;
			from->empty = realloc(from->empty, sizeof(struct State *) * from->empty_len);
			from->empty[from->empty_len-1]=to;
		}
	}

}

void State_destroy(struct State *state)
{
	assert(state != NULL);
	free(state);
}

void State_print(struct State *state)
{
	printf("%s, START? %d, FINAL? %d, ZERO-> ", 
			state->name, state->start, state->final);
	int i;
	for (int i = 0; i < state->zero_len; i++) {
		printf("%s,", state->zero[i]->name);
	}
	printf(" ONE-> ");
	for (int i = 0; i < state->one_len; i++) {
		printf("%s,", state->one[i]->name);
	}
	printf(" EMPTY-> ");
	for (int i = 0; i < state->empty_len; i++) {
		printf("%s,", state->empty[i]->name);
	}
	printf("\n");
}

// Destroying an array of states of a given length
void States_destroy(struct State **states, int len)
{
	for (int i = 0; i < len; i++) {
		State_destroy(states[i]);
	}
	//(flag_verbose) && printf("Closed machine %s\n", machine_file);

}

// Used on machine file import
void strip_extra_spaces(char* str) {
	int i, x;
	for(i=x=0; str[i]; ++i)
		if(!isspace(str[i]) || (i > 0 && !isspace(str[i-1])))
			str[x++] = str[i];
	str[x] = '\0';
}

int is_empty(const char *s) {
	while (*s != '\0') {
		if (!isspace((unsigned char)*s)) return 0;
		s++;
	}
	return 1;
}

/*
   static volatile int keep_running=1;
   void int_handler(int dummy)
   {
   keep_running = 0;
   }
   */

// INITIALIZE FIRST STATE
struct State *state;
struct State *state_start;

int num_states = 0;
char *output_accept_file = NULL;
int accepted = 0;
int rejected = 0;
int nfa_run(char *run_input_string, struct State *run_state)
{
	// RUN MACHINE
	if (run_state->final==1 && run_input_string[0] == '\0' ) {
		accepted = 1;
	} else if (run_state->empty[0] != NULL) {
		for (int j = 0; j < run_state->empty_len; j++) {
			(flag_verbose) && printf("[E]%.20s: %s -> %s", run_input_string,
					run_state->name,run_state->empty[j]->name);
			if (flag_verbose) {
				if (run_state->empty[j]->final) printf(" (F)\n"); else printf("\n");
			}
			nfa_run(run_input_string, run_state->empty[j]);
		}
	}
	if (run_input_string[0] == '0') {
		for (int j = 0; j < run_state->zero_len; j++) {
			(flag_verbose) && printf("[%c]%.20s: %s -> %s", run_input_string[0], run_input_string+1, 
					run_state->name,run_state->zero[j]->name);
			if (flag_verbose) {
				if (run_state->zero[j]->final) printf(" (F)\n"); else printf("\n");
			}
			nfa_run(run_input_string+1, run_state->zero[j]);
		}
	} else if (run_input_string[0] == '1') {
		for (int j = 0; j < run_state->one_len; j++) {
			(flag_verbose) && printf("[%c]%.20s: %s -> %s", run_input_string[0], run_input_string+1,
					run_state->name,run_state->one[j]->name);
			if (flag_verbose) {
				if (run_state->one[j]->final) printf(" (F)\n"); else printf("\n");
			}
			nfa_run(run_input_string+1, run_state->one[j]);
		}
	}
}

int nfa_init(char *run_input_string) {	
	nfa_run(run_input_string, state_start);

	// ACCEPTED OR REJECTED?
	//	if (nfa_run(run_input_string) == 1) {
	if (accepted == 1) {
		(flag_verbose) && printf("(ACCEPTED):\n\t==>%s\n", input_string);
		if (flag_verbose && !output_accept_file) print_date();

		// output accepted strings to a supplied file
		if (output_accept_file != NULL ) {
			FILE *output_accept_fp;
			output_accept_fp = fopen(output_accept_file, "a");

			if (output_accept_fp == NULL) {
				fprintf(stderr, "Error opening output accepted strings file %s", output_accept_file);
				return 3;
			}
			fprintf(output_accept_fp, "(ACCEPTED):\n\t==>%s\n\t%s\n", input_string, print_date());
			fclose(output_accept_fp);

		}

		// Delay output on accept?
		if (flag_verbose && sleep_accept_msec >=0) {
			if (sleep_accept_msec == 0)
				sleep_accept_msec = SLEEP_ACCEPT_MSEC;
			nsleep(sleep_accept_msec);
		}
		return 0;

	} else {
		(flag_verbose) && printf("(REJECTED):\n\t==>%s\n", run_input_string);

		// Delay output on reject?
		if (flag_verbose && sleep_reject_msec >=0) {
			if (sleep_reject_msec == 0)
				sleep_reject_msec = SLEEP_REJECT_MSEC;
			nsleep(sleep_reject_msec);
		}
	
		return 1;
	}
}

int main (int argc, char **argv)
{
	char *input_string_file = NULL;
	//char *output_accept_file = NULL;

	int index;
	int opt;
	opterr = 0;
	int nonopt_index=0;	
	while ((opt = getopt (argc, argv, "-:qsa:r:f:o:")) != -1)
	{
		switch (opt)
		{
			case 'q':
				flag_verbose = 0;
				break;
			case 's':
				flag_string = 1;
				break;
			case 'a':
				sleep_accept_msec = atoi(optarg);
				break;
			case 'r':
				sleep_reject_msec = atoi(optarg);
				break;
			case 'f':
				input_string_file = optarg;
				break;
			case 'o':
				output_accept_file = optarg;
				break;
			case '?':
				fprintf (stderr, "Unknown option `-%c'.\n", optopt);
				return 2;
			case ':':
				fprintf (stderr, "Option -%c requires an argument.\n", optopt);
				return 2;
			case 1:
				switch (nonopt_index)
				{
					// 1st non-option index is always the machine file
					// 2nd non-option index is always the input string
					case 0:
						machine_file = optarg;
						break;
					case 1:
						input_string = optarg;
						break;
				}
				nonopt_index++;
		}
	}

	/*
	// PARAMTER DEBUGGING
	printf ("\t(-q)  flag_verbose = %d\n"
	"\t(-s)  flag_string = %d\n"
	"\t(-a*)  sleep_accept_msec = %d\n"
	"\t(-r*)  sleep_reject_msec = %d\n"
	"\t(-f*) input_string_file = %s\n"
	"\t(-o*) output_accept_file = %s\n"
	"\t(1st) machine_file = %s\n"
	"\t(2nd) input_string= %s\n",
	flag_verbose, sleep_accept_msec, sleep_reject_msec, input_string_file, output_accept_file, machine_file, input_string);
	// return 0;
	*/

	if (input_string_file && input_string) {
		(flag_verbose) && printf ("Input string file ignored due to prescence of direct input string.\n");
	} else if (!input_string_file && !input_string) {
		fprintf(stderr, "No input string supplied. Supply via command-line for via file with '-f' paramter\n");
		return 1;
	}

	// OPEN MACHINE FILE
	FILE *machine_fp;
	if (machine_file != NULL) {
		machine_fp = fopen(machine_file, "r");
		if (machine_fp == NULL) {
			fprintf(stderr, "Error opening %s\n", machine_file);
			return 3;
		}
	}

	// DETERMINE NUMBER OF STATES IN MACHINE FILE
	// MACHINE FILE ERROR HANDLING
	char *line = NULL;
	size_t linelen = 0;
	int num_lines = 0;
	int scanned;
	while( getline(&line, &linelen, machine_fp) > 0 ) {
		num_lines++;
		strip_extra_spaces(line);

		int d_start, d_final;
		char d_name[STATE_NAME_MAX], d_zero[STATE_NAME_MAX], d_one[STATE_NAME_MAX],d_empty[STATE_NAME_MAX];
		scanned = sscanf(line, "NAME %s START %d FINAL %d ZERO %s ONE %s EMPTY %s",
				d_name, &d_start, &d_final, d_zero, d_one, d_empty);
		if (scanned == 6) {
			num_states=num_states+1;
			/* // Machinefile parser debugging
			   } else if (scanned == -1 && is_empty(line)) {
			   printf("Found empty line at line %d\n", num_lines);
			   } else if (scanned == 0 && line[0] == '#') {
			   printf("Found commented line at line %d\n", num_lines);
			   } else {
			   */
			// Allow blank and commented (#) lines
		} else if ( !(scanned == -1 && is_empty(line)) && !(scanned == 0 && line[0] == '#')) {
			line[strcspn(line, "\r\n")] = 0;
			fprintf(stderr, "Error importing state on line %d:\n%s\n", num_lines, line);
			return 3;
		}
	}
	(flag_verbose) && printf("Machine file %s contains %d legal states.\n", machine_file, num_states);
	fseek(machine_fp, 0, SEEK_SET);

	// PARSE STATES FROM MACHINE FILE INTO ARRAYS
	char s_name[num_states][STATE_NAME_MAX], 
		 s_zero[num_states][STATE_NAME_MAX], 
		 s_one[num_states][STATE_NAME_MAX],
		 s_empty[num_states][STATE_NAME_MAX];
		 int s_start[num_states], s_final[num_states], s_zero_indexes[num_states], s_one_indexes[num_states];

		 char state_strings[num_states];
		 line = NULL;
		 linelen = 0;
		 num_lines = 0;
		 int i = 0;
		 while( getline(&line, &linelen, machine_fp) > 0 ) {
			 num_lines++;
			 strip_extra_spaces(line);
			 scanned = sscanf(line, "NAME %s START %d FINAL %d ZERO %s ONE %s EMPTY %s",
					 s_name[i], 
					 &s_start[i], 
					 &s_final[i], 
					 s_zero[i], 
					 s_one[i],
					 s_empty[i]);
			 if (scanned == 6) {
				 i=i+1;
				 // Allow blank and commented (#) lines
			 } else if ( !(scanned == -1 && is_empty(line)) && !(scanned == 0 && line[0] == '#')) {
				 line[strcspn(line, "\r\n")] = 0;
				 fprintf(stderr, "Error importing state on line %d:\n%s\n", num_lines, line);
				 return 3;
			 }
		 }
		 fclose(machine_fp);

		 // Ensure states have unique names
		 for (int i=0; i < num_states; i++) {
			 for (int j=0; j < num_states; j++) {
				 if (!(strcmp(s_name[i], s_name[j])) && i != j) {
					 fprintf(stderr, "Duplicate state names detected in machine file: [%s]\n", s_name[i]);
					 return 4;
				 }
			 }
		 }

		 // ENSURE LEGAL NUMBER OF START AND END STATES
		 int starts = 0;
		 int finals = 0;
		 //int i = 0;
		 for (i = 0; i < num_states; i++) {
			 if (s_start[i]) { 
				 starts += 1;
			 }
			 if (starts > 1) {
				 fprintf(stderr, "More than one start state detected. Aborting...\n");
				 return 4;
			 }
			 if (s_final[i]) finals += 1;
		 }

		 if (!starts) {
			 fprintf(stderr, "No start states detected. Aborting...\n");
			 return 4;
		 }
		 if (!finals) {
			 fprintf(stderr, "No final states detected. Aborting...\n");
			 return 4;
		 }

		 // CREATE ARRAY OF STATES
		 struct State *states[num_states];

		 // FILL ARRAY WITH STATE NAMES, START, FINAL
		 for (i = 0; i < num_states; i++) {
			 struct State *new_state = State_create(s_name[i], 
					 s_start[i], 
					 s_final[i]);
			 states[i] = new_state;

			 // Set start state
			 if (new_state->start) state_start = new_state;
		 }

		 // FILL STATE ARRAY WITH TRANSITIONS
		 for (i = 0; i < num_states; i++) {
			 char *state_zero = s_zero[i];
			 char *state_one = s_one[i];
			 char *state_empty = s_empty[i];

			 if (strcasecmp(state_zero, "x") != 0) {
				 char *single_zero;
				 single_zero = strtok(state_zero, ",");
				 while(single_zero != NULL) {
					 //printf("zero_state: %s\n", single_zero);
					 int j;
					 for (j=0; strcmp(single_zero, states[j]->name); j++);
					 State_transition_add(0, states[i], states[j]);

					 single_zero = strtok(NULL, ",");
				 }
			 }

			 if (strcasecmp(state_one, "x") != 0) {
				 char *single_one;
				 single_one = strtok(state_one, ",");
				 while(single_one != NULL) {
					 //printf("one_state: %s\n", single_one);
					 int j;
					 for (j=0; strcmp(single_one, states[j]->name); j++);
					 State_transition_add(1, states[i], states[j]);

					 single_one = strtok(NULL, ",");
				 }
			 }

			 if (strcasecmp(state_empty, "x") != 0) {
				 char *single_empty;
				 single_empty = strtok(state_empty, ",");
				 while(single_empty != NULL) {
					 //printf("empty_state: %s\n", single_empty);
					 int j;
					 for (j=0; strcmp(single_empty, states[j]->name); j++);
					 State_transition_add(2, states[i], states[j]);

					 single_empty = strtok(NULL, ",");
				 }
			 }

			 if (flag_verbose) State_print(states[i]);

		 }

		 if (input_string_file && !input_string)
		 {
			 // READ INPUT STRING(S) FROM FILE
			 FILE *input_string_fp;
			 char *input_string_bits = NULL;
			 size_t len = 0;
			 ssize_t read;

			 input_string_fp = fopen(input_string_file, "r");
			 if (input_string_fp == NULL) {
				 fprintf(stderr, "Error opening %s\n", input_string_file);
				 return 3;
			 }

			 while ((read = getline(&input_string_bits, &len, input_string_fp)) != -1)
			 {
				 input_string = input_string_bits;
				 input_string[strcspn(input_string, "\r\n")] = 0;

				 if (flag_string) {
					 char *input_ascii = string_to_bin(input_string);
					 input_string = input_ascii;
				 }
				 state = state_start;
				 nfa_init(input_string);

				 /*
					signal(SIGINT, int_handler);
					while(!keep_running) {
					fclose(input_string_fp);
					States_destroy(states, num_states);
					return 0;
					}
					*/
			 }
			 fclose(input_string_fp);

		 } else {
			 if (flag_string) {
				 char *input_ascii = string_to_bin(input_string);
				 input_string = input_ascii;
			 }
			 state = state_start;
			 nfa_init(input_string);
		 }

		 States_destroy(states, num_states);

		 return 0;
}
