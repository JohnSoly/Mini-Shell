
/*
 * CS-413 Spring 98
 * shell.y: parser for shell
 *
 * This parser compiles the following grammar:
 *
 *	cmd [arg]* [> filename]
 *
 * you must extend it to understand the complete shell grammar
 *
 */

%token	<string_val> WORD

%token 	NOTOKEN GREAT GREATER LESS NEWLINE PIPE BACKGROUND ERROR

%union	{
		char   *string_val;
	}

%{
extern "C" 
{
	int yylex();
	void yyerror (char const *s);
}
#define yylex yylex
#include <stdio.h>
#include "command.h"
#include "y.tab.h"
%}

%%

goal:	
	commands
	;

commands: 
	command
	| commands command 
	;

command: simple_command
        ;

simple_command:

	command_and_args iomodifiers_opt PIPE command {
		
	}
	|	
	command_and_args iomodifiers_opt NEWLINE {
		printf("   Yacc: Execute command\n");
		Command::_currentCommand.execute();
	}
	| NEWLINE 
	| error NEWLINE { yyerrok; }
	;

command_and_args:
	command_word arg_list {
		Command::_currentCommand.
			insertSimpleCommand( Command::_currentSimpleCommand );
	}
	;

arg_list:
	arg_list argument
	| /* can be empty */
	;

argument:
	WORD {
               printf("   Yacc: insert argument \"%s\"\n", $1);

	       Command::_currentSimpleCommand->insertArgument( $1 );\
	}
	;

command_word:
	WORD {
               printf("   Yacc: insert command \"%s\"\n", $1);
	       
	       Command::_currentSimpleCommand = new SimpleCommand();
	       Command::_currentSimpleCommand->insertArgument( $1 );
	}
	;
iomodifiers_opt:
    /* can be empty */
    | iomodifiers_opt iomodifier
    ;

iomodifier:
    input_redirection
    | output_redirection
    | error_redirection
    ;

input_redirection:
    LESS WORD {
        printf("   Yacc: insert input \"%s\"\n", $2);
        Command::_currentCommand._inputFile = $2;
    }
    ;

output_redirection:
    GREAT WORD {
        printf("   Yacc: insert output \"%s\"\n", $2);
        Command::_currentCommand._outFile = $2;
    }
    | GREATER WORD {
        printf("   Yacc: insert output \"%s\"\n", $2);
        Command::_currentCommand._outFile = $2;
        Command::_currentCommand._append = 1;
    }
    ;
error_redirection:
    ERROR WORD {
        printf("   Yacc:insert output & error\"%s\"\n" , $2 );
        Command::_currentCommand._outFile = $2;
        Command::_currentCommand._errFile = $2;
        }
        |
     BACKGROUND {
        printf("   Yacc:insert background=YES\n");
        Command::_currentCommand._background= 1;
        }
        ;

%%

void
yyerror(const char * s)
{
	fprintf(stderr,"%s", s);
}

#if 0
main()
{
	yyparse();
}
#endif
