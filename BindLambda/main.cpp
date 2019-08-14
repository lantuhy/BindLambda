// Created on 2019/8/12.

#include <iostream>
#include <functional>
using namespace std;

void do_work(void(*start_routine)(void*), void* arg);
void example1();
void example2();
void example3();
void example4();
void example5();
void example6();

int main()
{
	void(*examples[])(void) = {
		example1, example2, example3, example4, example5, example6,
	};
	for (size_t i = 0; i < sizeof(examples) / sizeof(*examples); ++i)
	{
		cout << "example " << i + 1 << endl;
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

struct sum_args
{
	const int* arr;         // 数组
	const size_t size;      // 数组元素个数
	int* sum;               // 存放数组求和的结果
};

// 计算数组所有元素之和
void sum_routine(void* arg)
{
	sum_args* args = (sum_args*)arg;
	*(args->sum) = 0;
	for (size_t i = 0; i < args->size; ++i)
		* (args->sum) += args->arr[i];
}

void example1()
{
	const int arr[] = { 10, 20, 30 };
	int sum;
	sum_args args = { arr, sizeof(arr) / sizeof(*arr), &sum };
	do_work(sum_routine, &args);
	cout << "sum : " << sum << endl;
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
	const int arr[] = { 10, 20, 30 };
	int sum;
	auto lambda = [&arr, &sum]() {
		sum = 0;
		const size_t n = sizeof(arr) / sizeof(*arr);
		for (size_t i = 0; i < n; ++i)
			sum += arr[i];
	};
	do_work(start_routine_t<decltype(lambda)>, &lambda);
	cout << "sum : " << sum << endl;
}

template <typename Fx>
void do_work_t(Fx&& func)
{
	do_work(start_routine_t<remove_reference_t<Fx>>, &func);
}

void example4()
{
	const int arr[] = { 10, 20, 30 };
	int sum;
	do_work_t([&arr, &sum]() {
		sum = 0;
		const size_t n = sizeof(arr) / sizeof(*arr);
		for (size_t i = 0; i < n; ++i)
			sum += arr[i];
	});
	cout << "sum : " << sum << endl;
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

void* max(void* val1, void* val2, int(*compare)(void*, void*))
{
	return compare(val1, val2) >= 0 ? val1 : val2;
}

void example6()
{
	int x = 3, y = 2;
	int *z = (int *)max(&x, &y, [](void* val1, void* val2) {
		return *(int*)val1 - *(int*)val2;
	});
	cout << "*z: " << *z << endl;
}