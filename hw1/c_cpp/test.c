// Testing Procedure: Viterbi Algorithm
#include "hmm.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

double viterbi_alorithm(HMM *hmm, Observ *observ, Variable *delta, Variable *psi)
{
    int i, j, t, argmax;
    double max, p;
    double P = 0;
    Observ q;

    delta->seq_num = observ->seq_num;
    delta->state_num = hmm->state_num;
    psi->seq_num = observ->seq_num;
    psi->state_num = hmm->state_num;

    q.seq_num = observ->seq_num;

    // Initialization
    for (i = 0; i < delta->state_num; i++){
        delta->variable[0][i] = hmm->initial[i] * hmm->observation[observ->obs[0]][i];   //delta[0][i] = pi[i] * b[o[0]][i]
                                                                                         // 0<=i<=(N-1)
                                                                                         // Here the items start from 0 instead of 1, which is different from the lecture.
    }

    // Recursion
    for (t = 0; t < delta->seq_num -1 ; t++){
        for (j = 0; j < delta->state_num; j++){
            max=0;
            for (i = 0; i < delta->state_num; i++){
                p = delta->variable[t][i] * hmm->transition[i][j];
                if (p > max){
                    max = p;
                    argmax = i;
                }
            }
            delta->variable[t+1][j] = max * hmm->observation[observ->obs[t+1]][j];   //delta[t+1][j] = max_{0<=i<=(N-1)}{delta[t][i] * a[i][j]} * b[o[t+1]][j]
                                                                                     // 0<=t<=(T-2), 0<=j<=(N-1)
            psi->variable[t+1][j] = argmax;
        }
    }

    // Termination
    for (i = 0; i < delta->state_num; i++){
        if (delta->variable[delta->seq_num-1][i] > P){
            P = delta->variable[delta->seq_num-1][i];
            q.obs[delta->seq_num-1] = i;
        }
    }

    // Path Backtracking
    for (t = delta->seq_num-2; t > -1; t--){
        q.obs[t] = psi->variable[t+1][q.obs[t+1]]; // q[t] = psi[t+1][q[t+1]], 0<=t<=(T-2)
    }

    return P;
}

int main(int argc, char *argv[])
{
    if (argc !=  4 &&argc !=  5){
        printf("Error: Wrong command format\n");
        printf("Format: ./test [modellist.txt] [testing_data.txt] [result.txt] [all]\n");
        exit(1);
    }

    int i, j, iter;
    int sample_num; //total number of samples in the data file
    int argmax;
    double max, p;
    int prediction[MAX_SAMPLE_NUM];
    double probability[MAX_SAMPLE_NUM];
    Observ data[MAX_SAMPLE_NUM];
    Variable delta, psi;

    int test_all = 0;
    const char *modellist = argv[1];
    const char *test_data = argv[2];
    const char *result_file = argv[3];

    if (argc == 5 && !strcmp(argv[4], "all")){
        test_all = 1;
    }

    printf("modellist: %s\n", modellist);
    printf("test_data: %s\n", test_data);
    printf("result_file: %s\n", result_file);
    
    sample_num = fetch_data(data, test_data);
    printf("data_num: %d\n", sample_num);

    // Load HMM models
    HMM hmms[5];

    if(test_all){
        
        char *result_file_iter = (char*) malloc(strlen(result_file) + 1);
        strcpy(result_file_iter, result_file);
        char *ext_ptr = strrchr (result_file_iter, '.');
        size_t ext_size = strlen (ext_ptr);
        size_t slash_size = strlen ("_");

        strcat(result_file_iter, "_");
        strncpy (ext_ptr, "_", slash_size);

        if (slash_size < ext_size)
            *(ext_ptr+slash_size) = 0;

        for (iter = 25; iter < 1050 ; iter += 25){

            char iters[10];
            sprintf(iters, "%05d", iter);
            strcat(iters, ".txt");
            
            char *slash_ptr = strrchr (result_file_iter, '_');
            size_t ext_size = strlen (slash_ptr);
            size_t iters_size = strlen (iters);
    
            strncpy (slash_ptr+1, iters, iters_size);
            if (iters_size > ext_size)
                *(slash_ptr+1+iters_size) = 0;
            
            // Inference the testing data with each iteration model
            load_models_by_iter( modellist, hmms, 5, iter);
            //dump_models_by_iter( hmms, 5, iter);
            for (i = 0; i < sample_num; i++){
                max = 0;
                for (j = 0; j < 5 ;j++){
                    p = viterbi_alorithm(&hmms[j], &data[i], &delta, &psi);
                    
                    if (p > max){
                        max = p;
                        argmax = j;
                    }
                }

                prediction[i] = argmax;
                probability[i] = max;
            }

            // Save result of each iteration
            printf("Save result file: %s\n", result_file_iter);
            
            FILE *fp = open_or_die(result_file_iter, "w");

            for (i = 0; i < sample_num; i++) {
                fprintf(fp, "%s ", hmms[prediction[i]].model_name);
                fprintf(fp, "%e\n", probability[i]);
            }

            fclose(fp);

        }

        if (result_file_iter) free(result_file_iter);

    }

    // Inference the testing data with final model
    load_models( modellist, hmms, 5);
    dump_models( hmms, 5);

    for (i = 0; i < sample_num; i++){
        max = 0;
        for (j = 0; j < 5 ;j++){
            p = viterbi_alorithm(&hmms[j], &data[i], &delta, &psi);
            
            if (p > max){
                max = p;
                argmax = j;
            }
        }

        prediction[i] = argmax;
        probability[i] = max;
    }

    // Save result of final iteration
    printf("Save result file: %s\n", result_file);
    
    FILE *fp = open_or_die(result_file, "w");

    for (i = 0; i < sample_num; i++) {
        fprintf(fp, "%s ", hmms[prediction[i]].model_name);
        fprintf(fp, "%e\n", probability[i]);
    }

    fclose(fp);

    return 0;
}