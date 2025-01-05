#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>

#define WIDTH 20
#define HEIGHT 10

// Function to get non-blocking user input
int kbhit(void)
{
	struct termios oldt, newt;
	int ch;
	int oldf;
	tcgetattr(STDIN_FILENO, &oldt);
	newt = oldt;
	newt.c_lflag &= ~(ICANON | ECHO);
	newt.c_cc[VMIN] = 1;
	newt.c_cc[VTIME] = 0;
	tcsetattr(STDIN_FILENO, TCSANOW, &newt);
	oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
	fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
	ch = getchar();
	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
	fcntl(STDIN_FILENO, F_SETFL, oldf);
	if (ch != EOF)
	{
		ungetc(ch, stdin);
		return 1;
	}
	return 0;
}

// Function to clear the terminal screen
void clear_screen()
{
	printf("\033[H\033[J");
}

// Function to draw the game board
void draw_board(int caret_pos, int ball_x, int ball_y)
{
	clear_screen();

	for (int y = 0; y < HEIGHT; y++)
	{
		for (int x = 0; x < WIDTH; x++)
		{
			// Draw the wider caret, which is two characters wide ("##")
			if (y == HEIGHT - 1 && (x == caret_pos || x == caret_pos + 1))
			{
				printf("##");
				x++; // Skip the next cell because the caret is two characters wide
			}
			// Draw the ball
			else if (y == ball_y && x == ball_x)
			{
				printf("O");
			}
			else
			{
				printf(".");
			}
		}
		printf("\n");
	}
}

// Main game loop
int main()
{
	int caret_pos = WIDTH / 2;
	int ball_x = rand() % WIDTH;
	int ball_y = 0;
	int ball_speed = 1;		// Ball moves one step at a time
	int ball_direction = 1; // 1 means moving down, -1 means moving up
	int game_over = 0;
	int ball_movement_counter = 0; // Counter to slow down the ball

	while (!game_over)
	{
		draw_board(caret_pos, ball_x, ball_y);

		// Ball movement (only move once every few frames)
		if (ball_movement_counter >= 5) // Control how often the ball moves
		{
			if (ball_direction == 1)
			{
				ball_y += ball_speed;
			}
			else
			{
				ball_y -= ball_speed;
			}

			// Ball bounce logic
			if (ball_y >= HEIGHT - 1)
			{
				// Ball hits the ground
				if (ball_x >= caret_pos && ball_x < caret_pos + 2)
				{
					ball_y = 0;
					ball_x = rand() % WIDTH;
					ball_direction = -1;
				}
				else
				{
					game_over = 1; // Ball missed, game over
				}
			}
			else if (ball_y <= 0)
			{
				ball_direction = 1; // Ball hits the top and starts moving down
			}

			// Reset the counter for ball movement
			ball_movement_counter = 0;
		}
		else
		{
			ball_movement_counter++; // Increase the counter if the ball hasn't moved yet
		}

		// Check for user input to move caret
		if (kbhit())
		{
			char ch = getchar();
			if (ch == 'a' && caret_pos > 0)
			{
				caret_pos--;
			}
			else if (ch == 'd' && caret_pos < WIDTH - 2) // caret is two characters wide
			{
				caret_pos++;
			}
		}

		usleep(100000); // Slow down the game loop, so the caret can be moved multiple times
	}

	clear_screen();
	printf("Game Over! You missed the ball.\n");

	return 0;
}
