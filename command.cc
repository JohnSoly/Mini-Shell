/*
  * CS354: Shell project
 *
 * Template file.
 * You will need to add more code here to execute the command table.
 *
 * NOTE: You are responsible for fixing any bugs this code may have!
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h> 
#include <fstream>
#include <wait.h>
#include <glob.h> 
#include "command.h"

int parentID;
void childHandler (int sig){
	 pid_t pid;
	 int stat;
	 pid = wait(&stat);
	 std::ofstream outfile;
	 outfile.open("logs.txt", std::ios_base::app);
	 outfile << "child Process with parent id="<< parentID << "terminated\n";
}
SimpleCommand::SimpleCommand()
{
	// Creat available space for 5 arguments
	_numberOfAvailableArguments = 5;
	_numberOfArguments = 0;
	_arguments = (char **) malloc( _numberOfAvailableArguments * sizeof( char * ) );
}

void ignoreSignal(int sig) {
    printf("\n");
    Command::_currentCommand.prompt();
    return;
}
void
SimpleCommand::insertArgument( char * argument )
{
	if ( _numberOfAvailableArguments == _numberOfArguments  + 1 ) {
		// Double the available space
		_numberOfAvailableArguments *= 2;
		_arguments = (char **) realloc( _arguments,
				  _numberOfAvailableArguments * sizeof( char * ) );
	}
	
	_arguments[ _numberOfArguments ] = argument;

	// Add NULL argument at the end
	_arguments[ _numberOfArguments + 1] = NULL;
	
	_numberOfArguments++;
}

Command::Command()
{
	// Create available space for one simple command
	_numberOfAvailableSimpleCommands = 1;
	_simpleCommands = (SimpleCommand **)
		malloc( _numberOfSimpleCommands * sizeof( SimpleCommand * ) );

	_numberOfSimpleCommands = 0;
	_outFile = 0;
	_inputFile = 0;
	_errFile = 0;
	_background = 0;
	_catFile = 0;
	_append=0;
}

void
Command::insertSimpleCommand( SimpleCommand * simpleCommand )
{
	if ( _numberOfAvailableSimpleCommands == _numberOfSimpleCommands ) {
	        //duplicate available space for simple commmands
		_numberOfAvailableSimpleCommands *= 2;
		_simpleCommands = (SimpleCommand **) realloc( _simpleCommands,
			 _numberOfAvailableSimpleCommands * sizeof( SimpleCommand * ) );
	}
	
	_simpleCommands[ _numberOfSimpleCommands ] = simpleCommand;
	_numberOfSimpleCommands++;
}

void
Command:: clear()
{
	for ( int i = 0; i < _numberOfSimpleCommands; i++ ) {
		for ( int j = 0; j < _simpleCommands[ i ]->_numberOfArguments; j ++ ) {
			free ( _simpleCommands[ i ]->_arguments[ j ] );
		}
		
		free ( _simpleCommands[ i ]->_arguments );
		free ( _simpleCommands[ i ] );
	}

	if ( _outFile ) {
		free( _outFile );
	}
	
	if ( _catFile ) {
		free( _catFile );
	}
	
	if ( _inputFile ) {
		free( _inputFile );
	}

	if ( _errFile ) {
		free( _errFile );
	}

	_numberOfSimpleCommands = 0;
	_outFile = 0;
	_inputFile = 0;
	_errFile = 0;
	_background = 0;
	_append=0;
	_catFile = 0;
}

void
Command::print()
{
	printf("\n\n");
	printf("              COMMAND TABLE                \n");
	printf("\n");
	printf("  #   Simple Commands\n");
	printf("  --- ----------------------------------------------------------\n");
	
	for ( int i = 0; i < _numberOfSimpleCommands; i++ ) {
		printf("  %-3d ", i );
		for ( int j = 0; j < _simpleCommands[i]->_numberOfArguments; j++ ) {
			printf("\"%s\" \t", _simpleCommands[i]->_arguments[ j ] );
		}
	}

	printf( "\n\n" );
	printf( "  Output       Input        Error        Background\n" );
	printf( "  ------------ ------------ ------------ ------------\n" );
	printf( "  %-12s %-12s %-12s %-12s\n", _outFile?_outFile:"default",
		_inputFile?_inputFile:"default", _errFile?_errFile:"default",
		_background?"YES":"NO");
	printf( "\n\n" );
	
}

void
Command:: excecute_command(int i)
{

	int pid = fork();
		if ( pid == -1 ) {
			perror( "ls: fork\n");
			exit( 2 );
		}

		if (pid == 0) {
			execvp(_simpleCommands[i]->_arguments[0], _simpleCommands[i]->_arguments);
			// exec() is not suppose to return, something went wrong
			// perror( "ls: exec ls");
			exit( 2 );
		}
		
		// Wait for last process in the pipe line
		waitpid( pid, 0, 0 );
	}

void
Command::execute()
{
//CASE1:NO COMMANDS
	if ( _numberOfSimpleCommands == 0 ) {
		prompt();
		return;
	}
	
	print(); //print table
	
	//standard files:
	int default_input = dup( 0 );
	int default_output = dup( 1 );
	int default_error = dup( 2 );
	
//CASE2:MORE THAN ONE COMMAND	
	if (_numberOfSimpleCommands > 1){
	
		int pid0;
		int fdpipe[_numberOfSimpleCommands-1][2]; //create a pipe list where [i][0]--> input and [i][1]--> output
		
			for (int i=0; i < _numberOfSimpleCommands-1; i++){
			
				if (pipe(fdpipe[i]) == -1)
					perror("Error in pipe!");
			}
		
		int infd = default_input;
		int outfd = default_output;
		
			for(int i=0;i<_numberOfSimpleCommands;i++){
				parentID= getpid();
				signal(SIGCHLD,childHandler);
				if (strcmp(_simpleCommands[i]->_arguments[0], "exit") == 0)  
					{
			    		printf("Goodbye !!!\n");
			    		exit(0);
					}
				if ( _inputFile ) {
				infd = open(_inputFile, O_RDONLY);//read only
				}
				if (_append) {
					outfd = open(_outFile, O_WRONLY | O_CREAT | O_APPEND, 0666);
				}
				else if (_outFile){
					outfd = open(_outFile, O_CREAT | O_TRUNC | O_WRONLY, 0777); 
				}
				
				if (_catFile){
					outfd = open(_catFile, O_CREAT | O_WRONLY | O_APPEND, 0777);
				}
				
				if(i==0){//first command
					dup2( infd, 0 );
					dup2( fdpipe[0][1], 1 );//redirects standard output to the first pipe
				}
				else if(i==_numberOfSimpleCommands-1){//last command
					dup2( fdpipe[_numberOfSimpleCommands-2][0], 0 );//redirects standard input to 
					dup2( outfd, 1 );	
				}
				else{//commands in between
					dup2( fdpipe[i-1][0], 0 );
					dup2( fdpipe[i][1], 1 );
				}
			
				dup2( default_error, 2 );
				
				pid0 = fork();
				if ( pid0 == -1 ) {
					perror( "cat_grep: fork\n");
					exit( 0 );
				}

				if (pid0 == 0) {//child process
				for (int j=0;j<_numberOfSimpleCommands-1;j++){
				        //close unneeded directory
					close(fdpipe[j][0]);
					close(fdpipe[j][1]);
					}
					close( infd );
					close( outfd );
					close( default_error );
				execvp(_simpleCommands[i]->_arguments[0], _simpleCommands[i]->_arguments);
				perror( "Error in execvp"); //if execvp succeeded it doesn't return back
				exit( 0 );
				}
			}//end of for loop on commands
			dup2( default_input, 0 );
			dup2( default_output, 1 );
			dup2( default_error, 2 );
			for (int j=0;j<_numberOfSimpleCommands-1;j++){
			//close pipes in parent process
				close(fdpipe[j][0]);
				close(fdpipe[j][1]);
			}
			close( default_input );
			close( default_output );
			close( default_error );
			waitpid( pid0, 0, 0 );
	}// end of if #commands>1
	
//CASE3:JUST ONE COMMAND TO EXECUTE:	
	else if ( _numberOfSimpleCommands == 1 ){
	
		if (strcmp(_simpleCommands[0]->_arguments[0], "exit") == 0) {
	    		printf("Goodbye !!!\n");
	    		exit(0);
		}
			
	if (strcmp(_simpleCommands[0]->_arguments[0], "cd") == 0) {
	    char s[100];
	    printf("%s\n", getcwd(s, 100));

	    if (_simpleCommands[0]->_arguments[1] == NULL) {
		// If dir is not specified, change to the home directory
		const char* homeDir = getenv("HOME");
		if (homeDir == NULL) {
		    fprintf(stderr, "cd: No home directory found\n");
		} else {
		    if (chdir(homeDir) != 0) {
		        perror("cd");
		    }
		}
	    } else {
		// Change to the specified directory
		if (chdir(_simpleCommands[0]->_arguments[1]) != 0) {
		    perror("cd");
		}
	    }

	    printf("%s\n", getcwd(s, 100));
	    // exit(0);
	}
		
		if (_outFile || _catFile || _inputFile || _errFile){
		
			int outfd = -1;
			if (_append) {
				outfd = open(_outFile, O_WRONLY | O_CREAT | O_APPEND, 0666);
			}
			else if (_outFile){
				outfd = open(_outFile, O_CREAT | O_TRUNC | O_WRONLY);
			}
			else{
				outfd = open(_catFile, O_CREAT | O_WRONLY | O_APPEND, 0777);
			}
			int infd = open(_inputFile, O_RDONLY);
			int errorfd = open(_errFile, O_WRONLY | O_APPEND, 0777);
			
			//If any error while opening return to default file descriptor
			if ( outfd < 0 ) {
				outfd = default_output;
			}
			if ( infd < 0 ) {
				infd = default_input;
			}
			if ( errorfd < 0 ) {
				errorfd = default_error;
			}
			
			// Redirect standard output to the output file
			dup2( outfd, 1 );
			close( outfd );

			// Redirect input
			dup2( infd, 0 );
			close( infd );
			
			// Repeat for error handling
			dup2( outfd, 1 );
			close( outfd );
			
			dup2( errorfd, 2 );
			close( errorfd );
			
			excecute_command(0);
			
			dup2( default_input, 0 );
			dup2( default_output, 1 );
			dup2( default_error, 2 );

			// Close unneeded fd
			close( default_input );
			close( default_output );
			close( default_error );
		}
		else{   // No need for file descriptors
			parentID= getpid();
			signal(SIGCHLD, childHandler);
			excecute_command(0);
		}
		
	}//end if of an only command
	
	/*if(!_background) {
	    int status;
	    waitpid(pid0, &status, 0);
	}*/
// Handle wildcard expansion
    glob_t globResult;
    int flags = 0;
    int i;

    // Loop through the arguments and check for wildcard characters
    for (i = 1; _simpleCommands[0]->_arguments[i] != NULL; i++) {
        if (strchr(_simpleCommands[0]->_arguments[i], '*') != NULL ||
            strchr(_simpleCommands[0]->_arguments[i], '?') != NULL) {
            flags |= GLOB_NOCHECK;
            break;
        }
    }

    // Perform wildcard expansion if any wildcard characters are found
    if (flags != 0) {
        int result = glob(_simpleCommands[0]->_arguments[i], flags, NULL, &globResult);
        if (result == 0) {
            // Replace the wildcard argument with the expanded file names
            for (size_t j = 0; j < globResult.gl_pathc; j++) {
                printf("%s ", globResult.gl_pathv[j]);
            }
            printf("\n");
            globfree(&globResult);
            clear();
            prompt();
            return;
        } else if (result == GLOB_NOMATCH) {
            fprintf(stderr, "No match found for wildcard: %s\n", _simpleCommands[0]->_arguments[i]);
        } else {
            fprintf(stderr, "Wildcard expansion failed\n");
        }
    }
	// Clear to prepare for next command
	clear();
	
	// Print new prompt
	prompt();
}

// Shell implementation

void
Command::prompt()
{
	char s[100];
	printf("myshell%s > ",getcwd(s,100));
	fflush(stdout);
}

Command Command::_currentCommand;
SimpleCommand * Command::_currentSimpleCommand;

int yyparse(void);

int 
main()
{
	Command::_currentCommand.prompt();
	signal(SIGINT, ignoreSignal);
	yyparse();
	return 0;
}
