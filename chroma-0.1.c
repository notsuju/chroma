#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <string.h>
#include <ctype.h>

#define MAX_LINES 1000
#define MAX_LINE_LENGTH 1024

// FUNCTIONS
void set_cursor_position(int r, int c);
void enable_raw_mode();
void disable_raw_mode();
int check_boundaries(int r, int c, int number_of_lines, char **buffer);
void clear_screen();
void refresh_screen(char **buffer, int number_of_lines);

struct termios orig_termios;
int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Use './chroma-0.1 help' for more information\n");
        return 1;
    }
    // Help prompt
    if (strcmp(argv[1], "help") == 0)
    {
        printf(" Usage: './chroma-0.1 file_name'\n\n");
        printf(" Command Mode: \n\n");
        printf(" k --move up\n j --move down\n l --move right\n h --move left\n s --save file\n q --quit the program\n i --edit mode\n x <line_number> enter <column_number> enter --jump to a specific line\n\n");
        printf(" Edit Mode: \n\n");
        printf(" esc --quit edit mode and enter command mode\n");
        return -1;
    }

    clear_screen();
    // Enabling raw mode to read keystrokes without waiting for enter key
    enable_raw_mode();

    FILE *input = fopen(argv[1], "r");
    if (input == NULL)
    {
        FILE *new = fopen(argv[1], "w");
        fclose(new);
    }

    char **buffer = (char **)malloc(MAX_LINES * sizeof(char *));
    for (int i = 0; i < MAX_LINES; i++)
    {
        buffer[i] = (char *)malloc(MAX_LINE_LENGTH * sizeof(char));
    }
    if (buffer == NULL)
    {
        printf("Error allocating memory\n");
        disable_raw_mode();
        return 2;
    }
    int number_of_lines = 0;

    if (input != NULL)
    {
        while (fgets(buffer[number_of_lines], MAX_LINE_LENGTH, input) != NULL)
        {
            number_of_lines++;
        }
        for (int i = 0; i < number_of_lines; i++)
        {
            printf("%s", buffer[i]);
        }

        // adding a newline and null terminator to the last line of the file
        int last_line_length = strlen(buffer[number_of_lines - 1]);
        if (buffer[number_of_lines - 1][last_line_length - 1] != '\n')
        {
            if (last_line_length + 1 <= MAX_LINE_LENGTH)
            {
                buffer[number_of_lines - 1][last_line_length] = '\n';
                buffer[number_of_lines - 1][last_line_length + 1] = '\0';
            }
        }
    }

    // Navigation
    char keystroke;
    int r = 1;
    int c = 1;
    int editing = 0;
    int insert_mode;
    while (1)
    {
        // DISPLAY THE FILE AFTER EVERY CHANGE
        if (editing)
        {
            refresh_screen(buffer, number_of_lines);
        }
        set_cursor_position(r, c);

        keystroke = getchar();

        // COMMAND MODE
        if (!editing)
        {
            insert_mode = 0;
            if (keystroke == 'k' && r > 1)
            {
                if (c >= strlen(buffer[r - 2]))
                {
                    c = strlen(buffer[r - 2]);
                    r--;
                }
                else
                {
                    r--;
                }
            }
            else if (keystroke == 'j' && r < number_of_lines)
            {
                if (c >= strlen(buffer[r]))
                {
                    c = strlen(buffer[r]);
                    r++;
                }
                else
                {
                    r++;
                }
            }
            else if (keystroke == 'l' && c < strlen(buffer[r - 1]))
            {
                c++;
            }
            else if (keystroke == 'h' && c > 1)
            {
                c--;
            }
//            else if(keystroke == 'm')
//            {
//                r = getchar();
//                c = getchar();
//            }
            else if (keystroke == 'q')
            {
                clear_screen();
                break;
            }
            else if (keystroke == 'i')
            {
                editing = 1;
            }
            else if (keystroke == 'x')
            {
                int x, y;
                scanf(" %d", &x);
                scanf(" %d", &y);
                if (check_boundaries(x, y, number_of_lines, buffer))
                {
                    r = x;
                    c = y;
                }
            }
            else if (keystroke == 's')
            {
                FILE *output = fopen(argv[1], "w");
                if (output == NULL)
                {
                    printf("Error saving file");
                    disable_raw_mode();
                    return 3;
                }
                for (int i = 0; i < number_of_lines; i++)
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
                    int line_length = strlen(buffer[r - 2]);
                    // Setting the cursor to the end of the previous line
                    r--;
                    c = strlen(buffer[r - 1]);

                    // Removing the endline character from the previous line
                    buffer[r - 1][line_length - 1] = '\0';

                    // Concatenating the previous line with the current line
                    strcat(buffer[r - 1], buffer[r]);

                    // Shifting all the lines up
                    for (int i = r; i < number_of_lines; i++)
                    {
                        memset(buffer[i], '\0', MAX_LINE_LENGTH);
                        strcpy(buffer[i], buffer[i + 1]);
                    }
                    number_of_lines--;
                }
            }
            // NEW LINE
            else if (keystroke == '\n')
            {

                // Shift all lines down
                for (int i = number_of_lines; i > r; i--)
                {
                    // Clearing the line to avoid overlapping
                    memset(buffer[i], '\0', MAX_LINE_LENGTH);
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
                    buffer[r][0] = '\0';
                }

                r++;
                c = 1;
                number_of_lines++;
            }
            // INSERT CHARACTER
            else if (c < MAX_LINE_LENGTH)
            {
                if (insert_mode)
                {
                    int line_length = strlen(buffer[r - 1]);
                    // For handling empty first line
                    if (c == 1 && r == 1 && line_length == 0)
                    {
                        buffer[0][0] = '\n';
                        buffer[0][1] = '\0';
                        number_of_lines++;
                        line_length++;
                    }
                    for (int i = line_length; i >= c - 1; i--)
                    {
                        buffer[r - 1][i] = buffer[r - 1][i - 1];
                    }
                    buffer[r - 1][c - 1] = keystroke;
                    c++;
                }
                insert_mode = 1;
            }
        }
    }
    disable_raw_mode();
    if (input != NULL)
    {
        fclose(input);
    }
    for (int i = 0; i < MAX_LINES; i++)
    {
        free(buffer[i]);
    }
    free(buffer);
    buffer = NULL;
    return 0;
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

int check_boundaries(int r, int c, int number_of_lines, char **buffer)
{
    if (r > 0 && r <= number_of_lines && c <= strlen(buffer[r - 1]) && c > 0)
    {
        return 1;
    }
    return 0;
}

void clear_screen()
{
    printf("\033[2J"); // Clear entire screen
    printf("\033[H");  // Move cursor to top-left corner
}

void refresh_screen(char **buffer, int number_of_lines)
{
    printf("\033[2J"); // Clear the screen first
    printf("\033[H");  // Move cursor to home position (1,1) so that the next print starts from 1,1
    for (int i = 0; i < number_of_lines; i++)
    {
        printf("%s", buffer[i]);
    }
}