#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <time.h>

// MACHINE CONSTANTS
// Change the following three values as you see fit.
// Just remember to re-compile this dfa
// program when you change these constants:

#define INPUT_STRING_MAX 10000		// applies only to input strings supplied by file (-f argument)
#define STATES_MAX 200				// undefined behavior for less than 46 states
#define STATE_NAME_MAX 50			// undefined behavior for less than 2 characters

// OTHER CONSTANTS
#define SLEEP_ACCEPT_MSEC 1000
#define SLEEP_REJECT_MSEC 100

int flag_verbose = 1;
int sleep_accept_msec = -1;
int sleep_reject_msec = -1;
char *input_string=NULL;
char *machine_file=NULL;

char datebuf[100];

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

struct State{
	char *name;
	struct State *zero;
	struct State *one;
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
	state->zero = state;
	state->one = state;

	return state;
}

void State_destroy(struct State *state)
{
	assert(state != NULL);
	free(state);
}

void State_print(struct State *state)
{
	printf("%s, START? %d, FINAL? %d, ZERO-> %s, ONE-> %s\n", 
			state->name, state->start, state->final, 
			state->zero->name, state->one->name);
}

// Destroying an array of states of a given length
void States_destroy(struct State **states, int len)
{
	int i;
	for (i = 0; i < len; i++) {
		//printf("Destroying state %s...\n", states[i]->name);
		State_destroy(states[i]);
	}
}

int main (int argc, char **argv)
{
	char *input_string_file = NULL;
	char *output_accept_file = NULL;

	int index;
	int opt;

	opterr = 0;

	int nonopt_index=0;	
	while ((opt = getopt (argc, argv, "-:qa:r:f:o:")) != -1)
	{
		switch (opt)
		{
			case 'q':
				flag_verbose = 0;
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
	"\t(-a*)  sleep_accept_msec = %d\n"
	"\t(-r*)  sleep_reject_msec = %d\n"
	"\t(-f*) input_string_file = %s\n"
	"\t(-o*) output_accept_file = %s\n"
	"\t(1st) machine_file = %s\n"
	"\t(2nd) input_string= %s\n",
	flag_verbose, sleep_accept_msec, sleep_reject_msec, input_string_file, output_accept_file, machine_file, input_string);
	// return 0;
	*/

	if (input_string_file && input_string) 
	{
		(flag_verbose) && printf ("Input string file ignored due to prescence of direct input string.\n");
	}

	if (input_string_file && !input_string)
	{
		(flag_verbose) && printf ("Reading input string from file %s:\n", input_string_file);

		FILE *input_string_filestream;
		char input_string_bits[INPUT_STRING_MAX];

		input_string_filestream = fopen(input_string_file, "r");
		if (input_string_filestream == NULL) {
			fprintf(stderr, "Error opening %s\n", input_string_file);
			return 3;
		}

		fgets(input_string_bits, INPUT_STRING_MAX, input_string_filestream);
		input_string = input_string_bits;
		fclose(input_string_filestream);

		// Remove newline from input string
		//size_t ln = strlen(input_string) - 1;
		//if (input_string[ln] == '\n') input_string[ln] = '\0';
		input_string[strcspn(input_string, "\r\n")] = 0;

	} else {
		(flag_verbose) && printf("Reading input string directly from command line:\n");
	}

	(flag_verbose) && printf ("\t==>%s\n",input_string);

	// OPEN MACHINE FILE
	FILE *machine_filestream;
	if (machine_file != NULL) {
		machine_filestream = fopen(machine_file, "r");
		if (machine_filestream == NULL) {
			fprintf(stderr, "Error opening %s\n", machine_file);
			return 3;
		}
	}

	// READ STATES FROM FILE
	char s_name[STATES_MAX][STATE_NAME_MAX];
	int s_start[STATES_MAX], s_final[STATES_MAX], s_zero[STATES_MAX], s_one[STATES_MAX];

	int num_states = 0;
	int s_indexes[STATES_MAX];
	char state_strings[STATES_MAX];
	for (num_states = 0; fgets(state_strings, STATES_MAX, machine_filestream); num_states++) {
		int pos;
		if (sscanf(state_strings, 
					"[%d] NAME %s START %d FINAL %d ZERO %d ONE %d%n", 
					&s_indexes[num_states], 
					s_name[num_states], 
					&s_start[num_states], 
					&s_final[num_states],
					&s_zero[num_states],
					&s_one[num_states],
					&pos) != 6 || pos != strlen(state_strings) - 1) 
		{
			fprintf(stderr, "Error parsing machine file %s\n"
					"Maximum allowable states is %d.\n"
					"Please adjust the STATES_MAX definition in dfa.c and recompile if you need more states.\n",
					machine_file, STATES_MAX);

			return 4;
		} 
	}

	fclose(machine_filestream);

	/* DEBUGGING STATE ARRAY vs. STATE INDEXES
	for (int k=0; k < num_states; k++) {
		printf ("state_indexes[%d] = %d\n", k, s_indexes[k]);
	}
	*/

	// Ensure indexes in machine file are unique
	for (int i=0; i < num_states; i++) {
		for (int j=0; j < num_states; j++) {
			if (s_indexes[i] == s_indexes[j] && i != j) {
				fprintf(stderr, "Duplicate indexes detected in machine file: [%d]\n", s_indexes[i]);
				return 4;
			}
		}
	}
	//return 0;

		//printf("Number of states: %d\n", num_states);
		

	// ENSURE LEGAL NUMBER OF START AND END STATES
	int starts = 0;
	int finals = 0;
	int i = 0;
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

	// INITIALIZE FIRST STATE
	struct State *state;

	// FILL ARRAY WITH STATE NAMES, START, FINAL
	int state_index = s_indexes[0];
	for (i = 0; i < num_states; i++) {
		struct State *new_state = State_create(s_name[i], 
				s_start[i], 
				s_final[i]);
		states[state_index] = new_state;

		// Set start state
		if (new_state->start) state = new_state;

		state_index = s_indexes[i+1];
	}

	// FILL ARRAY WITH TRANSITIONS
	state_index = s_indexes[0];
	for (i = 0; i < num_states; i++) {
		states[state_index]->zero = states[s_zero[i]];
		states[state_index]->one = states[s_one[i]];

		// Print states to console
		if (flag_verbose) State_print(states[i]);

		state_index = s_indexes[i+1];
	}

	// RUN MACHINE
	for (i = 0; input_string[i] != '\0'; i++) {
		(flag_verbose) && printf("%c: %s -> ", input_string[i], state->name);
		if (input_string[i] == '0') {
			state = state->zero;
	//} else if (input_string[i] == '1') {
	//	state = state->one;
	} else {
		state = state->one;
	}

	// Finish printing current transition
	(flag_verbose) && printf("%s", state->name);
	if (state->final)
		(flag_verbose) && printf(" (F)\n");
	else
		(flag_verbose) && printf("\n");
	}

	// ACCEPTED OR REJECTED?
	if (state->final == 1) {
		(flag_verbose) && printf("(ACCEPTED):\n\t==>%s\n", input_string);
		if (flag_verbose && !output_accept_file) print_date();
	
		// output accepted strings to a supplied file
		if (output_accept_file != NULL ) {
			FILE *output_accept_filestream;
			output_accept_filestream = fopen(output_accept_file, "a");

			if (output_accept_filestream == NULL) {
				fprintf(stderr, "Error opening output accepted strings file %s", output_accept_file);
				return 3;
			}
			fprintf(output_accept_filestream, "(ACCEPTED):\n\t==>%s\n\t%s\n", input_string, print_date());
			fclose(output_accept_filestream);

		}

		// Delay output on accept?
		if (flag_verbose && sleep_accept_msec >=0) {
			if (sleep_accept_msec == 0)
				sleep_accept_msec = SLEEP_ACCEPT_MSEC;
			nsleep(sleep_accept_msec);
		}
		return 0;

	//} else if (state->final == 0) {
	} else {
		(flag_verbose) && printf("(REJECTED):\n\t==>%s\n", input_string);
		
		// Delay output on reject?
		if (flag_verbose && sleep_reject_msec >=0) {
			if (sleep_reject_msec == 0)
				sleep_reject_msec = SLEEP_REJECT_MSEC;
			nsleep(sleep_reject_msec);
		}
		return 1;
	}

	States_destroy(states, num_states);

	return 0;
}

