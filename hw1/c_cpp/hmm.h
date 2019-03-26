#ifndef HMM_HEADER_
#define HMM_HEADER_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifndef MAX_STATE
#	define MAX_STATE 10
#endif

#ifndef MAX_OBSERV
#	define MAX_OBSERV 26
#endif

#ifndef MAX_SEQ
#	define	MAX_SEQ 200
#endif

#ifndef MAX_LINE
#	define MAX_LINE 256
#endif

// Define the maximun number of samples in one data file
#ifndef MAX_SAMPLE_NUM
#	define MAX_SAMPLE_NUM 10000
#endif

typedef struct{
   char *model_name;
   int state_num;					//number of state
   int observ_num;					//number of observation
   double initial[MAX_STATE];			//initial prob.(pi)
   double transition[MAX_STATE][MAX_STATE];	//transition prob.(a)
   double observation[MAX_OBSERV][MAX_STATE];	//observation prob.(b)
} HMM;

typedef struct{
   int seq_num;
   int obs[MAX_SEQ];
} Observ;

typedef struct{
   int seq_num;
   int state_num;
   double variable[MAX_SEQ][MAX_STATE];
} Variable;

typedef struct{
   int seq_num;
   int state_num;
   double variable[MAX_SEQ][MAX_STATE][MAX_STATE];
} Epsilon;

static FILE *open_or_die( const char *filename, const char *ht )
{
   FILE *fp = fopen( filename, ht );
   if( fp == NULL ){
      perror( filename);
      exit(1);
   }

   return fp;
}

static void loadHMM( HMM *hmm, const char *filename )
{
   int i, j;
   FILE *fp = open_or_die( filename, "r");

   hmm->model_name = (char *)malloc( sizeof(char) * (strlen( filename)+1));
   strcpy( hmm->model_name, filename );

   char token[MAX_LINE] = "";
   while( fscanf( fp, "%s", token ) > 0 )
   {
      if( token[0] == '\0' || token[0] == '\n' ) continue;

      if( strcmp( token, "initial:" ) == 0 ){
         fscanf(fp, "%d", &hmm->state_num );

         for( i = 0 ; i < hmm->state_num ; i++ )
            fscanf(fp, "%lf", &( hmm->initial[i] ) );
      }
      else if( strcmp( token, "transition:" ) == 0 ){
         fscanf(fp, "%d", &hmm->state_num );

         for( i = 0 ; i < hmm->state_num ; i++ )
            for( j = 0 ; j < hmm->state_num ; j++ )
               fscanf(fp, "%lf", &( hmm->transition[i][j] ));
      }
      else if( strcmp( token, "observation:" ) == 0 ){
         fscanf(fp, "%d", &hmm->observ_num );

         for( i = 0 ; i < hmm->observ_num ; i++ )
            for( j = 0 ; j < hmm->state_num ; j++ )
               fscanf(fp, "%lf", &( hmm->observation[i][j]) );
      }
   }
   fclose(fp);
}

static void dumpHMM( FILE *fp, HMM *hmm )
{
   int i, j;

   //fprintf( fp, "model name: %s\n", hmm->model_name );
   fprintf( fp, "initial: %d\n", hmm->state_num );
   for( i = 0 ; i < hmm->state_num - 1; i++ )
      fprintf( fp, "%.5lf ", hmm->initial[i]);
   fprintf(fp, "%.5lf\n", hmm->initial[ hmm->state_num - 1 ] );

   fprintf( fp, "\ntransition: %d\n", hmm->state_num );
   for( i = 0 ; i < hmm->state_num ; i++ ){
      for( j = 0 ; j < hmm->state_num - 1 ; j++ )
         fprintf( fp, "%.5lf ", hmm->transition[i][j] );
      fprintf(fp,"%.5lf\n", hmm->transition[i][hmm->state_num - 1]);
   }

   fprintf( fp, "\nobservation: %d\n", hmm->observ_num );
   for( i = 0 ; i < hmm->observ_num ; i++ ){
      for( j = 0 ; j < hmm->state_num - 1 ; j++ )
         fprintf( fp, "%.5lf ", hmm->observation[i][j] );
      fprintf(fp,"%.5lf\n", hmm->observation[i][hmm->state_num - 1]);
   }
}

static int load_models( const char *listname, HMM *hmm, const int max_num )
{
   FILE *fp = open_or_die( listname, "r" );

   int count = 0;
   char filename[MAX_LINE] = "";
   while( fscanf(fp, "%s", filename) == 1 ){
      loadHMM( &hmm[count], filename );
      count ++;

      if( count >= max_num ){
         return count;
      }
   }
   fclose(fp);

   return count;
}

static int load_models_by_iter( const char *listname, HMM *hmm, const int max_num , const int iter)
{
   FILE *fp = open_or_die( listname, "r" );

   int count = 0;
   char filename[MAX_LINE] = "";
   char *model_file_iter = (char*) malloc(strlen(filename) + 1);
   char iters[10];

   while( fscanf(fp, "%s", filename) == 1 ){

      strcpy(model_file_iter, filename);

      char *ext_ptr = strrchr (model_file_iter, '.');
      size_t ext_size = strlen (ext_ptr);
      size_t slash_size = strlen ("_");

      strcat(model_file_iter, "_");
      strncpy (ext_ptr, "_", slash_size);

      if (slash_size < ext_size)
         *(ext_ptr+slash_size) = 0;

      sprintf(iters, "%05d", iter);
      strcat(iters, ".txt");
      strcat (model_file_iter, iters);
      //printf("model_iter: %s\n", model_file_iter);
      loadHMM( &hmm[count], model_file_iter );
      count ++;

      if( count >= max_num ){
         return count;
      }
   }
   fclose(fp);
   if (model_file_iter) free(model_file_iter);

   return count;
}

static void dump_models( HMM *hmm, const int num )
{
   int i = 0;
   for( ; i < num ; i++ ){ 
      //		FILE *fp = open_or_die( hmm[i].model_name, "w" );
      printf("model_%02d\n", i+1);
      dumpHMM( stderr, &hmm[i] );
      printf("===============================================\n");
   }
}

static void dump_models_by_iter( HMM *hmm, const int num, const int iter)
{
   int i = 0;
   for( ; i < num ; i++ ){ 
      //		FILE *fp = open_or_die( hmm[i].model_name, "w" );
      printf("model_%02d_%05d\n", i+1, iter);
      dumpHMM( stderr, &hmm[i] );
      printf("===============================================\n");
   }
}

static int fetch_data(Observ *observs, const char *filename)
{
   FILE *fp = open_or_die( filename, "r" );

   char token[MAX_LINE] = "";
   int sample_num = 0, i, idx;

   while( fscanf( fp, "%s", token ) > 0 )
   {
      if( token[0] == '\0' || token[0] == '\n' ) continue;

      for( i = 0 ; i < MAX_LINE ; i++ ){
         switch (token[i]) {
            case 'A':
               idx = 0;
               break;
            case 'B':
               idx = 1;
               break;
            case 'C':
               idx = 2;
               break;
            case 'D':
               idx = 3;
               break;
            case 'E':
               idx = 4;
               break;
            case 'F':
               idx = 5;
               break;
            default:
               idx = -1;
               break;

         }

         if (idx == -1){
            observs[sample_num].seq_num = i;
            sample_num++;
            break;
         }
         
         observs[sample_num].obs[i] = idx;
      }
   }

   fclose(fp);
   return sample_num;

}
#endif

