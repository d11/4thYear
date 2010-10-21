%{
#include <stdio.h>
#include <string.h>

void yyerror(const char *str)
{
	fprintf(stderr,"error: %s\n",str);
}

int yywrap()
{
	return 1;
}

main()
{
	yyparse();
}

%}

%token FALSE TRUE PVAR NOT AND OR LPAREN RPAREN AX

%%


formula:
	false | true | pvar | not | and | or | ax ; 

false:
   FALSE
   {
      printf("\t[False]\n");
   }
   ;

true:
   TRUE 
   {
      printf("\t[True]\n");
   }
   ;

pvar:
   PVAR
   {
     printf("\tVariable: %s\n",$1);
   }
   ;

not:
   NOT formula
   ;

and:
   LPAREN formula AND formula RPAREN
   ;

or:
   LPAREN formula OR formula RPAREN
   ;

ax:
   AX formula
   ;



