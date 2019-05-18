#include "Ngram.h"
#include "VocabMap.h"
#include <stdio.h>
#include <stdlib.h>

#ifndef MAX_CANDIDATE
    #define MAX_CANDIDATE 1050 // Maximum number of candidate for one zhuyin is 1014
#endif

using namespace std;

static char arg_test[128], arg_lm[128], arg_map[128];
static unsigned arg_order = 0;

/**
 * Get unigram: P(W1)
 */
LogP getUigramProb(Vocab &voc, Ngram &lm, string w1)
{
    VocabIndex wid1 = voc.getIndex(w1.c_str());

    if(wid1 == Vocab_None)  //OOV
        wid1 = voc.getIndex(Vocab_Unknown);

    VocabIndex context[] = { Vocab_None };
    return lm.wordProb( wid1, context);
}

/**
 * Get bigram: P(W2 | W1)
 */
LogP getBigramProb(Vocab &voc, Ngram &lm, string w1, string w2)
{
    VocabIndex wid1 = voc.getIndex(w1.c_str());
    VocabIndex wid2 = voc.getIndex(w2.c_str());

    if(wid1 == Vocab_None)  //OOV
        wid1 = voc.getIndex(Vocab_Unknown);
    if(wid2 == Vocab_None)  //OOV
        wid2 = voc.getIndex(Vocab_Unknown);

    VocabIndex context[] = { wid1, Vocab_None };
    return lm.wordProb( wid2, context);
}

/**
 * Arguments parser
 */
void argParser(int argc, char* argv[]){
    
    for(int i = 0; i < argc ; i++){
        if(!strcmp(argv[i], "-text")) strcpy(arg_test, argv[i+1]);
        if(!strcmp(argv[i], "-map")) strcpy(arg_map, argv[i+1]);
        if(!strcmp(argv[i], "-lm")) strcpy(arg_lm, argv[i+1]);
        if(!strcmp(argv[i], "-order")) sscanf(argv[i+1], "%u", &arg_order);
    }
    return;
}

/**
 * Viterbi algorithm
 */
void Viterbi(VocabString *chars, VocabString *output_chars, unsigned int chars_length,\
            Vocab &voc, Vocab &big5, Vocab &zhuyin, VocabIndex &big5_idx, VocabMap &map, Ngram &lm){

    /**
     * 1) Initialization
     */
    int i, t;
    LogP delta[chars_length][MAX_CANDIDATE];
    VocabIndex delta_index[chars_length][MAX_CANDIDATE]; // Store big5 index in each entry
    int psi[chars_length][MAX_CANDIDATE]; // For path backtracking
    int num_candidate[chars_length]; // Number of candidate at each time step

    // All sentence start from "<s>" with probability of 1
    delta[0][0] = LogP_One;
    delta_index[0][0] = big5.getIndex(Vocab_SentStart);
    num_candidate[0] = 1;

    /**
     * 2) Recursion
     */
    for (t = 1; t < chars_length; t++) {
        VocabMapIter map_iter(map, zhuyin.getIndex(chars[t])); // VocabMapIter(VocabMap &vmap, VocabIndex w);			
        VocabString w1, w2;
        LogP prob, max_prob;
        Prob p; // Useless
        int best_w1, candidate_cnt = 0;

        while (map_iter.next(big5_idx, p)) { // VocabMapIter.next(VocabIndex &w, Prob &prob);
            w2 = big5.getWord(big5_idx); // w2 = candidate word
            if (voc.getIndex(w2) == Vocab_None) {
                w2 = Vocab_Unknown;
            }

            // Iterate over every candidates at last time step
            max_prob = LogP_Zero;
            for (i = 0; i < num_candidate[t-1]; i++) {
                w1 = big5.getWord(delta_index[t-1][i]);
                prob = getBigramProb(voc, lm, w1, w2);
                // If prob too small, backoff to unigram
                if (prob == LogP_Zero) { // TODO: check if small enough
                    prob = getUigramProb(voc, lm, w2);
                }
                
                // Get total prob
                prob += delta[t-1][i];

                if (prob > max_prob) {
                    max_prob = prob;
                    best_w1 = i;
                }
            }

            delta[t][candidate_cnt] = max_prob;
            delta_index[t][candidate_cnt] = big5.getIndex(w2);
            psi[t][candidate_cnt] = best_w1;

            candidate_cnt++;
        }
        
        num_candidate[t] = candidate_cnt;
    }

    /**
     * 3) Termination
     */
    LogP max_prob = LogP_Zero;
    int bt_index; // Backtrack index

    for (i = 0; i < num_candidate[chars_length-1]; i++) {
        if (delta[chars_length-1][i] > max_prob) {
            max_prob = delta[chars_length-1][i];
            output_chars[chars_length-1] = big5.getWord(delta_index[chars_length-1][i]);
            bt_index = i;
        }
    }

    /**
     * 4) Path backtracking
     */
    for (t = chars_length - 2; t >= 0; t--) {
        bt_index = psi[t+1][bt_index];
        output_chars[t] = big5.getWord(delta_index[t][bt_index]);
    }

    /**
     * Check if words change from big5 to <unk>,
     * return them back to original words
     */
    for (t = 0; t < chars_length; t++) {
        if (strcmp(chars[t], Vocab_Unknown) && !strcmp(output_chars[t], Vocab_Unknown)) {
            output_chars[t] = chars[t];
        }
    }

    return;
}

int main(int argc, char* argv[]){
    // cout << "Hello mydisambig!" << endl;

    /**
     * Parse the arguments
     */
    argParser(argc, argv);

    if (arg_order != 2){
        cerr << "Only support order = 2." <<endl;
        exit(1);
    }
    
    if (!(strlen(arg_test) && strlen(arg_lm) && strlen(arg_map) && arg_order)){
        cerr << "Usage: ./mydisambig -text testdata/$$i.txt -map ZhuYin-Big5.map -lm bigram.lm -order 2 > result/$$i.txt" << endl;
        exit(1);
    }
    
    //cout << arg_test << endl; 
    //cout << arg_lm << endl; 
    //cout << arg_map << endl; 
    //cout << arg_order << endl; 

    
    Vocab voc, zhuyin, big5;
    VocabIndex big5_idx;

    /**
     * Read the language model
     */
    Ngram lm(voc, arg_order);
    {
        File lm_file(arg_lm, "r");
        lm.read(lm_file);
        lm_file.close();
    }

    /**
     * Read the map
     */
    VocabMap map(zhuyin, big5);	
    {
        File map_file(arg_map, "r");
        map.read(map_file);
        map_file.close();
    }
    
    /**
     * Read the test file
     */
    File test_file(arg_test, "r");
    char *line;
    while(line =test_file.getline()){

        /**
         * Tokenize each characters
         * 
         * Constant in File.h:
         * const unsigned int maxWordsPerLine = 50000;
         * 
         * Constant in Vocab.h:
         * const VocabIndex	Vocab_None = (VocabIndex)-1;
         * const VocabString	Vocab_Unknown = "<unk>";
         * const VocabString	Vocab_SentStart = "<s>";
         * const VocabString	Vocab_SentEnd = "</s>";
         * 
         * characters = ["<s>", "char_1", "char_2", ..., "char_n", "</s>"]
        */

        VocabString characters[maxWordsPerLine];
        unsigned int length;
        length = Vocab::parseWords(line, &(characters[1]), maxWordsPerLine);
        characters[0] = Vocab_SentStart; // Vocab_SentStart = "<s>"
        characters[length+1] = Vocab_SentEnd; // Vocab_SentEnd = "</s>"
        length += 2;

        VocabString output_characters[length];
        Viterbi(characters, output_characters, length, voc, big5, zhuyin, big5_idx, map, lm);
        
        for (int i = 0; i < length; i++) {
            cout << output_characters[i];
            if (i == length - 1) {
                cout << endl;
            } else {
                cout << " ";
            }
        }
    }
    
    test_file.close();

    return 0;
}