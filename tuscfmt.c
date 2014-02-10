/*
1. tuscfmt  - used to format tusc logfile, stop here if you don't know what tusc is.
Copyright (C) fengao

When we run tusc with "-T" argument, but forget to specifiy time format, we will get below output

--sniped from a tusc logfile
( Attached to process 2766 ("ora_lgwr_R11203") [64-bit] )
1388730764.600953 [2766] pw_wait(0x9fffffffffffb290) ..... [sleeping]
1388730765.780235 [2766] getrusage(RUSAGE_SELF, 0x9fffffffffffbc40) = 0
1388730765.780378 [2766] getrusage(RUSAGE_SELF, 0x9fffffffffffbc10) = 0
--end 

1388730764.600953 is a UNIX timestamp and hard to interpret.
This tool is used to convert it to readable time format: 2014-01-03 14:32:44.600953

2. You can't specify time format in this version.

3. Compile it with below command:

--64bit
$ gcc -m64 -o ./tuscfmt ./tuscfmt.c
--32bit
$ gcc -m32 -o ./tuscfmt ./tuscfmt.c

4. Run it as below:

eg:
$ ./tuscfmt /root/C/tusc/tusc.log ./tusc.fmt.log

5(optional): In fact, below awk code can do the same thing

awk -F. --re-interval '$1~/^[0-9]{10}$/ && $2~/^[0-9]{6}\s+*/ {$1=strftime("%F %T",$1)}1' OFS=. tusc.log

*/


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

/*********************************************/
/* GLOBAL VARIABLES                          */
/*********************************************/
char    *tusc_logfile = NULL;           /* tusc logfile pointer*/
char	*tusc_output = NULL;			/* tusc output logfile pointer*/
FILE    *tusc_logfp, *output_fp;        /* file pointer */
int     bufsiz = 4096;                  /* buffer size */
char    buffer[4096];                 	/* tusc log file buffer */
int     total_lines = 0;                /* how many rows proceed */
int     bad_lines = 0;                  /* how many bad lines */


void usage(char *argv[])
{
	printf("Usage:\n"); 
	printf("       %s tusc_logfile(full/absolute path) outputfile\n", argv[0]);
	printf("\n");
}

int main(int argc, char *argv[])
{

/*********************************************/
/* VARIABLES								 */
/*********************************************/
	time_t start_time;	/* store the start time - unix timestamp */
	time_t end_time;	/* store the end time - unix timestamp */

	/* start time */
	start_time = time(0);

	/* verify the arguments */
	if (argc != 3){
		usage(argv);			/* print usage */
		exit(1);
	}
	
	/* verify tusc logfile */
	tusc_logfile = argv[1];
	if (tusc_logfile)
	{
		tusc_logfp = fopen(tusc_logfile, "r");
		if(!tusc_logfp)
		{
			fprintf(stderr, "Error: can't open tusc log file %s\n", tusc_logfile);
			fprintf(stderr, "You have to specify absolute path, eg starting with /\n");
			exit(1);
		}
	}

	/* verify output logfile */
	tusc_output = argv[2];	
	if (tusc_output)
	{
		output_fp = fopen(tusc_output,"w");
		if(!output_fp)
		{
			fprintf(stderr, "Error: can't open tusc output file %s\n", tusc_output);
			fprintf(stderr, "You may want to check the file permissions");
			exit(1);
		}
	}
	
	/*read the logfile*/
	while((fgets(buffer, bufsiz, tusc_logfp)) != NULL)
	{
		total_lines++;
		if ( strlen(buffer) == (bufsiz - 1))
		{
			bad_lines++;  
			/* the line is too long, truncat it and skip */
			
			/* read the rest of this line */
			while((fgets(buffer, bufsiz, tusc_logfp)) != NULL)
			{
				if (strlen(buffer) < bufsiz -1)
				{
					break;
				}
			}
			continue;  
		}
		
		/* here we have one line which need proceeded*/
		
		char *c_l_t1=buffer;	/* generic char local temp variable */
		char *c_l_t2;			/* c_l_t2 is used to point to the rest characters except timestamp */
		int i_l_t1 = 0;
		int time_length=0;		/* length of the unix timestamp */
		char *c_time;			/* used to convert the time */
		struct tm *tm_time;		/* used to convert the time */
		time_t lt;				/* used to convert the time */
		char buf_time[80];		/* used to convert the time */
		int i_l_t2=0;

		while(*c_l_t1 != '\0')
		{
			switch(*c_l_t1)
			{
				case '.':	i_l_t1 = 1; c_l_t2=c_l_t1+1;break;  /* we check whether "." exists, if yes, it should be a valid record */
			}
			
			if(i_l_t1 == 1)  /* escape if we got "." */
			{
				break;
			}
			else	
			{
				c_l_t1++;
				time_length++;	
			}
		}	
		if(i_l_t1 == 1)
		{	
			/* printf("rest of the line we got: %s\n", c_l_t2);		*/		

			if ( (c_time=malloc(time_length+1))==NULL) 
			{
				exit(1);
			}
			memset(c_time, '\0', sizeof(c_time));	
			strncpy(c_time, buffer, time_length);
		
			/*convert string to time*/
			i_l_t2 = atoi(c_time);							/* convert char* to int */
			lt=i_l_t2;
			tm_time=localtime(&lt);
			strftime(buf_time,80,"%Y-%m-%d %X", tm_time);  /* format the time as YYYY-MM-DD HH24:MI:SS */
			fprintf(output_fp,"%s.%s",buf_time,c_l_t2);    /* print the time and append with . plus c_l_t2 */
			free(c_time);
		}	
		else
		{
			/* not valid to convert, just print as it be */
			fprintf(output_fp,"%s",buffer);

		}	
	}

	/*clean up resources*/
	fclose(tusc_logfp);
	fclose(output_fp);
	
	/*final*/
	end_time = time(0);
	printf("%d rows proceeded in %ld seconds, done!\n", total_lines, (end_time - start_time));	
	if(bad_lines>0)
	{
		printf("%d rows skipped due to too long, you may want to check them manually.\n", bad_lines);
	}

	return 0;	
}
