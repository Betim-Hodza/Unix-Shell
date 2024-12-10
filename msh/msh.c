
#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <ctype.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>

#define WHITESPACE " \t\n"      /* We want to split our command line up into tokens
                                   so we need to define what delimits our tokens.
                                   In this case  white space
                                   will separate the tokens on our command line */

#define MAX_COMMAND_SIZE 255    // The maximum command-line size

#define MAX_NUM_ARGUMENTS 11     

int main(int argc, char **argv)
{
    // grab our cmd string in put it into heap
    char * command_string = (char*) malloc( MAX_COMMAND_SIZE );
    // we setup our fp, count, looped for batch functions
    FILE *fp = NULL;
    int count = 0;
    int looped = 0;

    // if more than 1 file passed into batch mode we throw an error amd exit(1)
    if (argc > 2)
    {
        char error_message[30] = "An error has occurred\n";
        write(STDERR_FILENO, error_message, strlen(error_message));  
        exit(1);
    }

    // batch mode, read in the file passed through
    // and store it in command_string to be parsed and ran
    // we similarly get a MAX cmd size and get all input from
    // a file and execute it immediatly not waiting for the user
    if (argc == 2)
    {
        fp = fopen(argv[1], "r");
        // error opening file
        if (fp == NULL)
        {
            char error_message[30] = "An error has occurred\n";
            write(STDERR_FILENO, error_message, strlen(error_message)); 
            exit(1);
        }

        char buffer[MAX_COMMAND_SIZE];

        // counting the lines in the file to loop count amount of times
        while (fgets(buffer, sizeof(buffer), fp))
        {
            count++;
        }
        // rewind fp so we can properly loop
        rewind(fp);

    }
    
   

    while( 1 )
    {   
        
        // interactive mode
        // Read the command from the command line.  The
        // maximum command that will be read is MAX_COMMAND_SIZE
        // This while command will wait here until the user inputs something.
        if ( argv[1] == NULL)
        {
            // Print out the msh prompt and take user input
            printf ("msh> ");
            while( !fgets (command_string, MAX_COMMAND_SIZE, stdin) );
        }
        else
        {
            // Batch mode
            // Read line by line of our file 
            while(!fgets(command_string, MAX_COMMAND_SIZE, fp));

            // count how many times we loop
            looped++;
        }

        /* Parse input */
        char *token[MAX_NUM_ARGUMENTS];

        int token_count = 0;
                                                            
        // Pointer to point to the token
        // parsed by strsep
        char *argument_pointer;                         
        char *working_string  = strdup( command_string );                

        // we are going to move the working_string pointer so
        // keep track of its original value so we can deallocate
        // the correct amount at the end
        char *head_ptr = working_string;
        
        // Tokenize the input with whitespace used as the delimiter
        int found_command = 0; 
        while ( ( (argument_pointer = strsep(&working_string, WHITESPACE ) ) != NULL) && (token_count<MAX_NUM_ARGUMENTS))
        {
            // skip leading whitespace tokens
            if (!found_command && strlen(argument_pointer) == 0)
            {
                continue;
            }

            // copy the argument to our specific token
            token[token_count] = strndup( argument_pointer, MAX_COMMAND_SIZE );

            // if we run into a empty token, make it null
            if( strlen( token[token_count] ) == 0 )
            {
                token[token_count] = NULL;
            }
            else if (!found_command)
            {
                // turn on our flag to stop trimming whitespacesa
                found_command = 1;
            }
            token_count++;
            
        }

        // Shell functionalilty goes here!

        int token_index  = 0;
        for( token_index = 0; token_index < token_count; token_index ++ ) 
        {

            // Now print the tokenized input as a debug check
            // printf("token[%d] = %s\n", token_index, token[token_index] );  

            // Special cases: empty, exit, quit, and cd
            // if the user hits enter, we want to print a new msh line
            if(token[0] == NULL)
            {
                continue;
            }

            // Exit iff there are no args after exit
            if ( ((strcmp("exit", token[0])) == 0 ) && token_index == 0) 
            {
                
                // if arguments passed throw an error (shouldn't have args on exit)
                if (token[1] != NULL)
                {
                    // Error to pass args to quit || exit
                    char error_message[30] = "An error has occurred\n";
                    write(STDERR_FILENO, error_message, strlen(error_message));  
                    // continue to loop again
                    continue; 
                } 
                else 
                {
                    exit(0);
                }
                
            }

            // quit iff there are no args after quit
            if ( ((strcmp("quit", token[0])) == 0) && token_index == 0)
            {
                // if arguments passed throw an error (shouldn't have args on quit)
                if (token[1] != NULL)
                {
                    // Error to pass args to quit || exit
                    char error_message[30] = "An error has occurred\n";
                    write(STDERR_FILENO, error_message, strlen(error_message)); 
                    continue;  
                } 
                else 
                {
                    exit(0);
                }
            }
            

            // CD
            // Check the if token[0] is our custom command
            // and if theres a total of 2 args cd [directory]
            if ( ((strcmp("cd", token[0])) == 0) && token_index == 0 )
            {
                // if our second argument doesnt have a null we can't change dir (too many args)
                if ( token[2] != NULL )
                {
                    char error_message[30] = "An error has occurred\n";
                    write(STDERR_FILENO, error_message, strlen(error_message)); 
                    continue;
                }
                else if( token[1] == NULL )
                {
                    // if our first argument is null this is an error (not enough args)
                    char error_message[30] = "An error has occurred\n";
                    write(STDERR_FILENO, error_message, strlen(error_message)); 
                    continue;
                }
                else 
                {
                    // we change our directory and loop again
                    // this way we dont call execvp
                    char dir[MAX_COMMAND_SIZE];
                    strcpy(dir, token[1]);
                    
                    // if our directory is not valid, throw an error
                    if ( chdir(dir) != 0 )
                    {
                        char error_message[30] = "An error has occurred\n";
                        write(STDERR_FILENO, error_message, strlen(error_message)); 
                    }
                    continue;
                }
            } 

            // Executing commands with execvp

            // forking a child process that will run execvp
            // to follow fork in gdb: set follow-fork-mode child
            pid_t child_pid;
            child_pid = fork();

            // Prevent fork bombs
            if( child_pid == -1 )
            {
                // When fork() returns -1, an error happened.
                char error_message[30] = "An error has occurred\n";
                write(STDERR_FILENO, error_message, strlen(error_message)); 
            }
            else if ( child_pid == 0 )
            {

                // we need to check if the command exists in /bin/ || /usr/bin/ || /usr/local/bin
                // we can do this with execvp

                // flag for redirection
                // 0 == false, 1 == true, -1 err
                int redirection = 0;

                // Check for redirection if ">" exists in our arguments
                // only redirect on first token_index
                if (token_index == 0)
                {
                    // loop through all of our tokens to find the ">"
                    // to start redirection and trim it out 
                    for(int i = 0; i < token_count-1; i++ )
                    { 
                        if( (strcmp( token[i], ">" ) == 0) )
                        {
                            redirection = 1;

                            // Check if more than 1 output file and less than 1 input
                            // protection from creating / editing a nameless file
                            if(token[i+1] == NULL || token[i+2] != NULL || token[i-1] == NULL)
                            {
                                // printf("redirection error\n");
                                char error_message[30] = "An error has occurred\n";
                                write(STDERR_FILENO, error_message, strlen(error_message)); 

                                // Trim off the > output part of the command
                                // token[i] = NULL;  
                                redirection = -1;
                                exit(EXIT_FAILURE);
                            }

                            // create a file if it doesnt exist with the token name
                            // and give standard RW permissions to user
                            int fd = 0;
                            fd = open( token[i+1], O_RDWR | O_CREAT, S_IRUSR | S_IWUSR );
                            
                            // error if opening a file doesnt work
                            if( fd < 0 )
                            {
                                char error_message[30] = "An error has occurred\n";
                                write(STDERR_FILENO, error_message, strlen(error_message));   
                                break;              
                            }
                            // redirects from standard output
                            dup2( fd, 1 );
                            close( fd );
                            
                            // Trim off the > output part of the command
                            token[i] = NULL;
                        }
                    }
                }
                
                // if theres redirection, we execute w/out caring for token_index
                // because all the tokens have been redirected to the file
                if (redirection == 1) 
                {
                    if ( (execvp(token[0], token) == -1))
                    {
                        // printf("execvp w/ redir error\n");
                        char error_message[30] = "An error has occurred\n";
                        write(STDERR_FILENO, error_message, strlen(error_message));                
                    }
                }
                
                // if theres no redirection we should execute this only once based on token_index
                if (token_index == 0 && redirection == 0 ) 
                {
                    if ( execvp(token[0], token) == -1)
                    {
                        // printf("execvp error\n");
                        char error_message[30] = "An error has occurred\n";
                        write(STDERR_FILENO, error_message, strlen(error_message));        
                    }
                }

                // child process should end if exec has any failures
                exit( EXIT_FAILURE );
            }
            else
            {
                // force the parent process to wait for the child to exit
                // as to not do weird looping
                int status;
                waitpid(child_pid, &status, 0);     
            }
            
        }

        free( head_ptr );
        // if we're in batch mode, just exit
        if (argv[1] != NULL && looped >= count)
        {
            exit(0);
        }

    }
    return 0;

}

