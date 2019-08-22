// Created on 2019/8/12.

#include <iostream>
#include <string>
#include <functional>
using namespace std;

void do_work(void(*start_routine)(void*), void* arg);
void example1();
void example2();
void example3();
void example4();
void example4();
void example5();
void example6();
void example7();


int main()
{
	void(*examples[])(void) = {
		example1, example2, example3, example4,  example5, example6, example7
	};
	for (size_t i = 0; i < sizeof(examples) / sizeof(*examples); ++i)
	{
		cout << "-- example " << i + 1 << " --" << endl;
		examples[i]();
		cout << endl;
	}
	return 0;
}

void do_work(void(*start_routine)(void*), void* arg)
{
	cout << "begin work" << endl;
	start_routine(arg);
	cout << "end   work" << endl;
}

// 连接字符串s1和s2，结果存入s3
struct string_join_args
{
	const char* s1;
	const char* s2;
	string& s3;      
};

void string_join_routine(void* arg)
{
	string_join_args* args = (string_join_args*)arg;
	args->s3 = args->s1;
	args->s3 += args->s2;
}

void example1()
{
	const char* s1 = "example1_s1_";
	const char* s2 = "s2";
	string s3;
	string_join_args args = { s1, s2, s3 };
	do_work(string_join_routine, &args);
	cout << "s3 : " << s3 << endl;
}

void example2()
{
	auto add = [](int x, int y) {
		return x + y;
	};
	void* pv = &add;                        // Lambda地址转换成void*
	auto addptr = (decltype(&add))pv;       // void*转换成Lambda指针
	int sum = (*addptr)(2, 3);              // 调用Lambda
	cout << "sum : " << sum << endl;
}

template <typename Fx>
void start_routine_t(void* arg)
{
	Fx* func = static_cast<Fx*>(arg);
	(*func)();
}

void example3()
{
	const char* s1 = "example3_s1_";
	const char* s2 = "s2";
	string s3;
	auto lambda = [=, &s3]() {
		s3 = s1;
		s3 += s2;
	};
	do_work(start_routine_t<decltype(lambda)>, &lambda);
	cout << "s3: " << s3 << endl;
}

template <typename Fx>
void do_work_t(Fx&& func)
{
	do_work(start_routine_t<remove_reference_t<Fx>>, &func);
}

void example4()
{
	const char*	s1 = "example4_s1_";
	const char* s2 = "s2";
	string s3;
	do_work_t([=, &s3]() {
		s3 = s1;
		s3 += s2;
	});
	cout << "s3 : " << s3 << endl;
}

void example5()
{
	int x = 3, y = 2;

	int z1;
	do_work_t([=, &z1]() {
		z1 = x + y;
	});
	cout << "z1 : " << z1 << endl;

	int z2;
	function<void(void)> sub = [=, &z2]() {
		z2 = x - y;
	};
	do_work_t(sub);
	cout << "z2 : " << z2 << endl;

	int z3;
	auto mul = [&z3](int a, int b) {
		z3 = a * b;
	};
	auto binder = bind(mul, x, y);
	do_work_t(binder);
	cout << "z3 : " << z3 << endl;
}


// 根据比较函数compare找出字符串数组strings中最大的一个
const char* max(const char** strings, size_t size,  int(*compare)(const char*, const char*))
{
	if (strings == nullptr || size == 0)
		return nullptr;
	size_t idx = 0;
	for(size_t i = 1; i < size; ++i)
	{
		if (compare(strings[i], strings[idx]) > 0)
			idx = i;
	}
	return strings[idx];
}

void example6()
{
	int (*cmp)(const char*, const char*)  = [](const char* s1, const char* s2) 
	{
		return strcmp(s1, s2);
	};
	const size_t N = 3;
	const char*	strings[] = { "2", "12", "112" };
	const char*	max1 = max(strings, N, cmp);
	cout << "max1 : " << max1 << endl;

	const char* max2 = max(strings, N, [](const char* s1, const char* s2)
	{
		int i1 = atoi(s1);
		int i2 = atoi(s2);
		return i1 - i2;
	});
	cout << "max2 : " << max2 << endl;
}

// 在堆上创建lambda的副本
template <typename LambdaType>
LambdaType* copy_lambda(LambdaType&& lambda)
{
	return new LambdaType(forward<LambdaType>(lambda));
}

// 返回lambda的地址
template <typename LambdaType>
LambdaType* get_lambda_pointer(LambdaType&& lambda)
{
	return &lambda;
}

class mystring
{
	char s[16];
public:
	mystring(const char *arg)
	{
		memcpy(s, arg, sizeof(s) - 1);
		s[sizeof(s) - 1] = '\0';
	}
	mystring(const mystring& arg)
	{
		memcpy(s, arg.s, sizeof(s));
	}
	~mystring()
	{
		s[0] = '\0';
	}
	operator const char*() const
	{
		return s;
	}
};

void example7()
{
	mystring s1("example7_s1");
	auto lambda1 = copy_lambda([=]() {
		cout << "s1 : " << (const char *)s1 << endl;
	});
	(*lambda1)();		//	输出"s1 : example7_s1"
	delete lambda1;

	mystring s2("example7_s2" );
	auto lambda2 = get_lambda_pointer([=]() {
		cout << "s2 : " << (const char *)s2 << endl;
	});
	char s3[] = "example7_s3";
	// 危险！lambda2指向的对象已经销毁了！
	(*lambda2)();		// 不会输出"s2 : example7_s2"
}
