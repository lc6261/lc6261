// ConsoleApplication1.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
#include<iostream>
using namespace std;
#define HANOI


#ifdef RECOURSION
int f(int n, int k);
int main(int argc, char const* argv[])
{
    cout << "ÇëÊäÈënÓëk" << endl;
    int n, k;
    cin >> n;
    cin >> k;
    cout << f(n, k) << endl;
    system("pause");
    return 0;
}

int f(int n, int k)
{
    if ((n == k) || (k == 0))
    {
        return 1;
    }
    else
    {
        return f(n - 1, k - 1) + f(n - 1, k);
    }

}
#endif

#ifdef HANOI
#include<iostream>
using namespace std;
void move(char A, char B);
void hanoi(int n, char A, char B, char C);
int main(int argc, char const* argv[])
{
    cout << "HANOI";
    int disks;
    cin >> disks;
    hanoi(disks, 'A', 'B', 'C');
    system("pause");
    return 0;
}


void move(char A, char B)
{
    cout << A << "->" << B << endl;
}

void hanoi(int n, char A, char B, char C)
{
    if (n == 1)
    {
        move(A, C);
    }
    else
    {
        hanoi(n - 1, A, C, B);
        move(A, C);
        hanoi(n - 1, B, A, C);
    }
}
#endif




// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门使用技巧: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件
