#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fcntl.h"
#include "kernel/param.h" // For MAXARG

// Split the input string into arguments based on whitespace.
// This function is crucial for parsing user commands into executable formats.
void split(char *buf, char *argv[]) {
    int n = 0;

    // Iterate over the input buffer to identify and isolate arguments.
    while (*buf != '\0') {
        // Remove leading whitespaces by replacing them with null characters.
        // This helps in demarcating the end of each argument.
        while (*buf == ' ' || *buf == '\n' || *buf == '\t')
            *buf++ = '\0';

        // Store each argument in the argv array if it's not the end of the buffer.
        if (*buf != '\0')
            argv[n++] = buf;

        // Advance the buffer pointer to skip over the current argument.
        while (*buf != '\0' && *buf != ' ' && *buf != '\n' && *buf != '\t')
            buf++;
    }

    // Mark the end of the arguments array with a null pointer for exec system call compatibility.
    argv[n] = 0;
}

// Manage redirections for input and output within the shell.
// This function allows the shell to redirect input from files or output to files.
int redirect_io(char *argv[]) {
    int fd;
    int redirectionHandled = 0; // To keep track if any redirection is handled

    // Look for redirection symbols in the arguments and process them.
    for (int i = 0; argv[i] != 0; i++) {
        // Handle output redirection ('>') by creating/opening the output file.
        if (strcmp(argv[i], ">") == 0 && argv[i+1] != 0) {
            fd = open(argv[i+1], O_CREATE|O_WRONLY);
            if (fd < 0) return -1;
            close(1); // Close the standard output.
            dup(fd); // Redirect the output to the specified file.
            close(fd); // Close the file descriptor to prevent resource leaks.
            argv[i] = 0; // Clear the redirection symbol from the command.
            redirectionHandled = 1;
        } 
        // Handle input redirection ('<') by opening the specified input file.
        else if (strcmp(argv[i], "<") == 0 && argv[i+1] != 0) {
            fd = open(argv[i+1], O_RDONLY);
            if (fd < 0) return -1;
            close(0); // Close the standard input.
            dup(fd); // Redirect the input from the specified file.
            close(fd); // Close the file descriptor to maintain resource efficiency.
            argv[i] = 0; // Remove the redirection symbol from the command.
            redirectionHandled = 1; // allowing the function to continue checking for additional redirections within the same command.
        }
    }
    return redirectionHandled; // Return 1 if any redirection is handled, 0 otherwise.
}

// Execute the parsed command.
// This function is the core of the shell, handling execution of all commands.
void run_command(char *cmd) {
    char *argv[MAXARG]; // Declare an array to hold arguments of a command.
    int fd[2], fd_in = 0; // File descriptors for pipe and input redirection.

    // Process each command, potentially with pipes for inter-process communication.
    while (cmd != 0) { // Loop until there are no more commands to process.
        char *next_cmd = strchr(cmd, '|'); // Find the next pipe character in the command.
        if (next_cmd != 0) { // If a pipe character is found,
            *next_cmd = '\0'; // Terminate the current command at the pipe character.
            next_cmd++; // Move to the next command after the pipe.
        }

        split(cmd, argv); // Split the command into arguments.
        if (argv[0] == 0) break; // If there are no arguments, exit the loop.

        // Special handling for built-in 'cd' command.
        // This command changes the shell's current directory.
        if (strcmp(argv[0], "cd") == 0) { // If the command is 'cd',
            if (argv[1] == 0 || chdir(argv[1]) < 0) { // Try to change the directory.
                printf("cd: failed to change directory\n"); // Print an error if it fails.
            }
            return; // Return after handling 'cd' to skip executing the rest of the function.
        }

        // Set up a pipeline if the command involves piping.
        if (next_cmd != 0) pipe(fd); // Create a pipe if there is a next command.

        // Fork the process to create a child process for command execution.
        if (fork() == 0) { // In the child process,
            // Redirect input/output as necessary.
            if (fd_in != 0) { // If there is an input redirection,
                close(0); // Close the standard input.
                dup(fd_in); // Duplicate the input redirection file descriptor.
                close(fd_in); // Close the original file descriptor.
            }
            if (next_cmd != 0) { // If there is a next command in the pipeline,
                close(fd[0]); // Close the read end of the pipe.
                close(1); // Close the standard output.
                dup(fd[1]); // Duplicate the write end of the pipe to standard output.
                close(fd[1]); // Close the original write end of the pipe.
            }

            // Attempt to execute the command and handle redirection.
            if (redirect_io(argv) < 0) { // Handle input/output redirection.
                printf("Redirection failed\n"); // Print an error message if redirection fails.
                exit(1); // Exit with an error code.
            }
            exec(argv[0], argv); // Execute the command.
            printf("%s: command not found\n", argv[0]); // Print an error if the command is not found.
            exit(1); // Exit with an error code.
        }

        // Close the file descriptors and prepare for the next command in the pipeline.
        if (fd_in != 0) close(fd_in); // Close the input redirection file descriptor if it exists.
        if (next_cmd != 0) { // If there is a next command in the pipeline,
            close(fd[1]); // Close the write end of the pipe.
            fd_in = fd[0]; // Set the read end of the pipe as the new input redirection.
        }

        cmd = next_cmd; // Move to the next command in the pipeline.
    }

    // Wait for all child processes to finish before accepting new commands.
    int wstatus; // Variable to store the status of the wait call.
    while ((wstatus = wait(0)) != -1) { // Wait for all child processes to terminate.
        // Continuously wait for all child processes to terminate.
    }
}

// The main function: the entry point of the shell.
// This loop continuously prompts the user for input and processes each command.
int main(void) {
    static char buf[100]; // Buffer to store the user input.

    // The main loop: Read commands from the user and execute them.
    while (1) { // Infinite loop to continuously read user input.
        printf(">>> "); // Print the prompt.
        memset(buf, 0, sizeof(buf)); // Clear the buffer.
        gets(buf, sizeof(buf)); // Read a line of input from the user.
        if (buf[0] == 0) break; // Exit the loop if the input is empty.

        // Process each command, handling ';' for sequential command execution.
        char *cmd = buf; // Start with the first command in the buffer.
        char *next_cmd; // Variable to hold the next command after ';'.
        while ((next_cmd = strchr(cmd, ';')) != 0) { // Find the next ';' in the command.
            *next_cmd = '\0'; // Terminate the current command at the ';'.
            run_command(cmd); // Execute the current command.
            cmd = next_cmd + 1; // Move to the next command after the ';'.
        }
        run_command(cmd); // Execute the last or only command.
    }
    exit(0); // Exit the program.
}

