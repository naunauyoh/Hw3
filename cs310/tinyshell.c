#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include  <unistd.h>

int main(){
const char * ut; 
char input[100]; 
while(1){
	scanf("%s", input); 

	if ( strlen(input) > 0){
	if ( fork()==0 ) execl(input,ut);
	
	}
	}


}
