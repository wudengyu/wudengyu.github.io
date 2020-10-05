#include <stdio.h>
int main(){
    int i=0,j=1;
    int& x=i;
    x=j;
    printf("i=%d",i);
}