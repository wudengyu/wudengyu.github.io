#include <iostream>
using namespace std;

class point{
    private:
    int x;
    int y;
    public:
    point();
    point(int x,int y);
    point(const point&);
    void print(){
        cout<<'('<<x<<','<<y<<')';
    };
};
point::point(){}
point::point(int x,int y){
    this->x=x;
    this->y=y;
}
point::point(const point & source){
    this->x=source.x;
    this->y=source.y;
    cout<<"copy construction is run!"<<endl;
}
void test(point x){}
int main(){
    point p=point(1,2),q;
    p.print();
    q=p;
    //point r=p;
}