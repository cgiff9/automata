#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

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
	printf("%s, ZERO->%s, ONE->%s, START? %d, FINAL? %d\n", 
          state->name, state->zero->name, state->one->name, state->start, state->final);
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
			char bits[10000];
			fgets(bits, 10000, string_file); 
			input = bits;
			size_t ln = strlen(input) - 1;
			if (input[ln] == '\n') input[ln] = '\0';
		}
	} else if (argc == 3) {
		if (!strcmp(argv[2], "-f")) {
			printf("Argument error\n"); 
			return -1;
		}
		input = argv[2];
	} else if (argc != 3) {
		return -1;
	}

	// PARSE STATES FILE
	FILE *lang = fopen(argv[1], "r");
	if (lang == NULL) {
		printf("Error opening %s\n", argv[1]);
		return -2;
	}

	// READ STATES FROM FILE
	int size = 46;        // state NAME must contain ONLY two chars	
	char s_name[size][3]; //
	int s_start[size], s_final[size], s_zero[size], s_one[size];

	int i = 0;
	int j;	
	char buff[size];
	for (i = 0; fgets(buff, size, lang); i++) {
		int pos;
		if (sscanf(buff, 
                 "[%d] NAME %s START %d FINAL %d ZERO-> %d ONE-> %d%n", 
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
	printf("INPUT: %s\n", input);
	
	// SET NUMBER OF STATES
	int num_states = i;

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
	}

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
		printf("ACCEPTED.\n");
		States_destroy(states, num_states);
		return 0;
	} else if (state->final == 0) {
		printf("REJECTED.\n");
		States_destroy(states, num_states);
		return 1;
	}
}
