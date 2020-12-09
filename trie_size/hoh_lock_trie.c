#include "generate.h"
#include "../trie.c"
#include "../trie.h"

#define TRIE_SIZE 100
#define CONCURRENT_THREADS 100

trie_t trie;

FILE* ins_fp, *find_fp, *rem_fp, *pref_fp;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

void* insert_task(){
    char key[TRIE_SIZE + 5];
    int val;
    while (1)
    {
        pthread_mutex_lock(&lock);
        if(fscanf(ins_fp,"%s",key) == EOF){
            pthread_mutex_unlock(&lock);
            break;
        }
        if (key[0]=='-'){
            pthread_mutex_unlock(&lock);
            break;
        }
        fscanf(ins_fp,"%d",&val);
        pthread_mutex_unlock(&lock);
        insert(trie,key,val);
    }
    return NULL;
}

void* find_task(){
    char key[TRIE_SIZE + 5];
    while (1)
    {
        int f_ret = -101;
        if(fscanf(find_fp,"%s",key) == EOF){
            break;
        }
        if (key[0]=='-'){
            break;
        }
        find(trie,key,&f_ret);
    }
    return NULL;
}

void *rem_task(){
    char key[TRIE_SIZE + 5];
    while (1)
    {
        if(fscanf(rem_fp,"%s",key) == EOF){
            break;
        }
        if (key[0]=='-') break;
        delete_kv(trie,key);
    }
    return NULL;
}

void* pref_task(){
    char key[TRIE_SIZE + 5];
    while (1)
    {
        char** list;
        if(fscanf(pref_fp,"%s",key) == EOF){
            break;
        }
        if (key[0]=='-') break;
        list = keys_with_prefix(trie, key);
        free(list);
    }
    return NULL;
}


int main(int argc, char* argv[]){

    clock_t start_t, end_t;
    double total_t;
    FILE *fp = fopen("./CSV/hoh_lock.csv", "w");

    if(fp == NULL){
        printf("Cannot open file\n");
        return 0;
    }

    printf(YELLOW "Generating Data for Plot...\n" RESET);ff;

    for(int count=1; count <= TRIE_SIZE; count++){ 
            start_t = clock();
            trie = init_trie();

            /*
                Insert Function
            */
            char path[1024];
            sprintf(path, "./data/initial/%d.txt",count);
            ins_fp = fopen(path,"r");
            pthread_mutex_init(&lock,NULL);
            pthread_t ins_thread[CONCURRENT_THREADS];
            for(int i=0;i<CONCURRENT_THREADS; i++){
                pthread_create(&ins_thread[i],NULL, insert_task, NULL);
            }
            for(int i=0;i<CONCURRENT_THREADS; i++){
                pthread_join(ins_thread[i],NULL);
            }
            fclose(ins_fp);

            /*
                Find Function
            */
            char path_find[1024];
            sprintf(path_find, "./data/find/%d.txt",count);
            find_fp = fopen(path_find,"r");
            pthread_t find_thread[CONCURRENT_THREADS];
            for(int i=0;i<CONCURRENT_THREADS; i++){
                pthread_create(&find_thread[i],NULL, find_task, NULL);
            }
            for(int i=0;i<CONCURRENT_THREADS; i++){
                pthread_join(find_thread[i],NULL);
            }
            fclose(find_fp);

            /*
                Delete Function
            */
            // char path[1024];
            // sprintf(path, "./data/rem/%d.txt",count);
            // rem_fp = fopen(path,"r");
            // pthread_t rem_thread[CONCURRENT_THREADS];
            // for(int i=0;i<CONCURRENT_THREADS; i++){
            //     pthread_create(&rem_thread[i],NULL, rem_task, NULL);
            // }
            // for(int i=0;i<CONCURRENT_THREADS; i++){
            //     pthread_join(rem_thread[i],NULL);
            // }
            // fclose(rem_fp);

            /*
                Prefix Function
            */
            // char path[1024];
            // sprintf(path, "./data/pref/%d.txt",count);
            // pref_fp = fopen(path,"r");
            // pthread_t pref_thread[CONCURRENT_THREADS];
            // for(int i=0;i<CONCURRENT_THREADS; i++){
            //     pthread_create(&pref_thread[i],NULL, pref_task, NULL);
            // }
            // for(int i=0;i<CONCURRENT_THREADS; i++){
            //     pthread_join(pref_thread[i],NULL);
            // }
            // fclose(pref_fp);


            end_t = clock();
            total_t = (double)(end_t - start_t) / CLOCKS_PER_SEC;
            delete_trie(trie);

            fprintf(fp,"%d,%f\n",count, total_t);
        }
    printf(GREEN "Generated\n" RESET);ff;

    fclose(fp);
return 0;
}