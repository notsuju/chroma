#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <string.h>

#define MAX_LINES 1000
#define MAX_LINE_LENGTH 1024

// FUNCTIONS
void move_cursor_up();
void move_cursor_down();
void move_cursor_right();
void move_cursor_left();
void set_cursor_position(int r, int c);
void enable_raw_mode();
void disable_raw_mode();

struct termios orig_termios;

int main(int argc, char *argv[])
{
    system("clear");
    if (argc != 2)
    {
        printf("Usage: ./chroma filename\n");
        return 1;
    }
    enable_raw_mode();
    FILE *input = fopen(argv[1], "r");
    if (input == NULL)
    {
        printf("Error opening the file\n");
        return 2;
    }
    char buffer[MAX_LINES][MAX_LINE_LENGTH];
    int line_no = 0;
    // char *buffer = (char*)malloc(1024 * sizeof(char));
    while (fgets(buffer[line_no], MAX_LINE_LENGTH, input) != NULL)
    {
        line_no++;
    }
    for (int i = 0; i < line_no; i++)
    {
        printf("%s", buffer[i]);
    }

    // Navigation
    char keystroke;
    int r = 1;
    int c = 1;
    int editing = 0;
    int counter;
    while (1)
    {
        set_cursor_position(r, c);

        keystroke = getchar();
        // COMMAND MODE
        if (!editing)
        {
            counter = 0;
            if (keystroke == 'k' && r > 1)
            {
                move_cursor_up();
                r--;
            }
            else if (keystroke == 'j' && r < line_no)
            {
                move_cursor_down();
                r++;
            }
            else if (keystroke == 'l' && c < strlen(buffer[r - 1]))
            {
                move_cursor_right();
                c++;
            }
            else if (keystroke == 'h' && c > 1)
            {
                move_cursor_left();
                c--;
            }
            // else if (keystroke == 'x')
            // {
            //     scanf(" %d", &r);
            //     scanf(" %d", &c);
            // }

            else if (keystroke == 'q')
            {
                break;
            }
            else if (keystroke == 'i')
            {
                editing = 1;
            }
            else if (keystroke == 's')
            {
                // fclose(input);
                FILE *output = fopen(argv[1], "w");
                if (output == NULL)
                {
                    printf("Error saving file");
                    return 3;
                }
                for (int i = 0; i < line_no; i++)
                {
                    fputs(buffer[i], output);
                }
                fclose(output);
            }
        }

        // EDITING MODE
        if (editing)
        {
            // EXIT EDITING MODE
            if (keystroke == '\033')
            {
                editing = 0;
            }
            // DELETE CHARACTER
            else if (keystroke == 127 || keystroke == '\b')
            {
                if (c == 1 && r == 1)
                {
                    continue;
                }
                else if (c > 1)
                {
                    for (int i = c - 1; i < MAX_LINE_LENGTH; i++)
                    {
                        buffer[r - 1][i - 1] = buffer[r - 1][i];
                    }
                    c--;
                }
                else
                {
                    for (int i = c; i < MAX_LINE_LENGTH; i++)
                    {
                        buffer[r - 1][i - 1] = buffer[r - 1][i];
                    }
                    r--;
                    c = strlen(buffer[r - 1]);
                }
            }
            // NEW LINE
            else if (keystroke == '\n')
            {

                // Shift all lines down
                for (int i = line_no; i > r; i--)
                {
                    memset(buffer[i], '\0', MAX_LINE_LENGTH); // Clears the line and set it to null to avoid overlapping
                    strcpy(buffer[i], buffer[i - 1]);
                }

                memset(buffer[r], '\0', MAX_LINE_LENGTH);
                int line_length = strlen(buffer[r - 1]);
                if (c - 1 < line_length)
                {
                    strcpy(buffer[r], &buffer[r - 1][c - 1]);
                    buffer[r - 1][c - 1] = '\n';
                    buffer[r - 1][c] = '\0';
                }
                else
                {
                    buffer[r - 1][line_length - 1] = '\n';
                    buffer[r - 1][line_length] = '\0';
                    buffer[r][0] = '\n';
                    buffer[r][1] = '\0';
                }

                r++;
                c = 1;
                line_no++;
            }
            // INSERT CHARACTER
            else if (c < MAX_LINE_LENGTH)
            {
                if (counter)
                {
                    int line_length = strlen(buffer[r - 1]);
                    if (line_length > 0)
                    {
                        for (int i = line_length; i >= c - 1; i--)
                        {
                            buffer[r - 1][i] = buffer[r - 1][i - 1];
                        }
                    }
                    buffer[r - 1][c - 1] = keystroke;
                    c++;
                }
                counter = 1;
            }
        }
        // DISPLAY THE FILE AFTER EVERY CHANGE
        system("clear");
        for (int i = 0; i < line_no; i++)
        {
            printf("%s", buffer[i]);
        }
    }
    disable_raw_mode();
    fclose(input);
    return 0;
}

void move_cursor_up()
{
    printf("\033[A");
}

void move_cursor_down()
{
    printf("\033[B");
}

void move_cursor_right()
{
    printf("\033[C");
}

void move_cursor_left()
{
    printf("\033[D");
}

void set_cursor_position(int r, int c)
{
    printf("\033[%d;%dH", r, c);
}

void enable_raw_mode()
{
    tcgetattr(STDIN_FILENO, &orig_termios); // Get the current terminal settings
    struct termios raw = orig_termios;
    raw.c_lflag &= ~(ICANON | ECHO);          // Disable canonical mode and echo
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw); // Apply raw mode settings
}

void disable_raw_mode()
{
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios); // Restore the original settings
}