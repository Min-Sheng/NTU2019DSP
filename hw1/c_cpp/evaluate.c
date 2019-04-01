#include "hmm.h"

int main(int argc, char*argv[])
{
    if (argc != 3 && argc != 4 && argc != 5){
        printf("Wrong command format\n");
		printf("Format: ./evaluate [result1.txt] [testing_answer.txt] [(acc.txt)] [all]\n");
		exit(1);
    }

    const char *result_file = argv[1];
	const char *answer_file = argv[2];
    const char *acc_file;
    
    int test_all = 0;
    if(argc == 5 && !strcmp(argv[4], "all")){
        test_all=1;
        acc_file = argv[3];
    }
    else if (argc == 4 && !strcmp(argv[3], "all")){
        test_all=1;
        acc_file = "acc.txt";
    }
    else if (argc == 4 && strcmp(argv[3], "all")){
        acc_file = argv[3];
    }
    else{
        acc_file = "acc.txt";
    }

    int TP = 0, N = 0;
    char prediction[MAX_LINE];
    char GT[MAX_LINE];
    double prob, acc;

    if(test_all){

        int iter;
        char *result_file_iter = (char*) malloc(strlen(result_file) + 1);
        strcpy(result_file_iter, result_file);
        char *ext_ptr = strrchr (result_file_iter, '.');
        size_t ext_size = strlen (ext_ptr);
        size_t slash_size = strlen ("_");

        strcat(result_file_iter, "_");
        strncpy (ext_ptr, "_", slash_size);

        if (slash_size < ext_size)
            *(ext_ptr+slash_size) = 0;

        FILE *fp = open_or_die(acc_file, "w");

        for (iter = 25; iter < 1050 ; iter += 25){

            TP = 0, N = 0;
            char iters[10];
            sprintf(iters, "%05d", iter);
            strcat(iters, ".txt");

            char *slash_ptr = strrchr (result_file_iter, '_');
            size_t ext_size = strlen (slash_ptr);
            size_t iters_size = strlen (iters);
    
            strncpy (slash_ptr+1, iters, iters_size);
            if (iters_size > ext_size)
                *(slash_ptr+1+iters_size) = 0;

            
            FILE *fp_result_iter, *fp_answer;
            fp_result_iter = open_or_die(result_file_iter, "r");
            fp_answer = open_or_die(answer_file, "r");
        
            while (!feof(fp_result_iter) && !feof(fp_answer)){
                fscanf(fp_result_iter, "%s %le", prediction, &prob);
                fscanf(fp_answer, "%s", GT);
                //printf("%s", prediction);
                //printf(" %s\n", GT);
                
                int idx = (int)(strrchr (prediction, '_')-prediction);

                if (idx > 5 ){
                    if (strncmp(prediction, GT, idx) == 0) {
                        TP++;
                    }

                }else{
                    if (strcmp(prediction, GT) == 0) {
                        TP++;
                    }
                }
                

                N++;
            }
        
            fclose(fp_result_iter);
            fclose(fp_answer);
            printf("=====Iter #%d=====\n", iter);
            printf("True Positive: %d\n", TP);
            printf("Total Number: %d\n", N);

            acc = (double)TP/(double)N;
            printf("Accuracy: %f\n", acc);
            fprintf(fp, "%05d\t", iter);
            fprintf(fp, "%f\n", acc);
        }

        TP = 0, N = 0;
        FILE *fp_result, *fp_answer;
        fp_result = open_or_die(result_file, "r");
        fp_answer = open_or_die(answer_file, "r");
        
        while (!feof(fp_result) && !feof(fp_answer)){
            fscanf(fp_result, "%s %le", prediction, &prob);
            fscanf(fp_answer, "%s", GT);
            //printf("%s", prediction);
            //printf(" %s\n", GT);
            
            int idx = (int)(strrchr (prediction, '_')-prediction);

            if (idx > 5 ){
                if (strncmp(prediction, GT, idx) == 0) {
                    TP++;
                }

            }else{
                if (strcmp(prediction, GT) == 0) {
                    TP++;
                }
            }
            

            N++;
        }
        
        fclose(fp_result);
        fclose(fp_answer);

        printf("True Positive: %d\n", TP);
        printf("Total Number: %d\n", N);

        acc = (double)TP/(double)N;
        printf("Accuracy: %f\n", acc);
        fprintf(fp, "%05d\t", iter);
        fprintf(fp, "%f\n", acc);
        fclose(fp);
        if (result_file_iter) free(result_file_iter);

    }
    else{

        FILE *fp_result, *fp_answer;
        fp_result = open_or_die(result_file, "r");
        fp_answer = open_or_die(answer_file, "r");
        while (!feof(fp_result) && !feof(fp_answer)){
            fscanf(fp_result, "%s %le", prediction, &prob);
            fscanf(fp_answer, "%s", GT);
            //printf("%s", prediction);
            //printf(" %s\n", GT);
            
            int idx = (int)(strrchr (prediction, '_')-prediction);

            if (idx > 5 ){
                if (strncmp(prediction, GT, idx) == 0) {
                    TP++;
                }

            }else{
                if (strcmp(prediction, GT) == 0) {
                    TP++;
                }
            }
            

            N++;
        }
        
        fclose(fp_result);
        fclose(fp_answer);

        printf("True Positive: %d\n", TP);
        printf("Total Number: %d\n", N);

        acc = (double)TP/(double)N;

        printf("Accuracy: %f\n", acc);
        FILE *fp = open_or_die(acc_file, "w");
        fprintf(fp, "%f", acc);
        fclose(fp);

    }

    return 0;
}