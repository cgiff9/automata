#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <time.h>

// MACHINE CONSTANTS
// Change the following three values as you see fit.
// Just remember to re-compile this dfa
// program when you change these constants:

const unsigned long input_string_max = 10000;		// unenforced minimum (be careful)
const unsigned long states_max = 200;				// absolute mimimum of 46 states	
const unsigned long state_name_max = 50;			// absolute minimum of 2 characters

// input_string_max = the maximum number of bits (0,1) allowed for an input string
// states_max = the maximum number of states allowed in a states file
// state_name_max = the maximum number of characters (letters) for a state name

// sleep function for slight pause on ACCEPTED strings
int nsleep(long miliseconds)
{
	struct timespec req, rem;

	if(miliseconds > 999)
	{   
		req.tv_sec = (int)(miliseconds / 1000);                            /* Must be Non-Negative */
		req.tv_nsec = (miliseconds - ((long)req.tv_sec * 1000)) * 1000000; /* Must be in range of 0 to 999999999 */
	}   
	else
	{   
		req.tv_sec = 0;                         /* Must be Non-Negative */
		req.tv_nsec = miliseconds * 1000000;    /* Must be in range of 0 to 999999999 */
	}   

	return nanosleep(&req , &rem);
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
	state->zero = state;
	state->one = state;
	state->start = start;
	state->final = final;

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

void States_destroy(struct State **states, int len)
{
	int i;
	for (i = 0; i < len; i++) {
		//printf("Destroying state %s...\n", states[i]->name);
		State_destroy(states[i]);
	}
}

int main(int argc, char *argv[]) 
{
	// READ INPUT STRING FROM CONSOLE OR FILE*
	char *input;
	if (argc == 4) {
		if (!strcmp(argv[2], "-f")) {
			FILE *string_file;	
			string_file = fopen(argv[3], "r+");
			if (string_file == NULL) {
				printf("Error opening %s\n", argv[3]);
				return -2;
			}
			char bits[input_string_max];
			fgets(bits, input_string_max, string_file); 
			input = bits;
			size_t ln = strlen(input) - 1;
			if (input[ln] == '\n') input[ln] = '\0';
		}
	} else if (argc == 3) {
		if (!strcmp(argv[2], "-f")) {
			printf("If invoking -f argument, please supply a valid file name\n"); 
			return -1;
		}
		input = argv[2];
	} else if (argc != 3) {
		return -1;
	}

	// OPEN STATES FILE
	FILE *lang = fopen(argv[1], "r");
	if (lang == NULL) {
		printf("Error opening %s\n", argv[1]);
		return -2;
	}

	// READ STATES FROM FILE
	unsigned long size = states_max;
	//int size = 200;						// minimum 46	
	char s_name[size][state_name_max];		// state NAME must contain at least 2 chars
	int s_start[size], s_final[size], s_zero[size], s_one[size];

	int i = 0;
	int j;	
	char buff[size];
	for (i = 0; fgets(buff, size, lang); i++) {
		int pos;
		if (sscanf(buff, 
					"[%d] NAME %s START %d FINAL %d ZERO %d ONE %d%n", 
					&j, 
					s_name[i], 
					&s_start[i], 
					&s_final[i],
					&s_zero[i],
					&s_one[i],
					&pos) != 6 || pos != strlen(buff) - 1) 
		{
			fprintf(stderr, "Error parsing states format: <%s>\n", buff);
			exit(1);
		} 
	}

	// SET NUMBER OF STATES
	int num_states = i;
	//printf("Number of states: %d\n", i);

	// ENSURE LEGAL NUMBER OF START AND END STATES
	int starts = 0;
	int finals = 0;
	for (i = 0; i < num_states; i++) {
		if (s_start[i]) starts += 1;
		if (s_final[i]) finals += 1;
		if (starts > 1) {
			printf("More than one start state detected. Aborting...\n");
			return -2;
		}
	}
	if (!starts) {
		printf("No start states detected. Aborting...\n");
		return -2;
	}
	if (!finals) {
		printf("No final states detected. Aborting...\n");
		return -2;
	}

	// CREATE "ARRAY" OF STATES
	struct State *states[num_states];

	// INITIALIZE START STATE
	struct State *state;

	// FILL ARRAY WITH STATE NAMES, START, FINAL
	for (i = 0; i < num_states; i++) {
		struct State *new_state = State_create(s_name[i], s_start[i], s_final[i]);
		states[i] = new_state;

		// Set start state
		if (new_state->start) state = new_state;;
	}

	// FILL ARRAY WITH TRANSITIONS
	for (i = 0; i < num_states; i++) {
		states[i]->zero = states[s_zero[i]];
		states[i]->one = states[s_one[i]];

		// Print states to console
		State_print(states[i]);
	}

	// PRINT INPUT STRING
	printf("INPUT: %s\n", input);

	// RUN MACHINE
	for(i = 0; input[i] != '\0'; i++) {
		printf("%c: %s -> ", input[i], state->name);
		if (input[i] == '0') {
			state = state->zero;
		} else if (input[i] == '1') {
			state = state->one;
		} else {

		}

		// Finish printing current transition
		printf("%s", state->name);
		if (state->final)
			printf(" (F)\n");
		else
			printf("\n");
	}

	// ACCEPTED OR REJECTED?
	if (state->final == 1) {
		printf("        (ACCEPTED):\n\t-->%s\n", input);
		States_destroy(states, num_states);

		// Print time output of accepted matches
		time_t rawtime;
		struct tm *info;
		char datebuf[80];

		time( &rawtime );
		info = localtime( &rawtime );
		strftime(datebuf,80,"%x %H:%M:%S", info);
		printf("         %s\n", datebuf);

		nsleep(250);
		return 0;
	} else if (state->final == 0) {
		printf("(REJECTED):\n\t-->%s\n", input);
		States_destroy(states, num_states);
		//nsleep(10);
		return 1;
	}
}
