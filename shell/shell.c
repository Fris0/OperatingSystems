#include "arena.h"
#include "stdlib.h"
#include "sys/wait.h"
#include "sys/types.h"
#include "front.h"
#include "parser/ast.h"
#include "shell.h"
#include "stdio.h"
#include "unistd.h"
#include "time.h"
#include "string.h"
#include "signal.h"

int PID, PID2, STATUS, EXIT_CODE;

/* Free the memory reserved by the tree.
 *
 * Output: Not present.
 * Side-effect: The memory used by the tree is freed.
*/
void my_free_tree(void *pt)
{
	free_tree((node_t *)pt);
    return;
}

/* Kill a running process.
 *
 * Side-effect: Process is killed.
*/
void handler_sigint(int sig)
{
    kill(PID, SIGKILL);
    printf("%d", sig);
    return;
}

/* Redirect signal of CTRL-C when pressed.
 *
 * Output: Not present.
 * Side-effect: Change the sigint handler.
*/
void initialize(void)
{
    signal(SIGINT, &handler_sigint);
    return;
}

/* Exit the current top process.
 *
 * Output: Not present.
 * Side-effect: Quits a running process.
*/
void shell_exit(void)
{
    exit(EXIT_CODE);
}

/* Run a command based on the values specified in the given node.
 *
 * node_t *node = Pointer to struct containing type and arguments.
 *
 * Output: Not present.
 * Side-effects: Print to stdout when an error has occured, or
 *               the result of the command.
*/
void node_command(node_t *node)
{
    if (strcmp(node->command.argv[0], "exit") == 0 && atoi(node->command.argv[1]) >= 0)
    {
        EXIT_CODE = atoi(node->command.argv[1]);
        shell_exit();
    }
    else if (strcmp(node->command.argv[0], "cd") == 0)
    {
        if (chdir(node->command.argv[1]) == -1)
        {
            perror("Requested path could not be found.\n");
        }
    }
    else
    {
        PID = fork();
        if (PID == 0)
        {
            if (execvp(node->command.program, node->command.argv) == -1)
            {
                perror("Basic-command execution failed.\n");
            }
        }
        waitpid(PID, &STATUS, 0);
        return;
    }
}

/* Run two procceses where the output from one proccess
 * becomes the input of second process.
 *
 * node_t *node = A struct of type node_t holding the
 *                two process command name and arguments.
 *
 * Output: Not present.
 * Side-Effects: Error messages are send to STDOUT when
 *               forks and executes have failed. Also
 *               the input from one process is send to another
 *               by the use of a pipe.
*/
void pipe_command(node_t *node)
Olaf Erkemeij a year ago

In this function, instead of checking what kind of command is given to the pipes, you could simply pass the parts to run_command. This will then handle this check for you, resulting in a shorter pipe function and cleaner code. As of right now, this function is too long.
Click to start a new thread…

    {
        node_t *n_node = node->pipe.parts[0];
        node_t *n2_node = node->pipe.parts[1];

        if (node->pipe.n_parts == 2)
        {
            int fd[2];
            if (pipe(fd) == -1)
            {
                perror("Eror occured creating pipe.\n");
                return;
            }

            PID = fork();
            if (PID < 0)
            {
                perror("Error occured creating process.\n");
                return;
            }

            if (PID == 0)
            {
                dup2(fd[1], STDOUT_FILENO);
                close(fd[0]);
                close(fd[1]);
                if (strcmp(n_node->command.argv[0], "exit") == 0 && atoi(n_node->command.argv[1]) >= 0)
                {
                    EXIT_CODE = atoi(n_node->command.argv[1]);
                    shell_exit();
                }
                else if (strcmp(n_node->command.argv[0], "cd") == 0)
                {
                    if (chdir(n_node->command.argv[1]) == -1)
                    {
                        perror("Requested path could not be found.\n");
                    }
                }
                else
                {
                    execvp(n_node->command.program, n_node->command.argv);
                    perror("Error occured with exec.");
                }
            }
            int PID2 = fork();
            if (PID2 < 0)
            {
                perror("Error occured creating 2nd process.\n");
            }

            if (PID2 == 0)
            {
                dup2(fd[0], STDIN_FILENO);
                close(fd[0]);
                close(fd[1]);

                if (strcmp(n2_node->command.argv[0], "exit") == 0 && atoi(n2_node->command.argv[1]) >= 0)
                {
                    EXIT_CODE = atoi(n2_node->command.argv[1]);
                    shell_exit();
                }
                else if (strcmp(n2_node->command.argv[0], "cd") == 0)
                {
                    if (chdir(n2_node->command.argv[1]) == -1)
                    {
                        perror("Requested path could not be found.\n");
                    }
                }
                else
                {
                    execvp(n2_node->command.program, n2_node->command.argv);
                    perror("Error occured with exec.");
                }
            }

            close(fd[0]);
            close(fd[1]);

            waitpid(PID, NULL, 0);
            waitpid(PID2, NULL, 0);
            return;
        }
    }

    /* Reads the type given to the node and runs the appropiate function.
     *
     * node_t *node = Pointer to struct containing type and arguments.
     *
     * Output: Not present.
     * Side-effects: Sends string to STDOUT when the node type
     *               is not set. Caused by faulty input.
    */
    void run_command(node_t *node)
    {
        switch (node->type)
        {
            case NODE_COMMAND:
                node_command(node);
                break;
            case NODE_PIPE:
                pipe_command(node);
                break;
            case NODE_REDIRECT:
                break;
            case NODE_SUBSHELL:
                break;
            case NODE_SEQUENCE:
                run_command(node->sequence.first);
                run_command(node->sequence.second);
                break;
            case NODE_DETACH:
                break;
            default:
                printf("No sufficient input!\n");
                break;
        }
    }

    Valid submission
    1
    ⁄
    1
    AT
    Simple commands
    1
    ⁄
    1
    AT
    exit
    1
    ⁄
    1
    AT
    cd
    1
    ⁄
    1
    AT
    Sequences
    1.
