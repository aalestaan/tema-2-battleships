#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Structure to organize data for a single ship
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
} Player;

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
			printf("Coordonatele (%d, %d) au fost deja atacate de jucatorul %d.\n", x_attack, y_attack, attacker);
			player++;
		} else if (cell == 'F') {
			// Case 2: Ship destroyed, but cell unhit ('F'). Treat as simple miss for message output.
			opponent->board[r][c] = 'D'; // Mark as hit/destroyed
			printf("Jucatorul %d a ratat o lovitura la coordonatele (%d, %d).\n", attacker, x_attack, y_attack);
			player++;
		} else if (cell == '0') {
			// Case 3: Miss (Clean Water)
			opponent->board[r][c] = '!';
			printf("Jucatorul %d a ratat o lovitura la coordonatele (%d, %d).\n", attacker, x_attack, y_attack);
			player++;
		} else if (cell >= '1' && cell <= '5') {
			// Case 4: Successful hit on a valid ship

			// Mark cell as hit and identify ship
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

void read_stats_input(void)
{
	char key[256];
	char command[256];
	int read_key = 0;
	if (scanf(" %s", key) == 1) {
		read_key = 1;
	}
	if (read_key) {
		while (scanf(" %s", command) == 1) {
			if (strcmp(command, "Q") == 0)
				break;
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

	for (int i = 0; i < j; i++) {
		if (scanf("%d %d", &n, &m) != 2)
			break;

		// 1. Board allocation and Player initialization
		char **board1 = allocate_board(n, m);
		char **board2 = allocate_board(n, m);

		Player p1, p2;
		int total = initialize_player(&p1, n, m, board1);
		initialize_player(&p2, n, m, board2);

		// Check for allocation errors
		if (!board1 || !board2 || (total > 0 && (!p1.ships || !p2.ships))) {
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

		// 5. Deallocate Memory
		deallocate_board(p1.board, n);
		deallocate_board(p2.board, n);
		deallocate_player_ships(&p1);
		deallocate_player_ships(&p2);
	}

	read_stats_input();

	return 0;

}