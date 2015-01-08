
#include <iostream>
#include <vector>
#include <string>
#include <utility>
#include <sstream>
#include <stack>
#include "calculate.h"
using namespace std;

vector<pair<string, int> > word;
vector<pair<string, int> > output;

int idx = 0;
int sym;
int err = 0; // 错误

void Tsign();
void Fsign();
/*--------------------------------词法分析----------------------------*/
int word_analysis(vector<pair<string, int> >& word, const string expr)
{
    for(int i=0; i<expr.length(); ++i)
    {
        // 如果是 + - * / ( )
        if(expr[i] == '(' || expr[i] == ')' || expr[i] == '+'
            || expr[i] == '-' || expr[i] == '*' || expr[i] == '/')
        {
            string tmp;
            tmp.push_back(expr[i]);
            switch (expr[i])
            {
            case '+':
                word.push_back(make_pair(tmp, 1));
                break;
            case '-':
                word.push_back(make_pair(tmp, 2));
                break;
            case '*':
                word.push_back(make_pair(tmp, 3));
                break;
            case '/':
                word.push_back(make_pair(tmp, 4));
                break;
            case '(':
                word.push_back(make_pair(tmp, 6));
                break;
            case ')':
                word.push_back(make_pair(tmp, 7));
                break;
            }
        }
        // 如果是数字开头
        else if(expr[i]>='0' && expr[i]<='9')
        {
            string tmp;
            while(expr[i]>='0' && expr[i]<='9')
            {
                tmp.push_back(expr[i]);
                ++i;
            }
            if(expr[i] == '.')
            {
                ++i;
                if(expr[i]>='0' && expr[i]<='9')
                {
                    tmp.push_back('.');
                    while(expr[i]>='0' && expr[i]<='9')
                    {
                        tmp.push_back(expr[i]);
                        ++i;
                    }
                }
                else
                {
                    return -1;  // .后面不是数字，词法错误
                }
            }
            word.push_back(make_pair(tmp, 5));
            --i;
        }
        // 如果以.开头
        else
        {
            return -1;  // 以.开头，词法错误
        }
    }
    return 0;
}
/*--------------------------------语法分析----------------------------*/
// 读下一单词的种别编码
void Next()
{
    if(idx < word.size())
        sym = word[idx++].second;
    else
        sym = 0;

    //cout << sym;
}

// E → T { +T | -T }
void Esign()
{
    Tsign();
    while(sym == 1 || sym == 2)
    {
        Next();
        Tsign();
    }
}

// F → (E) | d
void Fsign()
{
    if (sym == 5)
    {
        Next();
    }
    else if(sym == 6)
    {
        Next();
        Esign();
        if (sym == 7)
        {
            Next();
        }
        else
        {
            err = -1;
        }
    }
    else
    {
        err = -1;
    }
}

// T → F { *F | /F }
void Tsign()
{
    Fsign();
    while(sym == 3 || sym == 4)
    {
        Next();
        Fsign();
    }
}



/*--------------------------------计算----------------------------*/
int prior(int sym)
{
    switch (sym)
    {
        case 1:
        case 2:
            return 1;
        case 3:
        case 4:
            return 2;
        default:
            return 0;
    }
}

// 通过 种别编码 判定是否是运算符
bool isOperator(int sym)
{
    switch (sym)
    {
        case 1:
        case 2:
        case 3:
        case 4:
            return true;
        default:
            return false;
    }
}

// 中缀转后缀
void getPostfix(const vector<pair<string, int> >& expr)
{
    stack<pair<string, int> > s;  // 操作符栈
    for(int i=0; i<expr.size(); ++i)
    {
        pair<string, int> p = expr[i];
        if(isOperator(p.second))
        {
            while(!s.empty() && isOperator(s.top().second) && prior(s.top().second)>=prior(p.second))
            {
                output.push_back(s.top());
                s.pop();
            }
            s.push(p);
        }
        else if(p.second == 6)
        {
            s.push(p);
        }
        else if(p.second == 7)
        {
            while(s.top().second != 6)
            {
                output.push_back(s.top());
                s.pop();
            }
            s.pop();
        }
        else
        {
            output.push_back(p);
        }
    }
    while (!s.empty())
    {
        output.push_back(s.top());
        s.pop();
    }
    return ;
}

// 从栈中连续弹出两个操作数
void popTwoNumbers(stack<double>& s, double& first, double& second)
{
    first = s.top();
    s.pop();
    second = s.top();
    s.pop();
}

// 把string转换为double
double stringToDouble(const string& str)
{
    double d;
    stringstream ss;
    ss << str;
    ss >> d;
    return d;
}

// 计算后缀表达式的值
double expCalculate(const vector<pair<string, int> >& postfix)
{
    double first,second;
    stack<double> s;
    for(int i=0; i<postfix.size(); ++i)
    {
        pair<string,int> p = postfix[i];
        switch (p.second)
        {
        case 1:
            popTwoNumbers(s, first, second);
            s.push(second+first);
            break;
        case 2:
            popTwoNumbers(s, first, second);
            s.push(second-first);
            break;
        case 3:
            popTwoNumbers(s, first, second);
            s.push(second*first);
            break;
        case 4:
            popTwoNumbers(s, first, second);
            s.push(second/first);
            break;
        default:
            s.push(stringToDouble(p.first));
            break;
        }
    }
    double result = s.top();
    s.pop();
    return result;
}


//int main(const char* strCmd)
extern string mainCal(string expr)
{
    //string calResult = "False";
    //string expr = strCmd;
    //vector<pair<string, int> > word;
    string calResult = "false";
    idx = 0;
    err = 0;
    word.clear();
    output.clear();
    int err_num = word_analysis(word, expr);
    if (-1 == err_num)
    {
        cout << "Word Error!" << endl;
    }
    else
    {
        // 测试输出
        vector<pair<string, int> >::iterator beg = word.begin();
        for(;beg!=word.end(); ++beg)
            cout << "   (" << beg->first << ", " << beg->second << ")" << endl;
            //cout << word.size();

        // 词法正确，进行语法分析
        Next();
        Esign();
        if (sym == 0 && err == 0)  // 注意要判断两个条件
        {
            //double result = expCalculate(getPostfix(word));
            getPostfix(word);
            double result = expCalculate(output);
            //string sResult;
            stringstream ss;
            ss << result;
            ss >> calResult;
           // vector<pair<string, int> >::iterator beg1 = output.begin();
            //for(;beg1!=word.end(); ++beg1)
              //  cout << "   (" << beg1->first << ", " << beg1->second << ")" << endl;

             //cout << expr + " = " << result << endl;
        }
        else
        {
            cout << "Wrong Expression." << endl;
        }

    }
    return calResult;
}
