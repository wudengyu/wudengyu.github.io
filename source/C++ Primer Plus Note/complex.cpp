#include <iostream>
using namespace std;
class complex{
    private:
    float x;
    float y;
    public:
    complex(float=0,float=0);
    void print();
    complex operator+(const complex&);
    friend complex operator+(int,complex);
};
complex::complex(float a,float b){
    x=a;
    y=b;
}
complex complex::operator+(const complex& a){
    complex temp;
    temp.x=x+a.x;
    temp.y=y+a.y;
    return temp;
}
void complex::print(){
    cout<<this->x<<'+'<<this->y<<'i';
}
complex operator+(int a,complex b){
    complex temp(a+b.x,b.y);
    return temp;
}

int main(){
    complex a(0,0),b(2.0,3.0);
    complex c=a+2;
    complex d=2+a;
    c.print();
    d.print();
}
