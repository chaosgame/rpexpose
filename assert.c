#include "assert.h"

void _assert_exit(int expression, int line_number, char *file, char *error_message){
	if(expression){
		if(errno){
			if( error_message )
				fprintf(stderr,"Fatal Error on line %i of %s:%s\n",line_number, file, error_message);
			else
				fprintf(stderr,"Fatal Error on line %i of %s:%s\n",line_number, file, strerror(errno));
			exit(errno);
		}else{
			fprintf(stderr,"Fatal Error on line %i of %s:%s\n",line_number, file, error_message);
			exit(1);
		}
	}
}

void _assert(int expression, int line_number, char *file, char *error_message){
	if(expression){
		if(errno){
			if( error_message )
				fprintf(stderr,"Error on line %i of %s:%s\n",line_number, file, error_message);
			else
				fprintf(stderr,"Error on line %i of %s:%s\n",line_number, file, strerror(errno));
		}else{
			fprintf(stderr,"Error on line %i of %s:%s\n",line_number, file, error_message);
		}
	}
}

void _error(int line_number, char *file, char *error_message){
	if(errno){
		if( error_message )
			fprintf(stderr,"Error on line %i of %s:%s\n",line_number, file, error_message);
		else
			fprintf(stderr,"Error on line %i of %s:%s\n",line_number, file, strerror(errno));
	}else{
		fprintf(stderr,"Error on line %i of %s:%s\n",line_number, file, error_message);
	}
}
