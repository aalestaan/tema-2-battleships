//Stan Alexandra 312CD

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// Structure to organize data for a single ship

typedef struct {
	int total_ships;
	int total_shots;
	int successful_hits;
} matchstats;

typedef struct {
	int index;
	double score;
	int K;
} matcheval;

typedef struct {
	int x_start;
	int y_start;
	char type;
	char orient;
	int hits;
	int destroyed;
} Ship;

// Structure to organize data for a player
typedef struct {
	char **board;
	Ship *ships;
	int total_ships;
	int valid_ships_count;
	int destroyed_ships_count;

	int total_shots;
	int successful_shots;
} Player;

int custom_round(double d)
{
	return (int)(d < 0.0 ? d - 0.5 : d + 0.5);
}

// Returns the length of the ship based on its type
int get_length(char type)
{
	if (type == 'S')
		return 5;
	if (type == 'Y')
		return 4;
	if (type == 'B')
		return 3;
	if (type == 'L')
		return 2;
	if (type == 'A')
		return 1;

	return 0;
}

// Returns the full name of the ship (for error display)
const char *get_ship_name(char type)
{
	if (type == 'S')
		return "Shinano";
	if (type == 'Y')
		return "Yamato";
	if (type == 'B')
		return "Belfast";
	if (type == 'L')
		return "Laffey";
	if (type == 'A')
		return "Albacore";

	return "Unknown";
}

// Dynamically allocates the game board (N x M)
char **allocate_board(int n, int m)
{
	char **board = (char **)malloc(n * sizeof(char *));
	if (!board)
		return NULL;
	for (int r = 0; r < n; r++) {
		board[r] = (char *)malloc(m * sizeof(char));
		if (!board[r]) {
		// Cleanup on allocation error
			for (int k = 0; k < r; k++)
				free(board[k]);
			free(board);
			return NULL;
		}
		for (int l = 0; l < m; l++) {
			board[r][l] = '0'; // '0' represents empty water
		}
	}
	return board;
}

// Deallocates the game board (N x M)
void deallocate_board(char **board, int n)
{
	if (board) {
		for (int r = 0; r < n; r++) {
			if (board[r])
				free(board[r]);
		}
		free(board);
	}
}

// Calculates the total number of ships required and allocates the Ship* array
int initialize_player(Player *p, int n, int m, char **board_ptr)
{
	int shinao = (n * m) / 70;
	int yamato = (n * m) / 55;
	int belfast = (n * m) / 40;
	int laffey = (n * m) / 30;
	int albacore = (n * m) / 20;
	int total = shinao + yamato + belfast + laffey + albacore;

	p->total_ships = total;
	p->valid_ships_count = 0;
	p->destroyed_ships_count = 0;
	p->board = board_ptr;

	if (total > 0) {
		p->ships = (Ship *)calloc(total, sizeof(Ship));
		if (!p->ships)
			return 0;
	}
	return total;
}

// Deallocates the ship vector of a player
void deallocate_player_ships(Player *p)
{
	if (p->ships) {
		free(p->ships);
	}
}

// Checks placement validity (boundaries and overlap)
int check_placement(char **board, int n, int m, char type, char orient,
					int x, int y)
{
	int L = get_length(type);

	if (x < 1 || x > n || y < 1 || y > m)
		return 0;

	if (orient == 'H') {
		if (y + L - 1 > m)
			return 0;
		// Check overlap
		for (int c_idx = y - 1; c_idx < y - 1 + L; c_idx++) {
			if (board[x - 1][c_idx] != '0')
				return 0;
		}
	} else if (orient == 'V') {
		if (x - L + 1 < 1)
			return 0;
		// Check overlap
		for (int r_idx = x - 1; r_idx > x - 1 - L; r_idx--) {
			if (board[r_idx][y - 1] != '0')
				return 0;
		}
	}
	return 1;
}

// Places a valid ship on the player's board and records data
void place_ship(Player *p, char type, char orient, int x, int y)
{

	int L = get_length(type);

	// Mark positions on the board with ship length ('1' to '5')
	if (orient == 'H') {
		for (int c_idx = y - 1; c_idx < y - 1 + L; c_idx++) {
			p->board[x - 1][c_idx] = L + '0';
		}
	} else { // 'V'
		for (int r_idx = x - 1; r_idx >= x - L; r_idx--) {
			p->board[r_idx][y - 1] = L + '0';
		}
	}

	// Store data in the structure
	int current_idx = p->valid_ships_count;
	p->ships[current_idx].x_start = x;
	p->ships[current_idx].y_start = y;
	p->ships[current_idx].type = type;
	p->ships[current_idx].orient = orient;
	p->ships[current_idx].hits = 0;
	p->ships[current_idx].destroyed = 0;

	p->valid_ships_count++;
}

// Main loop for reading and placing ships
void run_placement_phase(Player *p1, Player *p2, int n, int m)
{

	// Read up to 2 * total_ships commands (includes failed placement attempts)
	int total_to_read = p1->total_ships + p2->total_ships;

	for (int k = 0; k < 2 * total_to_read; k++) {
		char type, orient;
		int x, y, ok = 0;

		Player *current_p = (k % 2 == 0) ? p1 : p2;

		// Skip turns for players who have placed all required ships
		if (current_p->valid_ships_count >= current_p->total_ships)
			continue;

		while (!ok) {
			// Read placement attempt
			scanf(" %c %c %d %d", &type, &orient, &x, &y);

			// Check if the ship type is valid and placement is valid
			if (current_p->valid_ships_count < current_p->total_ships &&
				check_placement(current_p->board, n, m, type, orient, x, y)) {

				place_ship(current_p, type, orient, x, y);
				ok = 1;
			} else {
				// Invalid placement print error and re-read for the same player
				printf("Nava %s nu poate fi amplasata %s la coordonatele (%d, %d).\n",
						get_ship_name(type), (orient == 'V' ? "vertical" : "orizontal"), x, y);
			}
		}
	}
}

// Finds the index of the hit ship among the opponent's list
int find_hit_ship(Player *opponent, int r, int c, int ship_length)
{

	for (int j = 0; j < opponent->valid_ships_count; j++) {
		Ship *s = &opponent->ships[j];
		if (s->destroyed == 1)
			continue;

		int L = get_length(s->type);
		if (L != ship_length)
			continue;

		// Check if the (r, c) coordinate is part of the ship occupied space
		if (s->orient == 'H') {
			if (r == s->x_start - 1 && c >= s->y_start - 1 &&
				c <= s->y_start - 1 + L - 1) {
				return j;
			}
		} else if (s->orient == 'V') {
			if (c == s->y_start - 1 && r <= s->x_start - 1 &&
				r >= s->x_start - 1 - L + 1) {
				return j;
			}
		}
	}
	return -1;
}

// Processes the destruction of a ship and applies the 'D'/'F' markers
void process_destruction(Player *opponent, int ship_idx, int attacker)
{

	Ship *s = &opponent->ships[ship_idx];
	s->destroyed = 1;
	opponent->destroyed_ships_count++;

	int L = get_length(s->type);
	int x_start = s->x_start;
	int y_start = s->y_start;
	int x_end, y_end;

	// Marking cells with 'D' (hit/destroyed) or 'F' (unhit/destroyed)
	// 'F' is the marker for a cell that was part of a destroyed ship
	if (s->orient == 'H') {
		for (int c_idx = y_start - 1; c_idx < y_start - 1 + L; c_idx++) {
			if (opponent->board[x_start - 1][c_idx] != 'X') {
				opponent->board[x_start - 1][c_idx] = 'F';
				// Unhit part of destroyed ship
			} else {
				opponent->board[x_start - 1][c_idx] = 'D';
				// Hit part of destroyed ship
			}
		}
		x_end = x_start;
		y_end = y_start + L - 1;
	} else { // 'V'
		for (int r_idx = x_start - 1; r_idx >= x_start - 1 - L + 1; r_idx--) {
			if (opponent->board[r_idx][y_start - 1] != 'X') {
				opponent->board[r_idx][y_start - 1] = 'F';
				// Unhit part of destroyed ship
			} else {
				opponent->board[r_idx][y_start - 1] = 'D';
				// Hit part of destroyed ship
			}
		}
		x_end = x_start - L + 1;
		y_end = y_start;
	}

	// Display destruction message

	printf("Jucatorul %d a distrus o nava %s plasata intre coordonatele (%d, %d) si (%d, %d).\n", attacker, get_ship_name(s->type), x_start, y_start, x_end, y_end);

}

// Main loop for the attack phase
void run_attack_phase(Player *p1, Player *p2, int n, int m)
{

	int player = 0; // 0 for P1, 1 for P2

	while (p1->destroyed_ships_count < p1->valid_ships_count &&
		p2->destroyed_ships_count < p2->valid_ships_count) {

		int attacker = (player % 2) + 1;
		Player *current_p = (player % 2 == 0) ? p1 : p2;
		Player *opponent = (player % 2 == 0) ? p2 : p1;

		int x_attack, y_attack;
		if (scanf("%d %d", &x_attack, &y_attack) != 2)
			break;

		int r = x_attack - 1;
		int c = y_attack - 1;

		// 1. Check board boundaries
		if (r < 0 || r >= n || c < 0 || c >= m) {
			printf("Jucatorul %d a ratat o lovitura la coordonatele (%d, %d).\n", attacker, x_attack, y_attack);
			player++; // Turn ends
			continue;
		}

		char cell = opponent->board[r][c];

		if (cell == 'X' || cell == 'D' || cell == '!') {
			// Case 1: Already attacked coordinates (Hit 'X', Destroyed-Hit 'D', OR Missed Water '!')
			current_p->total_shots++;
			printf("Coordonatele (%d, %d) au fost deja atacate de jucatorul %d.\n", x_attack, y_attack, attacker);
			player++;
		} else if (cell == 'F') {
			// Case 2: Ship destroyed, but cell unhit ('F'). Treat as simple miss for message output.
			current_p->total_shots++;
			opponent->board[r][c] = 'D'; // Mark as hit/destroyed
			printf("Jucatorul %d a ratat o lovitura la coordonatele (%d, %d).\n", attacker, x_attack, y_attack);
			player++;
		} else if (cell == '0') {
			// Case 3: Miss (Clean Water)
			current_p->total_shots++;
			opponent->board[r][c] = '!';
			printf("Jucatorul %d a ratat o lovitura la coordonatele (%d, %d).\n", attacker, x_attack, y_attack);
			player++;
		} else if (cell >= '1' && cell <= '5') {
			// Case 4: Successful hit on a valid ship

			// Mark cell as hit and identify ship
			current_p->total_shots++;
			current_p->successful_shots++;
			opponent->board[r][c] = 'X';
			int ship_length = cell - '0';
			int ship_idx = find_hit_ship(opponent, r, c, ship_length);

			if (ship_idx != -1) {
				Ship *s = &opponent->ships[ship_idx];
				s->hits++;

				int destroyed = 0;
				// Destroyed if: hit at start (origin) OR all points hit
				if ((x_attack == s->x_start && y_attack == s->y_start) || s->hits == ship_length) {
					destroyed = 1;
				}

				if (destroyed) {
					process_destruction(opponent, ship_idx, attacker);
				} else {
					printf("Jucatorul %d a lovit o nava %s la coordonatele (%d, %d).\n",
							attacker, get_ship_name(s->type), x_attack, y_attack);
				}
			}
			player++; // Turn ends
		}
	}
}

char *read_stats_input()
{
	int i = 0;
	char c, *s = NULL, *tmp;
	scanf(" %c", &c);
	while(c != '\n') {
		tmp = realloc(s, (i+2) *  sizeof(char));
		if (!tmp)
			return 0;
		s = tmp;
		s[i] = c;
		i++;
		scanf("%c", &c);
	}
	s[i] = '\0';
	return s;
}

double match_acc(int successful_hits, int total_shots)
{
	if (total_shots == 0)
		return 0.0;
	return (double)successful_hits * 100.0 / total_shots;
}

double total_acc(matchstats *history, int num_match)
{
	double numerator = 0.0;
	int denominator = 0;
	for (int i = 0; i < num_match; i++) {
		double acc = match_acc(history[i].successful_hits, history[i].total_shots);
		numerator += acc * history[i].total_ships;
		denominator += history[i].total_ships;
	}
	if (denominator == 0)
		return 0.0;
	return numerator / denominator;
}

int min_misses_hit(matchstats match, double X)
{
	if (match.total_shots == 0)
		return 0;
	if (match_acc(match.successful_hits, match.total_shots) >= X)
		return 0;
	int missed_shots = match.total_shots - match.successful_hits;
	double required_successful_double = (X * match.total_shots) / 100.0;
	int required_successful_ceil = (int)(required_successful_double + 0.9999999);

	int min_not_miss = required_successful_ceil - match.successful_hits;

	if (min_not_miss > missed_shots)
		return missed_shots;
	return (min_not_miss > 0) ? min_not_miss : 0;
}

int max_misses_to_hit(matchstats match, double X)
{
	if (match.successful_hits == 0)
		return (X == 0.0) ? 9999 : 0;

	double total_max_allowed_double = (match.successful_hits * 100.0) / X;
	int total_max_allowed = (int)total_max_allowed_double;

	int max_misses = total_max_allowed - match.total_shots;

	return (max_misses > 0) ? max_misses : 0;
}

int compare_desc(const void *a, const void *b)
{
	double score_a = ((matcheval *)a)->score;
	double score_b = ((matcheval *)b)->score;
	if (score_a < score_b)
		return 1;
	if (score_a > score_b)
		return -1;
	return 0;
}

int min_matches_100(matchstats *history, int J, double X)
{
	double current_acc = total_acc(history, J);
	if (current_acc >= X)
		return 0;
	int total_K = 0;
	for (int i = 0; i < J; i++) {
		if (total_K == 0)
			return J + 1;
	}

	matchstats temp_history[J];
	memcpy(temp_history, history, J * sizeof(matchstats));

	matcheval *gains = (matcheval *)malloc(J * sizeof(matcheval));
	if (!gains)
		return -1;
	for (int i = 0; i < J; i++) {
		double acc_i = match_acc(temp_history[i].successful_hits, temp_history[i].total_shots);
		gains[i].index = i;
		gains[i].K = temp_history[i].total_ships;
		gains[i].score = (100.0 - acc_i) * gains[i].K;
	}
	qsort(gains, J, sizeof(matcheval), compare_desc);

	int min_matches = 0;
	for (int i = 0; i < J; i++) {
		if (gains[i].K == 0 || gains[i].score <= 0)
			continue;
		current_acc = total_acc(temp_history, J);
		if (current_acc >= X)
			break;
		temp_history[gains[i].index].successful_hits = temp_history[gains[i].index].total_shots;
		min_matches++;
	}
	if (total_acc(temp_history, J) < X) {
		min_matches = J + 1;
	}
	free(gains);
	return min_matches > J ? J + 1 : min_matches;
}

int compare_asc(const void *a, const void *b)
{
	double score_a = ((matcheval *)a)->score;
	double score_b = ((matcheval *)b)->score;
	if (score_a > score_b)
		return 1;
	if (score_a < score_b)
		return -1;
	return 0;
}

int max_matches_0(matchstats *history, int J, double X)
{
	double current_acc = total_acc(history, J);
	if (current_acc >= X)
		return 0;
	int total_K = 0;
	for (int i = 0; i < J; i++) {
		if (total_K == 0)
			return J;
	}

	matchstats temp_history[J];
	memcpy(temp_history, history, J * sizeof(matchstats));

	matcheval *losses = (matcheval *)malloc(J * sizeof(matcheval));
	if (!losses)
		return -1;
	for (int i = 0; i < J; i++) {
		double acc_i = match_acc(temp_history[i].successful_hits, temp_history[i].total_shots);
		losses[i].index = i;
		losses[i].K = temp_history[i].total_ships;
		losses[i].score = acc_i * losses[i].K;
	}
	qsort(losses, J, sizeof(matcheval), compare_asc);

	int max_can_fail = 0;
	for (int i = 0; i < J; i++) {
		if (losses[i].K == 0) {
			max_can_fail++;
			continue;
		}
		if (match_acc(temp_history[losses[i].index].successful_hits, temp_history[losses[i].index].total_shots) == 0.0) {
			max_can_fail++;
			continue;
		}

		temp_history[losses[i].index].successful_hits = 0;
		double acc_after_loss = total_acc(temp_history, J);

		if (acc_after_loss >= X)
			max_can_fail++;
		else {
			temp_history[losses[i].index].successful_hits = history[losses[i].index].successful_hits;
			break;
		}
	}
	free(losses);
	return max_can_fail;
}

void run_stats(matchstats *p1_stats, matchstats *p2_stats, int J)
{
	char key[256];
	char command[256];
	if (scanf(" %s", key) != 1)
		return;
	
	while (scanf(" %s", command) == 1) {
		if (strcmp(command, "Q") == 0)
			break;
		if (strcmp(command, "-") != 0) {
			//Hill chiper
		}

		char player_char = command[0];
		char tip1_char = command[1];
		char aaa_str[4] = {command[2], command[3], command[4], '\0'};
		char bbb_str[7] = {command[5], command[6], command[7], command[8], command[9], command[10], '\0'};
		char tip2_str[4] = {command[11], command[12], command[13], '\0'};

		int player_idx = (player_char == 'O') ? 0 : 1;
		int match_index = atoi(aaa_str);
		double X = atof(bbb_str);
		int min_max_type = strcmp(tip2_str, "MIN") == 0 ? 0 : 1;

		matchstats *history = (player_idx == 0) ? p1_stats : p2_stats;

		if (tip1_char == 'U' && (match_index < 1 || match_index > J)) {
			printf("Nu exista date pentru meciul cerut.\n");
			continue;
		}

		double acc_result = 0.0;
		int stats_result = 0;

		if (tip1_char == 'T') {
			acc_result = total_acc(history, J);
			if (min_max_type == 0)
				stats_result = min_matches_100(history, J, X);
			else
				stats_result = max_matches_0(history, J, X);
		} else {

			matchstats current_match = history[match_index - 1];
			acc_result = match_acc(current_match.successful_hits, current_match.total_shots);
			if (min_max_type == 0)
				stats_result = min_misses_hit(current_match, X);
			else
				stats_result = max_misses_to_hit(current_match, X);
		}
		char acc_part[16];
		int int_part = (int)acc_result;

		int frac_part = (int)((acc_result - int_part) * 100.0 + 0.5);

		if (frac_part >= 100) {
			int_part++;
			frac_part -= 100;
		}

		snprintf(acc_part, sizeof(acc_part), "%03d.%02d.%d", int_part, frac_part, stats_result);

		char output[256];
		snprintf(output, sizeof(output), "%s %d", acc_part, stats_result);

		if (strcmp(key, "-") != 0) {
			//hill cipher
		} else {
			printf("%s\n", output);
		}

	}
}

	// Displays a single board
void print_board(char **board, int n, int m)
{
	for (int r = 0; r < n; r++) {
		for (int l = 0; l < m; l++) {
			printf("%c", board[r][l]);
			if (l < m - 1) {
				printf(" ");
			}
		}
		printf("\n");
	}
}

int main(void)
{
	long long j;
	int n, m;
	if (scanf("%lld", &j) != 1)
		return 0;

	int J = (int)j;

	matchstats *p1_stats_history = (matchstats *)calloc(J, sizeof(matchstats));
	matchstats *p2_stats_history = (matchstats *)calloc(J, sizeof(matchstats));

	if (!p1_stats_history || !p2_stats_history) {
		if (p1_stats_history)
			free(p1_stats_history);
		if (p2_stats_history)
			free(p2_stats_history);
			return 0;
	}
	for (int i = 0; i < j; i++) {
		if (scanf("%d %d", &n, &m) != 2)
			break;

		// 1. Board allocation and Player initialization
		char **board1 = allocate_board(n, m);
		char **board2 = allocate_board(n, m);

		Player p1, p2;
		int total1 = initialize_player(&p1, n, m, board1);
		int total2 = initialize_player(&p2, n, m, board2);

		// Check for allocation errors
		if (!board1 || !board2 || (total1 > 0 && !p1.ships) || (total2 > 0 && !p2.ships)) {
			deallocate_board(board1, n);
			deallocate_board(board2, n);
			deallocate_player_ships(&p1);
			deallocate_player_ships(&p2);
			continue;
		}

		// 2. Placement Phase
		run_placement_phase(&p1, &p2, n, m);

		// Display boards
		print_board(p1.board, n, m);
		printf("\n");
		print_board(p2.board, n, m);

		// 3. Attack Phase
		run_attack_phase(&p1, &p2, n, m);

		// 4. Determine Winner
		if (p2.destroyed_ships_count == p2.valid_ships_count) {
			printf("Jucatorul 1 a castigat.\n");
		} else if (p1.destroyed_ships_count == p1.valid_ships_count) {
			printf("Jucatorul 2 a castigat.\n");
		}

		// 5. Save statistics
		p1_stats_history[i].total_ships = p1.valid_ships_count;
		p1_stats_history[i].total_shots = p1.total_shots;
		p1_stats_history[i].successful_hits = p1.successful_shots;
		
		p2_stats_history[i].total_ships = p2.valid_ships_count;
		p2_stats_history[i].total_shots = p2.total_shots;
		p2_stats_history[i].successful_hits = p2.successful_shots;
		
		// 6. Deallocate Memory
		deallocate_board(p1.board, n);
		deallocate_board(p2.board, n);
		deallocate_player_ships(&p1);
		deallocate_player_ships(&p2);
	}

	run_stats(p1_stats_history, p2_stats_history, J);

	free(p1_stats_history);
	free(p2_stats_history);

	return 0;

}