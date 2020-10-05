#include <stdio.h>
void swap1(int* a,int* b){
    int temp=*a;
    *a=*b;
    *b=temp;
}
void swap2(int& a,int& b){
    int temp=a;
    a=b;
    b=temp;
}
int main(){
    int x=1,y=2;
    printf("x=%d,y=%d\n",x,y);
    swap1(&x,&y);
    printf("x=%d,y=%d\n",x,y);
    swap2(x,y);
    printf("x=%d,y=%d\n",x,y);
}