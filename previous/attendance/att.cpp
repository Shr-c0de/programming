#include <iostream>
#include <climits>

using namespace std;
#define number 1024
u_int att[32];
struct electronics_hobby_club
{
    u_int a:10;
    u_int b:10;
    u_int c:10;
    u_int d:5;

};
void mark_att()
{
    u_int stud_id;
    cout << "student ID: ";
    cin >> stud_id;
    att[stud_id>>5] = att[stud_id>>5] | 1<<(stud_id&((1<<5)-1));
    cout << "student " << stud_id << " marked\n";
}

void get_att()
{
    u_int stud_id;
    cin >> stud_id;
    cout << ((att[stud_id>>5] & 1<<(stud_id&((1<<5)-1)))?"Present":"Absent") << endl;
}

int main()
{
    electronics_hobby_club g1;
    g1.a = 100;
    g1.b = 1024;
    g1.c = 500;
    
    cout << sizeof(g1) << " " << g1.a << " " << g1.b << " " << g1.c << endl;
    while (1)
    {
        cout << "1. mark\n2. get\n3. exit\n";
        int stu;
        cin >> stu;
        switch (stu)
        {
        case 1:
            mark_att();
            break;
        case 2:
            get_att();
            break;
        case 3:
            return 0;
        default:
            cout << "break\n";
        }
    }
}