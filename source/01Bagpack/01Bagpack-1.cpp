#include <iostream>
using namespace std;
int main(){
    int N,V;
    int c[301],w[301],acc[301];
    int dp[20001];
    while(!(cin>>V>>N).eof()){
        int sum=0;
        for(int i=0;i<=V;i++)
            dp[i]=0;
        for(int i=1;i<=N;i++){
            cin>>w[i]>>c[i];
            sum+=c[i];
        }
        for(int i=1;i<=N;i++){
            int tp=max(V-sum,c[i]);
            for(int j=V;j>=tp;j--)
                dp[j]=max(dp[j],dp[j-c[i]]+w[i]);
            sum-=c[i];
        }
        cout<<dp[V]<<endl;
    }
}