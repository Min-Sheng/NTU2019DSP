#include "Ngram.h"
#include "VocabMap.h"
#include <stdio.h>
#include <stdlib.h>

#ifndef MAX_CANDIDATE
    #define MAX_CANDIDATE 1024 // Maximum number of the candidate chinese characters 
                               // for one zhuyin character is 1024
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
            Vocab &voc, Vocab &big5, Vocab &zhuyin, VocabIndex &big5Chi_idx, VocabMap &map, Ngram &lm){

    /**
     * 1) Initialization
     */
    int i, t;
    LogP delta[chars_length][MAX_CANDIDATE]; // Store the highest likelihood
    VocabIndex delta_idx[chars_length][MAX_CANDIDATE]; // Store big5 Chinese character index
    int psi[chars_length][MAX_CANDIDATE]; // Store backtracking index for path backtracking
    int num_candidate[chars_length]; // Number of candidate chinese characters at each time step

    // All sentence start from "<s>" with probability of 1
    delta[0][0] = LogP_One;
    delta_idx[0][0] = big5.getIndex(Vocab_SentStart);
    num_candidate[0] = 1;

    /**
     * 2) Recursion
     */
    for (t = 1; t < chars_length; t++) {
        VocabMapIter map_iter(map, zhuyin.getIndex(chars[t])); // Use VocabMapIter(VocabMap &vmap, VocabIndex c) to traversal
                                                               // all chinese characters corresponding to each zhuyin character
        VocabString c1, c2; // Bigram (neighboring chinese characters)
        LogP likelihood, max_likelihood;
        Prob p; // Useless
        int best_c1, candidate_cnt = 0;

        while (map_iter.next(big5Chi_idx, p)) { // VocabMapIter.next(VocabIndex &c, Prob &prob);
            c2 = big5.getWord(big5Chi_idx); // c2 = candidate character
            if (voc.getIndex(c2) == Vocab_None) { // Replace OOV with <unk>
                c2 = Vocab_Unknown;
            }

            // Iterate over every candidates at last time step
            max_likelihood = LogP_Zero;
            for (i = 0; i < num_candidate[t-1]; i++) {
                c1 = big5.getWord(delta_idx[t-1][i]);
                likelihood = getBigramProb(voc, lm, c1, c2);
                
                if (likelihood == LogP_Zero) { // If likelihood too small, backoff to unigram
                    likelihood = getUigramProb(voc, lm, c2);
                }
                
                likelihood += delta[t-1][i]; // Sum up to get total likelihood


                if (likelihood > max_likelihood) { // Find maximum likelihood
                    max_likelihood = likelihood;
                    best_c1 = i;
                }
            }

            delta[t][candidate_cnt] = max_likelihood;
            delta_idx[t][candidate_cnt] = big5.getIndex(c2);
            psi[t][candidate_cnt] = best_c1;

            candidate_cnt++;
        }
        
        num_candidate[t] = candidate_cnt;
    }

    /**
     * 3) Termination
     */
    LogP max_likelihood = LogP_Zero;
    int bt_index; // Backtrack index

    for (i = 0; i < num_candidate[chars_length-1]; i++) {
        if (delta[chars_length-1][i] > max_likelihood) {
            max_likelihood = delta[chars_length-1][i];
            output_chars[chars_length-1] = big5.getWord(delta_idx[chars_length-1][i]);
            bt_index = i;
        }
    }

    /**
     * 4) Path backtracking
     */
    for (t = chars_length - 2; t >= 0; t--) {
        bt_index = psi[t+1][bt_index];
        output_chars[t] = big5.getWord(delta_idx[t][bt_index]);
    }

    /**
     * Check if characters change from big5 to <unk>,
     * return them back to original characters
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

    
    Vocab voc, zhuyin, big5Chi;
    VocabIndex big5Chi_idx;

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
    VocabMap map(zhuyin, big5Chi);	
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

        /**
         * Start running Viterbi algo.
         */
        VocabString output_characters[length];
        Viterbi(characters, output_characters, length, voc, big5Chi, zhuyin, big5Chi_idx, map, lm);
        
        /**
         * Output the result
         */
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