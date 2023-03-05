#include<iostream>
#include<cmath>
using namespace std;
int main()
{
	float num[8][2] = { {2,7},{2,5},{8,4},{7,8},{7,5},{6,4},{1,4},{3,9} };
	float n[8][2] = { 0 };
	float m[2][2] = { {2,7},{2,5} };
	
	//cout << "第" << i + 1 << "次迭代：" << endl;
	for (int j = 0; j < 3; j++)
	{
		float p = 0, q = 0, p1 = 0, p11 = 0, q1 = 0, q11 = 0, p2 = 0, q2 = 0;
		for (int i = 0; i < 8; i++)
		{
			p = abs(m[0][0] - num[i][0]) + abs(m[0][1] - num[i][1]);
			q = abs(m[1][0] - num[i][0]) + abs(m[1][1] - num[i][1]);
			cout << "|" << m[0][0] << "-" << num[i][0] << "|+|" << m[0][1] << "-" << num[i][1] << "|=" << p << "  " << "|" << m[1][0] << "-" << num[i][0] << "|+|" << m[1][1] << "-" << num[i][1] << "|=" << q << endl;
			if (p <= q)
			{
				p1 = p1 + num[i][0];
				p11 = p11 + num[i][1];
				p2++;
			}
			else
			{
				q1 = q1 + num[i][0];
				q11 = q11 + num[i][0];
				q2++;
			}
		}
		cout << "第" << j + 1 << "次迭代中心点：(" << p1 / p2 << ", " << p11 / p2 << ")    (" << q1 / q2 << ", " << q11 / q2 << ")" << endl;
		if (p1 / p2 == m[0][0] && p11 / p2 == m[0][1])
		{
			if (q1 / q2 == m[1][0] && q11 / q2 == m[1][1])
			{
				break;
			}
		}
		m[0][0] = p1 / p2;
		m[0][1] = p11 / p2;
		m[1][0] = q1 / q2;
		m[1][1] = q11 / q2;
	}

	return 0;
}