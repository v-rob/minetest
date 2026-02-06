// Luanti
// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (C) 2013 celeron55, Perttu Ahola <celeron55@gmail.com>

#pragma once

#include <cmath>
#include <functional>
#include <exception>
#include <sstream>
#include <vector>

#include "irrlichttypes_bloated.h"
#include "porting.h"
#include "filesys.h"
#include "mapnode.h"
#include "util/numeric.h"

// Don't derive from std::exception to avoid accidental catches.
class TestFailedException {
public:
	TestFailedException(std::string in_message, const char *in_file, int in_line)
		: message(std::move(in_message))
		, file(fs::GetFilenameFromPath(in_file))
		, line(in_line)
	{}

	const std::string message;
	const std::string file;
	const int line;
};

// Runs a unit test and reports the results.
#define TEST(fxn, ...) runTest(#fxn, [&] () { fxn(__VA_ARGS__); })

// Unconditionally fails the current unit test.
#define UASSERT_FAIL() throw TestFailedException("assert[fail]", __FILE__, __LINE__)

// Asserts the specified condition is true, or fails the current unit test.
#define UASSERT(actual) \
	do { \
		auto _a = (actual); \
		if (!_a) { \
			std::ostringstream _msg; \
			_msg << "assert[] " #actual \
				<< "\n    actual: " << _a; \
			throw TestFailedException(_msg.str(), __FILE__, __LINE__); \
		} \
	} while (false)

// Asserts the specified condition is false, or fails the current unit test.
#define UASSERT_NOT(actual) \
	do { \
		auto _a = (actual); \
		if (_a) { \
			std::ostringstream _msg; \
			_msg << "assert[!] " #actual \
				<< "\n    actual: " << _a; \
			throw TestFailedException(_msg.str(), __FILE__, __LINE__); \
		} \
	} while (false)

// Asserts the specified condition is true, or fails the current unit test
// and prints the given format specifier.
#define UASSERT_MSG(x, ...) \
	do { \
		if (!(x)) { \
			char utest_buf[1024]; \
			porting::mt_snprintf(utest_buf, sizeof(utest_buf), __VA_ARGS__); \
			throw TestFailedException(utest_buf, __FILE__, __LINE__); \
		} \
	} while (false)

// Asserts the comparison specified by the operator CMP is true, or fails the
// current unit test.
#define UASSERT_CMP(CMP, actual, expect) \
	do { \
		auto _a = (actual); \
		auto _e = (expect); \
		if (!(_a CMP _e)) { \
			std::ostringstream _msg; \
			_msg << "assert[" #CMP "] " #actual ", " #expect \
				<< "\n    actual: " << _a \
				<< "\n    expect: " << _e; \
			throw TestFailedException(_msg.str(), __FILE__, __LINE__); \
		} \
	} while (false)

// Asserts that two values are equal/not equal, or fails the current unit test.
#define UASSERT_EQ(actual, expect) UASSERT_CMP(==, actual, expect)
#define UASSERT_NE(actual, expect) UASSERT_CMP(!=, actual, expect)

#define UASSERT_FCMP_IMPL(CMP, actual, expect, eps) \
	do { \
		auto _a = (actual); \
		auto _e = (expect); \
		if (floatEq(_a, _e, eps) CMP false) { \
			std::ostringstream _msg; \
			_msg << "assert[~" #CMP "] " #actual ", " #expect \
				<< "\n    actual: " << _a \
				<< "\n    expect: " << _e; \
			throw TestFailedException(_msg.str(), __FILE__, __LINE__); \
		} \
	} while (false)

// Asserts the two floats are equal/not equal within an automatically computed
// float epsilon, or fails the current unit test.
#define UASSERT_FEQ(actual, expect) UASSERT_FCMP_IMPL(==, actual, expect, NAN)
#define UASSERT_FNE(actual, expect) UASSERT_FCMP_IMPL(!=, actual, expect, NAN)

// Asserts the two floats are equal/not equal within the given float epsilon,
// or fails the current unit test.
#define UASSERT_FEQ_EPS(actual, expect, eps) UASSERT_FCMP_IMPL(==, actual, expect, eps)
#define UASSERT_FNE_EPS(actual, expect, eps) UASSERT_FCMP_IMPL(!=, actual, expect, eps)

// Asserts the given statement throws an exception of type E, or fails the
// current unit test.
#define UASSERT_THROW(E, code) \
	do { \
		bool _thrown = false; \
		try { \
			code; \
		} catch (E &_e) { \
			_thrown = true; \
		} \
		if (!_thrown) \
			throw TestFailedException("assert[throw] " #E, __FILE__, __LINE__); \
	} while (false)

class IGameDef;

class TestBase {
public:
	bool testModule(IGameDef *gamedef);
	std::string getTestTempDirectory();
	std::string getTestTempFile();

	virtual void runTests(IGameDef *gamedef) = 0;
	virtual const char *getName() = 0;

	u32 num_tests_failed;
	u32 num_tests_run;

	void runTest(const char *name, std::function<void()> &&test);

private:
	std::string m_test_dir;
};

class TestManager {
public:
	static std::vector<TestBase *> &getTestModules()
	{
		static std::vector<TestBase *> m_modules_to_test;
		return m_modules_to_test;
	}

	static void registerTestModule(TestBase *module)
	{
		getTestModules().push_back(module);
	}
};

// A few item and node definitions for those tests that need them
extern content_t t_CONTENT_STONE;
extern content_t t_CONTENT_GRASS;
extern content_t t_CONTENT_TORCH;
extern content_t t_CONTENT_WATER;
extern content_t t_CONTENT_LAVA;
extern content_t t_CONTENT_BRICK;

bool run_tests();
bool run_tests(const std::string &module_name);
