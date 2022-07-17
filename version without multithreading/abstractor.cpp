#include <iostream>
#include <vector>
#include <list>
#include <stack>
#include <queue>
#include <fstream>
#include <string>
#include <regex>
#include <set>
#include <iterator>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
using namespace std;

//given a vector of abstracts files, function returns a vector calculating similarities for each
vector<double> JaccardSimilarity(vector<vector<string>> &abstracts, vector<string> &searched){    
    set<string> key(searched.begin(), searched.end());
    vector<double> result;
    for(int i = 0; i<abstracts.size(); i++){
        set<string> s;
        for(int j = 0; j < abstracts[i].size(); j++) s.insert(abstracts[i][j]);
        set<string> intersection;
        set<string> unison;
        set_intersection(key.begin(), key.end(), s.begin(), s.end(), inserter(intersection, intersection.begin()));
        set_union(key.begin(), key.end(), s.begin(), s.end(), inserter(unison, unison.begin()));
        double union_size = unison.size();
        double intersection_size = intersection.size();
        result.push_back(intersection_size / union_size);
    }
    return result;
}

//given an abstract file, returns the sentences containing at least one of keywords
string ParseText(string file, vector<string> &key)
{
    string sentences = "";
    file = " "+ file;
    set<string> ky(key.begin(), key.end());
    regex rule_for_words("[a-zA-Z0-9]+");
    regex rule_for_sentences("[ \na-zA-Z0-9]*\\.");
    auto beginning = sregex_iterator(file.begin(), file.end(), rule_for_sentences);
    auto ending = sregex_iterator();
    for(sregex_iterator i = beginning; i != ending; i++){
        string pl = (*i).str();
        auto beginning = sregex_iterator(pl.begin(), pl.end(), rule_for_words);
        auto ending = sregex_iterator();
        for(sregex_iterator j = beginning; j != ending; j++){
            bool flag = true;
            for(set<string>::iterator k=ky.begin(); k!=ky.end(); ++k){
                if((*j).str() == (*k)){
                    flag = false;
                    sentences += pl;
                    break;
                }
            }
            if(!flag) break;
        }    
    }
    return sentences;
}

// returns a vector of words and a dot
vector<string> tokenize(string &st){
    vector<string> sA;
    regex rule_for_words("[a-zA-Z0-9\\.]+");
    auto beginning = sregex_iterator(st.begin(), st.end(), rule_for_words);
    auto ending = sregex_iterator();
    for(sregex_iterator j = beginning; j != ending; j++){sA.push_back((*j).str());}
    return sA;
}

int main(int args, char* argv[])
{
	ifstream input;
	input.open(argv[1]);
	ofstream output;
	output.open(argv[2]);
    int num_of_threads, scanned_abstracts, summarized_abstracts;
    string query;
    input >> num_of_threads;
    input >> scanned_abstracts;
    input >> summarized_abstracts;
    getline(input, query);
	getline(input, query);
    vector<string> text_file_paths;
    for(int i = 0; i<scanned_abstracts; i++){
        string txt;
        getline(input, txt);
        txt = "../abstracts/"+txt;
        text_file_paths.push_back(txt);   
    }
    input.close();

    vector<double> jaccard;
    vector<vector<string>> word_arrays_for_abstracts;
    vector<string> srch;
    vector<string> key_found_sentences;

    srch = tokenize(query);

    for(int i = 0; i<scanned_abstracts; i++){
        ifstream in;
        in.open(text_file_paths[i]);
        string st = "";
    	while(!in.eof()){
            string n;
            getline(in, n);
            st+=n;
        }
        
        word_arrays_for_abstracts.push_back(tokenize(st));
        key_found_sentences.push_back(ParseText(st, srch));
        in.close();
    }

    jaccard = JaccardSimilarity(word_arrays_for_abstracts, srch);
    for(int p = 0; p<summarized_abstracts; p++){
        int max_index = max_element(jaccard.begin(), jaccard.end()) - jaccard.begin();
        output << "Result " << p+1 << ":\n";
        string temp = text_file_paths[max_index];
        output << "File: " << temp.substr(13,temp.length()) << "\n";
        output << "Score: " << jaccard[max_index] << "\n";
        output << "Summary: " << key_found_sentences[max_index];
        jaccard[max_index] = -1;
    }
    output << "###";
    output.close();
    return 0;
}