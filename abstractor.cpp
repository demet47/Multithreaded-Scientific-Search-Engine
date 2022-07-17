//@author: Demet YAYLA
//school number: 2019400105

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
#include <iomanip>
#include "Monitor.h"
using namespace std;

/*
Main idea for the project is implementing a concurrent read exclusive write (CREW) program that reads from files and outputs the
files that have the most similarity, with the endex we chose to compare and the similar sentences extracted from them. Aim of
the project for this course is for us to practice multithreading, specifically threads that are utilized until no work is left
(we don't kill them) which speeds up the process since killing a process and creating them from scratch is a waste of time. Also,
we are enforced to implement the threads such that no thread is idle (which is actually one of the basic aims of using threads) and
multiple read writes cause some producer-consumer problems and we had to use some mechanism such as mutex, semaphore or monitor to 
solve that issue (I chose to implement it with monitor while providing conditions with mutex). I included two seperate flags to
regulate multiple read and writes. Within flags, I merged some statements to act like atomic instructions to prevent clashes.
*/


//
int num_of_threads, scanned_abstracts, summarized_abstracts;
//below I have three arrays which I use as hashtables. Same index for array corresponds to a data of a single abstract. 
string *file_names; //stores the corresponding filenames for the abstracts
double *jaccard_endexes; //stores the calculated jaccard endex for each abstract
string *matched_strings; //stores the matched string that contains at least one keyword.
vector<string> srch; //stores the words to be searched for
Monitor *monitor; //monitor object that regulates the data flow
ofstream output;
int thread_id;
pthread_mutex_t lock1; //lock for monitor state changes
pthread_mutex_t lock2; //lock for writing operations


//given a vector of abstracts files, function returns a vector calculating similarities for each
double JaccardSimilarity(vector<string> &abstract, vector<string> &searched)
{
    set<string> key(searched.begin(), searched.end()); //used to exclude reccurance of a word in the query
    double result; //return value
    set<string> s; //used to exlucde recurrance of a word for text in abstract used with the following line
    for (int j = 0; j < abstract.size(); j++) s.insert(abstract[j]);
    set<string> intersection; //stores intersection of set of words in abstract and query
    set<string> unison; //stores unison of set of words in abstract and query
    set_intersection(key.begin(), key.end(), s.begin(), s.end(), inserter(intersection, intersection.begin())); //doing it to find cardinality of intersection
    set_union(key.begin(), key.end(), s.begin(), s.end(), inserter(unison, unison.begin())); //doing it to find cardinality of union
    double union_size = unison.size(); 
    double intersection_size = intersection.size();
    result = intersection_size / union_size; //calculation of jaccard endex
    return result;
}


//given an abstract file, returns the sentences containing at least one of keywords
string ParseText(string file, vector<string> &key)
{
    string sentences = ""; //the string containing sentences with at least one common word with the query. In-place implementation
    file = " " + file;
    set<string> ky(key.begin(), key.end()); //removing multiple occurences of words from the query
    regex rule_for_words("[a-zA-Z0-9]+"); //rule for tokenizing words from a sentence
    regex rule_for_sentences("[ \na-zA-Z0-9]*\\."); //rule for tokenizing sentences from a text
    //below is for finding a sentence with requested property and appending it to string "sentences"
    auto beginning = sregex_iterator(file.begin(), file.end(), rule_for_sentences);
    auto ending = sregex_iterator();
    for (sregex_iterator i = beginning; i != ending; i++)
    {
        string pl = (*i).str();
        auto beginning = sregex_iterator(pl.begin(), pl.end(), rule_for_words);
        auto ending = sregex_iterator();
        for (sregex_iterator j = beginning; j != ending; j++)
        {
            bool flag = true;
            for (set<string>::iterator k = ky.begin(); k != ky.end(); ++k)
            {
                if ((*j).str() == (*k))
                {
                    flag = false;
                    sentences += pl;
                    break;
                }
            }
            if (!flag)
                break;
        }
    }
    return sentences;
}

// returns a vector of atoms of a sentence. Atom is defined as word and dot. 
vector<string> tokenize(string st)
{
    vector<string> sA; //vector of words given a sentence string
    regex rule_for_words("[a-zA-Z0-9\\.]+"); //rule for words
    auto beginning = sregex_iterator(st.begin(), st.end(), rule_for_words);
    auto ending = sregex_iterator();
    for (sregex_iterator j = beginning; j != ending; j++) {sA.push_back((*j).str());}
    return sA;
}

// generic method that threads will run on multiple threads
void *runner(void *param)
{
    int id = thread_id++; //for naming the threads
    while (monitor->num_of_processed_abstracts < scanned_abstracts)
    {   
        ifstream in;
        string path;
        int index_for_records; //the index used to identify the abstract we are to use with this thread currently
        //below I atomised popping an abstract to process and updating the state of monitor
        pthread_mutex_lock(&lock1);
        if(monitor->num_of_processed_abstracts >= scanned_abstracts){ pthread_mutex_unlock(&lock1); in.close(); break;} 
        path = monitor->reading_queue.front();
        monitor->reading_queue.pop();
        index_for_records = monitor->num_of_processed_abstracts++;
        file_names[index_for_records] = path.substr(13, path.length());
        pthread_mutex_unlock(&lock1);
        // I release lock above
        //below three lines I atomise writing to a file. It is required because it is not atmoic in hardware level
        pthread_mutex_lock(&lock2);
        output << "Thread " << (char)(id + 65) << " is calculating " << file_names[index_for_records] << "\n";
        pthread_mutex_unlock(&lock2);
        //below till line 141 I read the whole abstract file to a string st
        in.open(path);
        string st = "";
        while (!in.eof())
        {
            string n;
            getline(in, n);
            st += n;
        }
        vector<string> word_arrays_for_abstract; //tokenized version of text from abstract
        double jaccard; //jaccard endex for the text
        word_arrays_for_abstract = tokenize(st); //I tokenize each word (and dots) of a string
        jaccard_endexes[index_for_records] = JaccardSimilarity(word_arrays_for_abstract, srch); //calculating the jaccard index
        matched_strings[index_for_records] = ParseText(st, srch); //extracting the string that has overlapping words with the key string
        in.close();
    }
    pthread_exit(0); //killing the thread when there is no abstract to be processed in the monitor queue
}

int main(int args, char *argv[])
{
    ifstream input;
    input.open(argv[1]);
    output.open(argv[2]);
    string query;
    input >> num_of_threads;
    monitor = new Monitor(num_of_threads);
    input >> scanned_abstracts;
    input >> summarized_abstracts;
    jaccard_endexes = new double[scanned_abstracts];
    matched_strings = new string[scanned_abstracts];
    file_names = new string[scanned_abstracts];
    getline(input, query);
    getline(input, query);
    for (int i = 0; i < scanned_abstracts; i++)
    {
        string txt; //for file names
        getline(input, txt);
        txt = "../abstracts/" + txt; //prepends an extension path. Required for the given directory structure.
        monitor->reading_queue.push(txt);
    }
    input.close();
 
    srch = tokenize(query);
    //below till line 198 I create mutexes one for output file manipulation to prevent intermixed writing and one for monitor state updates.
    pthread_t threads[num_of_threads];

    if (pthread_mutex_init(&lock1, NULL) != 0) {
        return 1;
    }else if (pthread_mutex_init(&lock2, NULL) != 0) {
        return 1;
    }

    //below for creates the threads
    for (int i = 0; i < num_of_threads; i++)
    {
        pthread_create(&monitor->threads[i], NULL, runner, NULL);
    }
    //below for waits for the threads to finish
    for (int i = 0; i < num_of_threads; i++)
    {
        pthread_join(monitor->threads[i], NULL);
    }

    //below I find the number of requested files and report them in requested format
    for (int p = 0; p < summarized_abstracts; p++)
    {
        output << "###" << endl;
        int max_index = max_element(jaccard_endexes, jaccard_endexes + scanned_abstracts) - jaccard_endexes;
        output << "Result " << p + 1 << ":\n";
        output << "File: " << file_names[max_index] << setprecision(3) << endl;
        output << "Score: " << jaccard_endexes[max_index] << setprecision(3) << endl;
        output << "Summary:" << matched_strings[max_index] << setprecision(3) << endl;
        jaccard_endexes[max_index] = -1;
    }
    output << "###";
    output.close();
    pthread_mutex_destroy(&lock1);
    pthread_mutex_destroy(&lock2);

    return 0;
}

/*
Conclusion: There still is idleness due to providing atomic operations. But apart form that I utilized a monitor structure.
I have a seperate class for monitor. My program works as I want it to be. I vividly observed producer-consumer problem.
*/
