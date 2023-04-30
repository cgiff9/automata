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
int dfa_run()
{
	// SET START STATE
	state=state_start;

	// RUN MACHINE
	for (int i = 0; input_string[i] != '\0'; i++) {
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
		//free(input_string);
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
		//free(input_string);
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
		char d_name[STATE_NAME_MAX], d_zero[STATE_NAME_MAX], d_one[STATE_NAME_MAX];
		scanned = sscanf(line, "NAME %s START %d FINAL %d ZERO %s ONE %s",
					d_name, &d_start, &d_final, &d_zero, &d_one);
		if (scanned == 5) {
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
		 s_one[num_states][STATE_NAME_MAX];
	int s_start[num_states], s_final[num_states], s_zero_indexes[num_states], s_one_indexes[num_states];

	char state_strings[num_states];
	line = NULL;
	linelen = 0;
	num_lines = 0;
	int i = 0;
	while( getline(&line, &linelen, machine_fp) > 0 ) {
		num_lines++;
		strip_extra_spaces(line);
		scanned = sscanf(line, "NAME %s START %d FINAL %d ZERO %s ONE %s",
					s_name[i], 
					&s_start[i], 
					&s_final[i], 
					&s_zero[i], 
					&s_one[i]);
		if (scanned == 5 ) {
			i=i+1;
		// Allow blank and commented (#) lines
		} else if ( !(scanned == -1 && is_empty(line)) && !(scanned == 0 && line[0] == '#')) {
			line[strcspn(line, "\r\n")] = 0;
			fprintf(stderr, "Error importing state on line %d:\n%s\n", num_lines, line);
			return 3;
		}
	}
	fclose(machine_fp);

	/* // DEBUGGING STATE ARRAY vs. MACHINE FILE STATE INDEXES
	for (int k=0; k < num_states; k++) {
		printf ("state_indexes[%d] = %d\n", k, s_indexes[k]);
	}
	*/

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

	// MATCH TRANSITION STRINGS WITH STATE INDEXES
	for (i = 0; i < num_states; i++) {
		char *state_zero = s_zero[i];
		char *state_one = s_one[i];
		int j;
		for (j = 0; strcmp(state_zero, states[j]->name); j++);
		s_zero_indexes[i]=j;			
		for (j = 0; strcmp(state_one, states[j]->name); j++);
		s_one_indexes[i]=j;			
	}

	// FILL ARRAY WITH TRANSITIONS
	for (i = 0; i < num_states; i++) {
		states[i]->zero = states[s_zero_indexes[i]];
		states[i]->one = states[s_one_indexes[i]];

		// Print states to console
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
			dfa_run();
			
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
		dfa_run();
	}

	States_destroy(states, num_states);

	return 0;
}
